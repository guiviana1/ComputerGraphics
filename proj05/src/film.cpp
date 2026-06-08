#include "film.h"
#include <fstream>
#include <cmath>
#include <algorithm>

Film::Film(int w, int h, std::string fname, bool gamma_corrected)
    : width(w), height(h), pixels(w * h),
      filename(std::move(fname)), gamma_corrected(gamma_corrected)
{}

// Codifica um canal linear (em [0,255]) para o espaco de exibicao.
// Se gamma estiver ligado, aplica (v/255)^(1/2.2) * 255; caso contrario
// apenas faz o clamp e arredonda.
static int encode_channel(float v, bool gamma) {
    float n = std::max(0.0f, std::min(1.0f, v / 255.0f));   // normaliza e satura em [0,1]
    if (gamma)
        n = std::pow(n, 1.0f / 2.2f);                       // linear -> sRGB (aprox.)
    return int(n * 255.0f + 0.5f);                          // volta para [0,255] arredondando
}

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
        img << encode_channel(c.r, gamma_corrected) << " "
            << encode_channel(c.g, gamma_corrected) << " "
            << encode_channel(c.b, gamma_corrected) << "\n";
    }
}

int Film::getWidth()  const { return width; }
int Film::getHeight() const { return height; }
