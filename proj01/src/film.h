#pragma once

#include <vector>
#include <string>
#include "background.h"

class Film {
private:
    int width;
    int height;
    std::vector<RGBColor> pixels;

public:
    Film(int w, int h);

    void add(int x, int y, const RGBColor& color);

    void write_image(const std::string& filename) const;

    int getWidth() const;
    
    int getHeight() const;
};