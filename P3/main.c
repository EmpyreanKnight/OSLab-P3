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
    mem_free(c);
    mem_dump();
    mem_free(a);
    mem_dump();
}

int main() {
    test();
    return 0;
}