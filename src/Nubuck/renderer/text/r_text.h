#pragma once

#include <string>
#include <vector>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>

/*
a text is a collection of strings. a string is a sequence of characters.
each character is represented by a quad. the string vertices are stored
in 2D STRING SPACE, with y-axis pointing down and origin in the upper
left hand corner. the vertices are transformed into object space in the
vertex shader.
additional vertex attributes:
A0 = origin
A1 = center of string bbox (in string space)
A2 = depth proxy point (reference position for custom depht test)
*/

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

    struct VertexIndex {
        unsigned p; // page index
        unsigned v; // vertex index in p.mesh
    };

    struct String {
        M::Vector2                  lowerLeft; // upper left hand corner of oob
        M::Vector2                  upperRight; // computed size of oob

        // index of first vertex of each char quad
        std::vector<VertexIndex>    vertices;
    };
    std::vector<String> _strings;

    void DestroyMesh();
public:
    Text();

    const M::Vector2& GetLowerLeft(unsigned sidx) const;
    const M::Vector2& GetUpperRight(unsigned sidx) const;

    unsigned NumStrings() const { return _strings.size(); }

    void Begin(const TexFont& texFont);
    unsigned AddString(
        const TexFont& texFont,
        const std::string& text,
        const char refChar = 'A',
        const float refCharSize = 2.5f,
        const Color& color = Color::Black);
    void End();

    void SetOriginAndDepthProxy(unsigned sidx, const M::Vector3& origin, const M::Vector3& depthProxy);

    void GetRenderJobs(const M::Matrix4& transform, RenderList& renderList);
    void GetRenderJobsAlt(const M::Matrix4& transform, RenderList& renderList, bool xray);
};

} // namespace R