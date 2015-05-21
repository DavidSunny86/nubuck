#pragma once

#include <unordered_map>
#include <Nubuck\renderer\color\color.h>

namespace R {

class Program;
class Texture;

class Material {
private:
    enum {
        // first three texture units are used by renderer
        MATERIAL_BASE_TEXUNIT = 3
    };

    struct UniformType {
        enum Enum {
            TEXTURE = 0,
            COLOR
        };
    };

    struct UniformBinding {
        const char*         name;
        UniformType::Enum   type;
        union Variant {
            Texture*    v_tex;
            float       v_color[4];
        } variant;

        UniformBinding() : name(0) { }

        bool IsValid() const { return 0 != name; }
    };
    enum { NUM_UNIFORM_BINDINGS = 3 };

    // TODO: remove this eventually
    bool            isDiffuseColorValid;

    Color           diffuseColor;
    UniformBinding  uniformBindings[NUM_UNIFORM_BINDINGS];
    int             _numBindings;

    UniformBinding& FindBindingSlot(const char* name);
public:
    static void Bind(
        Program& prog,
        std::unordered_map<std::string, int>& unorm,
        int timestamp,
        const Material& mat);

    static Material White;

    Material();
    Material(const Color& diffuseColor);

    bool IsDiffuseColorValid() const { return isDiffuseColorValid; }

    void SetDiffuseColor(const Color& color);

    void SetUniformBinding(const char* name, Texture* val);
    void SetUniformBinding(const char* name, const Color& val);
    void ClearUniformBinding(const char* name);
};

} // namespace R