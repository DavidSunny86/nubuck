#pragma once

#ifndef NULL
#define NULL 0
#endif

namespace GEN {

    namespace PointerImpl {

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

    } // namespace PointerImpl

    template<
        typename TYPE, 
        typename REFCNT = PointerImpl::RefCount
    >
    class Pointer {
        template<typename, typename> friend class Pointer;
    private:
        TYPE*   _raw;
        REFCNT* _cnt;
    public:
        Pointer(void);
        Pointer(const Pointer& other);
        template<typename IMPLICIT> 
        Pointer(const Pointer<IMPLICIT, REFCNT>& other);
        Pointer(TYPE* const raw);
        ~Pointer(void);

        Pointer& operator=(const Pointer& other);
        template<typename IMPLICIT>
        Pointer& operator=(const Pointer<IMPLICIT, REFCNT>& other);

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
    inline Pointer<TYPE, REFCNT>::Pointer(void) : _raw(NULL), _cnt(NULL) { }

    template<typename TYPE, typename REFCNT>
    inline Pointer<TYPE, REFCNT>::Pointer(const Pointer& other) : _raw(other._raw), _cnt(other._cnt) {
        if(_raw) _cnt->Inc();
    }

    template<typename TYPE, typename REFCNT>
    template<typename IMPLICIT>
    inline Pointer<TYPE, REFCNT>::Pointer(const Pointer<IMPLICIT, REFCNT>& other) : _raw(other._raw), _cnt(other._cnt) {
        if(_raw) _cnt->Inc();
    }

    template<typename TYPE, typename REFCNT>
    inline Pointer<TYPE, REFCNT>::Pointer(TYPE* const raw) : _raw(NULL), _cnt(NULL) {
        if(NULL != raw) {
            REFCNT* cnt = new REFCNT();
            cnt->val = 1;
            _raw = raw;
            _cnt = cnt;
        }
    }

    template<typename TYPE, typename REFCNT>
    inline Pointer<TYPE, REFCNT>::~Pointer(void) {
        Drop();
    }

    template<typename TYPE, typename REFCNT>
    inline Pointer<TYPE, REFCNT>& Pointer<TYPE, REFCNT>::operator=(const Pointer& other) {
        if(this != &other) {
            Drop();
            if((_raw = other._raw) && (_cnt = other._cnt)) _cnt->Inc();
        }
        return *this;
    }

    template<typename TYPE, typename REFCNT>
    template<typename IMPLICIT>
    inline Pointer<TYPE, REFCNT>& Pointer<TYPE, REFCNT>::operator=(const Pointer<IMPLICIT, REFCNT>& other) {
        if(this != &other) {
            Drop();
            if((_raw = other._raw) && (_cnt = other._cnt)) _cnt->Inc();
        }
        return *this;
    }

    template<typename TYPE, typename REFCNT>
    inline void Pointer<TYPE, REFCNT>::Drop(void) {
        if(_raw && !_cnt->Dec()) {
            delete _raw;
            delete _cnt;
        }
        _raw = NULL;
        _cnt = NULL;
    }

    template<typename TYPE, typename REFCNT>
    inline const TYPE* Pointer<TYPE, REFCNT>::Raw(void) const {
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    inline TYPE* Pointer<TYPE, REFCNT>::Raw(void){
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    inline unsigned Pointer<TYPE, REFCNT>::Count(void) const {
        return *_cnt;
    }

    template<typename TYPE, typename REFCNT>
    inline bool Pointer<TYPE, REFCNT>::IsValid(void) const {
        return NULL != _raw;
    }

    template<typename TYPE, typename REFCNT> 
    const TYPE* Pointer<TYPE, REFCNT>::operator->(void) const {
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    const TYPE& Pointer<TYPE, REFCNT>::operator*(void) const {
        return *_raw;
    }

    template<typename TYPE, typename REFCNT>
    TYPE* Pointer<TYPE, REFCNT>::operator->(void) {
        return _raw;
    }

    template<typename TYPE, typename REFCNT>
    TYPE& Pointer<TYPE, REFCNT>::operator*(void) {
        return *_raw;
    }

} // namespace GEN