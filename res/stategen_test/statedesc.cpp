/*                                                             
this file is automatically generated.                          
changes will be overwritten.                                   
*/                                                             
#include <stddef.h>                                            
#include <renderer\effects\statedesc.h>                      
#include <renderer\effects\state.h>                          
                                                               
StateFieldDesc g_stateDesc[] = {                               
{ "state", 0, 1, 6, SFT_STRUCT }, 
{ "culling", offsetof(R::State, culling) + 0, 7, 8, SFT_STRUCT }, 
{ "blend", offsetof(R::State, blend) + 0, 9, 11, SFT_STRUCT }, 
{ "depth", offsetof(R::State, depth) + 0, 12, 14, SFT_STRUCT }, 
{ "stencil", offsetof(R::State, stencil) + 0, 15, 17, SFT_STRUCT }, 
{ "raster", offsetof(R::State, raster) + 0, 18, 18, SFT_STRUCT }, 
{ "color", offsetof(R::State, color) + 0, 19, 19, SFT_STRUCT }, 
{ "hw", offsetof(R::State::Culling, hw) + offsetof(R::State, culling) + 0, 20, 21, SFT_STRUCT }, 
{ "sw", offsetof(R::State::Culling, sw) + offsetof(R::State, culling) + 0, 22, 24, SFT_STRUCT }, 
{ "enabled", offsetof(R::State::Blending, enabled) + offsetof(R::State, blend) + 0, 25, 24, SFT_BOOL }, 
{ "srcFactor", offsetof(R::State::Blending, srcFactor) + offsetof(R::State, blend) + 0, 25, 24, SFT_ENUM }, 
{ "dstFactor", offsetof(R::State::Blending, dstFactor) + offsetof(R::State, blend) + 0, 25, 24, SFT_ENUM }, 
{ "enabled", offsetof(R::State::DepthBuffer, enabled) + offsetof(R::State, depth) + 0, 25, 24, SFT_BOOL }, 
{ "maskEnabled", offsetof(R::State::DepthBuffer, maskEnabled) + offsetof(R::State, depth) + 0, 25, 24, SFT_BOOL }, 
{ "func", offsetof(R::State::DepthBuffer, func) + offsetof(R::State, depth) + 0, 25, 24, SFT_ENUM }, 
{ "enabled", offsetof(R::State::StencilBuffer, enabled) + offsetof(R::State, stencil) + 0, 25, 24, SFT_BOOL }, 
{ "func", offsetof(R::State::StencilBuffer, func) + offsetof(R::State, stencil) + 0, 25, 27, SFT_STRUCT }, 
{ "op", offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 28, 29, SFT_STRUCT }, 
{ "lineWidth", offsetof(R::State::Rasterizing, lineWidth) + offsetof(R::State, raster) + 0, 30, 29, SFT_FLOAT }, 
{ "maskEnabled", offsetof(R::State::ColorBuffer, maskEnabled) + offsetof(R::State, color) + 0, 30, 33, SFT_STRUCT }, 
{ "enabled", offsetof(R::State::Culling::Hardware, enabled) + offsetof(R::State::Culling, hw) + offsetof(R::State, culling) + 0, 34, 33, SFT_BOOL }, 
{ "cullFace", offsetof(R::State::Culling::Hardware, cullFace) + offsetof(R::State::Culling, hw) + offsetof(R::State, culling) + 0, 34, 33, SFT_ENUM }, 
{ "enabled", offsetof(R::State::Culling::Software, enabled) + offsetof(R::State::Culling, sw) + offsetof(R::State, culling) + 0, 34, 33, SFT_BOOL }, 
{ "cullFace", offsetof(R::State::Culling::Software, cullFace) + offsetof(R::State::Culling, sw) + offsetof(R::State, culling) + 0, 34, 33, SFT_ENUM }, 
{ "alpha", offsetof(R::State::Culling::Software, alpha) + offsetof(R::State::Culling, sw) + offsetof(R::State, culling) + 0, 34, 33, SFT_FLOAT }, 
{ "func", offsetof(R::State::StencilBuffer::Func, func) + offsetof(R::State::StencilBuffer, func) + offsetof(R::State, stencil) + 0, 34, 33, SFT_ENUM }, 
{ "ref", offsetof(R::State::StencilBuffer::Func, ref) + offsetof(R::State::StencilBuffer, func) + offsetof(R::State, stencil) + 0, 34, 33, SFT_INT }, 
{ "mask", offsetof(R::State::StencilBuffer::Func, mask) + offsetof(R::State::StencilBuffer, func) + offsetof(R::State, stencil) + 0, 34, 33, SFT_UINT }, 
{ "front", offsetof(R::State::StencilBuffer::Op, front) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 34, 36, SFT_STRUCT }, 
{ "back", offsetof(R::State::StencilBuffer::Op, back) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 37, 39, SFT_STRUCT }, 
{ "red", offsetof(R::State::ColorBuffer::Mask, red) + offsetof(R::State::ColorBuffer, maskEnabled) + offsetof(R::State, color) + 0, 40, 39, SFT_BOOL }, 
{ "green", offsetof(R::State::ColorBuffer::Mask, green) + offsetof(R::State::ColorBuffer, maskEnabled) + offsetof(R::State, color) + 0, 40, 39, SFT_BOOL }, 
{ "blue", offsetof(R::State::ColorBuffer::Mask, blue) + offsetof(R::State::ColorBuffer, maskEnabled) + offsetof(R::State, color) + 0, 40, 39, SFT_BOOL }, 
{ "alpha", offsetof(R::State::ColorBuffer::Mask, alpha) + offsetof(R::State::ColorBuffer, maskEnabled) + offsetof(R::State, color) + 0, 40, 39, SFT_BOOL }, 
{ "fail", offsetof(R::State::StencilBuffer::Op::Front, fail) + offsetof(R::State::StencilBuffer::Op, front) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 40, 39, SFT_ENUM }, 
{ "zfail", offsetof(R::State::StencilBuffer::Op::Front, zfail) + offsetof(R::State::StencilBuffer::Op, front) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 40, 39, SFT_ENUM }, 
{ "zpass", offsetof(R::State::StencilBuffer::Op::Front, zpass) + offsetof(R::State::StencilBuffer::Op, front) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 40, 39, SFT_ENUM }, 
{ "fail", offsetof(R::State::StencilBuffer::Op::Back, fail) + offsetof(R::State::StencilBuffer::Op, back) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 40, 39, SFT_ENUM }, 
{ "zfail", offsetof(R::State::StencilBuffer::Op::Back, zfail) + offsetof(R::State::StencilBuffer::Op, back) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 40, 39, SFT_ENUM }, 
{ "zpass", offsetof(R::State::StencilBuffer::Op::Back, zpass) + offsetof(R::State::StencilBuffer::Op, back) + offsetof(R::State::StencilBuffer, op) + offsetof(R::State, stencil) + 0, 40, 39, SFT_ENUM }, 
{ NULL, 0, 0, 0, 0 }                                           
};                                                             
