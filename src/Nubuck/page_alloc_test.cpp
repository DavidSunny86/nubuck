#include <assert.h>
#include <stdio.h>
#include <Nubuck\generic\page_alloc.h>

struct MyType {
    void*       p;
    unsigned    i[2];
};

typedef MyType* myTypePtr_t;

void Touch(MyType& t) {
    t.p = &t;
    t.i[0] = 40;
    t.i[1] = 8;
}

void TestPageAlloc() {
    printf("TestPageAlloc() ... ");

    GEN::PageAlloc<MyType> pageAlloc;

    const unsigned N = 4096;
    const unsigned th = RAND_MAX / 2;

    MyType** mem = new myTypePtr_t[N];
    unsigned numAlloc = 0;
    unsigned numPush = 0, numPop = 0;
    unsigned wasEmpty = false;

    for(unsigned i = 0; i < (1 << 15); ++i) {
        while(numAlloc < N && rand() < th) {
            mem[numAlloc] = pageAlloc.Malloc();
            Touch(*mem[numAlloc]);
            numAlloc++;
            numPush++;
        }
        while(0 < numAlloc && rand() < th) {
            pageAlloc.Free(mem[numAlloc - 1]);
            numAlloc--;
            numPop++;
            wasEmpty = true;
        }
    }

    delete[] mem;

    printf("%d pushes and %d pops, wasEmpty = %d, done.\n", numPush, numPop, wasEmpty);
}