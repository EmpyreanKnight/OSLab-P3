#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mem.h"

void test1() {
    if (mem_init(4000) == -1) {
        perror("init");
        printf("Error: %d.\n", m_error);
        exit(1);
    }
    mem_dump();
    void *a = mem_alloc(100, M_FIRSTFIT); // 104
    mem_dump();
    void *b = mem_alloc(200, M_FIRSTFIT); // 200
    mem_dump();
    void *c = mem_alloc(45, M_FIRSTFIT);  // 48
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
    void *a[10];
    int idx[] = {2, 5, 4, 3, 8, 9, 1, 6, 0, 7};
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
    mem_init(2000);
    mem_dump();
    void *a = mem_alloc(32, M_FIRSTFIT);
    mem_dump();
    void *b = mem_alloc(32, M_BESTFIT);
    mem_dump();
    void *c = mem_alloc(32, M_WORSTFIT);
    mem_dump();
    mem_free(b);
    mem_dump();
    b = mem_alloc(24, M_BESTFIT);
    mem_dump();
    void *d = mem_alloc(32, M_WORSTFIT);
    mem_dump();
    mem_free(c);
    mem_dump();
    c = mem_alloc(16, M_BESTFIT);
    mem_dump();
    mem_free(b);
    mem_dump();
    b = mem_alloc(8, M_BESTFIT);
    mem_dump();
    void *e = mem_alloc(8, M_BESTFIT);
    mem_dump();
    mem_free(e);
    mem_dump();
    mem_free(d);
    mem_dump();
    mem_free(c);
    mem_dump();
    mem_free(a);
    mem_dump();
    mem_free(b);
    mem_dump();
}

void test4() { // test read/write and boundary
    mem_init(4096);
    mem_dump();
    int *a = mem_alloc(4080, M_BESTFIT);
    if (a == NULL) {
        perror("alloc");
        exit(1);
    }
    mem_dump();
    a[0] = 114;
    a[1] = 514;
    a[508] = 1919;
    a[509] = 810;
    printf("%d\n", a[100]);
    printf("%d%d%d%d\n", a[0], a[1], a[508], a[509]);
    if (mem_free(a) == -1) {
        perror("free");
    }
    mem_dump();
}

#define N 100000
#define MAX_SIZE 1000
void *ptr[N];
int order[N];
int sz[N];

void Fisher_Yates() { // generate a 0 ~ N-1 permutation
    srand((unsigned) time(0));
    int i, j, tmp;
    for (i = 0; i < N; i++) {
        order[i] = i;
        sz[i] = rand() % MAX_SIZE + 1;
    }

    i = N - 1;
    while (i > 0) {
        j = rand() % i;
        tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
        i--;
    }
}

void benchmark1() { // test performance
    if (mem_init((N + 16) * MAX_SIZE) == -1) {
        perror("Too big!");
        exit(1);
    }
    mem_dump();
    Fisher_Yates();
    double begin = clock();
    int i;
    for (i = 0; i < N; i++) {
        ptr[order[i]] = mem_alloc(sz[order[i]], M_FIRSTFIT);
    }
    for (i = 0; i < N; i++) {
        mem_free(ptr[i]);
    }
    double end = clock();
    printf("My malloc: %f\n", (end - begin) / CLOCKS_PER_SEC);

    begin = clock();
    for (i = 0; i < N; i++) {
        ptr[order[i]] = malloc(sz[order[i]]);
    }
    for (i = 0; i < N; i++) {
        free(ptr[i]);
    }
    end = clock();
    printf("System malloc: %f\n", (end - begin) / CLOCKS_PER_SEC);
    mem_dump();
}

int main() {
    //test4();
    benchmark1();
    return 0;
}