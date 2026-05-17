#include "integrator.h"
#include "material.h"
#include "surfel.h"
#include "light.h"
#include <cmath>
#include <algorithm>

//multiplica duas cores componente a componente
static RGBColor colorMul(const RGBColor& a, const RGBColor& b) {
    return RGBColor(a.r * b.r, a.g * b.g, a.b * b.b);
}

//somar duas cores
static RGBColor colorAdd(const RGBColor& a, const RGBColor& b) {
    return RGBColor(a.r + b.r, a.g + b.g, a.b + b.b);
}

//escalar uma cor por um float
static RGBColor colorScale(const RGBColor& c, float s) {
    return RGBColor(c.r * s, c.g * s, c.b * s);
}

// clamp* de cor para [0, 255]
static RGBColor colorClamp255(const RGBColor& c) {
    auto cl = [](float v) { return std::max(0.0f, std::min(255.0f, v)); };
    return RGBColor(cl(c.r), cl(c.g), cl(c.b));
}

void SamplerIntegrator::render(const Scene& scene) {
    int w = film->getWidth();
    int h = film->getHeight();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            Ray ray = camera->generate_ray(i, j);

            // UV normalizado para o background (v invertido: j=0 é o topo)
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
//*
std::optional<RGBColor> BlinnPhongIntegrator::Li(const Ray& ray, const Scene& scene) const {

    Ray r = ray;
    Surfel sf;
    if (!scene.intersect(r, &sf))
        return std::nullopt; // nada atingido → usa background

    if (dot(sf.n, sf.wo) < 0.0f)
        return std::nullopt;

    const BlinnPhongMaterial* mat =
        dynamic_cast<const BlinnPhongMaterial*>(sf.primitive->get_material());

    if (!mat) {
        const FlatMaterial* fm =
            dynamic_cast<const FlatMaterial*>(sf.primitive->get_material());
        if (fm) return fm->kd();
        return std::nullopt;
    }

    Vector3 N = normalize(sf.n);
    Vector3 V = normalize(sf.wo); // wo já é -ray.d, basta normalizar

    RGBColor ka = mat->ka();
    RGBColor kd = mat->kd();
    RGBColor ks = mat->ks();
    float    g  = mat->glossiness();

    RGBColor L(0.0f, 0.0f, 0.0f);

    for (const auto& light : scene.lights) {

        LightSample ls = light->sample_Li(sf.p);

        Vector3  wi = ls.wi;    // l̂: direção normalizada PARA a luz
        RGBColor Ii = ls.intensity;     // intensidade efetiva (já com scale/att)

        float n_dot_l = std::max(0.0f, dot(N, wi));
        RGBColor diffuse = colorMul(Ii, colorScale(kd, n_dot_l));

        Vector3  H        = normalize(V + wi);
        float    n_dot_h  = std::max(0.0f, dot(N, H));
        float    spec_val = std::pow(n_dot_h, g);
        RGBColor specular = colorMul(Ii, colorScale(ks, spec_val));

        L = colorAdd(L, colorAdd(diffuse, specular));
    }

    if (scene.ambient_light) {
        RGBColor Ia     = scene.ambient_light->effective_I;
        RGBColor ambient = colorMul(Ia, ka);
        L = colorAdd(L, ambient);
    }

    L = colorClamp255(colorScale(L, 255.0f));

    return L;
}