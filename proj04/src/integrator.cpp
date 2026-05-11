#include "integrator.h"
#include "material.h"
#include "surfel.h"

void SamplerIntegrator::render(const Scene& scene) {
    int w = film->getWidth();
    int h = film->getHeight();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            Ray ray = camera->generate_ray(i, j);

            float u = float(i) / float(w - 1);
            float v = 1.0f - float(j) / float(h - 1);

            auto result = Li(ray, scene);
            RGBColor color = result.has_value()
                ? result.value()
                : scene.background->sampleUV(u, v);

            film->add(i, j, color);
        }
    }

    film->write_image();
}

std::optional<RGBColor> RayCastIntegrator::Li(const Ray& ray, const Scene& scene) const {
    Ray r = ray;
    Surfel sf;
    if (!scene.intersect(r, &sf))
        return std::nullopt;

    const FlatMaterial* fm = dynamic_cast<const FlatMaterial*>(sf.primitive->get_material());
    if (!fm) return std::nullopt;

    return fm->kd();
}
