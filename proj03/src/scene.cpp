#include "scene.h"

bool Scene::intersect(Ray& r, Surfel* sf) const {
    bool hit_anything = false;

    // Iteramos sobre todos os objetos.
    // A cada hit, r.t_max e atualizado pelo proprio Primitive::intersect(),
    // garantindo que ao fim da iteracao sf contem o hit MAIS PROXIMO.
    for (const auto& prim : primitives) {
        if (prim->intersect(r, sf)) {
            hit_anything = true;
            // Nao precisamos fazer nada aqui: intersect() ja atualizou t_max.
        }
    }

    return hit_anything;
}

bool Scene::intersect_p(const Ray& r) const {
    for (const auto& prim : primitives) {
        if (prim->intersect_p(r))
            return true; // Para no primeiro hit — nao precisamos do mais proximo
    }
    return false;
}