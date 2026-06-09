#pragma once

#include "primitive.h"
#include <vector>
#include <memory>
#include <string>

// BVHAccel — Bounding Volume Hierarchy (proj08).
//
// Aggregate recursivo: cada no interno guarda dois filhos (left/right) e a
// AABB que envolve tudo abaixo dele. As folhas sao primitivas individuais.
//
// Suporta dois metodos de divisao:
//   "equal_counts" — ordena pelo eixo depth%3 e divide ao meio (Shirley).
//   "middle"       — divide pelo ponto medio do eixo de maior extensao dos
//                    centroides (mais balanceado para cenas com distribuicao
//                    nao-uniforme de objetos).
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
