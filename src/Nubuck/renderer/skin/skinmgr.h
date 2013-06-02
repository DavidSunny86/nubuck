#pragma once

#include <system\locks\spinlock.h>

#include "skin.h"

namespace R {

    class SkinMgr {
    private:
        struct SkinData {
            SkinData *prev, *next;

            int             refCount;
            SYS::SpinLock   refCountLock;
        
            void IncRef(void);
            void DecRef(void);
        };

        SYS::SpinLock _dataLock;

        SkinData* _skins;
    public:
        class Handle {
            friend class SkinMgr;
        private:
            SkinData* _res;
            SkinData* Res(void);
            Handle(SkinData* const res);
        public:
            Handle(void);
            ~Handle(void);
            Handle(const Handle& other);
            Handle& operator=(const Handle& other);
            bool IsValid(void) const;
        };

        typedef Handle handle_t;

        SkinMgr(void);

        handle_t Create(const Skin& skin);
    };

    extern SkinMgr skinMgr;

} // namespace R