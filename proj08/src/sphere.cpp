#include "sphere.h"
#include <cmath>

Bounds3f Sphere::world_bound() const {
    return Bounds3f(
        Point3(center.x - radius, center.y - radius, center.z - radius),
        Point3(center.x + radius, center.y + radius, center.z + radius)
    );
}


Sphere::Sphere(const Point3& center, real_type radius, std::shared_ptr<Material> mat)
    : Primitive(std::move(mat)), center(center), radius(radius)
{}


bool Sphere::solve_intersection(const Ray& r, real_type& t_hit) const {
    Vector3 oc = r.o - center;

    real_type a      = dot(r.d, r.d);
    real_type half_b = dot(r.d, oc);

    // Reformulacao: discriminante via componente perpendicular de oc a d_hat.
    // Evita cancelamento catastrofico quando a camera esta longe da cena
    // (oc tem componente paralela gigante que se anularia algebricamente).
    Vector3   d_hat   = normalize(r.d);
    real_type proj    = dot(d_hat, oc);     // componente de oc paralela a d
    Vector3   oc_perp = oc - d_hat * proj;      // componente perpendicular
    real_type disc    = radius * radius - dot(oc_perp, oc_perp);

    if (disc < 0.0f)
        return false;

    // sqrt_disc = |d| * sqrt(disc), para manter t na escala do parametro do raio
    real_type sqrt_disc = std::sqrt(disc * a);

    real_type t0 = (-half_b - sqrt_disc) / a;
    real_type t1 = (-half_b + sqrt_disc) / a;

    if (t0 >= r.t_min && t0 <= r.t_max) { t_hit = t0; return true; }
    if (t1 >= r.t_min && t1 <= r.t_max) { t_hit = t1; return true; }

    return false;
}


bool Sphere::intersect(Ray& r, Surfel* sf) const {
    real_type t_hit;
    if (!solve_intersection(r, t_hit))
        return false;

    r.t_max = t_hit;

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


bool Sphere::intersect_p(const Ray& r) const {
    real_type t_hit;
    return solve_intersection(r, t_hit);
}