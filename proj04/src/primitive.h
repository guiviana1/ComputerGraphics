#pragma once

#include "ray.h"
#include "surfel.h"
#include "material.h"
#include <memory>


class Primitive {
protected:
    std::shared_ptr<Material> material; //!< Material associado a esta primitiva.

public:
    explicit Primitive(std::shared_ptr<Material> mat) : material(std::move(mat)) {}
    virtual ~Primitive() = default;

    /*!
     * @brief Testa intersecao completa do raio com a primitiva.
     *
     * Se houver intersecao no intervalo [r.t_min, r.t_max]:
     *   - Preenche o Surfel *sf com todos os dados do ponto de contato.
     *   - ATUALIZA r.t_max com o t do hit encontrado (assim o proximo objeto
     *     so sera aceito se for MAIS PROXIMO que este).
     *   - Retorna true.
     *
     * Se nao houver intersecao: retorna false sem modificar sf nem r.
     *
     * Por que atualizar r.t_max? Essa e a tecnica classica para encontrar
     * a intersecao MAIS PROXIMA numa lista de objetos: cada vez que achamos
     * um hit, restringimos o intervalo valido do raio. Objetos mais distantes
     * ficam automaticamente fora do intervalo.
     *
     * @param r  Raio de teste (t_max pode ser modificado em caso de hit).
     * @param sf Ponteiro para o Surfel a ser preenchido (pode ser nullptr
     *           se so quiser testar existencia, mas prefira intersect_p()).
     * @return   true se houve intersecao valida.
     */
    virtual bool intersect(Ray& r, Surfel* sf) const = 0;

    /*!
     * @brief Versao rapida: so retorna true/false, sem preencher Surfel.
     *
     * Usada para testes de visibilidade (sombras). Nao atualiza t_max.
     *
     * @param r Raio de teste (nao modificado).
     * @return  true se houve intersecao valida.
     */
    virtual bool intersect_p(const Ray& r) const = 0;

    /*!
     * @brief Retorna ponteiro constante para o material desta primitiva.
     */
    const Material* get_material() const { return material.get(); }
};