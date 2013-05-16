#pragma once

#include <system\locks\spinlock.h>

#ifndef NULL
#define NULL 0
#endif

namespace GEN {

    struct RefCount {
        unsigned val;

        void Inc(void) { val++; }
        unsigned Dec(void) { return --val; }
    };

    template<typename LOCK>
    struct LockedRefCount {
        unsigned    val;
        LOCK        lck;

        void Inc(void) {
            lck.Lock();
            val++;
            lck.Unlock();
        }

        unsigned Dec(void) {
            lck.Lock();
            unsigned ret = --val;
            lck.Unlock();
            return ret;
        }
    };

    template<
        typename TYPE, 
        typename REFCNT = RefCount
    >
    class ResPointer {
        template<typename, typename> friend class ResPointer;
    private:
        TYPE*   _raw;
        REFCNT* _cnt;
    public:
        ResPointer(void);
        ResPointer(const ResPointer& other);
        template<typename IMPLICIT> 
        ResPointer(const ResPointer<IMPLICIT, REFCNT>& other);
        ResPointer(TYPE* const raw);

        ResPointer& operator=(const ResPointer& other);
        template<typename IMPLICIT>
        ResPointer& operator=(const ResPointer<IMPLICIT, REFCNT>& other);

        void Drop(void);

        const TYPE*	Raw(void) const;
        TYPE*		Raw(void);
        unsigned	Count(void) const;
    
        bool		IsValid(void) const;

        const TYPE*	operator->(void) const;
        const TYPE&	operator*(void) const;
        TYPE*		operator->(void);
        TYPE&		operator*(void);
    };

    template<typename TYPE, typename REFCNT>
    inline ResPointer<TYPE, REFCNT>::ResPointer(void) : _raw(NULL), _cnt(NULL) { }

    template<typename TYPE, typename REFCNT>
    inline ResPointer<TYPE, REFCNT>::ResPointer(const ResPointer& other) : _raw(other._raw), _cnt(other._cnt) {
        if(_raw) _cnt->Inc();
    }

    template<typename TYPE, typename REFCNT>
    template<typename IMPLICIT>
    inline ResPointer<TYPE, REFCNT>::ResPointer(const ResPointer<IMPLICIT, REFCNT>& other) : _raw(other._raw), _cnt(other._cnt) {
        if(_raw) _cnt->Inc();
    }

    template<typename TYPE, typename REFCNT>
    inline ResPointer<TYPE, REFCNT>::ResPointer(TYPE* const raw) : _raw(NULL), _cnt(NULL) {
        if(NULL != raw) {
            REFCOUNT* cnt = new REFCOUNT();
            _raw = raw;
            _cnt = cnt;
        }
    }

    template<typename TYPE, typename REFCNT>
    inline ResPointer<TYPE, REFCNT>& ResPointer<TYPE, REFCNT>::operator=(const ResPointer& other) {
        if(this != &other) {
            Drop();
            if(_raw = other._raw && _cnt = other._cnt) _cnt->Inc();
        }
        return *this;
    }

    template<typename TYPE, typename REFCNT>
    template<typename IMPLICIT>
    inline ResPointer<TYPE, REFCNT>& ResPointer<TYPE, REFCNT>::operator=(const ResPointer<IMPLICIT, REFCNT>& other) {
        if(this != &other) {
            Drop();
            if(_raw = other._raw && _cnt = other._cnt) _cnt->Inc();
        }
        return *this;
    }

    template<typename TYPE, typename REFCNT>
    inline void ResPointer<TYPE, REFCNT>::Drop(void) {
        if(_raw && !_cnt->Dec()) {
            delete _raw;
            delete _cnt;
        }
        _raw = _cnt = NULL;
    }

    template<typename TYPE, typename REFCNT>
    inline const TYPE* ResPointer<TYPE, REFCNT>::Raw(void) const {
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    inline TYPE* ResPointer<TYPE, REFCNT>::Raw(void){
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    inline unsigned ResPointer<TYPE, REFCNT>::Count(void) const {
        return *_cnt;
    }

    template<typename TYPE, typename REFCNT>
    inline bool ResPointer<TYPE, REFCNT>::IsValid(void) const {
        return NULL != _raw;
    }

    template<typename TYPE, typename REFCNT> 
    const TYPE* ResPointer<TYPE, REFCNT>::operator->(void) const {
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    const TYPE& ResPointer<TYPE, REFCNT>::operator*(void) const {
        return *_raw;
    }

    template<typename TYPE, typename REFCNT>
    TYPE* ResPointer<TYPE, REFCNT>::operator->(void) {
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    TYPE& ResPointer<TYPE, REFCNT>::operator*(void) {
        return *_raw;
    }

} // namespace GEN