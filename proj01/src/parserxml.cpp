#include "parserxml.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <map>

// ── helpers internos ──────────────────────────────────────────────────────────

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open())
        throw std::runtime_error("Parser: nao foi possivel abrir o arquivo: " + path);
    std::ostringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

// Converte "153 204 255" → RGBColor
static RGBColor parseColorString(const std::string& s) {
    std::istringstream ss(s);
    float r, g, b;
    ss >> r >> g >> b;
    return RGBColor(r, g, b);
}

// ── métodos privados ──────────────────────────────────────────────────────────

// Extrai o valor de um atributo
// ex: extractAttr("<film w_res=\"400\"/>", "w_res") → "400"
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

// Retorna o conteúdo entre <tag ...> e </tag>
std::string Parser::extractTagContent(const std::string& xml, const std::string& tag) {
    std::string open  = "<" + tag;
    std::string close = "</" + tag + ">";

    size_t start = xml.find(open);
    if (start == std::string::npos)
        return "";

    size_t content_start = xml.find('>', start) + 1;
    size_t end = xml.find(close, content_start);
    if (end == std::string::npos)
        return "";

    return xml.substr(content_start, end - content_start);
}

// Retorna a string da tag de abertura <tag ...> ou <tag .../>
std::string Parser::extractOpenTag(const std::string& xml, const std::string& tag) {
    std::string open = "<" + tag;
    size_t start = xml.find(open);
    if (start == std::string::npos)
        return "";
    size_t end = xml.find('>', start);
    return xml.substr(start, end - start + 1);
}

// Retorna todas as ocorrências de tags self-closing <tag ... />
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

// Formato esperado (a.xml):
//
// <RT3>
//     <film type="image" w_res="200" h_res="100" filename="out.ppm" img_type="ppm"/>
//     <world_begin/>
//         <background type="single_color" color="153 204 255"/>
//         <!-- ou -->
//         <background type="4colors" bl="0 0 51" tl="0 255 51" tr="255 255 51" br="255 0 51"/>
//     <world_end/>
// </RT3>

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
        std::string colorStr = extractAttr(bgTag, "color");
        if (colorStr.empty())
            throw std::runtime_error("Parser: atributo 'color' ausente no background single_color");

        data.background = std::make_shared<BackgroundSingleColor>(parseColorString(colorStr));

    } else if (bgType == "4_colors") {
        // cada corner é um atributo inline: bl="r g b" tl="r g b" tr="r g b" br="r g b"
        std::vector<RGBColor> corners(4);
        corners[0] = parseColorString(extractAttr(bgTag, "bl")); // BL
        corners[1] = parseColorString(extractAttr(bgTag, "tl")); // TL
        corners[2] = parseColorString(extractAttr(bgTag, "tr")); // TR
        corners[3] = parseColorString(extractAttr(bgTag, "br")); // BR

        data.background = std::make_shared<Background4Colors>(corners);

    } else {
        throw std::runtime_error("Parser: tipo de background desconhecido: " + bgType);
    }

    return data;
}
