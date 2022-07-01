#include <common.h>



void global_init() {
    boundnow = PAGE;
    // global lock init:
    lockinit(&g_lock);
    lockinit(&b_lock);
    // cache init : 
    // cache for different slab size range from (2^4 byte to 2^12)
    cachelist = (cache *)(METADATA);
    cache *cachecur = cachelist;
    for(int i = 0; i < 9; i++) {
        cachecur = cachelist + i;
        cachecur -> freeslab = NULL;
        cachecur -> sz = i + 4;
        lockinit(&(cachecur -> m));
    }
    meta = (uintptr_t)((uintptr_t)cachecur + (uintptr_t)sizeof(cache));
    // buddytree init:
    intree = (uint8_t *)meta;
    for(int i = 0; i <= (1 << 11) + 1; i++)
        intree[i] = 0;
        
    meta = (uintptr_t)((uintptr_t)meta + (uintptr_t)(1 << 12));
}

slab *pagealloc(size_t sz){
    lock(&g_lock);
    slab *newslab = (slab *)boundnow;
    boundnow = (uintptr_t)(boundnow + PAGE_SIZE);
    if(boundnow > HUGE){
        printf("no free memory\n");
        unlock(&g_lock);
        return NULL;
    }
    lockinit(&(newslab -> m));
    unlock(&g_lock);
    // slab init:
    

    if(sz == 11 || sz == 12) // allocate one page all 
    {
        lock(&(newslab -> m));
        newslab -> sz = sz;
        newslab -> isfull = false;
        newslab -> freelist = NULL;
        newslab -> nxtslab = NULL;
        unlock(&(newslab -> m));
        return newslab;
    }


    lock(&(newslab -> m));
    newslab -> sz = sz;
    newslab -> isfull = false;
    newslab -> freelist = (slab_header *)(uintptr_t)(ROUNDUP((uintptr_t)newslab + (uintptr_t)(sizeof(slab)), (1 << sz)));
    uintptr_t slabound = (uintptr_t)((uintptr_t)newslab + PAGE_SIZE);
    slab_header *listcur = newslab -> freelist;
    while((uintptr_t) listcur + (uintptr_t)(1 << sz) < slabound) // initialization freelist in a slab;
    {
        listcur -> magic = MAGIC;
        listcur -> nex_slab = (slab_header *)(listcur + 1);
        listcur = listcur -> nex_slab;
    }
    listcur -> nex_slab = NULL;
    unlock(&(newslab -> m));
    return newslab;
}

void *smallalloc(size_t sz) {
    cache c = cachelist[sz - 4];
    retry : 
    lock(&(c.m));
    while(c.freeslab && c.freeslab -> isfull) c.freeslab = c.freeslab -> nxtslab;
    // guarantee that the header of slablist must be used first;
    if(!c.freeslab) // which means that this cache is short of slab, call pagealloc;
    {
        slab *newslab = pagealloc(sz);
        newslab -> nxtslab = c.freeslab;
        c.freeslab = newslab;
    }
    unlock(&(c.m));

    if(sz == 11 || sz == 12) // allocate one page 
    {   lock(&(c.m));
        if(c.freeslab){
        slab *s = c.freeslab;
        c.freeslab = c.freeslab -> nxtslab;
        void *ret = (void *)s;
        unlock(&(c.m));
        return ret;
        }
        else {
            unlock(&(c.m));
            goto retry;
        } 
    }
    // allocate memory in the slab:
    slab *s = c.freeslab;
    // operation inside slab:
    lock(&(s->m));
    if(s -> isfull) { // which means that other thread has alloced memory of that slab;
        unlock(&(s -> m));
        goto retry;
    }
    void *ret = (void *)(s -> freelist);
    s -> freelist -> magic = 0;
    s -> freelist = s -> freelist -> nex_slab; 
    if(!(s -> freelist)) s -> isfull = true;
    unlock(&(s -> m));
    return ret;
}

void smallfree(void *ptr) {
    int pageid = (int)(((uintptr_t)ptr - PAGE) / PAGE_SIZE); // find the index of the page ptr lies in;
    slab *s = (slab *)(uintptr_t)(PAGE + (uintptr_t)(pageid * PAGE_SIZE)); // cast it to slab 
    bool flag = false; // if flag is true, after freeing we need to add such slab into cache freelist
    if(s -> sz <= 10){
        // find the allocated memory in slab and add it to freelist:
        lock(&(s -> m)); // warning: if multiple threads free the same ptr at once ?
        slab_header *wfree = (slab_header *)ptr;
        wfree -> nex_slab = s -> freelist;
        wfree -> magic = MAGIC;
        if(s -> isfull){
            s -> isfull = false;
            flag = true;
        }
        unlock(&(s -> m));

        if(flag) {       // add this slab to cache slab list
            int sz = s -> sz;
            cache ca = cachelist[sz - 4];
            lock(&(ca.m));
            s -> nxtslab = ca.freeslab; // warning : no slab lock holding
            ca.freeslab = s;
            unlock(&(ca.m));
        
        }
        return ;
    }
 
    // else we need free the whole page;
    else {
        int sz = s -> sz;
        cache ca = cachelist[sz - 4];
        lock(&(ca.m));
        s -> nxtslab = ca.freeslab; // warning : no slab lock holding
        ca.freeslab = s;
        unlock(&(ca.m));
        return ;
    }
}

int dfs(int dep, int id, size_t sz){
    if(dep == sz) {
        if(intree[id] == 0) return id;
        else return -1;
    }
    if(intree[id] == 2) return -1;
    int ch1 = 2 * id + 1;
    int ch2 = ch1 + 1;
    int ans1 = dfs(dep + 1, ch1, sz), ans2 = dfs(dep + 1, ch2, sz);
    return (ans1 == -1) ? ans2 : ans1;
}

void *hugealloc(size_t sz) {
    size_t dep = 25 - sz;
    uintptr_t turesize = (1 << sz);
    lock(&(b_lock));
    int id = dfs(0, 0, dep);
    if(id == -1) return NULL;
    else {
        // modify intree :
        // from node id , traverse ancestors recursively, 
        // if the both of children of traversed node are 2, modify this node by 2; else by 1.
        int cur = id;
        bool merge = true;
        while(cur){
            if(merge) intree[cur] = 2;
            else intree[cur] = 1;
            merge = false;
            int neib = (cur & 1) ? cur + 1 : cur - 1;
            if(intree[neib] == 2 && intree[cur] == 2)
                merge = true;
            cur = (cur - 1) / 2;
        } 
        if(intree[1] == 2 && intree[2] == 2) intree[0] = 2;
        unlock(&(b_lock));
        // find exact address in heap:
        int seq = id - (1 << dep) + 1;
        void *ret = (void *)(uintptr_t)(turesize * seq + HUGE);
        return ret;
    }
}

void hugefree(void *ptr) {
    uintptr_t mask = 1 << 12;
    uintptr_t addr = (uintptr_t) ptr;
    assert((addr & mask) == 0);
    addr = addr >> 12;
    int dep = 0;
    for(dep = 0; ; dep ++){
        if(addr & 1) break;
        else addr = addr >> 1;
    }
    uintptr_t truesize = 1 << (12 + dep);
    dep = 13 - dep; // 25 - (12 - dep);
    int blockid = (int)(uintptr_t)(((uintptr_t)ptr - HUGE) / truesize);
    assert((uintptr_t)(blockid * truesize) + HUGE == (uintptr_t)ptr);
    // find the index of that block in buddy tree:
    int treeid = (1 << dep) - 1 + blockid;
    // the allocated memory block must be the left child of parent node
    // if curid is full but its children nodes are not both full, which means curid is the block has been allocated
    // else; recusive on left child
    int curid = treeid;
    while(intree[curid] != 2){
        curid = curid * 2 + 1;
        assert(intree[curid] != 0);
    }
    while(intree[curid *2 + 1] == 2 && intree[curid*2+2] == 2){
        curid = curid * 2 + 1;
        assert(intree[curid] != 0);
    }
    // curid denotes the block memory of ptr point to;
    // intree[curid] = 0 to free that block;
    // and traverse its ancestors recursively to update intree
    intree[curid] = 0;
    bool flag = true;
    // update buddy tree:
    while(flag){
        flag = false;
        int fa = (curid & 1) ? (curid - 1) / 2 : (curid - 2) / 2;
        int nei = (curid & 1) ? (curid + 1) : (curid - 1);
        if(intree[fa] == 2) { intree[fa] = 1; flag = true; }
        if(intree[curid] == 0 && intree[nei] == 0 && intree[fa] != 0) { intree[fa] = 0; flag = true; }
        curid = fa;
    }
}


static void *kalloc(size_t size) {
    int sz = ROUND_SELF(size);
    if(sz <= 4) {
       return smallalloc(4);
    }
    else if(sz <= 12) {
        return smallalloc(sz);
    }
    else return hugealloc(sz);
}

static void kfree(void *ptr) {
    if((uintptr_t)ptr >= HUGE)
    hugefree(ptr);
    else smallfree(ptr);
}

static void pmm_init() {
    uintptr_t pmsize = ((uintptr_t) heap.end - (uintptr_t)heap.start);
    printf("Got %ld MiB heap: [0x%x, 0x%x)\n", pmsize >> 20, heap.start, heap.end);
}


MODULE_DEF(pmm) = {
    .init = pmm_init, 
    .alloc = kalloc,
    .free = kfree,
} ;