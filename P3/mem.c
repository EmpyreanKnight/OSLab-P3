#include "mem.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/* A node in free list.
 * next pointer maintains address of the next node in free list
 * size save free memory bytes in current block
 */
typedef struct _Node {
    struct _Node *next;
    int size;
} Node;

/* A header tracking allocated memory.
 * size field stores allocated memory in bytes
 * magic field uses a pointer, which point to itself, to verify pointer to be freed
 */
typedef struct _Header {
    int size;
    struct _Header *magic;
} Header;

Node *head = NULL; // head of the free list, list is in ascending order of address
int m_error = 0;   // error type flag

/* Round up the value to the nearest multiple of base, base should greater than 0.
 * Return the round up result.
 * Bitwise version only applicable when base is the power of 2 and value is positive.
 */
int round_up(int value, int base) {
    /*
    int remainder = value % base;
    if (remainder == 0) {
        return value;
    }
    return value + base - remainder;
    */
    return (value + base - 1) & ~(base - 1);
}

/* Split the cur pointer if space enough, remove cur pointer from free list otherwise.
 * Return a Header pointer to allocated space
 * Notice that when size + 8 == cur->size, the exceed 8-byte will also be allocated
 */
Header *split_block(Node *pre, Node *cur, int size) {
    if (cur->size >= size + sizeof(Node)) { // split cur
        Node *new = (void *) cur + sizeof(Node) + size;
        new->next = cur->next;
        new->size = cur->size - size - sizeof(Node);
        if (head == cur) {
            head = new;
        } else {
            pre->next = new;
        }
    } else { // remove cur
        if (head == cur) {
            head = head->next;
        } else {
            pre->next = cur->next;
            if (cur->next != NULL) {
                cur->next = cur->next->next;
            }
        }
        size = cur->size; // solve 8-byte problem
    }
    // initialize new header
    Header *res = (Header *) cur;
    res->size = size;
    res->magic = res;
    return res;
}

/* Find a suitable block with designated style, and remove this node from free list
 * Return a pointer to the Header struct upon success, return NULL otherwise.
 */
Header *find_block(int size, int style) {
    Node *cur = head;
    Node *pre = NULL;

    Node *pre_dst = NULL;
    Node *dst = NULL;

    while (cur != NULL) { // find a suitable block in free list
        if (cur->size >= size) { // space of the current block is sufficient
            if (style == M_FIRSTFIT) {
                dst = cur;
                pre_dst = pre;
                break;
            } else if (style == M_BESTFIT) {
                if (dst == NULL || cur->size < dst->size) {
                    dst = cur;
                    pre_dst = pre;
                }
            } else if (style == M_WORSTFIT) {
                if (dst == NULL || cur->size > dst->size) {
                    dst = cur;
                    pre_dst = pre;
                }
            }
        }
        cur = cur->next;
        pre = pre == NULL ? head : pre->next;
    }

    if (dst == NULL) { // can not find a suitable free space
        return NULL;
    }
    return split_block(pre_dst, dst, size);
}

/* Insert Node pHead to correct position of free list
 * and merge with left and right neighbour after that
 * Return 0 when succeed, -1 for unpredicted errors
 */
int merge_block(Header *pHead) {
    // recover size field of node struct
    Node *pNode = (Node *) pHead;
    pNode->size = pHead->size;

    Node *pre = NULL;
    Node *cur = head;
    // search for insertion position
    while (cur != NULL && (pNode >= cur || (pre != NULL && pNode <= pre))) {
        cur = cur->next;
        pre = pre == NULL ? head : pre->next;
    }

    if (head != NULL && cur == NULL) { // insertion position not found
        return -1;
    }

    // insert pNode into free list
    if (pre == NULL) {
        head = pNode;
    } else {
        pre->next = pNode;
    }
    pNode->next = cur;

    if (pre != NULL && (void *) pre + pre->size + sizeof(Node) == pNode) { // merge with left
        pre->size += sizeof(Node) + pNode->size;
        pre->next = pNode->next;
        pNode = pre;
    }
    if (cur != NULL && (void *) pNode + pNode->size + sizeof(Node) == cur) { // merge with right
        pNode->size += sizeof(Node) + cur->size;
        pNode->next = cur->next;
    }
    return 0;
}

/* Should be called one time by a process using these routines.
 * size_of_region is the number of bytes request from the OS using mmap(),
 * which will be round up to suit page size.
 * Return 0 on a success. Otherwise, return -1 and set m_error to E_BAD_ARGS.
 */
int mem_init(int size_of_region) {
    if (size_of_region <= 0 || head != NULL) {
        // check wrong argument or redundant call
        m_error = E_BAD_ARGS;
        return -1;
    }

    // round up size_of_region to suit page size
    int page_size = getpagesize();
    size_of_region = round_up(size_of_region, page_size);

    int fd = open("/dev/zero", O_RDWR); // open the /dev/zero device
    head = mmap(NULL, (size_t) size_of_region, PROT_READ | PROT_WRITE,
                MAP_ANON | MAP_PRIVATE, fd, 0);
    close(fd);

    if (head == MAP_FAILED) {
        m_error = E_BAD_ARGS;
        return -1;
    }

    head->next = NULL;
    head->size = size_of_region - sizeof(Node);
    return 0;
}

/* Allocates size bytes and returns a pointer to the allocated memory.
 * The memory will not be cleared automatically.
 * Returns a pointer to the start position of allocated memory,
 * returns NULL if no enough contiguous free space or other incorrect usage.
 * The style parameter determines how to look through the list for a free space.
 * For performance reasons, return 8-byte aligned chunks of memory.
 */
void *mem_alloc(int size, int style) {
    if (size <= 0 || head == NULL) { // wrong argument or use without a mem_init() call
        m_error = E_BAD_ARGS;
        return NULL;
    }

    size = round_up(size, 8); // 8-byte alignment
    Header *pBlock = find_block(size, style);
    if (pBlock == NULL) {
        m_error = E_NO_SPACE;
        return NULL;
    }
    return (void *) pBlock + sizeof(Header);
}

/* Frees the memory object that ptr points to.
 * If ptr is NULL, no operation is performed.
 * Returns 0 on success, and -1 otherwise.
 */
int mem_free(void *ptr) {
    if (ptr == NULL) {
        return 0;
    }
    Header *pHead = ptr - sizeof(Header);
    if (pHead->magic != pHead) {
        m_error = E_BAD_POINTER;
        return -1;
    }

    if (merge_block(pHead) == -1) { // failed to insert into free list
        m_error = E_BAD_POINTER;
        return -1;
    }
    return 0;
}

/* Debugging routine for developers.
 * Simply print the blocks of free memory to screen.
 */
void mem_dump() {
    printf("Free list:\n");
    Node *cur = head;
    while (cur != NULL) {
        printf("Block starts from %p: Space = %10d, next = %p.\n",
               cur, cur->size, cur->next);
        cur = cur->next;
    }
    puts("");
}