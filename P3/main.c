#include <stdio.h>
#include <stdlib.h>
#include "mem.h"

void test() {
    if (mem_init(4000) == -1) {
        perror("init");
        printf("Error: %d.\n", m_error);
        exit(1);
    }
    mem_dump();
    void* a = mem_alloc(100, M_FIRSTFIT); // 104
    mem_dump();
    void* b = mem_alloc(200, M_FIRSTFIT); // 200
    mem_dump();
    void* c = mem_alloc(45, M_FIRSTFIT);  // 48
    mem_dump();
    mem_free(b);
    mem_dump();
    b = mem_alloc(192, M_BESTFIT); // 192
    mem_dump();
    mem_free(b);
    mem_dump();
    b = mem_alloc(184, M_BESTFIT); // 184
    mem_dump();
    mem_free(c);
    mem_dump();
    c = mem_alloc(15, M_WORSTFIT); // 16
    mem_dump();
    mem_free(c);
    mem_dump();
    mem_free(a);
    mem_dump();
    mem_free(b);
    mem_dump();
}

void test2() { // test long free list
    mem_init(8000);
    mem_dump();
    void* a[10];
    int idx[] = { 2, 5, 4, 3, 8, 9, 1, 6, 0, 7 };
    int i, len = 10;
    for (i = 0; i < len; i++) {
        a[i] = mem_alloc(16, M_BESTFIT);
        mem_dump();
    }
    for (i = 0; i < len; i++) {
        mem_free(a[idx[i]]);
        mem_dump();
    }
}

void test3() { // test fragment and fit
    mem_init(2000); mem_dump();
    void* a = mem_alloc(32, M_FIRSTFIT); mem_dump();
    void* b = mem_alloc(32, M_BESTFIT); mem_dump();
    void* c = mem_alloc(32, M_WORSTFIT); mem_dump();
    mem_free(b); mem_dump();
    b = mem_alloc(24, M_BESTFIT); mem_dump();
    void* d = mem_alloc(32, M_WORSTFIT); mem_dump();
    mem_free(c); mem_dump();
    c = mem_alloc(16, M_BESTFIT); mem_dump();
    mem_free(b); mem_dump();
    b = mem_alloc(8, M_BESTFIT); mem_dump();
    void* e = mem_alloc(8, M_BESTFIT); mem_dump();
    mem_free(e); mem_dump();
    mem_free(d); mem_dump();
    mem_free(c); mem_dump();
    mem_free(a); mem_dump();
    mem_free(b); mem_dump();
}

int main() {
    test3();
    return 0;
}