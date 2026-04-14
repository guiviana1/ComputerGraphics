#pragma once

#include "camera.h"
#include "backgroundd.h"
#include "primitive.h"
#include "surfel.h"
#include "ray.h"
#include <memory>
#include <vector>

class Scene {
public:
    std::shared_ptr<Camera>     camera;
    std::shared_ptr<Background> background;
    std::vector<std::shared_ptr<Primitive>> primitives;

    Scene() = default;

    Scene(std::shared_ptr<Camera>     cam,
          std::shared_ptr<Background> bg,
          std::vector<std::shared_ptr<Primitive>> prims)
        : camera(std::move(cam))
        , background(std::move(bg))
        , primitives(std::move(prims))
    {}

    bool intersect(Ray& r, Surfel* sf) const;
    bool intersect_p(const Ray& r) const;
};