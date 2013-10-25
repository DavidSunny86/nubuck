#include <algorithm>
#include <vector>

#include <generic\pointer.h>
#include <common\common.h>
#include <renderer\glcall.h>
#include <renderer\metrics\metrics.h>
#include <renderer\mesh\staticbuffer.h>
#include <renderer\mesh\mesh.h>
#include "giant_buffer.h"

namespace R {

struct GB_BufSeg {
    GB_BufSeg *prev, *next;
    unsigned    off;
    unsigned    size;
    bool        cached;
};

struct GB_MemItem {
    Mesh::Vertex*   vertices;
    unsigned        numVertices;
    GB_BufSeg*      bufSeg;
};

static std::vector<GB_MemItem> memItems;

static GLuint       giantBufferId   = 0;
static unsigned     giantBufferSize = 0;
static GB_BufSeg*   usedList        = NULL;
static GB_BufSeg*   freeList        = NULL;

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

static void Unlink(GB_BufSeg* it) {
    assert(it);
    if(it->prev) it->prev->next = it->next;
    if(it->next) it->next->prev = it->prev;
    if(freeList == it) freeList = it->next;
    if(usedList == it) usedList = it->next;
    it->prev = it->next = NULL;
}

static void Prepend(GB_BufSeg** list, GB_BufSeg* it) {
    assert(list && it);
    GB_BufSeg* head = *list;
    it->next = head;
    it->prev = NULL;
    if(head) head->prev = it;
    *list = it;
}

typedef bool (*cmpBufSeg_t)(const GB_BufSeg* lhp, const GB_BufSeg* rhp);

static bool Cmp_Offset(const GB_BufSeg* lhp, const GB_BufSeg* rhp) {
    return lhp->off < rhp->off;
}

static void Sort(GB_BufSeg** list, cmpBufSeg_t cmp) {
    std::vector<GB_BufSeg*> bufSegs;
    GB_BufSeg *next, *it = *list;
    while(it) {
        next = it->next;
        it->prev = it->next = NULL;
        bufSegs.push_back(it);
        it = next;
    }
    std::sort(bufSegs.begin(), bufSegs.end(), cmp);
    *list = NULL;
    for(std::vector<GB_BufSeg*>::reverse_iterator revIt(bufSegs.rbegin()); bufSegs.rend() != revIt; ++revIt)
        Prepend(list, *revIt);
}

static void CoalesceFreeMem(void) {
    Sort(&freeList, Cmp_Offset);
    GB_BufSeg *tmp, *next, *it = freeList;
    while(it) {
        next = it->next;
        while(next && (it->off + it->size == next->off)) {
            it->size += next->size;
            tmp = next->next;
            Unlink(next);
            delete next;
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
    return handle;
}

void GB_FreeMemItem(gbHandle_t handle) {
    assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    memItem.bufSeg = NULL;
    if(bufSeg) {
        Unlink(bufSeg);
        Prepend(&freeList, bufSeg);
    }
    CoalesceFreeMem();
    PrintInfo();
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
    assert(0 == giantBufferId);
    GL_CALL(glGenBuffers(1, &giantBufferId));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, giantBufferId));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_DYNAMIC_DRAW));
    metrics.resources.totalVertexBufferSize += size;
    giantBufferSize = size;
}

static void DeleteGiantBuffer(void) {
    assert(0 != giantBufferId);
    GL_CALL(glDeleteBuffers(1, &giantBufferId));
    metrics.resources.totalVertexBufferSize -= giantBufferSize;
    giantBufferId = 0;
    giantBufferSize = 0;
}

static void InvalidateSegs(void) {
    GB_BufSeg* it = usedList;
    while(it) {
        it->cached = false;
        it = it->next;
    }
}

static void Resize(unsigned size) {
    assert(giantBufferSize < size);

    GB_BufSeg* bufSeg = new GB_BufSeg;
    bufSeg->off = giantBufferSize;
    bufSeg->size = size - giantBufferSize;
    bufSeg->cached = false;

    Prepend(&freeList, bufSeg);

    if(0 != giantBufferId) DeleteGiantBuffer(); 
    AllocateGiantBuffer(size);

    InvalidateSegs();
}

static GB_BufSeg* GB_AllocBufSeg(unsigned size) {
    GB_BufSeg* bufSeg = FindFreeBufSeg(size);
    if(!bufSeg) {
        Resize(giantBufferSize + size);
        bufSeg = FindFreeBufSeg(size);
    }
    assert(bufSeg);
    assert(size <= bufSeg->size);
    Unlink(bufSeg);
    Prepend(&usedList, bufSeg);
    PrintInfo();
    bufSeg->cached = false;
    return bufSeg;
}

static void GB_ForceCacheAll(void) {
    for(unsigned i = 0; i < memItems.size(); ++i) {
        if(memItems[i].bufSeg) {
            GB_BufSeg* bufSeg = memItems[i].bufSeg;
            unsigned size = sizeof(Mesh::Vertex) * memItems[i].numVertices;
            void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, bufSeg->off, bufSeg->size, GL_MAP_WRITE_BIT);
            assert(ptr);
            memcpy(ptr, memItems[i].vertices, size);
            GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
            bufSeg->cached = true;
        }
    }
}

void GB_CacheAll(void) {
    for(unsigned i = 0; i < memItems.size(); ++i) {
        if(memItems[i].bufSeg) {
            GB_BufSeg* bufSeg = memItems[i].bufSeg;
            if(bufSeg && !bufSeg->cached) {
                unsigned size = sizeof(Mesh::Vertex) * memItems[i].numVertices;
                void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, bufSeg->off, bufSeg->size, GL_MAP_WRITE_BIT);
                assert(ptr);
                memcpy(ptr, memItems[i].vertices, size);
                GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
                bufSeg->cached = true;
            }
        }
    }
}

static void GB_Cache(gbHandle_t handle) {
    assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    assert(bufSeg);
    assert(sizeof(Mesh::Vertex) * memItem.numVertices <= bufSeg->size);
    assert(bufSeg->off + bufSeg->size <= giantBufferSize);
    if(!bufSeg->cached) {
        unsigned size = sizeof(Mesh::Vertex) * memItem.numVertices;
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, bufSeg->off, bufSeg->size, GL_MAP_WRITE_BIT);
        assert(ptr);
        memcpy(ptr, memItem.vertices, size);
        GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
        bufSeg->cached = true;
    }
}

void GB_Touch(gbHandle_t handle) {
    assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    if(!memItem.bufSeg) memItem.bufSeg = GB_AllocBufSeg(sizeof(Mesh::Vertex) * memItem.numVertices);
}

unsigned GB_GetOffset(gbHandle_t handle) {
    assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    assert(bufSeg);
    return bufSeg->off;
}

void GB_Invalidate(gbHandle_t handle) {
    assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    GB_BufSeg* bufSeg = memItem.bufSeg;
    if(bufSeg) bufSeg->cached = false;
}

void GB_Bind(void) {
    if(0 < giantBufferId) {
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, giantBufferId));
    }
}

} // namespace R