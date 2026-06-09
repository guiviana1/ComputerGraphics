#pragma once

#include "vec3.h"
#include "backgroundd.h"
#include <memory>

//enum class para dizer o tipo de luz sem dynamic_cast.
enum class LightType {
    ambient,
    directional,
    point,
    spot        // proj06
};

struct LightSample {
    Vector3  wi;           
    RGBColor intensity;   
    float    dist;         
};

class Light {
public:
    LightType type;

    RGBColor  effective_I;

    Light(LightType t, const RGBColor& I, const RGBColor& scale)
        : type(t)
        , effective_I(I.r * scale.r, I.g * scale.g, I.b * scale.b)
    {}

    virtual ~Light() = default;

    virtual LightSample sample_Li(const Point3& hit_point) const = 0;
};

class AmbientLight : public Light {
public:
    AmbientLight(const RGBColor& I, const RGBColor& scale)
        : Light(LightType::ambient, I, scale) {}

    // A ambient não tem direção; wi = zero, dist = 0 (nunca usada diretamente)
    LightSample sample_Li(const Point3& /*hit_point*/) const override {
        return { Vector3(0,0,0), effective_I, 0.0f };
    }
};

class DirectionalLight : public Light {
private:
    Vector3 direction;
    float   world_radius;   // ate onde o raio de sombra deve ser testado

public:
    DirectionalLight(const RGBColor& I, const RGBColor& scale,
                     const Point3& from, const Point3& to,
                     float world_radius = 1.0e6f);

    LightSample sample_Li(const Point3& hit_point) const override;
};

//Extra
class PointLight : public Light {
private:
    Point3 position;

    float Kc; //constante
    float Kl; //linear
    float Kq; //quadrática

public:

    PointLight(const RGBColor& I, const RGBColor& scale, const Point3& pos,
               float kc = 1.0f, float kl = 0.0f, float kq = 0.0f);

    LightSample sample_Li(const Point3& hit_point) const override;
};

// Spotlight: luz pontual cuja emissao e restrita a um cone (proj06).
// Dentro do cone interno (falloff) a intensidade e total; entre o cone
// interno e o externo (cutoff) ela decai linearmente ate zero.
class SpotLight : public Light {
private:
    Point3  position;     // localizacao da luz (from)
    Vector3 spot_dir;     // eixo do cone, normalizado (from -> to)
    float   cos_cutoff;   // cosseno do angulo externo (onde a luz zera)
    float   cos_falloff;  // cosseno do angulo interno (onde a luz e total)

    float   Kc, Kl, Kq;   // atenuacao opcional (igual a point)

public:
    SpotLight(const RGBColor& I, const RGBColor& scale,
              const Point3& from, const Point3& to,
              float cutoff_deg, float falloff_deg,
              float kc = 1.0f, float kl = 0.0f, float kq = 0.0f);

    LightSample sample_Li(const Point3& hit_point) const override;
};