/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/* single word (4) or double word (8) alignment */
// word == 4 byte, double word align 정책을 적용함
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
// 입력받은 size 보다 큰 8의 배수 중 가장 작은 숫자를 통해, 8 의 배수 align 을 맞춤
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

// typedef __SIZE_TYPE__ size_t; 라는 정의는 size_t 타입이 특정 플랫폼이나 컴파일러에 따라 정의되는 내장 타입 __SIZE_TYPE__로 설정되어 있다는 것을 의미한다.
// __SIZE_TYPE__은 일반적으로 메모리 주소의 크기에 맞춰져 있으며, 이는 플랫폼이 32비트인지 64비트인지에 따라 달라지게 된다.
// 32bit 운영체제 기준으로 __SIZE_TYPE__ 은 4 byte 의 크기를 가지게 된다.
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *) -1)
        return NULL;
    else {
        *(size_t *) p = size;
        return (void *) ((char *) p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) {
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t * )((char *) oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}














