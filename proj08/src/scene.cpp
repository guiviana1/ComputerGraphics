#include "scene.h"
#include <cmath>
#include <algorithm>

bool Scene::intersect(Ray& r, Surfel* sf) const {
    if (!aggregate) return false;
    return aggregate->intersect(r, sf);
}

bool Scene::intersect_p(const Ray& r) const {
    if (!aggregate) return false;
    return aggregate->intersect_p(r);
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