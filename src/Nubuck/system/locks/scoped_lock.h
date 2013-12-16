#pragma once

#include <generic\uncopyable.h>
#include <system\locks\spinlock.h>

namespace SYS {

class ScopedLock : GEN::Uncopyable {
private:
    SpinLock& _lock;
public:
    explicit ScopedLock(SpinLock& lock) : _lock(lock) {
        _lock.Lock();
    }
    ~ScopedLock() { _lock.Unlock(); }
};

} // namespace SYS