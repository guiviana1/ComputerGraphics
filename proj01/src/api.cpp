#include <api.h>
#include <parserxml.h>

std::unique_ptr<Background> API::background = nullptr;
std::unique_ptr<Film> API::film = nullptr;
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
    // futuramente:
    // parse(options.input_file);

    // POR ENQUANTO (sem parser):
    render();
    write_image();
}

// ================= SETTERS =================
void API::set_background(Background* bg) {
    background.reset(bg);
}

void API::set_film(int width, int height) {
    film = std::make_unique<Film>(width, height);
}

// ================= RENDER =================
void API::render() {
    if (!background || !film) {
        throw std::runtime_error("Background ou Film não inicializados!");
    }

    int w = film->getWidth();
    int h = film->getHeight();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {

            float u = float(i) / float(w);
            float v = float(j) / float(h);

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
    film.reset();
}