#include <film.h>
#include <fstream>

void Film::add(int x, int y, const RGBColor &color) {
    pixels[y * width + x] = color; //entender melhor isso aqui, o q isso substitui?
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


