#include "plane.h"
#include <cmath>

Plane::Plane(const Point3& point, const Vector3& normal, std::shared_ptr<Material> mat)
    : Primitive(std::move(mat)), point(point), normal(normalize(normal))
{}

// Plano e infinito: retorna uma caixa gigante para que o BVH nunca o descarte.
Bounds3f Plane::world_bound() const {
    const float INF = 1e9f;
    return Bounds3f(Point3(-INF,-INF,-INF), Point3(INF,INF,INF));
}

bool Plane::solve_intersection(const Ray& r, real_type& t_hit) const {
    // Intersecao raio-plano: t = ((point - o) . n) / (d . n).
    real_type denom = dot(r.d, normal);
    if (std::fabs(denom) < 1e-8f)
        return false;   // raio paralelo ao plano: sem intersecao

    real_type t = dot(point - r.o, normal) / denom;
    if (t < r.t_min || t > r.t_max)
        return false;

    t_hit = t;
    return true;
}

bool Plane::intersect(Ray& r, Surfel* sf) const {
    real_type t_hit;
    if (!solve_intersection(r, t_hit))
        return false;

    r.t_max = t_hit;

    if (sf != nullptr) {
        Point3 hit = r(t_hit);

        // Orienta a normal para o lado de onde o raio veio, de modo que o
        // plano seja sombreado corretamente visto de qualquer um dos lados.
        Vector3 n = (dot(r.d, normal) < 0.0f) ? normal : -normal;
        Vector3 wo = -r.d;

        *sf = Surfel(hit, n, wo, t_hit, Point2f(0.0f, 0.0f), this);
    }

    return true;
}

bool Plane::intersect_p(const Ray& r) const {
    real_type t_hit;
    return solve_intersection(r, t_hit);
}
