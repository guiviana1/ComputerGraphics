#pragma once

#include "ray.h"
#include "surfel.h"
#include "material.h"
#include <memory>


class Primitive {
protected:
    std::shared_ptr<Material> material; // Material associado a esta primitiva.

public:
    explicit Primitive(std::shared_ptr<Material> mat) : material(std::move(mat)) {}
    virtual ~Primitive() = default;

    // Testa intersecao do raio, preenche o Surfel e atualiza r.t_max com o hit mais proximo.
    virtual bool intersect(Ray& r, Surfel* sf) const = 0;

    // Versao rapida para sombras: so retorna se houve intersecao, sem preencher Surfel.
    virtual bool intersect_p(const Ray& r) const = 0;

    // Retorna o material desta primitiva.
    const Material* get_material() const { return material.get(); }

    // Define/atualiza o material desta primitiva.
    void set_material(std::shared_ptr<Material> mat) { material = std::move(mat); }
};