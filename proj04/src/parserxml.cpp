#include "parserxml.h"
#include "sphere.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <map>

// ── Utilitários de arquivo ────────────────────────────────────────────────────

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Parser: nao foi possivel abrir: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static std::string dirOf(const std::string& path) {
    size_t last = path.find_last_of("/\\");
    return (last == std::string::npos) ? "." : path.substr(0, last);
}

// ── Helpers de extração ───────────────────────────────────────────────────────

std::string Parser::extractAttr(const std::string& text, const std::string& attr) {
    std::string key = attr + "=\"";
    size_t pos = text.find(key);
    if (pos == std::string::npos) return "";
    pos += key.size();
    size_t end = text.find('"', pos);
    return text.substr(pos, end - pos);
}

int Parser::extractIntOpt(const std::string& text, const std::string& attr, int fallback) {
    std::string val = extractAttr(text, attr);
    return val.empty() ? fallback : std::stoi(val);
}

std::string Parser::tagName(const std::string& tag) {
    size_t start = 1;
    while (start < tag.size() && tag[start] == '/') ++start;
    size_t end = start;
    while (end < tag.size() && tag[end] != ' ' && tag[end] != '>' && tag[end] != '/') ++end;
    return tag.substr(start, end - start);
}

// ── Helpers de parse ──────────────────────────────────────────────────────────

RGBColor Parser::parseColor(const std::string& s) {
    std::istringstream ss(s);
    float r, g, b;
    ss >> r >> g >> b;
    return RGBColor(r, g, b);
}

Point3 Parser::parsePoint3(const std::string& s) {
    std::istringstream ss(s);
    float x, y, z;
    ss >> x >> y >> z;
    return Point3(x, y, z);
}

Vector3 Parser::parseVector3(const std::string& s) {
    std::istringstream ss(s);
    float x, y, z;
    ss >> x >> y >> z;
    return Vector3(x, y, z);
}

// ── Include ───────────────────────────────────────────────────────────────────

std::string Parser::processIncludes(const std::string& xml, const std::string& basedir) {
    std::string result = xml;
    size_t pos = 0;
    while ((pos = result.find("<include", pos)) != std::string::npos) {
        size_t end = result.find('>', pos);
        std::string inc_tag = result.substr(pos, end - pos + 1);
        std::string filename = extractAttr(inc_tag, "filename");
        std::string filepath = basedir + "/" + filename;
        std::string included = readFile(filepath);
        result = result.substr(0, pos) + included + result.substr(end + 1);
        // não avança pos: pode ter includes aninhados
    }
    return result;
}

// ── Construção de câmera ──────────────────────────────────────────────────────

std::shared_ptr<Camera> Parser::buildCamera(
    const Point3& look_from, const Point3& look_at, const Vector3& vup,
    const std::string& cam_tag, const std::string& cam_type,
    int film_w, int film_h)
{
    ScreenWindow sw;
    std::string swStr = extractAttr(cam_tag, "screen_window");

    if (!swStr.empty()) {
        std::istringstream ss(swStr);
        ss >> sw.l >> sw.r >> sw.b >> sw.t;
    } else {
        float aspect = float(film_w) / float(film_h);
        std::string fovyStr = extractAttr(cam_tag, "fovy");
        if (!fovyStr.empty() && cam_type == "perspective") {
            float fovy_rad = std::stof(fovyStr) * (3.14159265f / 180.0f);
            float half_h   = std::tan(fovy_rad / 2.0f);
            sw.b = -half_h;         sw.t = half_h;
            sw.l = -half_h * aspect; sw.r = half_h * aspect;
        } else {
            if (aspect >= 1.0f) {
                sw.l = -aspect; sw.r = aspect;
                sw.b = -1.0f;   sw.t = 1.0f;
            } else {
                sw.l = -1.0f;          sw.r = 1.0f;
                sw.b = -1.0f / aspect; sw.t = 1.0f / aspect;
            }
        }
    }

    if (cam_type == "orthographic") {
        return std::make_shared<OrthographicCamera>(
            look_from, look_at, vup, sw, film_w, film_h);
    } else {
        float fd = 1.0f;
        std::string fdStr = extractAttr(cam_tag, "focal_distance");
        if (!fdStr.empty()) fd = std::stof(fdStr);
        return std::make_shared<PerspectiveCamera>(
            look_from, look_at, vup, sw, film_w, film_h, fd);
    }
}

// ── Parse principal ───────────────────────────────────────────────────────────

std::vector<SceneData> Parser::parse(const std::string& filepath) {
    std::string xml = processIncludes(readFile(filepath), dirOf(filepath));

    std::vector<SceneData> jobs;

    // Estado atual de render (pode mudar entre render_again)
    Point3  look_from{0,0,0}, look_at{0,0,1};
    Vector3 vup{0,1,0};
    std::string cam_type  = "perspective";
    std::string cam_tag;
    std::string integrator_type = "flat";
    int    film_w = 800, film_h = 600;
    std::string film_filename = "output.ppm";

    // Estado do mundo (preservado para render_again)
    std::shared_ptr<Background> background;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::shared_ptr<Material> current_material;
    std::map<std::string, std::shared_ptr<Material>> named_materials;
    bool has_world = false;
    bool in_world  = false;

    size_t pos = 0;
    while (pos < xml.size()) {
        // Encontra próxima '<'
        size_t tag_start = xml.find('<', pos);
        if (tag_start == std::string::npos) break;

        // Comentário XML
        if (xml.substr(tag_start, 4) == "<!--") {
            size_t comment_end = xml.find("-->", tag_start + 4);
            pos = (comment_end != std::string::npos) ? comment_end + 3 : xml.size();
            continue;
        }

        size_t tag_end = xml.find('>', tag_start);
        if (tag_end == std::string::npos) break;

        std::string tag = xml.substr(tag_start, tag_end - tag_start + 1);
        pos = tag_end + 1;

        // Ignora tags de fechamento e declarações XML
        if (tag.size() < 2 || tag[1] == '/' || tag[1] == '?') continue;

        std::string name = tagName(tag);

        // ── Tags globais (fora do mundo) ──────────────────────────────────────
        if (name == "lookat") {
            look_from = parsePoint3 (extractAttr(tag, "look_from"));
            look_at   = parsePoint3 (extractAttr(tag, "look_at"));
            vup       = parseVector3(extractAttr(tag, "up"));
        }
        else if (name == "camera") {
            cam_type = extractAttr(tag, "type");
            cam_tag  = tag;
        }
        else if (name == "film") {
            film_w        = extractIntOpt(tag, "x_res", extractIntOpt(tag, "w_res", 800));
            film_h        = extractIntOpt(tag, "y_res", extractIntOpt(tag, "h_res", 600));
            film_filename = extractAttr(tag, "filename");
        }
        else if (name == "integrator") {
            integrator_type = extractAttr(tag, "type");
        }

        // ── Início do mundo ───────────────────────────────────────────────────
        else if (name == "world_begin") {
            in_world = true;
            primitives.clear();
            background.reset();
            current_material.reset();
        }

        // ── Tags dentro do mundo ──────────────────────────────────────────────
        else if (in_world) {

            if (name == "background") {
                std::string btype = extractAttr(tag, "type");
                // aceita "colors" ou "4_colors" para bilinear; "single_color" para sólido
                if (btype == "colors" || btype == "4_colors") {
                    std::vector<RGBColor> corners(4);
                    corners[0] = parseColor(extractAttr(tag, "bl"));
                    corners[1] = parseColor(extractAttr(tag, "tl"));
                    corners[2] = parseColor(extractAttr(tag, "tr"));
                    corners[3] = parseColor(extractAttr(tag, "br"));
                    background = std::make_shared<Background4Colors>(corners);
                } else if (btype == "single_color") {
                    background = std::make_shared<BackgroundSingleColor>(
                        parseColor(extractAttr(tag, "color")));
                } else {
                    throw std::runtime_error("Parser: tipo de background desconhecido: " + btype);
                }
            }

            else if (name == "material") {
                std::string mtype = extractAttr(tag, "type");
                if (mtype == "flat") {
                    current_material = std::make_shared<FlatMaterial>(
                        parseColor(extractAttr(tag, "color")));
                }
            }

            else if (name == "make_named_material") {
                std::string mname = extractAttr(tag, "name");
                std::string mtype = extractAttr(tag, "type");
                if (mtype == "flat") {
                    named_materials[mname] = std::make_shared<FlatMaterial>(
                        parseColor(extractAttr(tag, "color")));
                }
            }

            else if (name == "named_material") {
                std::string mname = extractAttr(tag, "name");
                auto it = named_materials.find(mname);
                if (it != named_materials.end())
                    current_material = it->second;
                else
                    throw std::runtime_error("Parser: material nomeado nao encontrado: " + mname);
            }

            else if (name == "object") {
                if (!current_material)
                    current_material = std::make_shared<FlatMaterial>(RGBColor(255, 255, 255));

                // verifica se o objeto declara material="nome" inline
                std::string mat_attr = extractAttr(tag, "material");
                std::shared_ptr<Material> obj_material = current_material;
                if (!mat_attr.empty()) {
                    auto it = named_materials.find(mat_attr);
                    if (it != named_materials.end())
                        obj_material = it->second;
                }

                std::string otype = extractAttr(tag, "type");
                if (otype == "sphere") {
                    float  radius = std::stof(extractAttr(tag, "radius"));
                    Point3 center = parsePoint3(extractAttr(tag, "center"));
                    primitives.push_back(
                        std::make_shared<Sphere>(center, radius, obj_material));
                }
            }

            else if (name == "world_end") {
                in_world  = false;
                has_world = true;

                SceneData data;
                data.camera          = buildCamera(look_from, look_at, vup, cam_tag, cam_type, film_w, film_h);
                data.integrator_type = integrator_type;
                data.film_width      = film_w;
                data.film_height     = film_h;
                data.film_filename   = film_filename;
                data.background      = background;
                data.primitives      = primitives;
                jobs.push_back(std::move(data));
            }
        }

        // ── Render again ──────────────────────────────────────────────────────
        else if (name == "render_again" && has_world) {
            SceneData data;
            data.camera          = buildCamera(look_from, look_at, vup, cam_tag, cam_type, film_w, film_h);
            data.integrator_type = integrator_type;
            data.film_width      = film_w;
            data.film_height     = film_h;
            data.film_filename   = film_filename;
            data.background      = background;
            data.primitives      = primitives;
            jobs.push_back(std::move(data));
        }
    }

    if (jobs.empty())
        throw std::runtime_error("Parser: nenhum bloco world_begin/world_end encontrado");

    return jobs;
}
