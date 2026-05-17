#include "film.h"
#include <fstream>

Film::Film(int w, int h, std::string fname)
    : width(w), height(h), pixels(w * h), filename(std::move(fname))
{}

void Film::add(int x, int y, const RGBColor& color) {
    pixels[y * width + x] = color;
}

void Film::write_image() const {
    write_image(filename);
}

void Film::write_image(const std::string& fname) const {
    std::ofstream img(fname);

    img << "P3\n";
    img << width << " " << height << "\n";
    img << "255\n";

    for (const auto& c : pixels) {
        img << int(c.r) << " "
            << int(c.g) << " "
            << int(c.b) << "\n";
    }
}

int Film::getWidth()  const { return width; }
int Film::getHeight() const { return height; }
