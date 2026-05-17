#pragma once

#include "backgroundd.h"

// ─────────────────────────────────────────────────────────────────────────────
// Classe base abstrata Material
// ─────────────────────────────────────────────────────────────────────────────
class Material {
public:
    virtual ~Material() = default;
    virtual RGBColor get_color() const = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// FlatMaterial — cor sólida, sem iluminação.
// Usado pelo RayCastIntegrator. Cor armazenada em [0,255].
// ─────────────────────────────────────────────────────────────────────────────
class FlatMaterial : public Material {
private:
    RGBColor color;

public:
    explicit FlatMaterial(const RGBColor& color) : color(color) {}

    RGBColor get_color() const override { return color; }
    RGBColor kd()        const          { return color; }
};

// ─────────────────────────────────────────────────────────────────────────────
// BlinnPhongMaterial — material para o modelo de iluminação Blinn-Phong.
//
// Todos os coeficientes (ka, kd, ks) estão em [0,1].
// O integrador multiplica esses valores pela intensidade da luz (também [0,1])
// e no final escala o resultado para [0,255] antes de gravar no Film.
//
// Membros:
//   ka  = coeficiente ambiente  (quanto da luz ambiente é refletida)
//   kd  = coeficiente difuso    (cor "base" do objeto)
//   ks  = coeficiente especular (cor do highlight; geralmente branco/cinza)
//   g   = glossiness / expoente de Phong
//           baixo (1-10)   → superfície fosca, highlight grande
//           alto  (>100)   → superfície polida, highlight pequeno e concentrado
// ─────────────────────────────────────────────────────────────────────────────
class BlinnPhongMaterial : public Material {
private:
    RGBColor ka_;         // coeficiente ambiente
    RGBColor kd_;         // coeficiente difuso
    RGBColor ks_;         // coeficiente especular
    float    glossiness_; // expoente g

public:
    BlinnPhongMaterial(const RGBColor& ka, const RGBColor& kd,
                       const RGBColor& ks, float glossiness)
        : ka_(ka), kd_(kd), ks_(ks), glossiness_(glossiness)
    {}

    // get_color() retorna kd para compatibilidade com a interface base
    RGBColor get_color()   const override { return kd_; }

    // Accessors usados pelo BlinnPhongIntegrator
    RGBColor ka()          const { return ka_; }
    RGBColor kd()          const { return kd_; }
    RGBColor ks()          const { return ks_; }
    float    glossiness()  const { return glossiness_; }
};