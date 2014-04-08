#include <algorithm>
#include <vector>

#include <Nubuck\generic\pointer.h>
#include <Nubuck\common\common.h>
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
    unsigned        touched;
    bool            dead;
};

static std::vector<GB_MemItem> memItems;

static GLuint       giantBufferId       = 0;
static unsigned     giantBufferPSize 	= 0; // physical size
static unsigned     giantBufferLSize    = 0; // logical size
static GB_BufSeg*   usedList            = NULL;
static GB_BufSeg*   freeList        	= NULL;

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
        it->cached = false;
        it = it->next;
    }
}

static void Resize(unsigned size) {
    COM_assert(giantBufferLSize < size);

    GB_BufSeg* bufSeg = new GB_BufSeg;
    bufSeg->off = giantBufferLSize;
    bufSeg->size = size - giantBufferLSize;
    bufSeg->cached = false;

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

    GB_BufSeg* rseg = new GB_BufSeg;

    rseg->off = lseg->off + size;
    rseg->size = lseg->size - size;
    rseg->cached = false;

    lseg->size = size;
    lseg->cached = false;

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
            COM_assert(ptr);
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
                COM_assert(ptr);
                memcpy(ptr, memItems[i].vertices, size);
                GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
                bufSeg->cached = true;
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
    if(!bufSeg->cached) {
        unsigned size = sizeof(Mesh::Vertex) * memItem.numVertices;
        void* ptr = glMapBufferRange(GL_ARRAY_BUFFER, bufSeg->off, bufSeg->size, GL_MAP_WRITE_BIT);
        COM_assert(ptr);
        memcpy(ptr, memItem.vertices, size);
        GL_CALL(glUnmapBuffer(GL_ARRAY_BUFFER));
        bufSeg->cached = true;
    }
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
    if(bufSeg) bufSeg->cached = false;
}

void GB_Bind(void) {
    if(0 < giantBufferId) {
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, giantBufferId));
    }
}

bool GB_IsCached(gbHandle_t handle) {
    COM_assert(GB_INVALID_HANDLE != handle);
    GB_MemItem& memItem = memItems[handle];
    return memItem.bufSeg && memItem.bufSeg->cached;
}

} // namespace R