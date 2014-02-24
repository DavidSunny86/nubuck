#include <Nubuck\common\common.h>

#include <renderer\glew\glew.h>
#include <renderer\glcall.h>
#include <renderer\program\program.h>

#include "skin.h"

namespace R {

    void Skin::Bind(int flags, Program& program) const {
        static const char* names[] = {
            "uniTexDiffuse", "uniTexNormal", "uniTexAdd", "uniTexSpec"
        };

        for(int i = 0; i < NUM_LAYERS; ++i) {
            if((1 << i) & flags) {
                if(!_textures[i].IsValid()) common.printf("Skin::Bind: attempting to bind non-existent texture on layer '%s'\n", names[i]);
                else {
                    program.SetUniform(names[i], i);
                    _textures[i]->Bind(i);
                }
            } else {
                GL_CALL(glActiveTexture(GL_TEXTURE0 + i));
                GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
            }
        }
    }

    void Skin::BindFFP(int flags) const {
        static const char* names[] = {
            "uniTexDiffuse", "uniTexNormal", "uniTexAdd", "uniTexSpec"
        };

        for(int i = 0; i < NUM_LAYERS; ++i) {
            if((1 << i) & flags) {
                if(!_textures[i].IsValid()) common.printf("Skin::BindFFP: attempting to bind non-existent texture on layer '%s'\n", names[i]);
                else {
                    _textures[i]->Bind(i);
                }
            } else {
                GL_CALL(glActiveTexture(GL_TEXTURE0 + i));
                GL_CALL(glBindTexture(GL_TEXTURE_2D, 0));
            }
        }
    }

} // namespace R