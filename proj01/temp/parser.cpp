#include "parser.h"

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

// ── métodos privados ──────────────────────────────────────────────────────────

// Extrai o valor de um atributo dentro de uma string de tag
// ex: extractAttr("<film xres=\"400\" yres=\"200\"/>", "xres") → "400"
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

// Retorna a string da tag de abertura: "<tag ...>" ou "<tag .../>",
// sem o conteúdo interno
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

SceneData Parser::parse(const std::string& filepath) {
    std::string xml = readFile(filepath);
    SceneData data;

    // --- <film> ---
    std::string filmTag = extractOpenTag(xml, "film");
    if (filmTag.empty())
        throw std::runtime_error("Parser: tag <film> nao encontrada");

    data.film_width    = extractInt(filmTag, "xres");
    data.film_height   = extractInt(filmTag, "yres");
    data.film_filename = extractAttr(filmTag, "filename");

    // --- <background> ---
    std::string bgOpenTag = extractOpenTag(xml, "background");
    if (bgOpenTag.empty())
        throw std::runtime_error("Parser: tag <background> nao encontrada");

    std::string bgType = extractAttr(bgOpenTag, "type");

    if (bgType == "single") {
        std::string bgContent = extractTagContent(xml, "background");
        auto colorTags = extractSelfClosingTags(bgContent, "color");
        if (colorTags.empty())
            throw std::runtime_error("Parser: background 'single' precisa de uma tag <color/>");

        RGBColor c;
        c.r = extractFloat(colorTags[0], "r");
        c.g = extractFloat(colorTags[0], "g");
        c.b = extractFloat(colorTags[0], "b");
        data.background = std::make_shared<BackgroundSingleColor>(c);

    } else if (bgType == "4colors") {
        std::string bgContent = extractTagContent(xml, "background");
        auto colorTags = extractSelfClosingTags(bgContent, "color");
        if (colorTags.size() != 4)
            throw std::runtime_error("Parser: background '4colors' precisa de exatamente 4 tags <color/>");

        // mapeia corner → índice esperado pelo Background4Colors (BL, TL, TR, BR)
        std::map<std::string, int> cornerIdx = {{"BL",0}, {"TL",1}, {"TR",2}, {"BR",3}};
        std::vector<RGBColor> corners(4);

        for (auto& ct : colorTags) {
            std::string corner = extractAttr(ct, "corner");
            if (cornerIdx.find(corner) == cornerIdx.end())
                throw std::runtime_error("Parser: corner invalido: " + corner);

            int idx = cornerIdx.at(corner);
            corners[idx].r = extractFloat(ct, "r");
            corners[idx].g = extractFloat(ct, "g");
            corners[idx].b = extractFloat(ct, "b");
        }
        data.background = std::make_shared<Background4Colors>(corners);

    } else {
        throw std::runtime_error("Parser: tipo de background desconhecido: " + bgType);
    }

    return data;
}
