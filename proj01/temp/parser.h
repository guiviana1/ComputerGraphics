#pragma once

#include <string>
#include <memory>
#include <vector>
#include "../src/backgroundd.h"

struct SceneData {
    std::shared_ptr<Background> background;
    int film_width;
    int film_height;
    std::string film_filename;
};

class Parser {
public:
    // Lê o arquivo XML e retorna os dados da cena prontos para uso
    static SceneData parse(const std::string& filepath);

private:
    static std::string extractAttr(const std::string& text, const std::string& attr);
    static float       extractFloat(const std::string& text, const std::string& attr);
    static int         extractInt(const std::string& text, const std::string& attr);

    // Retorna o conteúdo entre <tag ...> e </tag>
    static std::string extractTagContent(const std::string& xml, const std::string& tag);

    // Retorna a string completa da tag de abertura <tag ...>
    static std::string extractOpenTag(const std::string& xml, const std::string& tag);

    // Retorna todas as ocorrências de <tag .../> (self-closing)
    static std::vector<std::string> extractSelfClosingTags(const std::string& xml, const std::string& tag);
};
