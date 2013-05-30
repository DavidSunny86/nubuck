#include <common\common.h>
#include "meshmgr.h"

namespace R {

    MeshMgr meshMgr;

    void MeshMgr::Link(vrtData_t* vrtData) {
        _dataLock.Lock();
        vrtData->prev = NULL;
        if(vrtData->next = _vertices)
            vrtData->next->prev = vrtData;
        _vertices = vrtData;
        _dataLock.Unlock();
    }

    void MeshMgr::Link(idxData_t* idxData) {
        _dataLock.Lock();
        idxData->prev = NULL;
        if(idxData->next = _indices)
            idxData->next->prev = idxData;
        _indices = idxData;
        _dataLock.Unlock();
    }

    void MeshMgr::FreeUnused(vrtData_t* vrtData) {
        vrtData_t *prev, *next;
        int numFreed = 0;
        while(vrtData) {
            prev = vrtData->prev;
            next = vrtData->next;
            if(0 >= vrtData->refCount) {
                if(next) next->prev = prev;
                if(prev) prev->next = next;
                else _vertices = next;

                delete vrtData;
                numFreed++;
            }
            vrtData = next;
        }

        if(numFreed) common.printf("INFO - freed %d vertex data objects.\n", numFreed);
    }

    void MeshMgr::FreeUnused(idxData_t* idxData) {
        idxData_t *prev, *next;
        int numFreed = 0;
        while(idxData) {
            prev = idxData->prev;
            next = idxData->next;
            if(0 >= idxData->refCount) {
                if(next) next->prev = prev;
                if(prev) prev->next = next;
                else _indices = next;

                delete idxData;
                numFreed++;
            }
            idxData = next;
        }

        if(numFreed) common.printf("INFO - freed %d index data objects.\n", numFreed);
    }

    MeshMgr::MeshMgr(void) : _vertices(NULL), _indices(NULL) { }

    void MeshMgr::R_FrameUpdate(void) {
        _dataLock.Lock();
        FreeUnused(_vertices);
        FreeUnused(_indices);
        _dataLock.Unlock();
    }

} // namespace R
