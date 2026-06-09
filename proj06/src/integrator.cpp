#include "integrator.h"
#include "material.h"
#include "surfel.h"
#include "light.h"
#include <cmath>
#include <algorithm>
#include <limits>

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

// mapeia uma direcao 3D para coordenadas (u,v) equiretangulares do background
static void dirToUV(const Vector3& d, float& u, float& v) {
    Vector3 n = normalize(d);
    const float PI = 3.14159265358979f;
    u = 0.5f + std::atan2(n.x, n.z) / (2.0f * PI);
    v = 0.5f - std::asin(std::max(-1.0f, std::min(1.0f, n.y))) / PI;
}

// epsilon para afastar raios secundarios da superficie e evitar auto-intersecao
static const float RAY_EPS = 1e-3f;

void SamplerIntegrator::render(const Scene& scene) {
    int w = film->getWidth();
    int h = film->getHeight();

    for (int j = 0; j < h; j++) {
        for (int i = 0; i < w; i++) {
            Ray ray = camera->generate_ray(i, j);

            // UV normalizado para o background (v invertido: j=0 é o topo)
            float u = float(i) / float(w - 1);
            float v = 1.0f - float(j) / float(h - 1);

            auto result = Li(ray, scene, 0);
            RGBColor color = result.has_value()
                ? result.value()
                : scene.background->sampleUV(u, v);

            film->add(i, j, color);
        }
    }

    film->write_image();
}


std::optional<RGBColor> RayCastIntegrator::Li(const Ray& ray, const Scene& scene,
                                              int /*depth*/) const {
    Ray r = ray;
    Surfel sf;
    if (!scene.intersect(r, &sf))
        return std::nullopt;

    const FlatMaterial* fm = dynamic_cast<const FlatMaterial*>(sf.primitive->get_material());
    if (!fm) return std::nullopt;

    return fm->kd();
}

std::optional<RGBColor> BlinnPhongIntegrator::Li(const Ray& ray, const Scene& scene,
                                                 int depth) const {

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

        // sombra: se o raio ate a luz estiver bloqueado, esta luz nao contribui
        VisibilityTester vis(sf.p, N, wi, ls.dist);
        if (!vis.unoccluded(scene))
            continue;

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

    // Cor local do Blinn-Phong, ja no espaco de exibicao [0,255].
    RGBColor L255 = colorClamp255(colorScale(L, 255.0f));

    // reflexao espelho: segue o raio refletido e soma km * cor_refletida
    if (mat->is_mirror() && depth < max_depth) {
        Vector3 d    = normalize(r.d);            // direcao incidente
        Vector3 refl = reflect(d, N);             // reflexao perfeita sobre N

        // offset por epsilon para nao re-acertar a propria superficie
        Ray reflected(sf.p + refl * RAY_EPS, refl, RAY_EPS,
                      std::numeric_limits<float>::infinity());

        auto rc = Li(reflected, scene, depth + 1);

        RGBColor reflColor;
        if (rc.has_value()) {
            reflColor = rc.value();              // ja em [0,255]
        } else {
            // raio refletido nao acertou nada: amostra o background
            float u, v; dirToUV(refl, u, v);
            reflColor = scene.background->sampleUV(u, v);
        }

        RGBColor km = mat->mirror();             // coeficiente em [0,1]
        L255 = colorClamp255(colorAdd(L255, colorMul(km, reflColor)));
    }

    return L255;
}