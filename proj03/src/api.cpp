#include "api.h"
#include "parserxml.h"
#include "surfel.h"

std::shared_ptr<Scene> API::scene = nullptr;
std::unique_ptr<Film> API::film = nullptr;
RunningOptions API::options;

// ================= INIT =================
void API::init_engine(const RunningOptions& opt) {
    options = opt;

/*    if (options.quick_render) {
    }
*/

}

// ================= RUN =================
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

// ================= SETTERS =================
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

// Compat: set_background e set_camera criam uma scene vazia se necessario
void API::set_background(std::shared_ptr<Background> bg) {
    if (!scene) scene = std::make_shared<Scene>();
    scene->background = std::move(bg);
}


// ================= RENDER =================
/*!
 * Integrador "flat":
 *   - Para cada pixel, gera um raio.
 *   - Testa intersecao com todos os objetos da cena (via Scene::intersect).
 *   - Se houve hit: pinta o pixel com a cor do material do objeto atingido.
 *   - Se nao houve hit: amostra o background.
 *
 * Por que "flat"? Porque a cor do objeto e atribuida diretamente, sem
 * nenhum calculo de iluminacao. E o integrador mais simples possivel,
 * util para validar que a geometria e a intersecao estao corretas antes
 * de adicionar iluminacao.
 *
 * Conversao de cor:
 *   - Material guarda cor em [0, 1] (ex: 0.95 = quase 1.0)
 *   - Film/PPM espera [0, 255]
 *   - Portanto multiplicamos por 255 antes de armazenar no Film.
 *
 *   - Background ja guarda cor em [0, 255] (conforme uso no parserxml.cpp)
 *   - Entao o background nao precisa de conversao.
 *
 * NOTA: isso e uma inconsistencia ja presente no codigo original.
 * Futuramente, padronizar tudo em [0,1] e converter so na escrita do PPM
 * seria o ideal.
 */
void API::render() {
    if (!scene || !scene->camera || !scene->background || !film)
        throw std::runtime_error("Scene incompleta: camera, background ou film ausente.");

    int w = film->getWidth();
    int h = film->getHeight();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {

            // Gera o raio para o pixel (i, j)
            Ray ray = scene->camera->generate_ray(i, j);

            // Coordenadas UV normalizadas para amostragem do background
            float u = float(i) / float(w - 1);
            float v = 1.0f - float(j) / float(h - 1);

            // Cor default: background
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


// ================= OUTPUT =================
void API::write_image() {
    film->write_image(options.output_file);
}

// ================= CLEAN UP =================
void API::clean_up() {
    scene.reset();
    film.reset();
}