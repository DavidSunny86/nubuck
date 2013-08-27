#include "effectmgr.h"
#include "effect.h"

namespace R {

    void CreateDefaultEffects(void) {
        PassDesc pass0, pass1, pass2;
        EffectDesc fx;

        // Default Effect
        // unlit, constant color

        pass0.name = "Pass0";
        pass0.filenames[R::Shader::VERTEX]      = "Shaders\\default.vert";
        pass0.filenames[R::Shader::FRAGMENT]    = "Shaders\\default.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass0.state.SetDefault();
        pass0.type = DEFAULT;
        pass0.flags = 0;

        fx.name = "Default";
        fx.passes.push_back(pass0);
		effectMgr.Register(fx);
        fx.passes.clear();
            
        // NodeBillboard Effect
        // used to draw filled circles on billboards

        pass0.name = "Pass0";
        pass0.filenames[R::Shader::VERTEX]      = "Shaders\\node_billboard.vert";
        pass0.filenames[R::Shader::FRAGMENT]    = "Shaders\\node_billboard.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass0.state.SetDefault();
        pass0.type = DEFAULT;
        pass0.flags = 0;

        fx.name = "NodeBillboard";
        fx.passes.push_back(pass0);
		effectMgr.Register(fx);
        fx.passes.clear();

        // Default Wireframe Effect

        pass0.name = "Wireframe";
        pass0.filenames[R::Shader::VERTEX]      = "Shaders\\default.vert";
        pass0.filenames[R::Shader::FRAGMENT]    = "Shaders\\default.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass0.state.SetDefault();
        pass0.state.depth.func = GL_LEQUAL;
        pass0.state.raster.lineWidth = 3.0f;
        pass0.type = DEFAULT;
        pass0.flags = USE_MATERIAL;

        fx.name = "Wireframe";
        fx.passes.push_back(pass0);
        effectMgr.Register(fx);
        fx.passes.clear();

        // Create Generic Wireframe

        pass0.name = "Wireframe";
        pass0.filenames[R::Shader::VERTEX]      = "Shaders\\wireframe.vert";
        pass0.filenames[R::Shader::FRAGMENT]    = "Shaders\\wireframe.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "Shaders\\wireframe.geom";
        pass0.state.SetDefault();
        pass0.state.depth.func = GL_LEQUAL;
        pass0.state.raster.lineWidth = 2.0f;
        pass0.type = DEFAULT;
        pass0.flags = 0;

        fx.name = "GenericWireframe";
        fx.passes.push_back(pass0);
        effectMgr.Register(fx);
        fx.passes.clear();

        // Diffuse Texture only

        pass0.name = "TexDiffuse";
        pass0.filenames[R::Shader::VERTEX]      = "Shaders\\textured.vert";
        pass0.filenames[R::Shader::FRAGMENT]    = "Shaders\\textured.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass0.state.SetDefault();
        pass0.state.blend.enabled = true;
        pass0.state.blend.srcFactor = GL_SRC_ALPHA;
        pass0.state.blend.dstFactor = GL_ONE_MINUS_SRC_ALPHA;
        pass0.type = DEFAULT;
        pass0.flags = USE_TEX_DIFFUSE | USE_MATERIAL;

        fx.name = "TexDiffuse";
        fx.passes.push_back(pass0);
        effectMgr.Register(fx);
        fx.passes.clear();

        // Face Ring, move diffuse texture

        pass0.name = "FaceRing";
        pass0.filenames[R::Shader::VERTEX]      = "Shaders\\facering.vert";
        pass0.filenames[R::Shader::FRAGMENT]    = "Shaders\\facering.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass0.state.SetDefault();
        pass0.state.blend.enabled = true;
        pass0.state.blend.srcFactor = GL_SRC_ALPHA;
        pass0.state.blend.dstFactor = GL_ONE_MINUS_SRC_ALPHA;
        pass0.type = DEFAULT;
        pass0.flags = USE_TEX_DIFFUSE | USE_TIME | USE_MATERIAL;

        fx.name = "FaceRing";
        fx.passes.push_back(pass0);
        effectMgr.Register(fx);
        fx.passes.clear();

        // DefaultLit Effect
        // lit

        pass0.name = "Ambient";
        pass0.filenames[R::Shader::VERTEX]      = "Shaders\\lit_pass0.vert";
        pass0.filenames[R::Shader::FRAGMENT]    = "Shaders\\lit_pass0.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass0.state.SetDefault();
        pass0.state.blend.enabled = GL_TRUE;
        pass0.state.blend.srcFactor = GL_SRC_ALPHA;
        pass0.state.blend.dstFactor = GL_ONE_MINUS_SRC_ALPHA;
        pass0.state.color = Color(0.0f, 0.0f, 0.0f, 0.0f);
        pass0.type = DEFAULT;
        pass0.flags = USE_COLOR;

        pass1.name = "Light0";
        pass1.filenames[R::Shader::VERTEX]      = "Shaders\\lit_pass1.vert";
        pass1.filenames[R::Shader::FRAGMENT]    = "Shaders\\lit_pass1.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass1.state.SetDefault();
        pass1.state.blend.enabled = GL_TRUE;
        pass1.state.blend.srcFactor = GL_SRC_ALPHA;
        pass1.state.blend.dstFactor = GL_ONE_MINUS_SRC_ALPHA;
        pass1.state.depth.maskEnabled = GL_FALSE;
        pass1.state.depth.func = GL_EQUAL;
        pass1.type = FIRST_LIGHT_PASS;
        pass1.flags = 0;

        pass2.name = "LightN";
        pass2.filenames[R::Shader::VERTEX]      = "Shaders\\lit_pass1.vert";
        pass2.filenames[R::Shader::FRAGMENT]    = "Shaders\\lit_pass1.frag";
        pass0.filenames[R::Shader::GEOMETRY]    = "";
        pass2.state.SetDefault();
        pass2.state.blend.enabled = GL_TRUE;
        pass2.state.blend.srcFactor = GL_SRC_ALPHA;
        pass2.state.blend.dstFactor = GL_ONE;
        pass2.state.depth.maskEnabled = GL_FALSE;
        pass2.state.depth.func = GL_EQUAL;
        pass2.type = LIGHT_PASS;
        pass2.flags = 0;

        fx.name = "Lit";
        fx.passes.push_back(pass0);
        fx.passes.push_back(pass1);
        fx.passes.push_back(pass2);
        effectMgr.Register(fx);
        fx.passes.clear();
    }

} // namespace R