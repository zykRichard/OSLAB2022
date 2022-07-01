#ifndef TEST
#include <common.h>
#else 
#include <common_test.h>
#endif


static void *kalloc(size_t size) {
    int sz = ROUND_SELF(size);
    if(sz <= 4) return smallalloc(4);
    else if(sz < 12) return smallalloc(sz);
    else if(sz == 12 || sz == 13) return pagealloc(sz);
    else return hugealloc(sz);
}

static void kfree(void *ptr) {
    if((uintptr_t) ptr >= HUGESTART) hfree(ptr);
    else sfree(ptr);
}

#ifndef TEST
static void *safe_kalloc(size_t size) {
    int i = ienabled();
    iset(false);
    void *ret = kalloc(size);
    if(i) { iset(true);}
    return ret;
}

static void safe_kfree(void *ptr) {
    int i = ienabled();
    iset(false);
    kfree(ptr);
    if(i) iset(true);
}
#endif

#ifndef TEST
static void pmm_init() {
    //uintptr_t pmsize = ((uintptr_t) heap.end - (uintptr_t) heap.start);
    //printf("Got %ld MiB heap: [0x%x, 0x%x)\n", pmsize >> 20, heap.start, heap.end);
    kinit();
}

MODULE_DEF(pmm) = {
    .init = pmm_init,
    .alloc = safe_kalloc,
    .free = safe_kfree,
} ;

#else 
void pmm_init() {
    char *ptr = malloc(HEAPSIZE);
    heap.start = ptr;
    heap.end = ptr + HEAPSIZE;
    printf("heap from: [0x%x, 0x%x)\n", heap.start, heap.end);
    kinit();
}

#endif



void kinit() {
    hbound = HUGESTART;
    lockinit(&(g_lock));
    // small memory allocator initialization:
    for(int i = 0; i < 8; i++){
        kmem *mem = &(smallmem[i]);
        lockinit(&(mem -> m));
        mem -> freelist = NULL;
        size_t sz = 1 << (4 + i);
        char *p;
        p = (char *)ROUNDUP(STARTADDR(i), sz);
        for( ; p + sz <= (char *)STARTADDR(i + 1); p += sz) {
            assert((uintptr_t)p % sz == 0);
            assert((uintptr_t) p <= STARTADDR(i + 1));
            list *cur = (list *)p;
            lock(&(mem -> m));
            cur -> nxt = mem -> freelist;
            mem -> freelist = cur;
            unlock(&(mem -> m));
        }
    }

    // page memory allocator initialization:
    lockinit(&(pagemem[0].m));
    pagemem[0].freelist = NULL;
    char *p;
    p = (char *)ROUNDUP(PAGESTART, PAGE_SIZE);
    for( ; (uintptr_t)(p + PAGE_SIZE) <= MPAGESTART; p += PAGE_SIZE){
        assert((uintptr_t) p % PAGE_SIZE == 0);
        assert((uintptr_t) p <= MPAGESTART);
        list *cur = (list *)p;
        lock(&(pagemem[0].m));
        cur -> nxt = pagemem[0].freelist;
        pagemem[0].freelist = cur;
        unlock(&(pagemem[0].m));
    }

    // mpage memory allocator initialization:
    lockinit(&(pagemem[1].m));
    pagemem[1].freelist = NULL;
    p = (char *)ROUNDUP(MPAGESTART, MPAGE_SIZE);
    for( ; (uintptr_t)(p + MPAGE_SIZE) <= HUGESTART; p += MPAGE_SIZE){
        assert((uintptr_t) p % MPAGE_SIZE == 0);
        assert((uintptr_t) p <= HUGESTART);
        list *cur = (list *)p;
        lock(&(pagemem[1].m));
        cur -> nxt = pagemem[1].freelist;
        pagemem[1].freelist = cur;
        unlock(&(pagemem[1].m));
    }
    
}


void *smallalloc (size_t sz) {
    list *r = NULL;
    kmem *mem = &(smallmem[sz - 4]);
    lock(&(mem -> m));
    if(mem -> freelist) {
        r = mem -> freelist;
        mem -> freelist = r -> nxt;
    }
    unlock(&(mem -> m));
    if(r) return (void *)r;
    else return NULL;
}

void *pagealloc (size_t sz) {
    list *r = NULL;

    if(sz == 12){
    kmem *mem = &pagemem[0];
    lock(&(mem -> m));
    if(mem -> freelist) {
        r = mem -> freelist;
        mem -> freelist = r -> nxt;
    }
        unlock(&(mem -> m));
    }

    else if(sz == 13){
        kmem *mem = &pagemem[1];
        lock(&(mem -> m));
        if(mem -> freelist) {
        r = mem -> freelist;
        mem -> freelist = r -> nxt;
    }
        unlock(&(mem -> m));
    }
    if(r) return (void *)r;
    else return NULL;
}

void *hugealloc (size_t sz) {
    size_t tsz = 1 << sz;
    lock(&(g_lock));
    char *p = (char *)ROUNDUP(hbound, tsz);
    hbound = hbound + tsz;
    unlock(&(g_lock));
    if(hbound > (uintptr_t)heap.end) return NULL;
    else return (void *)p;
}

void sfree (void *ptr) {
    
    list *r = NULL;

    // small memory free:
    if((uintptr_t)ptr <= PAGESTART) {
    int id = -1;
    for(int i = 0; i < 8; i++)
        if((uintptr_t)ptr >= STARTADDR(i) && (uintptr_t)ptr < STARTADDR(i + 1))
            id = i;
    assert(id != -1);
    kmem *mem = &(smallmem[id]);
    lock(&(mem -> m));
    r = (list *)ptr;
    r -> nxt = mem -> freelist;
    mem -> freelist = r;
    unlock(&(mem -> m));
    }

    // page memory free :
    else {
        int id = (uintptr_t)ptr >= MPAGESTART ? 1 : 0;
        kmem *mem = &pagemem[id];
        lock(&(mem -> m));
        r = (list *)ptr;
        r -> nxt = mem -> freelist;
        mem -> freelist = r;
        unlock(&(mem -> m));
    }
    return ;
}

void hfree (void *ptr) {
    return ;
}

