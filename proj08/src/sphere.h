#pragma once

#include "primitive.h"
#include "vec3.h"
#include <memory>

class Sphere : public Primitive {
private:
    Point3    center; //!< Centro da esfera no espaco do mundo.
    real_type radius; //!< Raio da esfera.

public:
    Sphere(const Point3& center, real_type radius, std::shared_ptr<Material> mat);

    
    bool     intersect  (Ray& r, Surfel* sf) const override;
    bool     intersect_p(const Ray& r)       const override;
    Bounds3f world_bound()                   const override;

private:
    bool solve_intersection(const Ray& r, real_type& t_hit) const;
};