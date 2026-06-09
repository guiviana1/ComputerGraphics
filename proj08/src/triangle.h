#pragma once

#include "primitive.h"
#include "vec3.h"
#include "surfel.h"     // Point2f
#include <vector>
#include <memory>
#include <string>

// Malha de triangulos com indices (vertices/normais/uvs compartilhados).
struct TriangleMesh {
    int n_triangles = 0;

    std::vector<int> vertex_indices;  // indices na lista 'points'  (3 por triangulo)
    std::vector<int> normal_indices;  // indices na lista 'normals' (3 por triangulo)
    std::vector<int> uv_indices;      // indices na lista 'uvs'     (3 por triangulo)

    std::vector<Point3>  points;      // coordenadas 3D dos vertices
    std::vector<Vector3> normals;     // normais (podem nao estar normalizadas)
    std::vector<Point2f> uvs;         // coordenadas de textura (u,v)
};

// Triangulo individual: aponta para a malha e guarda indices dos seus 3 vertices.
class Triangle : public Primitive {
private:
    std::shared_ptr<TriangleMesh> mesh;
    int v[3];               // indices dos vertices na lista mesh->points
    int n[3];               // indices das normais na lista mesh->normals (-1 = sem)
    int uv[3];              // indices das uvs na lista mesh->uvs (-1 = sem)
    bool backface_cull;     // se true, ignora a face de tras

public:
    Triangle(std::shared_ptr<TriangleMesh> mesh, int tri_index,
             bool reverse_orientation, bool backface_cull);

    bool     intersect  (Ray& r, Surfel* sf) const override;
    bool     intersect_p(const Ray& r)       const override;
    Bounds3f world_bound()                   const override;

private:
    // Algoritmo de Moller-Trumbore. Devolve t e as baricentricas (b1,b2).
    // 'cull' liga/desliga o descarte da face de tras.
    bool moller_trumbore(const Ray& r, float& t, float& b1, float& b2,
                         bool cull) const;
};

// Cria a malha e um Triangle por face, devolvendo a lista de primitivas.
std::vector<std::shared_ptr<Primitive>>
create_triangle_mesh(std::shared_ptr<TriangleMesh> mesh,
                     std::shared_ptr<Material>     material,
                     bool reverse_orientation,
                     bool backface_cull,
                     bool compute_normals);

// Carrega uma malha de um arquivo Wavefront .obj. Preenche 'mesh'.
// Retorna false se o arquivo nao puder ser aberto.
bool load_obj(const std::string& path, TriangleMesh& mesh);
