#include "sphere.h"
#include <cmath>

// ── Construtor ────────────────────────────────────────────────────────────────

Sphere::Sphere(const Point3& center, real_type radius, std::shared_ptr<Material> mat)
    : Primitive(std::move(mat)), center(center), radius(radius)
{}

// ── Nucleo matematico ─────────────────────────────────────────────────────────

/*!
 * Por que half_b ao inves de b completo?
 *
 * A formula classica e: t = (-b ± sqrt(b^2 - 4ac)) / (2a)
 *
 * Aqui, b = 2 * dot(d, oc), entao b e sempre par.
 * Definindo half_b = dot(d, oc), a formula simplifica para:
 *
 *   t = (-half_b ± sqrt(half_b^2 - a*c)) / a
 *
 * Isso elimina multiplicacoes por 2 e 4, resultando em:
 *   - Menos operacoes de ponto flutuante
 *   - Menor acumulacao de erros numericos
 *   - Codigo mais limpo
 *
 * Nota sobre a camera ortografica e esferas distantes:
 * Quando a esfera e muito distante da camera, os valores de 'oc' ficam
 * grandes, e o discriminante pode sofrer cancelamento catastrofico
 * (dois numeros grandes e quase iguais sendo subtraidos).
 * A solucao robusta e usar a formula alternativa de Citardauq para
 * a raiz de maior magnitude, e derivar a outra por Vieta (t0*t1 = c/a).
 * Implementamos essa versao robusta abaixo.
 */
bool Sphere::solve_intersection(const Ray& r, real_type& t_hit) const {
    // oc = vetor do centro da esfera ate a origem do raio
    Vector3 oc = r.o - center;

    // Coeficientes da equacao quadratica at^2 + 2*half_b*t + c = 0
    real_type a      = dot(r.d, r.d);          // ||d||^2
    real_type half_b = dot(r.d, oc);           // d . (o - c)
    real_type c_coef = dot(oc, oc) - radius * radius; // ||o-c||^2 - r^2

    // Discriminante simplificado: half_b^2 - a*c
    // Se negativo: raio passa fora da esfera (sem intersecao real)
    real_type discriminant = half_b * half_b - a * c_coef;

    if (discriminant < 0.0f)
        return false;

    real_type sqrt_disc = std::sqrt(discriminant);

    // ── Formula robusta (Citardauq) ────────────────────────────────────────
    // A formula classica (-b ± sqrt(disc)) / (2a) pode perder precisao
    // quando half_b e sqrt_disc sao quase iguais (cancelamento catastrofico).
    //
    // A abordagem robusta:
    // 1) Calcula a raiz de MAIOR MAGNITUDE diretamente (sem cancelamento).
    // 2) A outra raiz e obtida pela relacao de Vieta: t0 * t1 = c / a
    //
    // Se half_b >= 0: a raiz negativa tem maior magnitude → usa -half_b - sqrt_disc
    // Se half_b <  0: a raiz positiva tem maior magnitude → usa -half_b + sqrt_disc
    real_type q = (half_b >= 0.0f)
                ? (-half_b - sqrt_disc)   // raiz menor (mais proxima, half_b >= 0)
                : (-half_b + sqrt_disc);  // raiz maior (mais proxima, half_b < 0)

    real_type t0 = q / a;
    real_type t1 = c_coef / q;  // Vieta: t0 * t1 = c/a → t1 = c/(a*t0) = c/q

    // Garantir t0 <= t1
    if (t0 > t1) std::swap(t0, t1);

    // Escolher a menor raiz valida no intervalo [t_min, t_max]
    // Primeiro testamos t0 (a mais proxima)
    if (t0 >= r.t_min && t0 <= r.t_max) {
        t_hit = t0;
        return true;
    }
    // Se t0 estiver fora (ex: atras da camera), testamos t1
    if (t1 >= r.t_min && t1 <= r.t_max) {
        t_hit = t1;
        return true;
    }

    // Ambas as raizes fora do intervalo valido
    return false;
}

// ── intersect() ──────────────────────────────────────────────────────────────

bool Sphere::intersect(Ray& r, Surfel* sf) const {
    real_type t_hit;
    if (!solve_intersection(r, t_hit))
        return false;

    // ── Atualiza t_max do raio ─────────────────────────────────────────────
    // Isso e FUNDAMENTAL para encontrar o objeto mais proximo numa lista:
    // depois deste hit, so aceitaremos objetos com t < t_hit.
    r.t_max = t_hit;

    // ── Preenche o Surfel ──────────────────────────────────────────────────
    if (sf != nullptr) {
        Point3 hit_point = r(t_hit); // p = o + d*t

        // Normal da esfera: vetor do centro ao ponto de hit, normalizado.
        // Dividimos pelo raio ao inves de normalize() para aproveitar que
        // sabemos exatamente o comprimento do vetor (= radius).
        Vector3 normal = (hit_point - center) * (1.0f / radius);

        // wo = direcao de saida da luz = direcao oposta ao raio incidente
        Vector3 wo = Vector3(-r.d.x, -r.d.y, -r.d.z);

        // Coordenadas UV (parametricas) da esfera — extra credit do enunciado.
        // phi   = atan2(ny, nx)  → azimute [0, 2π]
        // theta = acos(nz)       → elevacao [0, π]
        // u = phi   / (2π)  ∈ [0,1]
        // v = theta / π     ∈ [0,1]
        float phi   = std::atan2(normal.y, normal.x);
        float theta = std::acos(std::fmax(-1.0f, std::fmin(1.0f, normal.z)));
        if (phi < 0.0f) phi += 2.0f * 3.14159265f;
        Point2f uv(phi / (2.0f * 3.14159265f), theta / 3.14159265f);

        *sf = Surfel(hit_point, normal, wo, t_hit, uv, this);
    }

    return true;
}

// ── intersect_p() ─────────────────────────────────────────────────────────────

bool Sphere::intersect_p(const Ray& r) const {
    real_type t_hit;
    // Nao atualiza t_max, nao preenche Surfel — so testa existencia
    return solve_intersection(r, t_hit);
}