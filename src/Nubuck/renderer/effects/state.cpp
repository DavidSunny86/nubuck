#include "state.h"

namespace R {

void SetDefaultState(State& state) {
    state.culling.hw.enabled        = GL_FALSE;
    state.culling.hw.cullFace 		= GL_BACK;
    state.culling.sw.enabled  		= GL_FALSE;
    state.culling.sw.cullFace 		= GL_BACK;
    state.culling.sw.alpha    		= 0.0f;

    state.blend.enabled       		= GL_FALSE;
    state.blend.srcFactor     		= GL_ONE;
    state.blend.dstFactor     		= GL_ZERO;

    state.depth.enabled       		= GL_TRUE;
    state.depth.maskEnabled   		= GL_TRUE;
    state.depth.func          		= GL_LESS;

    state.stencil.enabled           = GL_FALSE;
    state.stencil.func.func   		= GL_ALWAYS;
    state.stencil.func.ref    		= 0;
    state.stencil.func.mask   		= ~0;
    state.stencil.op.front.fail     = GL_KEEP;
    state.stencil.op.front.zfail    = GL_KEEP;
    state.stencil.op.front.zpass    = GL_KEEP;
    state.stencil.op.back.fail      = GL_KEEP;
    state.stencil.op.back.zfail    	= GL_KEEP;
    state.stencil.op.back.zpass    	= GL_KEEP;

    state.raster.lineWidth          = 1.0f;

    state.raster.lineStipple.enabled    = GL_FALSE;
    state.raster.lineStipple.factor     = 1;
    state.raster.lineStipple.pattern    = 0x1111;

    state.color.maskEnabled.red     = GL_TRUE;
    state.color.maskEnabled.green   = GL_TRUE;
    state.color.maskEnabled.blue    = GL_TRUE;
    state.color.maskEnabled.alpha   = GL_TRUE;
}

} // namespace R