#pragma once

#include "ray.h"
#include "surfel.h"
#include "material.h"
#include <memory>


class Primitive {
protected:
    std::shared_ptr<Material> material; // material associado a esta primitiva

public:
    explicit Primitive(std::shared_ptr<Material> mat) : material(std::move(mat)) {}
    virtual ~Primitive() = default;

    // testa intersecao do raio com a primitiva e preenche o Surfel
    virtual bool intersect(Ray& r, Surfel* sf) const = 0;

    // versao rapida so retorna true/false, usada em testes de sombra
    virtual bool intersect_p(const Ray& r) const = 0;

    // retorna o material desta primitiva
    const Material* get_material() const { return material.get(); }
};