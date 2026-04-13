#pragma once

#include "backgroundd.h"  // RGBColor esta definida aqui

/*!
 * @brief Classe base abstrata para todos os materiais.
 *
 * Por que abstrata? Porque no futuro teremos Phong, PBR, etc.
 * O integrador so precisa saber que pode perguntar "qual a cor emitida?"
 * sem saber o tipo concreto do material.
 */
class Material {
public:
    virtual ~Material() = default;

    /*!
     * @brief Retorna a cor "flat" do material.
     *        Para materiais mais complexos no futuro, isso sera substituido
     *        por um calculo de BRDF completo.
     */
    virtual RGBColor get_color() const = 0;
};

/*!
 * @brief Material mais simples possivel: uma cor solida, sem iluminacao.
 *
 * O integrador "flat" usa esse material para pintar o objeto com sua cor
 * sem nenhum calculo de luz. E o equivalente de desenhar o objeto preenchido
 * com uma cor constante.
 *
 * A cor e armazenada em [0,1] por convencao (sera mapeada para [0,255]
 * na hora de escrever o PPM).
 */
class FlatMaterial : public Material {
private:
    RGBColor color; // cor em [0,1]

public:
    /*!
     * @param color Cor do material em [0,1]. Ex: RGBColor(0.95, 0.05, 0.05)
     *              para um vermelho quase puro.
     */
    explicit FlatMaterial(const RGBColor& color) : color(color) {}

    RGBColor get_color() const override { return color; }
};