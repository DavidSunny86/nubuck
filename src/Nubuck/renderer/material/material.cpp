#include <renderer\program\program.h>
#include <renderer\textures\texture.h>
#include "material.h"

namespace R {

void Material::Bind(Program& prog, const Material& mat) {
    int texUnit = MATERIAL_BASE_TEXUNIT;
    for(int i = 0; i < NUM_UNIFORM_BINDINGS && mat.uniformBindings[i].IsValid(); ++i) {
        const UniformBinding& ub = mat.uniformBindings[i];
        GLint loc = glGetUniformLocation(prog.GetID(), ub.name);
        if(0 <= loc) {
            if(UniformType::TEXTURE == ub.type) {
                prog.SetUniform(ub.name, texUnit);
                ub.variant.v_tex->Bind(texUnit);
                texUnit++;
            }
        }
    }
}

Material::Material() : _numBindings(0) { }

Material Material::White(Color::White);

void Material::SetUniformBinding(const char* name, Texture* val) {
    if(NUM_UNIFORM_BINDINGS <= _numBindings) {
        common.printf("ERROR - Material: cannot find a free uniform binding point\n");
        Crash();
    }
    UniformBinding& ub = uniformBindings[_numBindings++];
    ub.name             = name;
    ub.variant.v_tex    = val;
}

void Material::ClearUniformBinding(const char* name) {
    int i = 0;
    while(i < NUM_UNIFORM_BINDINGS && strcmp(name, uniformBindings[i].name)) i++;
    if(i < NUM_UNIFORM_BINDINGS) {
        std::swap(uniformBindings[i], uniformBindings[_numBindings - 1]);
        _numBindings--;
    }
}

} // namespace R