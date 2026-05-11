#pragma once

#include <string>
#include <memory>
#include <vector>
#include "backgroundd.h"
#include "camera.h"
#include "primitive.h"
#include "material.h"

struct SceneData {
    // Render settings (can change between render_again calls)
    std::shared_ptr<Camera> camera;
    std::string integrator_type = "flat";
    int film_width    = 800;
    int film_height   = 600;
    std::string film_filename = "output.ppm";

    // World (shared across render_again)
    std::shared_ptr<Background> background;
    std::vector<std::shared_ptr<Primitive>> primitives;
};

class Parser {
public:
    static std::vector<SceneData> parse(const std::string& filepath);

private:
    static std::string extractAttr(const std::string& text, const std::string& attr);
    static int         extractIntOpt(const std::string& text, const std::string& attr, int fallback);
    static std::string tagName(const std::string& tag);
    static std::string processIncludes(const std::string& xml, const std::string& basedir);

    static RGBColor    parseColor(const std::string& s);
    static Point3      parsePoint3(const std::string& s);
    static Vector3     parseVector3(const std::string& s);

    static std::shared_ptr<Camera> buildCamera(
        const Point3& look_from, const Point3& look_at, const Vector3& vup,
        const std::string& cam_tag, const std::string& cam_type,
        int film_w, int film_h);
};
