#pragma once

#include "vec3.h"
#include "backgroundd.h" // RGBColor
#include <memory>

// Usamos um enum class para identificar o tipo de luz sem dynamic_cast.
enum class LightType {
    ambient,
    directional,
    point
};

// ─────────────────────────────────────────────────────────────────────────────
// Estrutura de retorno de sample_Li():
//   wi      = direção normalizada DA LUZ ao ponto de hit (l̂ do README)
//   intensity = intensidade efetiva da luz (já aplicado scale e atenuação)
//   dist    = distância entre hit e fonte (infinito para direcional/ambiente)
// ─────────────────────────────────────────────────────────────────────────────
struct LightSample {
    Vector3  wi;           // direção normalizada para a fonte de luz
    RGBColor intensity;    // intensidade final (com scale e atenuação)
    float    dist;         // distância hit→fonte (∞ para direcionais)
};

// ─────────────────────────────────────────────────────────────────────────────
// Classe base abstrata Light
//
// Por que retornar LightSample ao invés de passar ponteiros de saída?
// O README sugere ponteiros (wi*, vis*), mas retornar uma struct é mais
// seguro, legível e se encaixa melhor no estilo do restante do projeto.
// ─────────────────────────────────────────────────────────────────────────────
class Light {
public:
    LightType type;

    // intensity * scale — calculado no construtor, armazenado pronto para uso.
    // Guardar o produto evita recalcular a cada sample_Li().
    RGBColor  effective_I;

    Light(LightType t, const RGBColor& I, const RGBColor& scale)
        : type(t)
        , effective_I(I.r * scale.r, I.g * scale.g, I.b * scale.b)
    {}

    virtual ~Light() = default;

    /*!
     * @brief Retorna a contribuição desta luz para um ponto de hit P.
     *
     * @param hit_point  Ponto na superfície onde o raio acertou.
     * @return LightSample com direção wi, intensidade e distância.
     */
    virtual LightSample sample_Li(const Point3& hit_point) const = 0;
};

// ─────────────────────────────────────────────────────────────────────────────
// AmbientLight
//
// Não tem direção nem posição. Afeta TODOS os objetos igualmente.
// O integrador a trata separado das demais luzes (soma Ia*ka uma vez só).
// ─────────────────────────────────────────────────────────────────────────────
class AmbientLight : public Light {
public:
    AmbientLight(const RGBColor& I, const RGBColor& scale)
        : Light(LightType::ambient, I, scale) {}

    // A ambient não tem direção; wi = zero, dist = 0 (nunca usada diretamente)
    LightSample sample_Li(const Point3& /*hit_point*/) const override {
        return { Vector3(0,0,0), effective_I, 0.0f };
    }
};

// ─────────────────────────────────────────────────────────────────────────────
// DirectionalLight
//
// Luz infinitamente distante (como o sol): todos os raios chegam paralelos.
// A direção é from→to, normalizada. dist = infinito (sem atenuação).
// ─────────────────────────────────────────────────────────────────────────────
class DirectionalLight : public Light {
private:
    Vector3 direction; // direção normalizada FROM→TO (ou seja, de onde vem a luz → ponto)

public:
    /*!
     * @param from  Posição simbólica da fonte (usada só para calcular direção).
     * @param to    Ponto para onde a luz aponta.
     */
    DirectionalLight(const RGBColor& I, const RGBColor& scale,
                     const Point3& from, const Point3& to);

    LightSample sample_Li(const Point3& hit_point) const override;
};

// ─────────────────────────────────────────────────────────────────────────────
// PointLight
//
// Ponto de luz com posição 3D. A direção é recalculada a cada hit (depende
// de onde o raio acertou). Suporta atenuação quadrática (extra credit).
//
// Atenuação: F = 1 / (Kc + Kl*d + Kq*d²)
//   Kc = constante, Kl = linear, Kq = quadrática
//   Sem atenuação: Kc=1, Kl=0, Kq=0  →  F=1 (sem efeito)
// ─────────────────────────────────────────────────────────────────────────────
class PointLight : public Light {
private:
    Point3 position;

    // Coeficientes de atenuação
    float Kc; // constante
    float Kl; // linear
    float Kq; // quadrática

public:
    /*!
     * @param pos       Posição da luz no espaço do mundo.
     * @param kc,kl,kq  Coeficientes de atenuação (default sem atenuação).
     */
    PointLight(const RGBColor& I, const RGBColor& scale, const Point3& pos,
               float kc = 1.0f, float kl = 0.0f, float kq = 0.0f);

    LightSample sample_Li(const Point3& hit_point) const override;
};