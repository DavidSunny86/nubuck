#include <assert.h>
#include <string>
#include <iostream>
#include <Nubuck\generic\page_alloc.h>
#include "timer/timer.h"
#include "page_alloc_type.h"

class CStdAlloc {
public:
    Type* Malloc() {
        return static_cast<Type*>(malloc(sizeof(Type)));
    }

    void Free(Type* ptr) {
        free(ptr);
    }
};

std::string ToString(const CStdAlloc&) {
    return "CStdAlloc";
}

template<typename TYPE, unsigned PAGE_SIZE>
std::string ToString(const GEN::PageAlloc<TYPE, PAGE_SIZE>&) {
    static char buffer[16];
    sprintf(buffer, "PageAlloc%d", PAGE_SIZE);
    return std::string(buffer);
}

template<typename ALLOC>
void random_allocs(unsigned m, unsigned n, ALLOC& alloc) {
    Type**      ptrs = new Type*[m];
    unsigned    sp = 0, maxSp = 0;
    unsigned    npush = 0, npop = 0;

    SYS::Timer timer;
    timer.Start();
    for(unsigned i = 0; i < n; ++i) {
        int coin = rand() % 10000;
        if(m == sp || (0 < sp && coin < 5000)) {
            assert(0 < sp);
            alloc.Free(ptrs[--sp]);
            npop++;
        } else {
            assert(m > sp);
            ptrs[sp++] = alloc.Malloc();
            if(maxSp < sp) maxSp = sp;
            npush++;
        }
    }
    float secs = timer.Stop();

    std::cout << "alloc:              " << ToString(alloc) << std::endl;
    std::cout << "secs:               " << secs << std::endl;
    std::cout << "max. stack pointer: " << maxSp << std::endl;
    std::cout << "number of pushes:   " << npush << std::endl;
    std::cout << "number of pops:     " << npop << std::endl;
    std::cout << std::endl;

    delete[] ptrs;
}

void random_allocs_test(unsigned m, unsigned n) {
    std::cout << "TEST random_allocs" << std::endl;
    std::cout << "stack size = " << m << ", number of iterations = " << n << std::endl;
    std::cout << std::endl;

    random_allocs(m, n, GEN::PageAlloc<Type, 1>());
    random_allocs(m, n, GEN::PageAlloc<Type, 64>());
    random_allocs(m, n, GEN::PageAlloc<Type, 512>());
    random_allocs(m, n, GEN::PageAlloc<Type, 1024>());
    random_allocs(m, n, GEN::PageAlloc<Type, 4096>());
    random_allocs(m, n, CStdAlloc());
}

template<typename ALLOC>
void seq_allocs(unsigned k, unsigned n, ALLOC& alloc) {
    Type**      ptrs = new Type*[k];
    unsigned    sp = 0;

    SYS::Timer timer;
    timer.Start();
    for(unsigned i = 0; i < n; ++i) {
        for(unsigned j = 0; j < k; ++j) ptrs[sp++] = alloc.Malloc();
        for(unsigned j = 0; j < k; ++j) alloc.Free(ptrs[--sp]);
    }
    float secs = timer.Stop();

    std::cout << "alloc:              " << ToString(alloc) << std::endl;
    std::cout << "secs:               " << secs << std::endl;
    std::cout << std::endl;

    delete[] ptrs;
}

void seq_allocs_test(unsigned k, unsigned n) {
    std::cout << "TEST seq_allocs" << std::endl;
    std::cout << "sequence size = " << k << ", number of iterations = " << n << std::endl;
    std::cout << std::endl;

    seq_allocs(k, n, GEN::PageAlloc<Type, 1>());
    seq_allocs(k, n, GEN::PageAlloc<Type, 5>());
    seq_allocs(k, n, GEN::PageAlloc<Type, 64>());
    seq_allocs(k, n, GEN::PageAlloc<Type, 512>());
    seq_allocs(k, n, GEN::PageAlloc<Type, 1024>());
    seq_allocs(k, n, GEN::PageAlloc<Type, 4096>());
    seq_allocs(k, n, CStdAlloc());
}

int main() {
    std::cout << "sizeof(Type) = " << sizeof(Type) << "b" << std::endl;

    random_allocs_test(1000, 100000);
    random_allocs_test(100000, 1000000);

    seq_allocs_test(5, 100000);
    seq_allocs_test(10, 10000);
    seq_allocs_test(100, 10000);
}
