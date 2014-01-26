#pragma once

#include <limits.h>
#include <renderer\mesh\mesh.h>

namespace R {

#define GB_INVALID_HANDLE UINT_MAX

typedef unsigned gbHandle_t;

gbHandle_t  GB_AllocMemItem(Mesh::Vertex* const vertices, unsigned numVertices);
void        GB_FreeMemItem(gbHandle_t handle);
void        GB_Touch(gbHandle_t handle); 
void        GB_CacheBuffer();
unsigned    GB_GetOffset(gbHandle_t handle);
void        GB_Invalidate(gbHandle_t handle);
void        GB_Bind(void);
bool        GB_IsCached(gbHandle_t handle);

} // namespace R