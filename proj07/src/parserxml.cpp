#include "parserxml.h"
#include "sphere.h"
#include "triangle.h"
#include "plane.h"

#include <fstream>
#include <vector>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <cctype>
#include <algorithm>
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
    // Procura o atributo tolerando espacos ao redor do '=' (ex.: glossiness ="256")
    // e exigindo uma fronteira de palavra antes do nome (evita casar substrings,
    // p.ex. "from" dentro de "look_from").
    auto is_ws = [](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; };

    size_t pos = 0;
    while ((pos = text.find(attr, pos)) != std::string::npos) {
        bool boundary_ok = (pos == 0) || is_ws(text[pos - 1]);

        size_t q = pos + attr.size();
        while (q < text.size() && is_ws(text[q])) ++q;          // espacos antes do '='

        if (boundary_ok && q < text.size() && text[q] == '=') {
            ++q;
            while (q < text.size() && is_ws(text[q])) ++q;       // espacos depois do '='
            if (q < text.size() && text[q] == '"') {
                ++q;
                size_t end = text.find('"', q);
                return text.substr(q, end - q);
            }
        }
        pos += attr.size();
    }
    return "";
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

// Le uma sequencia de floats separados por espaco (ex.: "0 1 0  1 0 0").
static std::vector<float> parseFloatArray(const std::string& s) {
    std::istringstream ss(s);
    std::vector<float> out;
    float x;
    while (ss >> x) out.push_back(x);
    return out;
}

// Le uma sequencia de inteiros separados por espaco (ex.: "0 1 2  0 2 3").
static std::vector<int> parseIntArray(const std::string& s) {
    std::istringstream ss(s);
    std::vector<int> out;
    int x;
    while (ss >> x) out.push_back(x);
    return out;
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
            sw.b = -half_h;          sw.t = half_h;
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
    std::string basedir = dirOf(filepath);
    std::string xml = processIncludes(readFile(filepath), basedir);

    std::vector<SceneData> jobs;

    // Estado de render (pode mudar entre render_again)
    Point3  look_from{0,0,0}, look_at{0,0,1};
    Vector3 vup{0,1,0};
    std::string cam_type        = "perspective";
    std::string cam_tag;
    std::string integrator_type = "flat";
    int    integrator_depth     = 1;
    int    film_w               = 800;
    int    film_h               = 600;
    std::string film_filename   = "output.ppm";
    bool   film_gamma           = false;

    // Estado do mundo (preservado para render_again)
    std::shared_ptr<Background>             background;
    std::vector<std::shared_ptr<Primitive>> primitives;
    std::vector<std::shared_ptr<Light>>     lights;         // NOVO
    std::shared_ptr<AmbientLight>           ambient_light;  // NOVO
    std::shared_ptr<Material>               current_material;
    std::map<std::string, std::shared_ptr<Material>> named_materials;
    bool has_world = false;
    bool in_world  = false;

    size_t pos = 0;
    while (pos < xml.size()) {
        size_t tag_start = xml.find('<', pos);
        if (tag_start == std::string::npos) break;

        // Comentários XML
        if (xml.substr(tag_start, 4) == "<!--") {
            size_t comment_end = xml.find("-->", tag_start + 4);
            pos = (comment_end != std::string::npos) ? comment_end + 3 : xml.size();
            continue;
        }

        size_t tag_end = xml.find('>', tag_start);
        if (tag_end == std::string::npos) break;

        std::string tag = xml.substr(tag_start, tag_end - tag_start + 1);
        pos = tag_end + 1;

        if (tag.size() < 2 || tag[1] == '/' || tag[1] == '?') continue;

        std::string name = tagName(tag);

        // ── Tags globais ──────────────────────────────────────────────────────
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
            std::string gc = extractAttr(tag, "gamma_corrected");
            film_gamma     = (gc == "true" || gc == "1");
        }
        else if (name == "integrator") {
            integrator_type = extractAttr(tag, "type");
            // profundidade maxima da reflexao espelho: aceita "depth" ou "max_depth"
            integrator_depth = extractIntOpt(tag, "depth",
                                   extractIntOpt(tag, "max_depth", 1));
        }

        // ── Início do mundo ───────────────────────────────────────────────────
        else if (name == "world_begin") {
            in_world = true;
            primitives.clear();
            lights.clear();
            ambient_light.reset();
            background.reset();
            current_material.reset();
        }

        // ── Tags dentro do mundo ──────────────────────────────────────────────
        else if (in_world) {

            // ── Background ───────────────────────────────────────────────────
            if (name == "background") {
                std::string btype = extractAttr(tag, "type");

                // O renderizador trabalha com cores de background em [0,255].
                // Mas algumas cenas declaram em [0,1]. O atributo 'normalized'
                // controla isso:
                //   normalized="true"  -> cores em [0,1], multiplica por 255
                //   normalized="false" -> cores em [0,255] (padrao historico)
                //   ausente            -> auto-detecta (se todo canal <= 1, e [0,1])
                std::string nrm = extractAttr(tag, "normalized");
                auto rescale = [&](std::vector<RGBColor> cols) {
                    bool normalized;
                    if      (nrm == "true"  || nrm == "1") normalized = true;
                    else if (nrm == "false" || nrm == "0") normalized = false;
                    else {
                        float maxc = 0.0f;
                        for (const auto& c : cols)
                            maxc = std::max(maxc, std::max(c.r, std::max(c.g, c.b)));
                        normalized = (maxc <= 1.0f);   // tudo <= 1 => provavelmente [0,1]
                    }
                    if (normalized)
                        for (auto& c : cols)
                            c = RGBColor(c.r * 255.0f, c.g * 255.0f, c.b * 255.0f);
                    return cols;
                };

                if (btype == "colors" || btype == "4_colors") {
                    std::vector<RGBColor> corners(4);
                    corners[0] = parseColor(extractAttr(tag, "bl"));
                    corners[1] = parseColor(extractAttr(tag, "tl"));
                    corners[2] = parseColor(extractAttr(tag, "tr"));
                    corners[3] = parseColor(extractAttr(tag, "br"));
                    corners = rescale(corners);
                    background = std::make_shared<Background4Colors>(corners);
                } else if (btype == "single_color") {
                    std::vector<RGBColor> one{ parseColor(extractAttr(tag, "color")) };
                    one = rescale(one);
                    background = std::make_shared<BackgroundSingleColor>(one[0]);
                } else {
                    throw std::runtime_error("Parser: tipo de background desconhecido: " + btype);
                }
            }

            // ── Materiais inline ─────────────────────────────────────────────
            else if (name == "material") {
                std::string mtype = extractAttr(tag, "type");
                if (mtype == "flat") {
                    current_material = std::make_shared<FlatMaterial>(
                        parseColor(extractAttr(tag, "color")));
                } else if (mtype == "blinn") {
                    // Material Blinn-Phong inline (sem nome)
                    RGBColor ka  = parseColor(extractAttr(tag, "ambient"));
                    RGBColor kd  = parseColor(extractAttr(tag, "diffuse"));
                    RGBColor ks  = parseColor(extractAttr(tag, "specular"));
                    float    g   = std::stof(extractAttr(tag, "glossiness"));
                    // mirror e opcional: ausente => (0,0,0) (sem reflexao)
                    std::string mir = extractAttr(tag, "mirror");
                    RGBColor km  = mir.empty() ? RGBColor(0,0,0) : parseColor(mir);
                    current_material = std::make_shared<BlinnPhongMaterial>(ka, kd, ks, g, km);
                }
            }

            // ── Materiais nomeados ────────────────────────────────────────────
            // Formato do README:
            //   <make_named_material type="blinn" name="gold"
            //       ambient="0.2 0.2 0.2" diffuse="1 0.65 0.0"
            //       specular="0.8 0.6 0.2" glossiness="256"/>
            else if (name == "make_named_material") {
                std::string mname = extractAttr(tag, "name");
                std::string mtype = extractAttr(tag, "type");
                if (mtype == "flat") {
                    named_materials[mname] = std::make_shared<FlatMaterial>(
                        parseColor(extractAttr(tag, "color")));
                } else if (mtype == "blinn") {
                    RGBColor ka = parseColor(extractAttr(tag, "ambient"));
                    RGBColor kd = parseColor(extractAttr(tag, "diffuse"));
                    RGBColor ks = parseColor(extractAttr(tag, "specular"));
                    float    g  = std::stof(extractAttr(tag, "glossiness"));
                    // mirror e opcional: ausente => (0,0,0) (sem reflexao)
                    std::string mir = extractAttr(tag, "mirror");
                    RGBColor km = mir.empty() ? RGBColor(0,0,0) : parseColor(mir);
                    named_materials[mname] = std::make_shared<BlinnPhongMaterial>(ka, kd, ks, g, km);
                }
            }

            // ── Referência a material nomeado ─────────────────────────────────
            else if (name == "named_material") {
                std::string mname = extractAttr(tag, "name");
                auto it = named_materials.find(mname);
                if (it != named_materials.end())
                    current_material = it->second;
                else
                    throw std::runtime_error("Parser: material nomeado nao encontrado: " + mname);
            }

            // ── Objetos ───────────────────────────────────────────────────────
            else if (name == "object") {
                if (!current_material)
                    current_material = std::make_shared<FlatMaterial>(RGBColor(1.0f, 1.0f, 1.0f));

                std::string mat_attr   = extractAttr(tag, "material");
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
                // ── Plano infinito ────────────────────────────────────────────
                else if (otype == "plane") {
                    Point3  p = parsePoint3 (extractAttr(tag, "point"));
                    Vector3 n = parseVector3(extractAttr(tag, "normal"));
                    primitives.push_back(
                        std::make_shared<Plane>(p, n, obj_material));
                }
                // ── Malha de triangulos (proj07) ──────────────────────────────
                else if (otype == "trianglemesh") {
                    auto mesh = std::make_shared<TriangleMesh>();

                    bool reverse = (extractAttr(tag, "reverse_vertex_order") == "true");
                    bool cull    = (extractAttr(tag, "backface_cull")       == "true");
                    bool comp_n  = (extractAttr(tag, "compute_normals")     == "true");

                    std::string fname = extractAttr(tag, "filename");
                    if (!fname.empty()) {
                        // Caminho relativo ao diretorio do arquivo de cena.
                        std::string objpath = basedir + "/" + fname;
                        if (!load_obj(objpath, *mesh))
                            throw std::runtime_error("Parser: nao foi possivel abrir o .obj: " + objpath);
                    } else {
                        // Declaracao manual da malha no proprio arquivo de cena.
                        std::vector<float> verts = parseFloatArray(extractAttr(tag, "vertices"));
                        std::vector<float> norms = parseFloatArray(extractAttr(tag, "normals"));
                        std::vector<float> uvs   = parseFloatArray(extractAttr(tag, "uvs"));

                        // aceita "vertex_indices" ou o alias "indices"
                        std::string vi_str = extractAttr(tag, "vertex_indices");
                        if (vi_str.empty()) vi_str = extractAttr(tag, "indices");
                        mesh->vertex_indices = parseIntArray(vi_str);
                        mesh->normal_indices = parseIntArray(extractAttr(tag, "normal_indices"));
                        mesh->uv_indices     = parseIntArray(extractAttr(tag, "uv_indices"));

                        for (size_t k = 0; k + 2 < verts.size(); k += 3)
                            mesh->points.push_back(Point3(verts[k], verts[k+1], verts[k+2]));
                        for (size_t k = 0; k + 2 < norms.size(); k += 3)
                            mesh->normals.push_back(Vector3(norms[k], norms[k+1], norms[k+2]));
                        for (size_t k = 0; k + 1 < uvs.size(); k += 2)
                            mesh->uvs.push_back(Point2f(uvs[k], uvs[k+1]));

                        mesh->n_triangles = extractIntOpt(tag, "ntriangles",
                                                (int)mesh->vertex_indices.size() / 3);

                        // Se vieram 'normals' SEM 'normal_indices' e o numero de
                        // normais bate com o de vertices, assumimos normais por
                        // vertice (normal_indices = vertex_indices). Sem isso, as
                        // normais fornecidas seriam ignoradas e calculariamos a
                        // normal geometrica — que pode apontar para o lado errado
                        // (depende do winding) e deixar a face invisivel.
                        if (mesh->normal_indices.empty() && !mesh->normals.empty()
                            && mesh->normals.size() == mesh->points.size()) {
                            mesh->normal_indices = mesh->vertex_indices;
                        }
                    }

                    auto tris = create_triangle_mesh(mesh, obj_material,
                                                     reverse, cull, comp_n);
                    for (auto& t : tris)
                        primitives.push_back(t);
                }
            }

            // ── Fontes de luz ─────────────────────────────────────────────────
            // Formato do README:
            //   <light_source type="ambient"     I="0.1 0.1 0.1" scale="1 1 1" />
            //   <light_source type="directional" I="0.9 0.9 0.9" scale="0.4 0.4 0.4"
            //                                    from="1 0.8 -1" to="0 0 1" />
            //   <light_source type="point"       I="0.95 0.9 0.8" scale="0.4 0.4 0.4"
            //                                    from="0.1 3.0 1" />
            //   <light_source type="point"       I="..." scale="..."
            //                                    from="..."
            //                                    attenuation="1.0 0.22 0.20" />
            else if (name == "light_source") {
                std::string ltype = extractAttr(tag, "type");

                // Intensidade e scale (ambos em [0,1]). Aceita "I" ou o alias "L".
                std::string I_str = extractAttr(tag, "I");
                if (I_str.empty()) I_str = extractAttr(tag, "L");
                RGBColor I     = parseColor(I_str);
                std::string sc = extractAttr(tag, "scale");
                RGBColor scale = sc.empty()
                    ? RGBColor(1.0f, 1.0f, 1.0f)
                    : parseColor(sc);

                if (ltype == "ambient") {
                    // Só deve existir uma ambient por cena
                    ambient_light = std::make_shared<AmbientLight>(I, scale);

                } else if (ltype == "directional") {
                    Point3 from = parsePoint3(extractAttr(tag, "from"));
                    Point3 to   = parsePoint3(extractAttr(tag, "to"));
                    // world_radius opcional: alcance do raio de sombra (default grande)
                    std::string wr = extractAttr(tag, "world_radius");
                    float world_radius = wr.empty() ? 1.0e6f : std::stof(wr);
                    lights.push_back(
                        std::make_shared<DirectionalLight>(I, scale, from, to, world_radius));

                } else if (ltype == "point") {
                    Point3 from = parsePoint3(extractAttr(tag, "from"));

                    // Atenuação: optional. Formato: "Kc Kl Kq"
                    // Sem atenuação: Kc=1, Kl=0, Kq=0
                    float kc = 1.0f, kl = 0.0f, kq = 0.0f;
                    std::string att = extractAttr(tag, "attenuation");
                    if (!att.empty()) {
                        std::istringstream ss(att);
                        ss >> kc >> kl >> kq;
                    }

                    lights.push_back(
                        std::make_shared<PointLight>(I, scale, from, kc, kl, kq));

                } else if (ltype == "spot") {
                    // Spotlight (proj06): luz pontual com cone de influencia.
                    //   from   = posicao da luz
                    //   to     = ponto para onde o cone aponta
                    //   cutoff = angulo externo (graus) — onde a luz zera
                    //   falloff= angulo interno (graus) — onde a luz e total
                    Point3 from   = parsePoint3(extractAttr(tag, "from"));
                    Point3 to     = parsePoint3(extractAttr(tag, "to"));
                    float  cutoff  = std::stof(extractAttr(tag, "cutoff"));
                    float  falloff = std::stof(extractAttr(tag, "falloff"));

                    // atenuacao opcional, igual a point
                    float kc = 1.0f, kl = 0.0f, kq = 0.0f;
                    std::string att = extractAttr(tag, "attenuation");
                    if (!att.empty()) {
                        std::istringstream ss(att);
                        ss >> kc >> kl >> kq;
                    }

                    lights.push_back(
                        std::make_shared<SpotLight>(I, scale, from, to,
                                                    cutoff, falloff, kc, kl, kq));

                } else {
                    throw std::runtime_error("Parser: tipo de luz desconhecido: " + ltype);
                }
            }

            // ── Fim do mundo ─────────────────────────────────────────────────
            else if (name == "world_end") {
                in_world  = false;
                has_world = true;

                SceneData data;
                data.camera           = buildCamera(look_from, look_at, vup,
                                                    cam_tag, cam_type, film_w, film_h);
                data.integrator_type  = integrator_type;
                data.integrator_depth = integrator_depth;
                data.film_width       = film_w;
                data.film_height      = film_h;
                data.film_filename    = film_filename;
                data.film_gamma       = film_gamma;
                data.background      = background;
                data.primitives      = primitives;
                data.lights          = lights;           // NOVO
                data.ambient_light   = ambient_light;    // NOVO
                jobs.push_back(std::move(data));
            }
        }

        // ── Render again ──────────────────────────────────────────────────────
        else if (name == "render_again" && has_world) {
            SceneData data;
            data.camera           = buildCamera(look_from, look_at, vup,
                                                cam_tag, cam_type, film_w, film_h);
            data.integrator_type  = integrator_type;
            data.integrator_depth = integrator_depth;
            data.film_width       = film_w;
            data.film_height      = film_h;
            data.film_filename    = film_filename;
            data.film_gamma       = film_gamma;
            data.background      = background;
            data.primitives      = primitives;
            data.lights          = lights;
            data.ambient_light   = ambient_light;
            jobs.push_back(std::move(data));
        }
    }

    if (jobs.empty())
        throw std::runtime_error("Parser: nenhum bloco world_begin/world_end encontrado");

    return jobs;
}