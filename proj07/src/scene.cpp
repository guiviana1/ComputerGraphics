#include "scene.h"
#include <cmath>
#include <algorithm>

bool Scene::intersect(Ray& r, Surfel* sf) const {
    bool hit_anything = false;

    // Iteramos sobre todos os objetos.
    // A cada hit, r.t_max e atualizado pelo proprio Primitive::intersect(),
    // garantindo que ao fim da iteracao sf contem o hit MAIS PROXIMO.
    for (const auto& prim : primitives) {
        if (prim->intersect(r, sf))
            hit_anything = true;
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

bool VisibilityTester::unoccluded(const Scene& scene) const {
    // Para evitar "auto-sombra" (shadow acne) por erro de arredondamento,
    // deslocamos a origem do raio de sombra um pouco ao longo da normal,
    // tirando-a de cima da superficie antes de mira-la para a luz.
    const float eps = 1e-2f;
    Point3 origin = p0 + n * eps;

    // Raio de sombra: de origin em direcao a luz, valido em [0, dist-eps].
    // O limite superior garante que objetos ALEM da luz nao contem como bloqueio.
    Ray shadow(origin, wi, 0.0f, dist - eps);

    // Se algo intercepta nesse intervalo, o ponto esta na sombra desta luz.
    return !scene.intersect_p(shadow);
}