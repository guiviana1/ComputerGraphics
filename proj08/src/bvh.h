#pragma once

#include "primitive.h"
#include <vector>
#include <memory>
#include <string>

// Bounding Volume Hierarchy: arvore de AABBs para acelerar intersecoes.
class BVHAccel : public Primitive {
    std::shared_ptr<Primitive> left;
    std::shared_ptr<Primitive> right;
    Bounds3f box;

public:
    BVHAccel(std::vector<std::shared_ptr<Primitive>> objects,
             const std::string& split_method = "equal_counts",
             int max_prims_per_node = 1,
             int depth = 0);

    bool     intersect  (Ray& r, Surfel* sf) const override;
    bool     intersect_p(const Ray& r)       const override;
    Bounds3f world_bound()                   const override { return box; }
};
