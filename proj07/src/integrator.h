#pragma once

#include "scene.h"
#include "film.h"
#include "camera.h"
#include "ray.h"
#include "backgroundd.h"
#include <memory>
#include <optional>

class Integrator {
public:
    virtual ~Integrator() = default;
    virtual void render(const Scene& scene) = 0;
};



class SamplerIntegrator : public Integrator {
protected:
    std::shared_ptr<Camera> camera;
    std::shared_ptr<Film>   film;

public:
    SamplerIntegrator(std::shared_ptr<Camera> cam, std::shared_ptr<Film> f)
        : camera(std::move(cam)), film(std::move(f)) {}

    virtual ~SamplerIntegrator() = default;

    // depth: nivel de recursao atual (usado pela reflexao espelho — proj06)
    virtual std::optional<RGBColor> Li(const Ray& ray, const Scene& scene,
                                       int depth = 0) const = 0;

    void render(const Scene& scene) override;
};



class RayCastIntegrator : public SamplerIntegrator {
public:
    RayCastIntegrator(std::shared_ptr<Camera> cam, std::shared_ptr<Film> f)
        : SamplerIntegrator(std::move(cam), std::move(f)) {}

    std::optional<RGBColor> Li(const Ray& ray, const Scene& scene,
                               int depth = 0) const override;
};



class BlinnPhongIntegrator : public SamplerIntegrator {
private:
    int max_depth;   //!< niveis maximos de recursao para a reflexao espelho

public:
    BlinnPhongIntegrator(std::shared_ptr<Camera> cam, std::shared_ptr<Film> f,
                         int max_depth = 1)
        : SamplerIntegrator(std::move(cam), std::move(f)), max_depth(max_depth) {}

    std::optional<RGBColor> Li(const Ray& ray, const Scene& scene,
                               int depth = 0) const override;
};