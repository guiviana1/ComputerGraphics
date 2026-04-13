#include "parserxml.h"
#include "sphere.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cmath>

// ── helpers internos ──────────────────────────────────────────────────────────

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Parser: nao foi possivel abrir o arquivo: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

static RGBColor parseColorString(const std::string& s) {
    std::istringstream ss(s);
    float r, g, b;
    ss >> r >> g >> b;
    return RGBColor(r, g, b);
}

static Point3 parsePoint3(const std::string& s) {
    std::istringstream ss(s);
    float x, y, z;
    ss >> x >> y >> z;
    return Point3(x, y, z);
}

static Vector3 parseVector3(const std::string& s) {
    std::istringstream ss(s);
    float x, y, z;
    ss >> x >> y >> z;
    return Vector3(x, y, z);
}

// ── metodos privados ──────────────────────────────────────────────────────────

std::string Parser::extractAttr(const std::string& text, const std::string& attr) {
    std::string key = attr + "=\"";
    size_t pos = text.find(key);
    if (pos == std::string::npos)
        return "";
    pos += key.size();
    size_t end = text.find('"', pos);
    return text.substr(pos, end - pos);
}

float Parser::extractFloat(const std::string& text, const std::string& attr) {
    std::string val = extractAttr(text, attr);
    if (val.empty())
        throw std::runtime_error("Parser: atributo '" + attr + "' nao encontrado");
    return std::stof(val);
}

int Parser::extractInt(const std::string& text, const std::string& attr) {
    std::string val = extractAttr(text, attr);
    if (val.empty())
        throw std::runtime_error("Parser: atributo '" + attr + "' nao encontrado");
    return std::stoi(val);
}

std::string Parser::extractTagContent(const std::string& xml, const std::string& tag) {
    std::string open  = "<" + tag;
    std::string close = "</" + tag + ">";
    size_t start = xml.find(open);
    if (start == std::string::npos) return "";
    size_t content_start = xml.find('>', start) + 1;
    size_t end = xml.find(close, content_start);
    if (end == std::string::npos) return "";
    return xml.substr(content_start, end - content_start);
}

std::string Parser::extractOpenTag(const std::string& xml, const std::string& tag) {
    std::string open = "<" + tag;
    size_t start = xml.find(open);
    if (start == std::string::npos) return "";
    size_t end = xml.find('>', start);
    return xml.substr(start, end - start + 1);
}

std::vector<std::string> Parser::extractSelfClosingTags(const std::string& xml, const std::string& tag) {
    std::vector<std::string> result;
    std::string open = "<" + tag;
    size_t pos = 0;
    while ((pos = xml.find(open, pos)) != std::string::npos) {
        size_t end = xml.find('>', pos);
        result.push_back(xml.substr(pos, end - pos + 1));
        pos = end + 1;
    }
    return result;
}

// ── parse principal ───────────────────────────────────────────────────────────

/*!
 * Formato XML suportado (dentro de <world_begin> / <world_end>):
 *
 *   <material type="flat" color="0.95 0.05 0.05" />
 *   <object type="sphere" radius="0.4" center="-1 0.5 5" />
 *
 * Logica de "material atual":
 *   O parser mantem um ponteiro para o ultimo material lido.
 *   Cada <object> que aparecer depois usa esse material.
 *   Isso espelha o padrao do XML descrito no README.
 *
 * Por que iterar manualmente pelo XML ao inves de usar uma lib XML?
 *   Manter a dependencia zero do projeto original. O parser artesanal
 *   funciona para o subset simples de XML usado aqui.
 *
 * Limitacao conhecida: o parser nao suporta multiplos materiais
 * intercalados com objetos de forma generica. Para isso, precisariamos
 * de um parser de verdade (ex: TinyXML2). Por ora, o ultimo <material>
 * antes dos <object>s e usado para todos eles — comportamento correto
 * para o XML do README.
 */
SceneData Parser::parse(const std::string& filepath) {
    std::string xml = readFile(filepath);
    SceneData data;

    // --- <film> ---
    std::string filmTag = extractOpenTag(xml, "film");
    if (filmTag.empty())
        throw std::runtime_error("Parser: tag <film> nao encontrada");

    data.film_width    = extractInt(filmTag, "w_res");
    data.film_height   = extractInt(filmTag, "h_res");
    data.film_filename = extractAttr(filmTag, "filename");

    // --- <background> ---
    std::string bgTag = extractOpenTag(xml, "background");
    if (bgTag.empty())
        throw std::runtime_error("Parser: tag <background> nao encontrada");

    std::string bgType = extractAttr(bgTag, "type");

    if (bgType == "single_color") {
        data.background = std::make_shared<BackgroundSingleColor>(
            parseColorString(extractAttr(bgTag, "color")));
    } else if (bgType == "4_colors") {
        std::vector<RGBColor> corners(4);
        corners[0] = parseColorString(extractAttr(bgTag, "bl"));
        corners[1] = parseColorString(extractAttr(bgTag, "tl"));
        corners[2] = parseColorString(extractAttr(bgTag, "tr"));
        corners[3] = parseColorString(extractAttr(bgTag, "br"));
        data.background = std::make_shared<Background4Colors>(corners);
    } else {
        throw std::runtime_error("Parser: tipo de background desconhecido: " + bgType);
    }

    // --- <lookat> ---
    std::string lookatTag = extractOpenTag(xml, "lookat");
    if (lookatTag.empty())
        throw std::runtime_error("Parser: tag <lookat> nao encontrada");

    Point3  look_from = parsePoint3 (extractAttr(lookatTag, "look_from"));
    Point3  look_at   = parsePoint3 (extractAttr(lookatTag, "look_at"));
    Vector3 vup       = parseVector3(extractAttr(lookatTag, "up"));

    // --- <camera> ---
    std::string camTag  = extractOpenTag(xml, "camera");
    if (camTag.empty())
        throw std::runtime_error("Parser: tag <camera> nao encontrada");

    std::string camType = extractAttr(camTag, "type");
    ScreenWindow sw;
    std::string swStr = extractAttr(camTag, "screen_window");

    if (!swStr.empty()) {
        std::istringstream ss(swStr);
        ss >> sw.l >> sw.r >> sw.b >> sw.t;
    } else {
        float aspect = static_cast<float>(data.film_width) / static_cast<float>(data.film_height);
        std::string arStr = extractAttr(camTag, "frame_aspectratio");
        if (!arStr.empty()) aspect = std::stof(arStr);

        std::string fovyStr = extractAttr(camTag, "fovy");
        if (!fovyStr.empty() && camType == "perspective") {
            float fovy_rad = std::stof(fovyStr) * (3.14159265f / 180.0f);
            float half_h   = std::tan(fovy_rad / 2.0f);
            sw.b = -half_h; sw.t =  half_h;
            sw.l = -half_h * aspect; sw.r =  half_h * aspect;
        } else {
            if (aspect >= 1.0f) {
                sw.l = -aspect; sw.r = aspect;
                sw.b = -1.0f;   sw.t = 1.0f;
            } else {
                sw.l = -1.0f;            sw.r = 1.0f;
                sw.b = -1.0f / aspect;   sw.t = 1.0f / aspect;
            }
        }
    }

    if (camType == "orthographic") {
        data.camera = std::make_shared<OrthographicCamera>(
            look_from, look_at, vup, sw, data.film_width, data.film_height);
    } else if (camType == "perspective") {
        float fd = 1.0f;
        std::string fdStr = extractAttr(camTag, "focal_distance");
        if (!fdStr.empty()) fd = std::stof(fdStr);
        data.camera = std::make_shared<PerspectiveCamera>(
            look_from, look_at, vup, sw, data.film_width, data.film_height, fd);
    } else {
        throw std::runtime_error("Parser: tipo de camera desconhecido: " + camType);
    }

    // ── NOVO: <material> e <object> dentro de <world_begin>...</world_end> ──

    // Extraimos o conteudo entre <world_begin/> e <world_end/>
    // Estrategia: buscamos tudo depois de "world_begin" e antes de "world_end"
    std::string worldBeginMarker = "world_begin";
    std::string worldEndMarker   = "world_end";

    size_t wb_pos = xml.find(worldBeginMarker);
    size_t we_pos = xml.find(worldEndMarker);

    // O conteudo do mundo e o trecho entre o fim de <world_begin/> e <world_end/>
    std::string world_content = "";
    if (wb_pos != std::string::npos && we_pos != std::string::npos) {
        size_t after_wb = xml.find('>', wb_pos) + 1;
        world_content = xml.substr(after_wb, we_pos - after_wb);
    }

    // Itera sobre o conteudo do mundo procurando <material> e <object>
    // de forma sequencial para respeitar a ordem do XML.
    //
    // Abordagem: buscamos alternadamente as proximas ocorrencias de
    // "<material" e "<object", e processamos a que aparecer primeiro.
    std::shared_ptr<Material> current_material = nullptr;

    size_t pos = 0;
    while (pos < world_content.size()) {
        size_t mat_pos = world_content.find("<material", pos);
        size_t obj_pos = world_content.find("<object",   pos);

        // Nenhum dos dois encontrado: acabou
        if (mat_pos == std::string::npos && obj_pos == std::string::npos)
            break;

        // Qual vem primeiro?
        bool mat_first = (mat_pos != std::string::npos) &&
                         (obj_pos == std::string::npos || mat_pos < obj_pos);

        if (mat_first) {
            // Extrai a tag completa do material
            size_t end = world_content.find('>', mat_pos);
            std::string matTag = world_content.substr(mat_pos, end - mat_pos + 1);
            pos = end + 1;

            std::string matType = extractAttr(matTag, "type");

            if (matType == "flat") {
                // cor em [0,1]: "0.95 0.05 0.05"
                RGBColor c = parseColorString(extractAttr(matTag, "color"));
                current_material = std::make_shared<FlatMaterial>(c);
            }
            // outros tipos de material serao adicionados aqui no futuro

        } else {
            // Processa <object>
            size_t end = world_content.find('>', obj_pos);
            std::string objTag = world_content.substr(obj_pos, end - obj_pos + 1);
            pos = end + 1;

            if (!current_material) {
                // Objeto sem material definido: usa branco por padrao
                current_material = std::make_shared<FlatMaterial>(RGBColor(1.0f, 1.0f, 1.0f));
            }

            std::string objType = extractAttr(objTag, "type");

            if (objType == "sphere") {
                float  radius = extractFloat(objTag, "radius");
                Point3 center = parsePoint3(extractAttr(objTag, "center"));

                data.primitives.push_back(
                    std::make_shared<Sphere>(center, radius, current_material));
            }
            // outros tipos de objeto (triangulo, plano...) serao adicionados aqui
        }
    }

    return data;
}