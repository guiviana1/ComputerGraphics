#pragma once

#include "backgroundd.h"
#include "primitive.h"
#include "surfel.h"
#include "ray.h"
#include "light.h"
#include <memory>
#include <vector>

class Scene {
public:
    std::shared_ptr<Background>  background;
    std::shared_ptr<Primitive>   aggregate;    // BVHAccel ou PrimList

    std::shared_ptr<AmbientLight>            ambient_light;
    std::vector<std::shared_ptr<Light>>      lights;

    Scene() = default;

    bool intersect  (Ray& r,        Surfel* sf) const;
    bool intersect_p(const Ray& r)              const;
};


// ─────────────────────────────────────────────────────────────────────────────
// VisibilityTester — encapsula a criacao e o teste do raio de sombra (proj06).
// Guarda o ponto de origem (na superficie), a direcao para a luz e a distancia
// ate ela. unoccluded() dispara o raio de sombra e responde se o caminho ate a
// luz esta LIVRE (true) ou bloqueado por algum objeto (false).
// ─────────────────────────────────────────────────────────────────────────────
class VisibilityTester {
public:
    Point3  p0;     //!< ponto de contato na superficie (origem do raio de sombra)
    Vector3 n;      //!< normal da superficie em p0 (para deslocar a origem)
    Vector3 wi;     //!< direcao normalizada para a luz
    float   dist;   //!< distancia ate a luz (world_radius para directional)

    VisibilityTester() = default;
    VisibilityTester(const Point3& p0, const Vector3& n,
                     const Vector3& wi, float dist)
        : p0(p0), n(n), wi(wi), dist(dist) {}

    bool unoccluded(const Scene& scene) const;
};