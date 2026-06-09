#include "bvh.h"
#include "primlist.h"
#include <algorithm>
#include <cassert>

BVHAccel::BVHAccel(std::vector<std::shared_ptr<Primitive>> objects,
                   const std::string& split_method,
                   int max_prims_per_node,
                   int depth)
    : Primitive(nullptr)
{
    int n = (int)objects.size();
    assert(n > 0);

    if (n == 1) {
        // Folha com um unico objeto: ambos os filhos apontam para ele.
        left = right = objects[0];

    } else if (n == 2) {
        left  = objects[0];
        right = objects[1];

    } else if (n <= max_prims_per_node) {
        // Folha com varios objetos: usa PrimList (loop simples).
        int mid = n / 2;
        left  = std::make_shared<BVHAccel>(
            std::vector<std::shared_ptr<Primitive>>(objects.begin(), objects.begin() + mid),
            split_method, max_prims_per_node, depth + 1);
        right = std::make_shared<BVHAccel>(
            std::vector<std::shared_ptr<Primitive>>(objects.begin() + mid, objects.end()),
            split_method, max_prims_per_node, depth + 1);

    } else if (split_method == "middle") {
        // Calcula o eixo de maior extensao dos centroides das AABBs.
        Bounds3f centroid_bounds;
        for (const auto& p : objects)
            centroid_bounds = centroid_bounds.expand(p->world_bound().centroid());

        int axis = centroid_bounds.max_extent();
        float mid_val = centroid_bounds.centroid()[axis];

        // Particiona em torno do ponto medio.
        auto pivot = std::partition(objects.begin(), objects.end(),
            [&](const std::shared_ptr<Primitive>& p) {
                return p->world_bound().centroid()[axis] < mid_val;
            });

        int mid = (int)(pivot - objects.begin());

        // Caso degenerado: todos os centroides coincidem → divide ao meio.
        if (mid == 0 || mid == n) {
            std::nth_element(objects.begin(), objects.begin() + n/2, objects.end(),
                [axis](const std::shared_ptr<Primitive>& a,
                       const std::shared_ptr<Primitive>& b) {
                    return a->world_bound().centroid()[axis] <
                           b->world_bound().centroid()[axis];
                });
            mid = n / 2;
        }

        left  = std::make_shared<BVHAccel>(
            std::vector<std::shared_ptr<Primitive>>(objects.begin(), objects.begin() + mid),
            split_method, max_prims_per_node, depth + 1);
        right = std::make_shared<BVHAccel>(
            std::vector<std::shared_ptr<Primitive>>(objects.begin() + mid, objects.end()),
            split_method, max_prims_per_node, depth + 1);

    } else {
        // equal_counts (default Shirley): ordena pelo eixo depth%3 e divide ao meio.
        int axis = depth % 3;
        std::sort(objects.begin(), objects.end(),
            [axis](const std::shared_ptr<Primitive>& a,
                   const std::shared_ptr<Primitive>& b) {
                return a->world_bound().centroid()[axis] <
                       b->world_bound().centroid()[axis];
            });
        int mid = n / 2;
        left  = std::make_shared<BVHAccel>(
            std::vector<std::shared_ptr<Primitive>>(objects.begin(), objects.begin() + mid),
            split_method, max_prims_per_node, depth + 1);
        right = std::make_shared<BVHAccel>(
            std::vector<std::shared_ptr<Primitive>>(objects.begin() + mid, objects.end()),
            split_method, max_prims_per_node, depth + 1);
    }

    // A caixa deste no e a uniao das caixas dos filhos.
    box = left->world_bound().expand(right->world_bound());
}

bool BVHAccel::intersect(Ray& r, Surfel* sf) const {
    if (!box.intersect_p(r)) return false;

    // Testa ambos os filhos. Como r.t_max e atualizado a cada hit,
    // o segundo filho so aceita objetos mais proximos que o primeiro.
    bool h1 = left ->intersect(r, sf);
    bool h2 = right->intersect(r, sf);
    return h1 || h2;
}

bool BVHAccel::intersect_p(const Ray& r) const {
    if (!box.intersect_p(r)) return false;
    // Para raios de sombra basta encontrar qualquer obstrucao.
    return left->intersect_p(r) || right->intersect_p(r);
}
