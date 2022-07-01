#include <stddef.h>
#include <stddef.h>
#include <stdbool.h>

//am.h:
typedef struct {
    void *start, *end;
} Area;

extern Area heap ;

int cpu_count   (void);
int cpu_current (void);
// Macros:
#define ROUNDUP(a, sz)      ((((uintptr_t)a) + (sz) - 1) & ~((sz) - 1))
#define ROUNDDOWN(a, sz)    ((((uintptr_t)a)) & ~((sz) - 1))
#define LENGTH(arr)         (sizeof(arr) / sizeof((arr)[0]))
#define RANGE(st, ed)       (Area) { .start = (void *)(st), .end = (void *)(ed) }
#define IN_RANGE(ptr, area) ((area).start <= (ptr) && (ptr) < (area).end)

#define STRINGIFY(s)        #s
#define TOSTRING(s)         STRINGIFY(s)
#define _CONCAT(x, y)       x ## y
#define CONCAT(x, y)        _CONCAT(x, y)

int atomic_xchg(int *addr, int newval); 