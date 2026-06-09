#include "api.h"
#include "parserxml.h"
#include "integrator.h"
#include "film.h"
#include "scene.h"
#include "bvh.h"
#include "primlist.h"
#include <stdexcept>
#include <memory>
#include <chrono>
#include <iostream>

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

        // Constroi o aggregate (BVH ou lista plana) e monta a cena
        std::shared_ptr<Primitive> aggregate;
        if (data.aggregator_type == "bvh" && !data.primitives.empty()) {
            auto t0 = std::chrono::steady_clock::now();
            aggregate = std::make_shared<BVHAccel>(
                data.primitives, data.bvh_split_method, data.bvh_max_prims);
            auto t1 = std::chrono::steady_clock::now();
            double ms = std::chrono::duration<double,std::milli>(t1-t0).count();
            std::cout << "[BVH] construido em " << ms << " ms  ("
                      << data.primitives.size() << " primitivas, split="
                      << data.bvh_split_method << ")\n";
        } else {
            aggregate = std::make_shared<PrimList>(data.primitives);
        }

        auto scene = std::make_shared<Scene>();
        scene->aggregate     = aggregate;
        scene->background    = data.background;
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
            integrator = std::make_shared<BlinnPhongIntegrator>(
                data.camera, film, data.integrator_depth);

        } else {
            throw std::runtime_error("Integrador desconhecido: " + data.integrator_type);
        }

        integrator->render(*scene);
    }
}

void API::clean_up() {}
