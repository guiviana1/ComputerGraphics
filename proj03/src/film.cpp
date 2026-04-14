#include "film.h"
#include <fstream>

Film::Film(int w, int h) : width(w), height(h), pixels(w * h) {}

void Film::add(int x, int y, const RGBColor &color) {
    pixels[y * width + x] = color;
} //essa formula adiciona no vector interno de film os valores dos pixels
//função que adiciona os resultados

void Film::write_image(const std::string& filename) const {
    std::ofstream img(filename);

    img << "P3\n";
    img << width << " " << height << "\n";
    img << "255\n";

    for (const auto& c : pixels) {
        img << int(c.r) << " "
            << int(c.g) << " "
            << int(c.b) << "\n";
    }//opera por cada cor de cada pixel em pixels e salva na imagem, formato ppm
}//e essa função escreve no arquivo as informações de 'pixels'

int Film::getHeight() const{
    return height;
}

int Film::getWidth() const {
    return width;
}

//o background é o que a câmera enxerga, o film é a foto final que ela tirou
//é o que fica, o que ela capturou, é para onde os raios vão voltar quando baterem no background e voltar

