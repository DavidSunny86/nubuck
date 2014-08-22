#pragma once

#include <string>
#include <vector>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>

namespace R {

struct TexFont;
struct RenderList;

class Text {
private:
    // mesh of all glyphs sharing the same page, ie. texture
    struct PageMesh {
        std::vector<Mesh::Vertex>    vertices;
        std::vector<Mesh::Index>     indices;

        meshPtr_t   mesh;
        tfmeshPtr_t tfmesh;

        std::string texFilename;
        Texture*    texture;

        PageMesh() : mesh(NULL), tfmesh(NULL) { }
    };
    std::vector<PageMesh> _pageMeshes;

    void DestroyMesh();
public:
    Text();

    void Rebuild(const TexFont& texFont, const std::string& text);
    void GetRenderJobs(RenderList& renderList);
};

} // namespace R