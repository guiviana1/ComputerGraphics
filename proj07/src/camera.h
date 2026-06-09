#pragma once

#include "ray.h"
#include "vec3.h"

struct ScreenWindow {
    real_type l, r, b, t;
};

class Camera {
protected:
    Point3  e;     // origem (look_from)
    Vector3 u_hat; // direita
    Vector3 v_hat; // cima
    Vector3 w_hat; // frente (gaze normalizado)

    // dimensões da imagem
    int nx, ny;

    // limites do screen space
    ScreenWindow sw;

    void build_frame(const Point3& look_from, const Point3& look_at, const Vector3& vup);

public:
    Camera(const Point3& look_from, const Point3& look_at, const Vector3& vup,
           const ScreenWindow& sw, int nx, int ny);

    virtual Ray generate_ray(int i, int j) const = 0;
    virtual ~Camera() = default;
};

class OrthographicCamera : public Camera {
public:
    OrthographicCamera(const Point3& look_from, const Point3& look_at, const Vector3& vup,
                       const ScreenWindow& sw, int nx, int ny);

    Ray generate_ray(int i, int j) const override;
};

class PerspectiveCamera : public Camera {
private:
    real_type fd; // focal distance
public:
    PerspectiveCamera(const Point3& look_from, const Point3& look_at, const Vector3& vup,
                      const ScreenWindow& sw, int nx, int ny, real_type focal_distance = 1.0f);

    Ray generate_ray(int i, int j) const override;
};
