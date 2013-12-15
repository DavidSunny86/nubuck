#pragma once

#include <list>
#include <generic\pointer.h>
#include <generic\singleton.h>
#include <system\locks\spinlock.h>

namespace SYS {

template<typename TYPE>
class SharedResources : public GEN::Singleton<SharedResources<TYPE> > {
    friend class GEN::Singleton<SharedResources>;
private:
    std::list<GEN::Pointer<TYPE> >  _res;
    SpinLock                        _resMtx;
public:
    GEN::Pointer<TYPE> Acquire(void) {
        GEN::Pointer<TYPE> r;
        _resMtx.Lock();
        if(_res.empty()) r = GEN::MakePtr(new TYPE());
        else {
            r = _res.back();
            _res.pop_back();
        }
        _resMtx.Unlock();
        return r;
    }

    void Release(GEN::Pointer<TYPE>& r) {
        _resMtx.Lock();
        _res.push_back(r);
        _resMtx.Unlock();
        r.Drop();
    }
};

template<typename TYPE>
class SharedResource : private GEN::Uncopyable {
private:
    GEN::Pointer<TYPE> _res;
public:
    SharedResource() : _res(SharedResources<TYPE>::Instance().Acquire()) { }
    ~SharedResource() { SharedResources<TYPE>::Instance().Release(_res); }

    const TYPE& Resource() const { return *_res; }
    TYPE& Resource() { return *_res; }
};

} // namespace SYS