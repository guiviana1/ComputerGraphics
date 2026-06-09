#pragma once

#include "primitive.h"
#include "vec3.h"
#include <memory>

// Plano infinito definido por um ponto e uma normal.
class Plane : public Primitive {
private:
    Point3  point;   //!< Um ponto qualquer pertencente ao plano.
    Vector3 normal;  //!< Normal do plano (normalizada).

public:
    Plane(const Point3& point, const Vector3& normal, std::shared_ptr<Material> mat);

    bool intersect(Ray& r, Surfel* sf) const override;
    bool intersect_p(const Ray& r)     const override;

private:
    bool solve_intersection(const Ray& r, real_type& t_hit) const;
};
