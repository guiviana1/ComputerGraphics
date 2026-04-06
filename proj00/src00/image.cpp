#include <bits/stdc++.h>

class RGBColor 
{
public:
    double r, g, b;
};

int main() {

    const double MAX_VALUE = 255;
    const double HEIGHT = 200;
    const double WIDTH = 400;

    RGBColor pixelteste;

    pixelteste.r = 0.0;
    pixelteste.g = MAX_VALUE;
    pixelteste.b = 0.2 * MAX_VALUE;

    double widthVariation = MAX_VALUE / (WIDTH - 1);
    double heightVariation = MAX_VALUE / (HEIGHT - 1);

    std::cout << widthVariation << std::endl;
    std::cout << heightVariation << std::endl;

    std::ofstream img("imagem.ppm");

    if(!img.is_open()) {
        std::cout << "Erro ao abrir o arquivo\n";
        return 1;
    }

    img << "P3" << std::endl;
    img << WIDTH << " " << HEIGHT << std::endl;
    img << MAX_VALUE << std::endl;

    for(int i = 0; i < HEIGHT; i++) {
      pixelteste.r = 0.0;
      for(int j = 0; j < WIDTH; j++) {
        img << static_cast<int>(pixelteste.r) << " "
            << static_cast<int>(pixelteste.g) << " "
            << static_cast<int>(pixelteste.b) << std::endl;
        pixelteste.r = pixelteste.r + widthVariation;
      }
      pixelteste.r = 0;
      pixelteste.g = pixelteste.g - heightVariation;
    }

    img.close();
    
    return 0;
}

/*NOTAS

- Os valores de r estavam sendo truncados para o 0, pois pixelteste.r é um inteiro e eu estava adicionando valores double (heightVariation e widthVariation)
por causa disso, como r começava em 0 e continuava sendo adicionando em valores que eram sempre truncados para o zero, ele nunca incrementava e permanecia
em zero pra sempre.

- Além disso, o bug do gradiente encerrar bem no meio também era aparentemente causado pelo problema do tipo do atributo de RGBColor, já que retornar o tipo 
para int retorna o problema. Não consigo imaginar o motivo, algo a se descobrir.

- Outro ponto: o problema do red de pixelteste era causado porque pixelteste.r era int e widthVariation double. Porque o truncamento acontecia para 
o int (<-, ou para a esquerda) ? Qual o motivo do tipo de pixelteste.r ser o "comandante" do truncamento?

*/