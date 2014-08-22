#pragma once

#include <string>
#include <vector>
#include <renderer\mesh\mesh.h>
#include <renderer\mesh\meshmgr.h>

namespace R {

struct TF_Char;
class TexFont;
class SDTexFont;
struct RenderList;

class Text {
private:
    // mesh of all glyphs sharing the same page, ie. texture
    struct PageMesh {
        std::vector<Mesh::Vertex>    vertices;
        std::vector<Mesh::Index>     indices;

        meshPtr_t   mesh;
        tfmeshPtr_t tfmesh;

        PageMesh() : mesh(NULL), tfmesh(NULL) { }
    };
    std::vector<PageMesh> _pageMeshes;

    void DestroyMesh();

    void _Rebuild(
        const int               numPages,
        const int               texWidth,
        const int               texHeight,
        const float             lineHeight,
        const std::string&      text,
        const TF_Char* const    chars);
public:
    Text();

    void Rebuild(const TexFont& texFont, const std::string& text);
    void GetRenderJobs(const TexFont& texFont, RenderList& renderList) const;

    // HACK
    void Rebuild(const SDTexFont& texFont, const std::string& text);
    void GetRenderJobs(SDTexFont& texFont, RenderList& renderList) const;
};

} // namespace R