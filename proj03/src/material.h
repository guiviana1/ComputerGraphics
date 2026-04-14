#pragma once

#include "backgroundd.h"

class Material {
public:
    virtual ~Material() = default;

    virtual RGBColor get_color() const = 0;
};


class FlatMaterial : public Material {
private:
    RGBColor color; // cor em [0,1]

public:
 
    explicit FlatMaterial(const RGBColor& color) : color(color) {}

    RGBColor get_color() const override { return color; }
};