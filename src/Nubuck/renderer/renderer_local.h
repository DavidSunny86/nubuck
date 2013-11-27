#pragma once

#include <common\config\config.h>
#include <renderer\glew\glew.h>
#include <renderer\glcall.h>

extern COM::Config::Variable<float>	cvar_r_nodeSize;
extern COM::Config::Variable<float> cvar_r_edgeRadius;

enum {
    IN_POSITION     = 0,
    IN_NORMAL       = 1,
    IN_COLOR        = 2,
    IN_TEXCOORDS    = 3,

    // encodes 3x4 matrix
    IN_A0           = 4,
    IN_A1           = 5,
    IN_A2           = 6,
    IN_A3           = 7,

    IN_HALF_HEIGHT_SQ   = 8,
    IN_RADIUS_SQ        = 9
};

namespace R {

struct  State;
class   Program;

void Uniforms_BindBuffers(void);
void Uniforms_BindBlocks(const Program& prog);

void SetState(const State& state);

} // namespace R