#include <renderer\renderer.h>
#include <renderer\texfont\texfont.h>
#include <renderer\text\r_text.h>

namespace R {

void Text::DestroyMesh() {
    for(unsigned i = 0; i < _pageMeshes.size(); ++i) {
        PageMesh& pageMesh = _pageMeshes[i];

        if(pageMesh.mesh) {
            meshMgr.Destroy(pageMesh.mesh);
            pageMesh.mesh = NULL;
        }
        if(pageMesh.tfmesh) {
            meshMgr.Destroy(pageMesh.tfmesh);
            pageMesh.tfmesh = NULL;
        }
    }
}

Text::Text() { }

static void ComputeTextureCoordinates(
    const int texWidth,
    const int texHeight,
    const TF_Char& texChar,
    M::Vector2& lowerLeft,
    M::Vector2& upperRight)
{
    // NOTE on bmfont coordinates:
    // - origin is upper left hand corner.
    // - xy specify upper left hand corner

    const float scaleW = 1.0f / texWidth;
    const float scaleH = 1.0f / texHeight;

    lowerLeft.x   = scaleW * texChar.x;
    upperRight.y  = 1.0f - scaleH * texChar.y;

    upperRight.x  = lowerLeft.x + scaleW * texChar.width;
    lowerLeft.y   = upperRight.y - scaleH * texChar.height;
}

static float ComputeScale(
    const int texWidth,
    const int texHeight,
    const TF_Char* const chars,
    const float size)
{
    const TF_Char& texChar = chars['A'];

    M::Vector2 tc_lowerLeft, tc_upperRight;
    ComputeTextureCoordinates(texWidth, texHeight, texChar, tc_lowerLeft, tc_upperRight);

    const float width_ts = tc_upperRight.x - tc_lowerLeft.x;
    return size / width_ts;
}

void Text::_Rebuild(
    const int               numPages,
    const int               texWidth,
    const int               texHeight,
    const float             lineHeight,
    const std::string&      text,
    const TF_Char* const    chars)
{
    DestroyMesh();

    _pageMeshes.clear();
    _pageMeshes.resize(numPages);

    const Mesh::Index indices[] = { 0, 1, 2, 0, 2, 3 };

    const M::Vector3 pos[] = {
        M::Vector3( 0.0f,  0.0f,  0.0f),
        M::Vector3( 0.0f, -1.0f,  0.0f),
        M::Vector3( 1.0f, -1.0f,  0.0f),
        M::Vector3( 1.0f,  0.0f,  0.0f)
        /*
        M::Vector3(-1.0f, -1.0f, 0.0f),
        M::Vector3( 1.0f, -1.0f, 0.0f),
        M::Vector3( 1.0f,  1.0f, 0.0f),
        M::Vector3(-1.0f,  1.0f, 0.0f)
        */
    };

    // scale to normalized texture space
    const float scaleW = 1.0f / texWidth;
    const float scaleH = 1.0f / texHeight;

    const float scale = ComputeScale(texWidth, texHeight, chars, 2.5f); // texture space to world space scale

    M::Vector2 cursor;

    const float ydiff = scale * scaleH * lineHeight;

    for(unsigned cidx = 0; cidx < text.size(); ++cidx) {
        const char c = text[cidx];

        if(' ' == c) {
            // let a space be the size of one 'x'
            cursor.x += scale * scaleW * chars['x'].xadvance;
            continue;
        }

        if('\n' == c) {
            cursor.x = 0.0f;
            cursor.y -= ydiff;
            continue;
        }

        const TF_Char& texChar = chars[c];

        PageMesh& pageMesh = _pageMeshes[texChar.page];

        M::Vector2 tc_lowerLeft, tc_upperRight;
        ComputeTextureCoordinates(texWidth, texHeight, texChar, tc_lowerLeft, tc_upperRight);

        const M::Vector2 size_ts = tc_upperRight - tc_lowerLeft;

        const float xoff_ts = scaleW * texChar.xoffset;
        const float yoff_ts = scaleH  *texChar.yoffset;

        unsigned idxOff = pageMesh.vertices.size();

        M::Vector2 texCoords[4];
        texCoords[0] = M::Vector2(tc_lowerLeft.x, tc_upperRight.y);
        texCoords[1] = M::Vector2(tc_lowerLeft.x, tc_lowerLeft.y);
        texCoords[2] = M::Vector2(tc_upperRight.x, tc_lowerLeft.y);
        texCoords[3] = M::Vector2(tc_upperRight.x, tc_upperRight.y);

        Mesh::Vertex vert;
        for(int i = 0; i < 4; ++i) {
            vert.position.x = cursor.x + scale * (size_ts.x * pos[i].x + xoff_ts);
            vert.position.y = cursor.y + scale * (size_ts.y * pos[i].y - yoff_ts);
            vert.position.z = pos[i].z;

            vert.texCoords = texCoords[i];

            vert.normal = M::Vector3(0.0f, 0.0f, 1.0f);
            vert.color = Color::White;

            pageMesh.vertices.push_back(vert);
        }

        cursor.x += scale * scaleW * texChar.xadvance;

        for(int i = 0; i < 6; ++i) {
            pageMesh.indices.push_back(idxOff + indices[i]);
        }
        pageMesh.indices.push_back(Mesh::RESTART_INDEX);
    }

    // rebuild pages
    for(unsigned i = 0; i < _pageMeshes.size(); ++i) {
        PageMesh& pageMesh = _pageMeshes[i];
        if(!pageMesh.vertices.empty()) {
            Mesh::Desc meshDesc;
            meshDesc.vertices       = &pageMesh.vertices[0];
            meshDesc.numVertices    = pageMesh.vertices.size();
            meshDesc.indices        = &pageMesh.indices[0];
            meshDesc.numIndices     = pageMesh.indices.size();
            meshDesc.primType       = GL_TRIANGLES;

            pageMesh.mesh = meshMgr.Create(meshDesc);
            pageMesh.tfmesh = meshMgr.Create(pageMesh.mesh);
        }
    }
}

void Text::Rebuild(const TexFont& texFont, const std::string& text) {
    const TexFont::Common& texCommon = texFont.GetCommon();
    _Rebuild(texCommon.pages, texCommon.scaleW, texCommon.scaleH, texCommon.lineHeight, text, texFont.GetChars());
}

void Text::Rebuild(const SDTexFont& texFont, const std::string& text) {
    const SDTexFont::Common& texCommon = texFont.GetCommon();
    _Rebuild(1, texCommon.scaleW, texCommon.scaleH, texCommon.lineHeight, text, texFont.GetChars());
}

void Text::GetRenderJobs(const TexFont& texFont, RenderList& renderList) const {
    for(unsigned i = 0; i < _pageMeshes.size(); ++i) {
        const PageMesh& pageMesh = _pageMeshes[i];

        R::MeshJob mjob;

        R::Material mat = R::Material::White;
        mat.texBindings[0].samplerName = "font";
        mat.texBindings[0].texture = texFont.GetPageTexture(i);

        mjob.fx         = "Text";
        mjob.layer      = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
        mjob.material   = mat;
        mjob.primType   = 0;
        mjob.tfmesh     = pageMesh.tfmesh;

        mjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
        renderList.meshJobs.push_back(mjob);

        mjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
        renderList.meshJobs.push_back(mjob);
    }
}

void Text::GetRenderJobs(SDTexFont& texFont, RenderList& renderList) const {
    for(unsigned i = 0; i < _pageMeshes.size(); ++i) {
        const PageMesh& pageMesh = _pageMeshes[i];

        R::MeshJob mjob;

        R::Material mat = R::Material::White;
        mat.texBindings[0].samplerName = "font";
        mat.texBindings[0].texture = texFont.GetPageTexture();

        mjob.fx         = "SDText";
        mjob.layer      = R::Renderer::Layers::GEOMETRY_0_SOLID_0;
        mjob.material   = mat;
        mjob.primType   = 0;
        mjob.tfmesh     = pageMesh.tfmesh;

        mjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_1;
        renderList.meshJobs.push_back(mjob);

        mjob.layer = R::Renderer::Layers::GEOMETRY_0_SOLID_2;
        renderList.meshJobs.push_back(mjob);
    }
}

} // namespace R