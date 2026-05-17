#include "light.h"
#include <cmath>
#include <limits>

// ── DirectionalLight ──────────────────────────────────────────────────────────

DirectionalLight::DirectionalLight(const RGBColor& I, const RGBColor& scale,
                                   const Point3& from, const Point3& to)
    : Light(LightType::directional, I, scale)
{
    // A direção da luz é o vetor de "from" apontando para "to".
    // Normalizamos aqui uma vez só (a direção é constante para todos os hits).
    direction = normalize(to - from);
}

LightSample DirectionalLight::sample_Li(const Point3& /*hit_point*/) const {
    // Para luz direcional, wi é sempre a mesma direção (paralela),
    // independente de onde o raio acertou.
    // dist = infinito → sem atenuação, e o shadow ray vai ao "infinito".
    return {
        direction,                                      // wi normalizado
        effective_I,                                    // intensidade (sem atenuação)
        std::numeric_limits<float>::infinity()          // dist
    };
}

// ── PointLight ────────────────────────────────────────────────────────────────

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

    // Direção normalizada (l̂ do README)
    Vector3 wi = normalize(to_light);

    // ── Atenuação ─────────────────────────────────────────────────────────────
    // F_att = 1 / (Kc + Kl*d + Kq*d²)
    // Quando Kc=1, Kl=0, Kq=0: F_att=1 (sem efeito — padrão sem atenuação)
    float att = 1.0f / (Kc + Kl * dist + Kq * dist * dist);

    // Intensidade efetiva após atenuação
    RGBColor I_att(
        effective_I.r * att,
        effective_I.g * att,
        effective_I.b * att
    );

    return { wi, I_att, dist };
}