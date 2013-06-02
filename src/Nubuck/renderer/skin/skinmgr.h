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

            SkinDesc    skinDesc;
            Skin        skin;
            bool        compiled;
        
            void IncRef(void);
            void DecRef(void);
        };

        SYS::SpinLock _dataLock;

        SkinData* _skins;

        void Link(SkinData* skinData);
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

        handle_t Create(const SkinDesc& skin);

        void R_Compile(handle_t& handle);
        void R_Bind(Program& program, handle_t& handle);
    };

    extern SkinMgr skinMgr;

} // namespace R