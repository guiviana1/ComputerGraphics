#pragma once

#include <string>
#include <memory>
#include <vector>
#include "backgroundd.h"
#include "camera.h"
#include "primitive.h"
#include "material.h"


struct SceneData {
    std::shared_ptr<Background> background;
    std::shared_ptr<Camera>     camera;
    std::vector<std::shared_ptr<Primitive>> primitives; 
    int film_width;
    int film_height;
    std::string film_filename;
};

class Parser {
public:
    static SceneData parse(const std::string& filepath);

private:
    static std::string extractAttr(const std::string& text, const std::string& attr);
    static float       extractFloat(const std::string& text, const std::string& attr);
    static int         extractInt(const std::string& text, const std::string& attr);
    static std::string extractTagContent(const std::string& xml, const std::string& tag);
    static std::string extractOpenTag(const std::string& xml, const std::string& tag);
    static std::vector<std::string> extractSelfClosingTags(const std::string& xml, const std::string& tag);
};