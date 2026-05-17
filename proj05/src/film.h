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

public:
    Film(int w, int h, std::string fname = "output.ppm");

    void add(int x, int y, const RGBColor& color);

    void write_image() const;
    void write_image(const std::string& fname) const;

    int getWidth()  const;
    int getHeight() const;
};
