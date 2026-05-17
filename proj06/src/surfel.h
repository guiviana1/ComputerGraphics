#pragma once

#include "vec3.h"

// Ponto 2D parametrico (u,v) usado para mapeamento de textura no futuro
struct Point2f {
    float u, v;
    Point2f() : u(0), v(0) {}
    Point2f(float u, float v) : u(u), v(v) {}
};

class Primitive; //será usada como ponteiro

struct Surfel {
    Point3  p;          //!< Ponto de contato no espaco 3D.
    Vector3 n;          //!< Normal da superficie no ponto de contato.
    Vector3 wo;         //!< Direcao de saida da luz: equivale a -ray.d (raio invertido).
    float   time;       //!< Tempo do hit (relevante para motion blur no futuro).
    Point2f uv;         //!< Coordenadas parametricas (u,v) do ponto na superficie.
    const Primitive* primitive = nullptr; //!< Ponteiro para a primitiva atingida.

    Surfel() : time(0) {}

    Surfel(const Point3& p, const Vector3& n, const Vector3& wo,
           float time, const Point2f& uv, const Primitive* pri)
        : p(p), n(n), wo(wo), time(time), uv(uv), primitive(pri)
    {}
};