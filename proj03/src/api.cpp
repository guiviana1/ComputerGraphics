#include "api.h"
#include "parserxml.h"
#include "surfel.h"

std::shared_ptr<Scene> API::scene = nullptr;
std::unique_ptr<Film> API::film = nullptr;
RunningOptions API::options;

void API::init_engine(const RunningOptions& opt) {
    options = opt;

    if (options.quick_render) {}
}

void API::run() {
    if (options.input_file.empty())
        throw std::runtime_error("Nenhum arquivo de entrada fornecido");

    SceneData data = Parser::parse(options.input_file);

    // Monta a Scene a partir dos dados do parser
    scene = std::make_shared<Scene>(data.camera, data.background, data.primitives);
    set_film(data.film_width, data.film_height);

    render();

    if (!options.output_file.empty() && options.output_file != "output.ppm")
        film->write_image(options.output_file);
    else
        film->write_image(data.film_filename);
}

void API::set_scene(std::shared_ptr<Scene> s) {
    scene = std::move(s);
}

void API::set_film(int width, int height) {
    film = std::make_unique<Film>(width, height);
}

void API::set_camera(std::shared_ptr<Camera> cam) {
    if (!scene) {
        scene = std::make_shared<Scene>();
    }    
    scene->camera = std::move(cam);
}

//set_background e set_camera criam uma scene vazia se necessario
void API::set_background(std::shared_ptr<Background> bg) {
    if (!scene) scene = std::make_shared<Scene>();
    scene->background = std::move(bg);
}


void API::render() {
    if (!scene || !scene->camera || !scene->background || !film)
        throw std::runtime_error("Scene incompleta: camera, background ou film ausente.");

    int w = film->getWidth();
    int h = film->getHeight();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {

            Ray ray = scene->camera->generate_ray(i, j);

            float u = float(i) / float(w - 1);
            float v = 1.0f - float(j) / float(h - 1);

            RGBColor color = scene->background->sampleUV(u, v);

            // Testa intersecao com os objetos da cena
            Surfel sf;
            if (scene->intersect(ray, &sf)) {
                // Houve hit: usa a cor do material (converte [0,1] → [0,255])
                RGBColor mat_color = sf.primitive->get_material()->get_color();
                color = RGBColor(mat_color.r * 255.0f,
                                 mat_color.g * 255.0f,
                                 mat_color.b * 255.0f);
            }

            film->add(i, j, color);
        }
    }
}


void API::write_image() {
    film->write_image(options.output_file);
}

void API::clean_up() {
    scene.reset();
    film.reset();
}