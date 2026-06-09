#pragma once

#include "primitive.h"
#include <vector>
#include <memory>

// Lista plana de primitivas — comportamento equivalente ao loop da Scene original.
// Usada como aggregate quando nenhuma aceleracao e solicitada (type="list").
class PrimList : public Primitive {
    std::vector<std::shared_ptr<Primitive>> prims;
    Bounds3f bounds;

public:
    explicit PrimList(std::vector<std::shared_ptr<Primitive>> p)
        : Primitive(nullptr), prims(std::move(p))
    {
        for (const auto& pr : prims)
            bounds = bounds.expand(pr->world_bound());
    }

    bool intersect(Ray& r, Surfel* sf) const override {
        bool hit = false;
        for (const auto& pr : prims)
            if (pr->intersect(r, sf)) hit = true;
        return hit;
    }

    bool intersect_p(const Ray& r) const override {
        for (const auto& pr : prims)
            if (pr->intersect_p(r)) return true;
        return false;
    }

    Bounds3f world_bound() const override { return bounds; }
};
