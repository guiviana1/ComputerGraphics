#include "camera.h"

// ===================== Camera base =====================

void Camera::build_frame(const Point3& look_from, const Point3& look_at, const Vector3& vup) {
    e     = look_from;
    w_hat = normalize(look_at - look_from); // left-hand: gaze é +w
    u_hat = normalize(cross(vup, w_hat));   // direita
    v_hat = normalize(cross(w_hat, u_hat)); // cima
}

Camera::Camera(const Point3& look_from, const Point3& look_at, const Vector3& vup,
               const ScreenWindow& sw, int nx, int ny)
    : sw(sw), nx(nx), ny(ny)
{
    build_frame(look_from, look_at, vup);
}

// ===================== Orthographic =====================

OrthographicCamera::OrthographicCamera(const Point3& look_from, const Point3& look_at,
                                       const Vector3& vup, const ScreenWindow& sw,
                                       int nx, int ny)
    : Camera(look_from, look_at, vup, sw, nx, ny) {}

Ray OrthographicCamera::generate_ray(int i, int j) const {
    // mapeia pixel -> screen space
    // j=0 é o topo da imagem (PPM), mas v cresce para cima no screen space,
    // então invertemos j para que j=0 → sw.t e j=ny-1 → sw.b.
    real_type u = sw.l + (sw.r - sw.l) * (i + 0.5f) / nx;
    real_type v = sw.t + (sw.b - sw.t) * (j + 0.5f) / ny;

    Point3  origin    = e + u_hat * u + v_hat * v;
    Vector3 direction = w_hat;

    return Ray(origin, direction);
}

// ===================== Perspective =====================

PerspectiveCamera::PerspectiveCamera(const Point3& look_from, const Point3& look_at,
                                     const Vector3& vup, const ScreenWindow& sw,
                                     int nx, int ny, real_type focal_distance)
    : Camera(look_from, look_at, vup, sw, nx, ny), fd(focal_distance) {}

Ray PerspectiveCamera::generate_ray(int i, int j) const {
    // mapeia pixel -> screen space (j invertido: j=0 é o topo da imagem)
    real_type u = sw.l + (sw.r - sw.l) * (i + 0.5f) / nx;
    real_type v = sw.t + (sw.b - sw.t) * (j + 0.5f) / ny;

    Vector3 direction = w_hat * fd + u_hat * u + v_hat * v;

    return Ray(e, direction);
}
