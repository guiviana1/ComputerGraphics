#pragma once

#include "vec3.h"
#include <limits>

class Ray {
public:
    Point3    o; // origem
    Vector3   d; // direção

    real_type t_min;
    real_type t_max;

    Ray() : t_min(0), t_max(std::numeric_limits<real_type>::infinity()) {}

    Ray(const Point3& o, const Vector3& d,
        real_type start = 0,
        real_type end   = std::numeric_limits<real_type>::infinity())
        : o(o), d(d), t_min(start), t_max(end) {}

    // retorna o ponto na posição t ao longo do raio
    Point3 operator()(real_type t) const { return o + d * t; }
};
