#pragma once

#include <stdlib.h>

namespace GEN {

template<
    typename TYPE,
    unsigned NUM_ITEMS = 64 // number of items per page
>
class PageAlloc {
private:
    struct Item {
        Item* next;
    };

    struct Page {
        Page*   next;
        char*   cur;
    };

    enum { 
        ITEM_SZ = sizeof(TYPE) > sizeof(void*) ? sizeof(TYPE) : sizeof(void*),
        PAGE_SZ = sizeof(Page) + ITEM_SZ * NUM_ITEMS
    };

    Item* _free;
    Page* _pages;

                PageAlloc(const PageAlloc& other);
    PageAlloc&  operator=(const PageAlloc& other);
public:
     PageAlloc();
    ~PageAlloc();

    void    Destroy();

    TYPE*   Malloc();
    void    Free(TYPE* ptr);
};

template<typename TYPE, unsigned NUM_ITEMS>
inline PageAlloc<TYPE, NUM_ITEMS>::PageAlloc()
    : _free(NULL)
    , _pages(NULL)
{ }

template<typename TYPE, unsigned NUM_ITEMS>
inline PageAlloc<TYPE, NUM_ITEMS>::~PageAlloc() {
    Destroy();
}

template<typename TYPE, unsigned NUM_ITEMS>
void PageAlloc<TYPE, NUM_ITEMS>::Destroy() {
    Page *next, *page = _pages;
    while(page) {
        next = page->next;
        free(page);
        page = next;
    }
    _pages = NULL;
    _free = NULL;
}

template<typename TYPE, unsigned NUM_ITEMS>
inline TYPE* PageAlloc<TYPE, NUM_ITEMS>::Malloc() {
    if(!_free) {
        if(!_pages || (char*)(&_pages->cur + 1) == _pages->cur) {
            Page* page = static_cast<Page*>(malloc(PAGE_SZ));
            if(!page) return NULL;
            page->next = _pages;
            page->cur = reinterpret_cast<char*>(page) + PAGE_SZ;
            _pages = page;
        }
        _pages->cur -= ITEM_SZ;
        return reinterpret_cast<TYPE*>(_pages->cur);
    }
    Item* it = _free;
    _free = _free->next;
    return reinterpret_cast<TYPE*>(it);
}

template<typename TYPE, unsigned NUM_ITEMS>
inline void PageAlloc<TYPE, NUM_ITEMS>::Free(TYPE* ptr) {
    Item* it = reinterpret_cast<Item*>(ptr);
    it->next = _free;
    _free = it;
}

} // namespace GEN
