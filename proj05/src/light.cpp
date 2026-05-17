#include "light.h"
#include <cmath>
#include <limits>


DirectionalLight::DirectionalLight(const RGBColor& I, const RGBColor& scale,
                                   const Point3& from, const Point3& to)
    : Light(LightType::directional, I, scale)
{

    direction = normalize(to - from);
}

LightSample DirectionalLight::sample_Li(const Point3& /*hit_point*/) const {
    return {
        direction,                                     
        effective_I,                                    
        std::numeric_limits<float>::infinity()          
    };
}

PointLight::PointLight(const RGBColor& I, const RGBColor& scale, const Point3& pos,
                       float kc, float kl, float kq)
    : Light(LightType::point, I, scale)
    , position(pos)
    , Kc(kc), Kl(kl), Kq(kq)
{}

LightSample PointLight::sample_Li(const Point3& hit_point) const {
    // Vetor do hit ao ponto de luz
    Vector3 to_light = position - hit_point;
    float dist       = to_light.length();

    // Direção normalizada
    Vector3 wi = normalize(to_light);

    float att = 1.0f / (Kc + Kl * dist + Kq * dist * dist);

    RGBColor I_att(
        effective_I.r * att,
        effective_I.g * att,
        effective_I.b * att
    );

    return { wi, I_att, dist };
}