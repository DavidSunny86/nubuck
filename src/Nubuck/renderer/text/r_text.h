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

    M::Vector2 _size; // computed size of oob

    void DestroyMesh();
public:
    Text();

    const M::Vector2& GetSize() const;

    void Rebuild(
        const TexFont& texFont,
        const std::string& text,
        const char refChar = 'A',
        const float refCharSize = 2.5f,
        const Color& color = Color::Black);
    void GetRenderJobs(const M::Matrix4& transform, RenderList& renderList);
};

} // namespace R