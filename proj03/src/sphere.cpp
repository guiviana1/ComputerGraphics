#include "sphere.h"
#include <cmath>

// ── Construtor ────────────────────────────────────────────────────────────────

Sphere::Sphere(const Point3& center, real_type radius, std::shared_ptr<Material> mat)
    : Primitive(std::move(mat)), center(center), radius(radius)
{}

// ── Nucleo matematico ─────────────────────────────────────────────────────────

bool Sphere::solve_intersection(const Ray& r, real_type& t_hit) const {
    Vector3 oc = r.o - center;

    real_type a      = dot(r.d, r.d);
    real_type half_b = dot(r.d, oc);
    real_type c_coef = dot(oc, oc) - radius * radius;

    real_type discriminant = half_b * half_b - a * c_coef;
    if (discriminant < 0.0f)
        return false;

    real_type sqrt_disc = std::sqrt(discriminant);

    // Citardauq: evita cancelamento catastrofico quando half_b ≈ sqrt_disc
    real_type q = (half_b >= 0.0f) ? (-half_b - sqrt_disc) : (-half_b + sqrt_disc);
    real_type t0 = q / a;
    real_type t1 = c_coef / q;  // Vieta: t0*t1 = c/a

    if (t0 > t1) std::swap(t0, t1);

    if (t0 >= r.t_min && t0 <= r.t_max) { t_hit = t0; return true; }
    if (t1 >= r.t_min && t1 <= r.t_max) { t_hit = t1; return true; }

    return false;
}

// ── intersect() ──────────────────────────────────────────────────────────────

bool Sphere::intersect(Ray& r, Surfel* sf) const {
    real_type t_hit;
    if (!solve_intersection(r, t_hit))
        return false;

    r.t_max = t_hit;

    // ── Preenche o Surfel ──────────────────────────────────────────────────
    if (sf != nullptr) {
        Point3 hit_point = r(t_hit); // p = o + d*t

        Vector3 normal = (hit_point - center) * (1.0f / radius);

        Vector3 wo = Vector3(-r.d.x, -r.d.y, -r.d.z);

        float phi   = std::atan2(normal.y, normal.x);
        float theta = std::acos(std::fmax(-1.0f, std::fmin(1.0f, normal.z)));
        if (phi < 0.0f) phi += 2.0f * 3.14159265f;
        Point2f uv(phi / (2.0f * 3.14159265f), theta / 3.14159265f);

        *sf = Surfel(hit_point, normal, wo, t_hit, uv, this);
    }

    return true;
}

// ── intersect_p() ─────────────────────────────────────────────────────────────

bool Sphere::intersect_p(const Ray& r) const {
    real_type t_hit;
    // Nao atualiza t_max, nao preenche Surfel — so testa existencia
    return solve_intersection(r, t_hit);
}