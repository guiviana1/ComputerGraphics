#include "api.h"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Uso: ./rt3 <scene.xml>\n";
        return 1;
    }

    RunningOptions opt;
    opt.input_file = argv[1];

    API::init_engine(opt);
    API::run();
    API::clean_up();

    return 0;
}