#include "scene.h"
#include <cmath>
#include <algorithm>

bool Scene::intersect(Ray& r, Surfel* sf) const {
    bool hit_anything = false;

    // a cada hit r.t_max e atualizado, deixando em sf o hit mais proximo
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
    // desloca a origem ao longo da normal para evitar auto-sombra (shadow acne)
    const float eps = 1e-2f;
    Point3 origin = p0 + n * eps;

    // raio de sombra ate a luz, valido em [0, dist-eps]
    Ray shadow(origin, wi, 0.0f, dist - eps);

    // se algo intercepta nesse intervalo, o ponto esta na sombra
    return !scene.intersect_p(shadow);
}