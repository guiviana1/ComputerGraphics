#include "triangle.h"

#include <cmath>
#include <fstream>
#include <sstream>
#include <array>

Bounds3f Triangle::world_bound() const {
    const Point3& p0 = mesh->points[v[0]];
    const Point3& p1 = mesh->points[v[1]];
    const Point3& p2 = mesh->points[v[2]];
    return Bounds3f(p0, p1).expand(p2);
}

Triangle::Triangle(std::shared_ptr<TriangleMesh> mesh, int tri_index,
                   bool reverse_orientation, bool backface_cull)
    : Primitive(nullptr)            // material e injetado por create_triangle_mesh
    , mesh(std::move(mesh))
    , backface_cull(backface_cull)
{
    const int base = 3 * tri_index;

    // Ordem dos vertices do triangulo. reverse_orientation troca a ordem
    // (inverte o "enrolamento"/winding), o que troca qual e a face da frente.
    int order[3] = {0, 1, 2};
    if (reverse_orientation) { order[1] = 2; order[2] = 1; }

    for (int k = 0; k < 3; ++k) {
        int o = order[k];
        v[k]  = this->mesh->vertex_indices[base + o];
        n[k]  = this->mesh->normal_indices.empty() ? -1 : this->mesh->normal_indices[base + o];
        uv[k] = this->mesh->uv_indices.empty()     ? -1 : this->mesh->uv_indices[base + o];
    }
}

bool Triangle::moller_trumbore(const Ray& r, float& t, float& b1, float& b2,
                               bool cull) const {
    const float EPS = 1e-7f;

    const Point3& p0 = mesh->points[v[0]];
    const Point3& p1 = mesh->points[v[1]];
    const Point3& p2 = mesh->points[v[2]];

    Vector3 e1   = p1 - p0;
    Vector3 e2   = p2 - p0;
    Vector3 pvec = cross(r.d, e2);
    float   det  = dot(e1, pvec);

    if (cull) {
        // backface culling: so aceita a face da frente (det > 0)
        if (det < EPS) return false;
    } else {
        // sem culling: rejeita apenas raios paralelos ao triangulo
        if (std::fabs(det) < EPS) return false;
    }

    float   invDet = 1.0f / det;
    Vector3 tvec   = r.o - p0;

    b1 = dot(tvec, pvec) * invDet;          // baricentrica do vertice 1
    if (b1 < 0.0f || b1 > 1.0f) return false;

    Vector3 qvec = cross(tvec, e1);
    b2 = dot(r.d, qvec) * invDet;           // baricentrica do vertice 2
    if (b2 < 0.0f || b1 + b2 > 1.0f) return false;

    t = dot(e2, qvec) * invDet;
    return (t >= r.t_min && t <= r.t_max);
}

bool Triangle::intersect(Ray& r, Surfel* sf) const {
    float t, b1, b2;
    if (!moller_trumbore(r, t, b1, b2, backface_cull))
        return false;

    r.t_max = t;

    if (sf != nullptr) {
        Point3 hit = r(t);

        // Baricentricas: vertice 0 recebe (1-b1-b2); 1 recebe b1; 2 recebe b2.
        float b0 = 1.0f - b1 - b2;

        // Normal: interpola as normais dos vertices (Phong shading). Como
        // create_triangle_mesh garante normais validas, basta interpolar.
        const Vector3& n0 = mesh->normals[n[0]];
        const Vector3& n1 = mesh->normals[n[1]];
        const Vector3& n2 = mesh->normals[n[2]];
        Vector3 normal = normalize(n0 * b0 + n1 * b1 + n2 * b2);

        // UV interpolada (se houver)
        Point2f uvcoord(0.0f, 0.0f);
        if (uv[0] >= 0 && !mesh->uvs.empty()) {
            const Point2f& t0 = mesh->uvs[uv[0]];
            const Point2f& t1 = mesh->uvs[uv[1]];
            const Point2f& t2 = mesh->uvs[uv[2]];
            uvcoord.u = b0 * t0.u + b1 * t1.u + b2 * t2.u;
            uvcoord.v = b0 * t0.v + b1 * t1.v + b2 * t2.v;
        }

        Vector3 wo = -r.d;
        *sf = Surfel(hit, normal, wo, t, uvcoord, this);
    }

    return true;
}

bool Triangle::intersect_p(const Ray& r) const {
    // Para sombras: qualquer intersecao bloqueia a luz, mesmo na face de tras.
    float t, b1, b2;
    return moller_trumbore(r, t, b1, b2, /*cull=*/false);
}

std::vector<std::shared_ptr<Primitive>>
create_triangle_mesh(std::shared_ptr<TriangleMesh> mesh,
                     std::shared_ptr<Material>     material,
                     bool reverse_orientation,
                     bool backface_cull,
                     bool compute_normals)
{
    const int n_tri = mesh->n_triangles;

    // Se pedirem para calcular normais, ou se a malha nao trouxe normais,
    // geramos UMA normal geometrica por face (flat shading) e ajustamos os
    // indices de normal para apontarem para ela.
    bool need_normals = compute_normals
                     || mesh->normals.empty()
                     || mesh->normal_indices.empty();

    if (need_normals) {
        mesh->normals.clear();
        mesh->normals.resize(n_tri);
        mesh->normal_indices.assign(3 * n_tri, 0);

        for (int i = 0; i < n_tri; ++i) {
            const Point3& p0 = mesh->points[mesh->vertex_indices[3*i + 0]];
            const Point3& p1 = mesh->points[mesh->vertex_indices[3*i + 1]];
            const Point3& p2 = mesh->points[mesh->vertex_indices[3*i + 2]];
            Vector3 fn = normalize(cross(p1 - p0, p2 - p0));
            if (reverse_orientation) fn = -fn;   // winding invertido => normal invertida
            mesh->normals[i] = fn;
            mesh->normal_indices[3*i + 0] = i;
            mesh->normal_indices[3*i + 1] = i;
            mesh->normal_indices[3*i + 2] = i;
        }
    }

    // Se nao ha uvs, criamos uma unica (0,0) e zeramos os indices.
    if (mesh->uvs.empty() || mesh->uv_indices.empty()) {
        mesh->uvs.assign(1, Point2f(0.0f, 0.0f));
        mesh->uv_indices.assign(3 * n_tri, 0);
    }

    std::vector<std::shared_ptr<Primitive>> tris;
    tris.reserve(n_tri);
    for (int i = 0; i < n_tri; ++i) {
        auto tri = std::make_shared<Triangle>(mesh, i, reverse_orientation, backface_cull);
        tri->set_material(material);
        tris.push_back(tri);
    }
    return tris;
}

// Leitor de .obj

// Converte um indice do OBJ (1-based; negativo conta do fim) para 0-based.
static int objIndex(int raw, int count) {
    if (raw > 0) return raw - 1;        // 1-based -> 0-based
    if (raw < 0) return count + raw;    // -1 => ultimo
    return -1;                           // 0 = ausente
}

// Faz o parse de um token de face "v", "v/vt", "v//vn" ou "v/vt/vn".
// Preenche vi/ti/ni com indices 0-based (ou -1 se ausentes).
static void parseFaceVertex(const std::string& tok,
                            int n_v, int n_vt, int n_vn,
                            int& vi, int& ti, int& ni) {
    vi = ti = ni = -1;
    int slot = 0;
    std::string cur;
    auto flush = [&](void) {
        int val = cur.empty() ? 0 : std::stoi(cur);
        if      (slot == 0) vi = objIndex(val, n_v);
        else if (slot == 1) ti = (val == 0 ? -1 : objIndex(val, n_vt));
        else if (slot == 2) ni = (val == 0 ? -1 : objIndex(val, n_vn));
        cur.clear();
    };
    for (char c : tok) {
        if (c == '/') { flush(); ++slot; }
        else          { cur += c; }
    }
    flush();
}

bool load_obj(const std::string& path, TriangleMesh& mesh) {
    std::ifstream f(path);
    if (!f.is_open()) return false;

    std::vector<Point3>  pts;
    std::vector<Vector3> nls;
    std::vector<Point2f> tex;
    std::vector<int> vIdx, nIdx, uvIdx;
    bool any_missing_normal = false;
    bool any_missing_uv     = false;

    std::string line;
    while (std::getline(f, line)) {
        std::istringstream ss(line);
        std::string tok;
        if (!(ss >> tok)) continue;

        if (tok == "v") {
            float x, y, z; ss >> x >> y >> z;
            pts.push_back(Point3(x, y, z));
        } else if (tok == "vn") {
            float x, y, z; ss >> x >> y >> z;
            nls.push_back(Vector3(x, y, z));
        } else if (tok == "vt") {
            float u = 0, v = 0; ss >> u >> v;
            tex.push_back(Point2f(u, v));
        } else if (tok == "f") {
            // le todos os vertices da face e triangula em leque (fan)
            std::vector<std::array<int,3>> face;   // (vi, ti, ni)
            std::string vert;
            while (ss >> vert) {
                int vi, ti, ni;
                parseFaceVertex(vert, (int)pts.size(), (int)tex.size(), (int)nls.size(),
                                vi, ti, ni);
                face.push_back({vi, ti, ni});
            }
            for (size_t i = 1; i + 1 < face.size(); ++i) {
                const std::array<int,3> tri[3] = { face[0], face[i], face[i+1] };
                for (int k = 0; k < 3; ++k) {
                    vIdx.push_back(tri[k][0]);
                    if (tri[k][2] < 0) any_missing_normal = true; else nIdx.push_back(tri[k][2]);
                    if (tri[k][1] < 0) any_missing_uv     = true; else uvIdx.push_back(tri[k][1]);
                }
            }
        }
        // demais tokens (comentarios '#', 'g', 's', 'mtllib', ...) sao ignorados
    }

    mesh.points      = std::move(pts);
    mesh.normals     = std::move(nls);
    mesh.uvs         = std::move(tex);
    mesh.vertex_indices = std::move(vIdx);
    mesh.n_triangles    = (int)mesh.vertex_indices.size() / 3;

    // So aproveitamos as normais/uvs do arquivo se TODAS as faces as tiverem
    // (caso contrario deixamos vazio para o create_triangle_mesh resolver).
    if (!any_missing_normal && (int)nIdx.size() == 3 * mesh.n_triangles)
        mesh.normal_indices = std::move(nIdx);
    else
        mesh.normals.clear();   // forca calculo de normais geometricas

    if (!any_missing_uv && (int)uvIdx.size() == 3 * mesh.n_triangles)
        mesh.uv_indices = std::move(uvIdx);

    return true;
}
