#include <backgroundd.h> 

    BackgroundSingleColor::BackgroundSingleColor(const RGBColor& c) {
        this->color = c;
    }
    //ctr
    RGBColor BackgroundSingleColor::sampleUV(float u, float v) const {
        return color;
    }
    //a função sampleuv da classe SINGLECOLOR só retorna a cor utilizada na construção



    //----------------------------------------------------------------------------------------------------------------------------------



    Background4Colors::Background4Colors(const std::vector<RGBColor>& colors) {
        if(colors.size() != 4) {
            std::cout << "Erro na inicialização do objeto.\n";
        }
        for(int i = 0; i < colors.size(); i++) {
            corners[i].r = colors[i].r;
            corners[i].g = colors[i].g;
            corners[i].b = colors[i].b;
        }
    }
    //ctr

    RGBColor Background4Colors::lerp( const RGBColor &A, const RGBColor &B, double t ) const {   
        RGBColor temp;

        temp.r = A.r + (B.r - A.r) * t;
        temp.g = A.g + (B.g - A.g) * t;
        temp.b = A.b + (B.b - A.b) * t;
        //interpolação aplicada à cada cor do pixel armazenados no pixel temporario temp

        return temp;
    }
    //lerp

    RGBColor Background4Colors::sampleUV(float u, float v) const {

        RGBColor bottom = lerp(corners[BL], corners[BR], u);
        RGBColor top    = lerp(corners[TL], corners[TR], u);

        return lerp(bottom, top, v);
    }
    //sampleuv



