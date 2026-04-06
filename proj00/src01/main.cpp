#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <array>
#include "background.h"

const double MAX_VALUE = 255;
const double HEIGHT = 200;
const double WIDTH = 400;

int main() {

  std::vector<RGBColor> colors;

  RGBColor b1{0,0, 51};
  RGBColor tl{0,255, 51};
  RGBColor tr{255, 255, 51};
  RGBColor br{255,0, 51};

  colors.push_back(b1);
  colors.push_back(tl);
  colors.push_back(tr);
  colors.push_back(br);

  BackgroundColor bkg(colors); 
  //Instancio um objeto BackgroundColor recebendo como parametro o vetor colors, definindo os 4 cantos da imagem
  
  std::ofstream img("imagem.ppm");
    
  if(!img.is_open()) {
    std::cout << "Erro ao abrir o arquivo\n";
    return 1;
  }
    
  img << "P3" << std::endl;
  img << WIDTH << " " << HEIGHT << std::endl;
  img << MAX_VALUE << std::endl;
  
  RGBColor pixelteste;

  for(int i = 0; i < HEIGHT; i++) {  
    for(int j = 0; j < WIDTH; j++) {
      pixelteste = bkg.sampleUV(j,HEIGHT - 1 - i, WIDTH, HEIGHT); //esse pixel vai receber uma cor misturada entre duas interpolações lineares realizadas em cima e embaixo
      img << pixelteste.r << " " << pixelteste.g << " " << pixelteste.b << "\n";
    }
  }

  //talvez esse for tenha um problema dos indices, pode ser que seja (j, i), analisar isso
  //porque o degradê está de cabeça pra baixo?
  
  img.close();
    
  return 0;
}

  //Por ultimo, tentar modularizar em uma função para salvar a imagem no arquivo (save_ppm()).
  //Essa função seria membro de bkg? Provavelmente sim.

  //como faz pra mudar para o image buffer?

/*NOTAS  

- Notas de conclusões
- Notas de conclusões

TO DO
1. Avaliar o motivo do porque o degradê ficou ao contrário e porque mudar a posição dos indices i e j afeta o degradê
2. Deveria acontecer a ocorrência de mais cores? 
3. Conseguir colocar a escrita da imagem em ppm para o "image buffer", tal qual solicitado


*/
