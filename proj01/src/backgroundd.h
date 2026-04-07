#pragma once

#include <bits/stdc++.h>
#include <array>
#include <vector>
#include <iostream>

class RGBColor {
public:
    float r, g, b;

    RGBColor();
    RGBColor(float r, float g, float b);
    //float para economizar espaço
};


class Background {
public:
    virtual ~Background() = default;

    virtual RGBColor sampleUV(float u, float v) const = 0;
};


class BackgroundSingleColor : public Background {
private:
    RGBColor color;
    
public:
    BackgroundSingleColor(const RGBColor& c);

    RGBColor sampleUV(float u, float v) const override; //recebe as coordenadas já normalizadas
};
//Essa classe é uma herança da classe background principal


class Background4Colors : public Background {
private:
    std::array<RGBColor,4> corners{RGBColor{0,0,0},RGBColor{0,0,0},RGBColor{0,0,0},RGBColor{0,0,0}};

    enum Corners {
        BL = 0,
        TL,
        TR,
        BR
    };

    RGBColor lerp( const RGBColor &A, const RGBColor &B, double t ) const;

public:
    Background4Colors(const std::vector<RGBColor> &colors);

    RGBColor sampleUV(float u, float v) const override;

    void show_corners(Background4Colors bkg){
      for(int j = 0; j < corners.size(); j++) {
        std::cout << corners[j].r << " " << corners[j].g << " " << corners[j].b << " I ";
      }
    }
};
//Essa classe é uma herança da classe background principal