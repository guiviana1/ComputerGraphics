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
    std::shared_ptr<Background>              background;
    std::vector<std::shared_ptr<Primitive>>  primitives;

    std::shared_ptr<AmbientLight>            ambient_light;
    std::vector<std::shared_ptr<Light>>      lights; // apenas point e directional

    Scene() = default;

    Scene(std::shared_ptr<Background>             bg,
          std::vector<std::shared_ptr<Primitive>> prims)
        : background(std::move(bg))
        , primitives(std::move(prims))
    {}

    bool intersect  (Ray& r,        Surfel* sf) const;
    bool intersect_p(const Ray& r)              const;
};


// VisibilityTester: monta e testa o raio de sombra ate a luz
class VisibilityTester {
public:
    Point3  p0;     // ponto de contato na superficie (origem do raio de sombra)
    Vector3 n;      // normal da superficie em p0
    Vector3 wi;     // direcao normalizada para a luz
    float   dist;   // distancia ate a luz

    VisibilityTester() = default;
    VisibilityTester(const Point3& p0, const Vector3& n,
                     const Vector3& wi, float dist)
        : p0(p0), n(n), wi(wi), dist(dist) {}

    bool unoccluded(const Scene& scene) const;
};