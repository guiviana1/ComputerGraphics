#include "background.h"

BackgroundColor::BackgroundColor(const std::vector<RGBColor>& colors) {
    if(colors.size() != 4) {
        std::cout << "Erro na inicialização do objeto.\n";
    }
    for(int i = 0; i < colors.size(); i++) {
        corners[i].r = colors[i].r;
        corners[i].g = colors[i].g;
        corners[i].b = colors[i].b;
    }
}

BackgroundColor::~BackgroundColor() {}

RGBColor BackgroundColor::lerp(const RGBColor &A, const RGBColor &B, double t) const {
    RGBColor temp;

    //nota: tentar mudar a formula pra deixar semelhante à 
    temp.r = A.r + (B.r - A.r) * t;
    temp.g = A.g + (B.g - A.g) * t;
    temp.b = A.b + (B.b - A.b) * t;
    //interpolação aplicada à cada cor do pixel armazenados no pixel temporario temp

    //armazenados no pixel temporario temp
    return temp;
}


//função que vai retornar a cor para um determinado ponto dentro da imagem
RGBColor BackgroundColor::sampleUV(int u, int v, double width, double height) const {

    //normalização, para que os valores tu e tv fiquem entre 0 e 1 (controlando quanto cada extremidade atua na cor final)
    double tu = (double)u / (width - 1);
    double tv = (double)v / (height - 1);
    //pra fazer a normalização, preciso receber width e height como argumentos da função

    RGBColor bottom = lerp(corners[bl], corners[br], tu);
    RGBColor top = lerp(corners[tl], corners[tr], tu);

    RGBColor final = lerp (bottom, top, tv); 

    return final;
}

