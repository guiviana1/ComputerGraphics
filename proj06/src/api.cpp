#include "api.h"
#include "parserxml.h"
#include "integrator.h"
#include "film.h"
#include "scene.h"
#include <stdexcept>
#include <memory>

RunningOptions API::options;

void API::init_engine(const RunningOptions& opt) {
    options = opt;
}

void API::run() {
    if (options.input_file.empty())
        throw std::runtime_error("Nenhum arquivo de entrada fornecido");

    auto jobs = Parser::parse(options.input_file);

    for (auto& data : jobs) {
        auto film  = std::make_shared<Film>(data.film_width, data.film_height,
                                            data.film_filename, data.film_gamma);

        // Monta a cena com background, primitivas e luzes
        auto scene = std::make_shared<Scene>(data.background, data.primitives);
        scene->lights        = data.lights;        
        scene->ambient_light = data.ambient_light;

        std::shared_ptr<Integrator> integrator;

        if (data.integrator_type == "flat") {
            integrator = std::make_shared<RayCastIntegrator>(data.camera, film);

        } else if (data.integrator_type == "blinnphong" ||
                   data.integrator_type == "blinn_phong" ||
                   data.integrator_type == "blinn-phong") {
            // Novo integrador: implementa Blinn-Phong Reflection Model.
            // Aceita as variacoes de nome usadas nos arquivos de cena.
            integrator = std::make_shared<BlinnPhongIntegrator>(data.camera, film);

        } else {
            throw std::runtime_error("Integrador desconhecido: " + data.integrator_type);
        }

        integrator->render(*scene);
    }
}

void API::clean_up() {}
