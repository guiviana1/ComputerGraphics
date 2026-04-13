#pragma once

#include "camera.h"
#include "backgroundd.h"
#include "primitive.h"
#include "surfel.h"
#include "ray.h"
#include <memory>
#include <vector>

/*!
 * @brief Contem todos os dados da cena virtual:
 *        camera, background e lista de primitivas.
 *
 * Por que Scene existe separada da API?
 * A API é o "sistema de controle" (recebe opcoes, chama o parser, gerencia
 * o filme). A Scene é o "dado" — o mundo virtual que sera renderizado.
 * Separar as duas responsabilidades facilita extensoes futuras (ex: multiplas
 * cenas, cenas procedurais, etc.).
 *
 * O metodo intersect() implementa a travessia linear da lista de objetos,
 * que e a abordagem mais simples (O(n) por raio). No futuro, essa lista
 * pode ser substituida por uma BVH (Bounding Volume Hierarchy) para O(log n).
 */
class Scene {
public:
    std::shared_ptr<Camera>     camera;
    std::shared_ptr<Background> background;

    //! Lista de objetos presentes na cena.
    //! Usamos shared_ptr para que camera e primitivas possam ser
    //! compartilhados com outros sistemas sem problemas de ownership.
    std::vector<std::shared_ptr<Primitive>> primitives;

    Scene() = default;

    Scene(std::shared_ptr<Camera>     cam,
          std::shared_ptr<Background> bg,
          std::vector<std::shared_ptr<Primitive>> prims)
        : camera(std::move(cam))
        , background(std::move(bg))
        , primitives(std::move(prims))
    {}

    /*!
     * @brief Testa intersecao do raio com TODOS os objetos da cena.
     *
     * Itera a lista e chama Primitive::intersect() em cada objeto.
     * Como intersect() atualiza r.t_max a cada hit encontrado, ao final
     * do loop o Surfel contera os dados do objeto MAIS PROXIMO.
     *
     * Complexidade: O(n) onde n = numero de primitivas.
     *
     * @param r  Raio (t_max sera atualizado progressivamente).
     * @param sf Surfel a ser preenchido com o hit mais proximo.
     * @return   true se pelo menos um objeto foi atingido.
     */
    bool intersect(Ray& r, Surfel* sf) const;

    /*!
     * @brief Versao rapida: retorna true se qualquer objeto for atingido.
     *
     * Para em cuanto encontra o primeiro hit (nao precisa do mais proximo).
     * Util para testes de sombra.
     *
     * @param r Raio de teste.
     * @return  true se qualquer primitiva for atingida.
     */
    bool intersect_p(const Ray& r) const;
};