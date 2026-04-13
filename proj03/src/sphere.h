#pragma once

#include "primitive.h"
#include "vec3.h"
#include <memory>

/*!
 * @brief Esfera geometrica no espaco 3D.
 *
 * Implementa Primitive. A equacao implicita da esfera e:
 *   f(p) = (p - center).(p - center) - radius^2 = 0
 *
 * Qualquer ponto p que satisfaz f(p) = 0 esta na superficie da esfera.
 */
class Sphere : public Primitive {
private:
    Point3    center; //!< Centro da esfera no espaco do mundo.
    real_type radius; //!< Raio da esfera.

public:
    /*!
     * @param center  Centro da esfera.
     * @param radius  Raio da esfera (deve ser positivo).
     * @param mat     Material associado (compartilhado com shared_ptr).
     */
    Sphere(const Point3& center, real_type radius, std::shared_ptr<Material> mat);

    /*!
     * @brief Intersecao completa raio-esfera.
     *
     * Resolve a equacao quadratica at^2 + 2*half_b*t + c = 0 onde:
     *   a      = d.d
     *   half_b = d.(o - center)
     *   c      = (o - center).(o - center) - radius^2
     *
     * Se houver hit valido, preenche o Surfel e atualiza r.t_max.
     *
     * @param r  Raio (t_max atualizado em caso de hit).
     * @param sf Surfel a ser preenchido.
     * @return   true se houve hit no intervalo [t_min, t_max].
     */
    bool intersect(Ray& r, Surfel* sf) const override;

    /*!
     * @brief Versao rapida: so retorna true/false.
     *
     * Identica a intersect() mas sem preencher Surfel nem atualizar t_max.
     *
     * @param r Raio de teste.
     * @return  true se houve hit no intervalo [t_min, t_max].
     */
    bool intersect_p(const Ray& r) const override;

private:
    /*!
     * @brief Nucleo da matematica de intersecao.
     *
     * Retorna true se houver hit, e nesse caso escreve o t do hit mais
     * proximo valido em 't_hit'. Usado internamente por intersect() e
     * intersect_p() para nao duplicar codigo.
     *
     * @param r     Raio de teste.
     * @param t_hit Saida: valor de t do primeiro hit valido.
     * @return      true se houve hit no intervalo [t_min, t_max].
     */
    bool solve_intersection(const Ray& r, real_type& t_hit) const;
};