/*
 * memlib.c - a module that simulates the memory system.  Needed because it
 *            allows us to interleave calls from the student's malloc package
 *            with the system's malloc package in libc.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <errno.h>

#include "memlib.h"
#include "config.h"

/* private variables */
static char *mem_start_brk;  /* points to first byte of heap */
static char *mem_brk;        /* points to last byte of heap */
static char *mem_max_addr;   /* largest legal heap address */

/*
 * mem_init - initialize the memory system model
 */
void mem_init(void) {
    /* allocate the storage we will use to model the available VM */
    // mem_start_brk 는 20MB 의 heap 영역을 할당하고, 그 start brk point 를 담는다.
    // 이 때 이 값이 NULL 이라면 malloc 과정에서 문제가 생겼다고 판단하고 에러를 반환한다.
    if ((mem_start_brk = (char *) malloc(MAX_HEAP)) == NULL) {
        fprintf(stderr, "mem_init_vm: malloc error\n");
        exit(1);
    }

    // 현재 사용할 수 있는 최대 address 값은 mem_start_brk 로 부터 할당받은 최대한의 용량만큼 뒤에 있는 address 이다.
    mem_max_addr = mem_start_brk + MAX_HEAP;  /* max legal heap address */

    // mem_brk 는 지금까지 할당한 heap 의 마지막 point 를 가리킨다.
    // 할당하지 않았을 때는 start 일 것이고, 16byte 를 할당했을 때는 start 로 부터 16byte 떨어진 위치를 가리킬 것이다.
    // 새로 메모리를 할당할 때는 mem_brk 를 기준으로 할당할 데이터를 추가한다.
    mem_brk = mem_start_brk;                  /* heap is empty initially */
}

/*
 * mem_deinit - free the storage used by the memory system model
 */
void mem_deinit(void) {
    free(mem_start_brk);
}

/*
 * mem_reset_brk - reset the simulated brk pointer to make an empty heap
 */
void mem_reset_brk() {
    mem_brk = mem_start_brk;
}

/*
 * mem_sbrk - simple model of the sbrk function. Extends the heap
 *    by incr bytes and returns the start address of the new area. In
 *    this model, the heap cannot be shrunk.
 */
void *mem_sbrk(int incr) {
    char *old_brk = mem_brk;
    // sbrk 함수는 원래 커널에 물리 메모리 페이지를 더 할당해 달라고 요구하는 함수이다.
    // 이 함수에서는 incr, 즉 증가 요청만 가능하고, 임의로 정해놓은 최대값인 20MB 를 초과하는 요청에 대해서는 error 를 반환한다.
    if ((incr < 0) || ((mem_brk + incr) > mem_max_addr)) {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: mem_sbrk failed. Ran out of memory...\n");
        return (void *) -1;
    }
    mem_brk += incr;
    return (void *) old_brk;
}

/*
 * mem_heap_lo - return address of the first heap byte
 */
void *mem_heap_lo() {
    return (void *) mem_start_brk;
}

/*
 * mem_heap_hi - return address of last heap byte
 */
void *mem_heap_hi() {
    return (void *) (mem_brk - 1);
}

/*
 * mem_heapsize() - returns the heap size in bytes
 */
size_t mem_heapsize() {
    return (size_t)(mem_brk - mem_start_brk);
}

/*
 * mem_pagesize() - returns the page size of the system
 */
size_t mem_pagesize() {
    return (size_t) getpagesize();
}
