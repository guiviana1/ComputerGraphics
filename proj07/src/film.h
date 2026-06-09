#pragma once

#include "backgroundd.h"
#include <vector>
#include <string>

class Film {
private:
    int width;
    int height;
    std::vector<RGBColor> pixels;
    std::string filename;
    bool gamma_corrected;   // Se true, aplica codificacao de gamma (linear -> sRGB) na escrita.

public:
    Film(int w, int h, std::string fname = "output.ppm", bool gamma_corrected = false);

    void add(int x, int y, const RGBColor& color);

    void write_image() const;
    void write_image(const std::string& fname) const;

    int getWidth()  const;
    int getHeight() const;
};
