#include <assert.h>
#include <algorithm>
#include <vector>

#include <Nubuck\generic\pointer.h>
#include <Nubuck\generic\page_alloc.h>
#include <Nubuck\common\common.h>
#include <renderer\glcall.h>
#include <renderer\metrics\metrics.h>
#include <renderer\mesh\staticbuffer.h>
#include <renderer\mesh\mesh.h>
#include "giant_buffer.h"

namespace R {

struct GB_PatchCmd {
    GB_PatchCmd*    next;
    unsigned        off;
    unsigned        size;
};

struct GB_BufSeg {
    GB_BufSeg       *prev, *next;
    unsigned        off;
    unsigned        size;
    GB_PatchCmd*    cmds;
};

struct GB_MemItem {
    Mesh::Vertex*   vertices;
    unsigned        numVertices;
    GB_BufSeg*      bufSeg;
    unsigned        touched;
    bool            dead;
};

static GEN::PageAlloc<GB_PatchCmd>  patchCmdAlloc;
static GEN::PageAlloc<GB_BufSeg>    bufSegAlloc;

static std::vector<GB_MemItem> memItems;

static GLuint       giantBufferId       = 0;
static unsigned     giantBufferPSize 	= 0; // physical size
static unsigned     giantBufferLSize    = 0; // logical size
static GB_BufSeg*   usedList            = NULL;
static GB_BufSeg*   freeList        	= NULL;

static bool IsSorted(GB_PatchCmd* list) {
    GB_PatchCmd *it = list, *next;
    while(it && (next = it->next)) {
        if(it->off + it->size > next->off)
            return false;
        it = next;
    }
    // a->off + a->size <= b->off + b->size forall a before b
    return true;
}

static void InsertPatchCmd(GB_PatchCmd** list, unsigned off, unsigned size) {
    COM_assert(list && IsSorted(*list));
    GB_PatchCmd *prev = NULL, *next = *list;
    while(next && off > next->off) {
        prev = next;
        next = next->next;
    }
    COM_assert(!prev || prev->off <= off);
    COM_assert(!next || off <= next->off);

    GB_PatchCmd* it = patchCmdAlloc.Malloc();
    COM_assert(it);
    it->next = next;
    it->off = off;
    it->size = size;
    if(prev) {
        if(prev->off + prev->size >= it->off) {
            prev->size = M::Max(prev->size, it->off - prev->off + it->size);
            it = prev;
        } else prev->next = it;
    } else *list = it;

    COM_assert(NULL != it && next == it->next);
    while(next && it->off + it->size >= next->off) {
        COM_assert(it->off <= next->off);
        it->size = M::Max(it->size, next->off - it->off + next->size);
        it->next = next->next;
        patchCmdAlloc.Free(next);
        next = it->next;
    }

    COM_assert(IsSorted(*list));
}

static void ClearPatchCmds(GB_PatchCmd** list) {
    COM_assert(list);
    GB_PatchCmd *next, *it = *list;
    while(it) {
        next = it->next;
        patchCmdAlloc.Free(it);
        it = next;
    }
    *list = NULL;
}

static void Invalidate(GB_BufSeg* bufSeg) {
    assert(bufSeg);
    ClearPatchCmds(&bufSeg->cmds);

    GB_PatchCmd* cmd = patchCmdAlloc.Malloc();
    cmd->next = NULL;
    cmd->off = 0;
    cmd->size = bufSeg->size;
    bufSeg->cmds = cmd;
}

static void ApplyPatchCmds(GB_MemItem* memItem) {
    GB_BufSeg* bufSeg = memItem->bufSeg;
    COM_assert(bufSeg);
    GB_PatchCmd* cmd = bufSeg->cmds;
    while(cmd) {
        unsigned size = sizeof(Mesh::Vertex) * memItem->numVertices;
        COM_assert(size >= cmd->size);
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, bufSeg->off + cmd->off, cmd->size, reinterpret_cast<char*>(memItem->vertices) + cmd->off));
        /*
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, bufSeg->off + cmd->off, cmd->size, GL_MAP_WRITE_BIT);
        COM_assert(ptr);
        memcpy(ptr, reinterpret_cast<char*>(memItem->vertices) + cmd->off, cmd->size);
        GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
        */
        cmd = cmd->next;
    }
    ClearPatchCmds(&bufSeg->cmds);
}

static void PrintInfo(void) {
    GB_BufSeg* it = NULL;
    common.printf("INFO - GiantBuffer {\n");
    it = usedList;
    while(it) {
        common.printf("\tused off=%d, size=%d\n", it->off, it->size);
        it = it->next;
    }
    it = freeList;
    while(it) {
        common.printf("\tfree off=%d, size=%d\n", it->off, it->size);
        it = it->next;
    }
    common.printf("} // GiantBuffer\n");
}

static bool IsSorted(GB_BufSeg* list) {
    GB_BufSeg *it = list, *next;
    while(it && (next = it->next)) {
        if(it->off + it->size > next->off)
            return false;
        it = next;
    }
    // a->off + a->size <= b->off + b->size forall a before b
    return true;
}

static void Unlink(GB_BufSeg* it) {
    COM_assert(it);
    if(it->prev) it->prev->next = it->next;
    if(it->next) it->next->prev = it->prev;
    if(freeList == it) freeList = it->next;
    if(usedList == it) usedList = it->next;
    it->prev = it->next = NULL;
}

static void Insert(GB_BufSeg** list, GB_BufSeg* it) {
    COM_assert(list && it && IsSorted(*list));
    GB_BufSeg *prev = NULL, *next = *list;
    while(next && it->off > next->off) {
        prev = next;
        next = next->next;
    }
    it->prev = prev;
    it->next = next;
    if(next) next->prev = it;
    if(prev) prev->next = it;
    if(*list == next) *list = it;
}

static void Prepend(GB_BufSeg** list, GB_BufSeg* it) {
    COM_assert(list && it);
    GB_BufSeg* head = *list;
    it->next = head;
    it->prev = NULL;
    if(head) head->prev = it;
    *list = it;
}

static void CoalesceFreeMem(void) {
    COM_assert(IsSorted(freeList));
    GB_BufSeg *tmp, *next, *it = freeList;
    while(it) {
        next = it->next;
        while(next && (it->off + it->size == next->off)) {
            it->size += next->size;
            tmp = next->next;
            Unlink(next);
            bufSegAlloc.Free(next);
            next = tmp;
        }
        it = next;
    }
}

gbHandle_t GB_AllocMemItem(Mesh::Vertex* const vertices, unsigned numVertices) {
    gbHandle_t handle = memItems.size();
    memItems.resize(memItems.size() + 1);
    GB_MemItem& memItem = memItems[handle];
    memItem.vertices    = vertices;
    memItem.numVertices = numVertices;
    memItem.bufSeg      = NULL;
    memItem.touched     = 0;
    memItem.dead        = false;
    return handle;
}

void GB_FreeMemItem(gbHandle_t handle) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    memItem.bufSeg = NULL;
    memItem.dead = true;
    if(bufSeg) {
        Unlink(bufSeg);
        Insert(&freeList, bufSeg);
    }
    CoalesceFreeMem();
}

static GB_BufSeg* FindFreeBufSeg(unsigned size) {
    GB_BufSeg* bufSeg = NULL;
    GB_BufSeg* it = freeList;
    while(it) {
        if(size <= it->size && (!bufSeg || bufSeg->size > it->size))
            bufSeg = it;
        it = it->next;
    }
    return bufSeg;
}

static void AllocateGiantBuffer(unsigned size) {
    COM_assert(0 == giantBufferId);
    GL_CALL(glGenBuffers(1, &giantBufferId));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, giantBufferId));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW));
    metrics.resources.totalVertexBufferSize += size;
    giantBufferPSize = size;
}

static void DeleteGiantBuffer(void) {
    COM_assert(0 != giantBufferId);
    GL_CALL(glDeleteBuffers(1, &giantBufferId));
    metrics.resources.totalVertexBufferSize -= giantBufferPSize;
    giantBufferId = 0;
    giantBufferPSize = 0;
}

static void InvalidateSegs(void) {
    GB_BufSeg* it = usedList;
    while(it) {
        Invalidate(it);
        it = it->next;
    }
}

static void Resize(unsigned size) {
    COM_assert(giantBufferLSize < size);

    GB_BufSeg* bufSeg = bufSegAlloc.Malloc();
    bufSeg->off = giantBufferLSize;
    bufSeg->size = size - giantBufferLSize;
    bufSeg->cmds = NULL;
    Invalidate(bufSeg);

    Prepend(&freeList, bufSeg);

    InvalidateSegs();

    giantBufferLSize = size;
}

static void ResizeBuffer() {
    if(giantBufferLSize > giantBufferPSize) {
        if(0 != giantBufferId) DeleteGiantBuffer();
        AllocateGiantBuffer(giantBufferLSize);
    }
}

static void Split(GB_BufSeg* lseg, unsigned size) {
    COM_assert(lseg->size >= size);
    if(lseg->size == size) return; // nothing to do

    GB_BufSeg* rseg = bufSegAlloc.Malloc();

    rseg->off = lseg->off + size;
    rseg->size = lseg->size - size;
    rseg->cmds = NULL;
    Invalidate(rseg);

    lseg->size = size;
    Invalidate(lseg);

    rseg->next = lseg->next;
    if(rseg->next) rseg->next->prev = rseg;
    rseg->prev = lseg;

    lseg->next = rseg;
}

static GB_BufSeg* GB_AllocBufSeg(unsigned size) {
    GB_BufSeg* bufSeg = FindFreeBufSeg(size);
    if(!bufSeg) {
        Resize(giantBufferLSize + size);
        bufSeg = FindFreeBufSeg(size);
    }
    COM_assert(bufSeg);
    COM_assert(size <= bufSeg->size);
    Split(bufSeg, size);
    COM_assert(bufSeg->size == size);
    Unlink(bufSeg);
    Prepend(&usedList, bufSeg);
    Invalidate(bufSeg);
    return bufSeg;
}

static void GB_ForceCacheAll(void) {
    for(unsigned i = 0; i < memItems.size(); ++i) {
        if(memItems[i].bufSeg) {
            GB_BufSeg* bufSeg = memItems[i].bufSeg;
            unsigned size = sizeof(Mesh::Vertex) * memItems[i].numVertices;
            void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, bufSeg->off, bufSeg->size, GL_MAP_WRITE_BIT);
            COM_assert(ptr);
            memcpy(ptr, memItems[i].vertices, size);
            GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
            ClearPatchCmds(&bufSeg->cmds);
        }
    }
}

void GB_CacheAll(void) {
    for(unsigned i = 0; i < memItems.size(); ++i) {
        if(memItems[i].bufSeg) {
            GB_BufSeg* bufSeg = memItems[i].bufSeg;
            if(bufSeg && bufSeg->cmds) {
                unsigned size = sizeof(Mesh::Vertex) * memItems[i].numVertices;
                void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, bufSeg->off, bufSeg->size, GL_MAP_WRITE_BIT);
                COM_assert(ptr);
                memcpy(ptr, memItems[i].vertices, size);
                GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
                ClearPatchCmds(&bufSeg->cmds);
            }
        }
    }
}

void GB_Cache(gbHandle_t handle) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    COM_assert(bufSeg);
    COM_assert(sizeof(Mesh::Vertex) * memItem.numVertices <= bufSeg->size);
    COM_assert(bufSeg->off + bufSeg->size <= giantBufferPSize);
    ApplyPatchCmds(&memItem);
}

void GB_Touch(gbHandle_t handle) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    memItem.touched = 1;
}

void GB_CacheBuffer() {
    for(unsigned i = 0; i < memItems.size(); ++i) {
        if(!memItems[i].dead && memItems[i].touched && !memItems[i].bufSeg)
            memItems[i].bufSeg = GB_AllocBufSeg(sizeof(Mesh::Vertex) * memItems[i].numVertices);
    }
    ResizeBuffer();
    COM_assert(giantBufferLSize <= giantBufferPSize);
    for(unsigned i = 0; i < memItems.size(); ++i) {
        if(!memItems[i].dead && memItems[i].touched) {
            GB_Cache(i);
            memItems[i].touched = 0;
        }
    }
}

unsigned GB_GetOffset(gbHandle_t handle) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    COM_assert(bufSeg);
    return bufSeg->off;
}

void GB_Invalidate(gbHandle_t handle) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    if(bufSeg) Invalidate(bufSeg);
}

void GB_Invalidate(gbHandle_t handle, unsigned off, unsigned size) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    if(off + size > sizeof(Mesh::Vertex) * memItem.numVertices) {
        __debugbreak();
    }
    COM_assert(off + size <= sizeof(Mesh::Vertex) * memItem.numVertices);
    GB_BufSeg* bufSeg = memItem.bufSeg;
    if(bufSeg) InsertPatchCmd(&bufSeg->cmds, off, size);
}

void GB_Bind(void) {
    if(0 < giantBufferId) {
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, giantBufferId));
    }
}

bool GB_IsCached(gbHandle_t handle) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    return memItem.bufSeg && !memItem.bufSeg->cmds;
}

} // namespace R