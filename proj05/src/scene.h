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