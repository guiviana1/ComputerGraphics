#pragma once

#include "backgroundd.h"

class Material {
public:
    virtual ~Material() = default;
    virtual RGBColor get_color() const = 0;
};

class FlatMaterial : public Material {
private:
    RGBColor color;

public:
    explicit FlatMaterial(const RGBColor& color) : color(color) {}

    RGBColor get_color() const override { return color; }
    RGBColor kd()        const          { return color; }
};


class BlinnPhongMaterial : public Material {
private:
    RGBColor ka_;         // coeficiente ambiente
    RGBColor kd_;         // coeficiente difuso
    RGBColor ks_;         // coeficiente especular
    float    glossiness_; // expoente g
    RGBColor km_;         // coeficiente espelho (mirror) — proj06

public:
    BlinnPhongMaterial(const RGBColor& ka, const RGBColor& kd,
                       const RGBColor& ks, float glossiness,
                       const RGBColor& mirror = RGBColor(0.0f, 0.0f, 0.0f))
        : ka_(ka), kd_(kd), ks_(ks), glossiness_(glossiness), km_(mirror)
    {}

    // get_color() retorna kd para compatibilidade com a interface base
    RGBColor get_color()   const override { return kd_; }

    RGBColor ka()          const { return ka_; }
    RGBColor kd()          const { return kd_; }
    RGBColor ks()          const { return ks_; }
    float    glossiness()  const { return glossiness_; }
    RGBColor mirror()      const { return km_; }

    // true se o material reflete (algum canal de km > 0)
    bool is_mirror() const {
        return km_.r > 0.0f || km_.g > 0.0f || km_.b > 0.0f;
    }
};