#include <renderer\program\program.h>
#include <renderer\textures\texture.h>
#include "material.h"

namespace R {

Material::UniformBinding& Material::FindBindingSlot(const char* name) {
    for(int i = 0; i < _numBindings; ++i) {
        if(!strcmp(uniformBindings[i].name, name)) {
            return uniformBindings[i];
        }
    }
    if(NUM_UNIFORM_BINDINGS <= _numBindings) {
        common.printf("ERROR - Material: cannot find a free uniform binding point\n");
        Crash();
    }
    return uniformBindings[_numBindings++];
}

// sets unorm[u] = timestamp, for every material uniform 'u'
void Material::Bind(Program& prog, std::unordered_map<std::string, int>& unorm, int timestamp, const Material& mat) {
    const char* mat_diffuseColor = "uDiffuseColor";

    // bind default uniforms
    prog.SetUniform(mat_diffuseColor, mat.diffuseColor, true);
    unorm[mat_diffuseColor] = timestamp;

    // bind optional uniforms
    int texUnit = MATERIAL_BASE_TEXUNIT;
    for(int i = 0; i < NUM_UNIFORM_BINDINGS && mat.uniformBindings[i].IsValid(); ++i) {
        const UniformBinding& ub = mat.uniformBindings[i];
        GLint loc = glGetUniformLocation(prog.GetID(), ub.name);
        if(0 <= loc) {
            std::unordered_map<std::string, int>::iterator unormIt = unorm.find(ub.name);
            if(unorm.end() == unormIt) {
                printf("WARNING - material sets non-material uniform '%s'\n", ub.name);
            } else {
                unormIt->second = timestamp;
            }
            if(UniformType::TEXTURE == ub.type && ub.variant.v_tex) {
                prog.SetUniform(ub.name, texUnit);
                ub.variant.v_tex->Bind(texUnit);
                texUnit++;
            }
            if(UniformType::COLOR == ub.type) {
                Color c(
                    ub.variant.v_color[0],
                    ub.variant.v_color[1],
                    ub.variant.v_color[2],
                    ub.variant.v_color[3]);
                prog.SetUniform(ub.name, Color(c));
            }
        }
    }
}

Material::Material() : _numBindings(0) { }

Material Material::White(Color::White);

void Material::SetDiffuseColor(const Color& color) {
    diffuseColor = color;
}

void Material::SetUniformBinding(const char* name, Texture* val) {
    UniformBinding& ub = FindBindingSlot(name);
    ub.name             = name;
    ub.variant.v_tex    = val;
}

void Material::SetUniformBinding(const char* name, const Color& val) {
    UniformBinding& ub = FindBindingSlot(name);
    ub.type                 = UniformType::COLOR;
    ub.name                 = name;
    ub.variant.v_color[0]   = val.r;
    ub.variant.v_color[1]   = val.g;
    ub.variant.v_color[2]   = val.b;
    ub.variant.v_color[3]   = val.a;
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