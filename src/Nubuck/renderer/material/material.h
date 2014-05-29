#pragma once

#include <Nubuck\renderer\color\color.h>
#include <renderer\textures\texture.h>

namespace R {

    struct Material {
        struct TexBinding {
            Texture*    texture;
            const char* samplerName;

            TexBinding() : texture(NULL), samplerName(NULL) { }

            bool IsValid() const { return NULL != texture && NULL != samplerName; }
        };
        enum { NUM_TEX_BINDINGS = 3 };

        bool        isTransparent;
        Color       diffuseColor;
        TexBinding  texBindings[NUM_TEX_BINDINGS];

		static Material White;

        Material() : isTransparent(false) { }
        Material(const Color& diffuseColor) : diffuseColor(diffuseColor) { }
    };

} // namespace R