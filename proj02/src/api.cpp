#include "api.h"
#include "parserxml.h"

std::shared_ptr<Background> API::background = nullptr;
std::shared_ptr<Camera>     API::camera     = nullptr;
std::unique_ptr<Film>       API::film       = nullptr;
RunningOptions API::options;

// ================= INIT =================
void API::init_engine(const RunningOptions& opt) {
    options = opt;

    // aqui você pode tratar flags como:
    if (options.quick_render) {
        // futuramente reduzir resolução
    }
}

// ================= RUN =================
void API::run() {

    if (options.input_file.empty()) {
        throw std::runtime_error("Nenhum arquivo de entrada fornecido");
    }

    SceneData data = Parser::parse(options.input_file);

    // configurar API com dados do parser
    set_background(data.background);
    set_camera(data.camera);
    set_film(data.film_width, data.film_height);

    // renderizar
    render();

    // salvar (usa nome do XML ou override)
    if (!options.output_file.empty())
        film->write_image(options.output_file);
    else
        film->write_image(data.film_filename);
}

// ================= SETTERS =================
void API::set_background(std::shared_ptr<Background> bg) {
    background = bg;
}

void API::set_camera(std::shared_ptr<Camera> cam) {
    camera = cam;
}

void API::set_film(int width, int height) {
    film = std::make_unique<Film>(width, height);
}

// ================= RENDER =================
void API::render() {
    if (!background || !camera || !film) {
        throw std::runtime_error("Background, Camera ou Film não inicializados!");
    }

    int w = film->getWidth();
    int h = film->getHeight();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {

            Ray ray = camera->generate_ray(i, j);

            // por enquanto ainda amostra o background com UV normalizado
            float u = float(i) / float(w - 1);
            float v = 1.0f - float(j) / float(h - 1);

            auto color = background->sampleUV(u, v);
            film->add(i, j, color);
        }
    }
}

// ================= OUTPUT =================
void API::write_image() {
    film->write_image(options.output_file);
}

// ================= CLEAN UP =================
void API::clean_up() {
    background.reset();
    camera.reset();
    film.reset();
}