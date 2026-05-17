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

    virtual std::optional<RGBColor> Li(const Ray& ray, const Scene& scene) const = 0;

    void render(const Scene& scene) override;
};



class RayCastIntegrator : public SamplerIntegrator {
public:
    RayCastIntegrator(std::shared_ptr<Camera> cam, std::shared_ptr<Film> f)
        : SamplerIntegrator(std::move(cam), std::move(f)) {}

    std::optional<RGBColor> Li(const Ray& ray, const Scene& scene) const override;
};



class BlinnPhongIntegrator : public SamplerIntegrator {
public:
    BlinnPhongIntegrator(std::shared_ptr<Camera> cam, std::shared_ptr<Film> f)
        : SamplerIntegrator(std::move(cam), std::move(f)) {}

    std::optional<RGBColor> Li(const Ray& ray, const Scene& scene) const override;
};