#pragma once

#include "vec3.h"
#include "ray.h"
#include <algorithm>
#include <cmath>

// Caixa alinhada com os eixos (AABB) em precisao float.
// Usada pela BVH para agrupar primitivas e acelerar intersecoes.
struct Bounds3f {
    Point3 p_min{ 1e9f,  1e9f,  1e9f};
    Point3 p_max{-1e9f, -1e9f, -1e9f};

    Bounds3f() = default;

    explicit Bounds3f(const Point3& p) : p_min(p), p_max(p) {}

    Bounds3f(const Point3& a, const Point3& b)
        : p_min(std::min(a.x,b.x), std::min(a.y,b.y), std::min(a.z,b.z))
        , p_max(std::max(a.x,b.x), std::max(a.y,b.y), std::max(a.z,b.z))
    {}

    // Expande a caixa para incluir um ponto.
    Bounds3f expand(const Point3& p) const {
        return Bounds3f(
            Point3(std::min(p_min.x,p.x), std::min(p_min.y,p.y), std::min(p_min.z,p.z)),
            Point3(std::max(p_max.x,p.x), std::max(p_max.y,p.y), std::max(p_max.z,p.z))
        );
    }

    // Uniao com outra caixa.
    Bounds3f expand(const Bounds3f& b) const {
        return Bounds3f(
            Point3(std::min(p_min.x,b.p_min.x), std::min(p_min.y,b.p_min.y), std::min(p_min.z,b.p_min.z)),
            Point3(std::max(p_max.x,b.p_max.x), std::max(p_max.y,b.p_max.y), std::max(p_max.z,b.p_max.z))
        );
    }

    // Centro geometrico da caixa.
    Point3 centroid() const {
        return Point3(0.5f*(p_min.x+p_max.x),
                      0.5f*(p_min.y+p_max.y),
                      0.5f*(p_min.z+p_max.z));
    }

    Vector3 diagonal() const { return p_max - p_min; }

    // Retorna 0 (X), 1 (Y) ou 2 (Z): eixo de maior extensao.
    int max_extent() const {
        Vector3 d = diagonal();
        if (d.x >= d.y && d.x >= d.z) return 0;
        if (d.y >= d.z)                return 1;
        return 2;
    }

    // Slab method: devolve true se o raio intersecta a caixa no intervalo [t_min, t_max].
    bool intersect_p(const Ray& r) const {
        float tmin = r.t_min;
        float tmax = r.t_max;

        // Eixo X
        if (std::fabs(r.d.x) > 1e-9f) {
            float inv = 1.0f / r.d.x;
            float t0  = (p_min.x - r.o.x) * inv;
            float t1  = (p_max.x - r.o.x) * inv;
            if (t0 > t1) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmin > tmax) return false;
        } else if (r.o.x < p_min.x || r.o.x > p_max.x) {
            return false;
        }

        // Eixo Y
        if (std::fabs(r.d.y) > 1e-9f) {
            float inv = 1.0f / r.d.y;
            float t0  = (p_min.y - r.o.y) * inv;
            float t1  = (p_max.y - r.o.y) * inv;
            if (t0 > t1) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmin > tmax) return false;
        } else if (r.o.y < p_min.y || r.o.y > p_max.y) {
            return false;
        }

        // Eixo Z
        if (std::fabs(r.d.z) > 1e-9f) {
            float inv = 1.0f / r.d.z;
            float t0  = (p_min.z - r.o.z) * inv;
            float t1  = (p_max.z - r.o.z) * inv;
            if (t0 > t1) std::swap(t0, t1);
            tmin = std::max(tmin, t0);
            tmax = std::min(tmax, t1);
            if (tmin > tmax) return false;
        } else if (r.o.z < p_min.z || r.o.z > p_max.z) {
            return false;
        }

        return true;
    }
};
