#include "light.h"
#include <cmath>
#include <limits>
#include <algorithm>


DirectionalLight::DirectionalLight(const RGBColor& I, const RGBColor& scale,
                                   const Point3& from, const Point3& to,
                                   float world_radius)
    : Light(LightType::directional, I, scale)
    , world_radius(world_radius)
{

    // wi aponta do ponto para a luz, oposto a direcao de propagacao
    direction = normalize(from - to);
}

LightSample DirectionalLight::sample_Li(const Point3& /*hit_point*/) const {
    // dist = world_radius: alcance finito do raio de sombra
    return {
        direction,
        effective_I,
        world_radius
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

// Spotlight
SpotLight::SpotLight(const RGBColor& I, const RGBColor& scale,
                     const Point3& from, const Point3& to,
                     float cutoff_deg, float falloff_deg,
                     float kc, float kl, float kq)
    : Light(LightType::spot, I, scale)
    , position(from)
    , Kc(kc), Kl(kl), Kq(kq)
{
    spot_dir = normalize(to - from);   // eixo do cone (direcao de emissao)

    const float deg2rad = 3.14159265358979f / 180.0f;
    cos_cutoff  = std::cos(cutoff_deg  * deg2rad);   // angulo externo (maior)
    cos_falloff = std::cos(falloff_deg * deg2rad);   // angulo interno (menor)
}

LightSample SpotLight::sample_Li(const Point3& hit_point) const {
    // Igual a point light: direcao e distancia ate a luz + atenuacao
    Vector3 to_light = position - hit_point;
    float   dist     = to_light.length();
    Vector3 wi       = normalize(to_light);

    float att = 1.0f / (Kc + Kl * dist + Kq * dist * dist);

    // angulo entre o eixo do spot e a direcao luz->ponto
    float cos_angle = dot(spot_dir, -wi);

    // fator do cone; quando cutoff == falloff a borda e dura (evita divisao por zero)
    float spot;
    float denom = cos_falloff - cos_cutoff;
    if (std::fabs(denom) < 1e-6f) {
        spot = (cos_angle >= cos_cutoff) ? 1.0f : 0.0f;   // borda dura
    } else {
        // interpola linearmente entre o cone externo (0) e o interno (1)
        float t = (cos_angle - cos_cutoff) / denom;
        spot = std::max(0.0f, std::min(1.0f, t));
    }

    float f = att * spot;
    RGBColor I_eff(effective_I.r * f, effective_I.g * f, effective_I.b * f);

    return { wi, I_eff, dist };
}