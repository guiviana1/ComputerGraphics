#pragma once

#include "backgroundd.h"
#include "film.h"
#include "camera.h"
#include <memory>
#include <string>

struct RunningOptions {
    std::string input_file;
    std::string output_file = "output.ppm";

    bool quick_render = false;

    // crop window (opcional)
    int x0 = 0, x1 = -1, y0 = 0, y1 = -1;
};

class API {
private:
    static std::shared_ptr<Background> background;
    static std::shared_ptr<Camera>     camera;
    static std::unique_ptr<Film>       film;

    static RunningOptions options;

public:
    // inicialização (command line)
    static void init_engine(const RunningOptions& opt);

    // execução principal
    static void run();

    // chamadas do parser
    static void set_background(std::shared_ptr<Background> bg);
    static void set_camera(std::shared_ptr<Camera> cam);
    static void set_film(int width, int height);

    // render
    static void render();

    // saída
    static void write_image();

    // limpeza
    static void clean_up();
};