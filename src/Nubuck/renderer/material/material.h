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

        bool                    isTransparent;
        Color                   diffuseColor;
        TexBinding              texture0;

		static Material White;

        Material() : isTransparent(false) { }
        Material(const Color& diffuseColor) : diffuseColor(diffuseColor) { }
    };

} // namespace R