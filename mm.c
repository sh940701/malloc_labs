///*
// * mm-naive.c - The fastest, least memory-efficient malloc package.
// *
// * In this naive approach, a block is allocated by simply incrementing
// * the brk pointer.  A block is pure payload. There are no headers or
// * footers.  Blocks are never coalesced or reused. Realloc is
// * implemented directly using mm_malloc and mm_free.
// *
// * NOTE TO STUDENTS: Replace this header comment with your own header
// * comment that gives a high level description of your solution.
// */
//#include <stdio.h>
//#include <stdlib.h>
//#include <assert.h>
//#include <unistd.h>
//#include <string.h>
//
//#include "mm.h"
//#include "memlib.h"
//
//
//team_t team = {
//        /* Team name */
//        "ateam",
//        /* First member's full name */
//        "Harry Bovik",
//        /* First member's email address */
//        "bovik@cs.cmu.edu",
//        /* Second member's full name (leave blank if none) */
//        "",
//        /* Second member's email address (leave blank if none) */
//        ""
//};
//
//static void *heap_listp;
//static void *next_fit_addr;
//
//#define WSIZE 4 // word size
//#define DSIZE 8 // double word size
//#define CHUNKSIZE (1 << 12) // 페이지 증가 요청 크기. 연산 결과는 4kb 에 해당한다.
//
//#define MAX(x, y) ((x) > (y) ? (x) : (y))
//
///* single word (4) or double word (8) alignment */
//// word == 4 byte, double word align 정책을 적용함
//#define ALIGNMENT 8
//
///* rounds up to the nearest multiple of ALIGNMENT */
//// 입력받은 size 보다 큰 8의 배수 중 가장 작은 숫자를 통해, 8 의 배수 align 을 맞춤
//#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
//
//// typedef __SIZE_TYPE__ size_t; 라는 정의는 size_t 타입이 특정 플랫폼이나 컴파일러에 따라 정의되는 내장 타입 __SIZE_TYPE__로 설정되어 있다는 것을 의미한다.
//// __SIZE_TYPE__은 일반적으로 메모리 주소의 크기에 맞춰져 있으며, 이는 플랫폼이 32비트인지 64비트인지에 따라 달라지게 된다.
//// 32bit 운영체제 기준으로 __SIZE_TYPE__ 은 4 byte 의 크기를 가지게 된다.
//#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
//
//// Header 에 들어갈 데이터를 생성 (블록 크기 | 할당 여부)
//#define PACK(size, alloc) ((size) | (alloc))
//
//// 해당 주소에서 데이터를 read
//#define GET(p) (*(unsigned int *)(p))
//
//// 해당 주소에 데이터를 write
//#define PUT(p, val) (*(unsigned int *)(p) = (val))
//
//// header 에 대해서 마지막 7bit 에 대한 not - and 연산을 취하여 (하위 3개 비트를 0 으로 치환해서) 블록의 크기를 반환
//#define GET_SIZE(p) (GET(p) & ~0x7)
//
//// header 에 대해서 마지막 1bit 에 대한 and 연산을 취하여 블록의 할당 여부를 반환
//#define GET_ALLOC(p) (GET(p) & 0x1)
//
//// 블록 포인터인 bp 를 받아 -4 byte 를 하여 header 의 주소를 read (이 때 bp 는 블록 내 payload 의 위치를 가리키는 것으로 보임)
//#define HDRP(bp) ((void *)(bp) - WSIZE)
//
//// 블록 포인터인 bp 를 받아 bp 에서 block size 만큼을 더한 후, block size 단위(여기선 8 byte) 만큼 빼줌
//// 이 때 bp 가 block 의 header 부터가 아닌 payload 부터라고 하면, footer 의 크기인 4 byte 가 아니라 footer + header 의 크기인 8 byte 만큼 빼 주는 것이 이해 됨
//#define FTRP(bp) ((void *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
//
//// 여기서 bp 또한 payload 의 시작 주소를 나타낸다고 볼 수 있다.
//// 1. payload addr - 4 byte 를 하여 header 의 주소를 구함
//// 2. header 에서 block 의 크기를 구함
//// 3. payload addr 에서 block 의 크기만큼 더하여, 다음 블록의 payload 의 주소를 반환
//#define NEXT_BLKP(bp) ((void *)(bp) + GET_SIZE(((void *)(bp) - WSIZE)))
//
//// 1. 현재 payload 주소에서 8 byte(현재 블록의 header, 이전 블록의 footer) 만큼 빼서 이전 블록의 footer 위치 파악
//// 2. 이전 블록의 footer 에서 이전 블록의 크기를 구한 후, 현재 payload 의 위치에서 빼면, 이전 블록의 payload 주소값을 구할 수 있음
//#define PREV_BLKP(bp) ((void *)(bp) - GET_SIZE(((void *)(bp) - DSIZE)))
//
//static void *coalesce(void *bp) {
//    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
//    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
//    size_t size = GET_SIZE(HDRP(bp));
//
//    // Case 1
//    // 전, 후 block 이 모두 allocation 되어있으면 연결 불가
//    if (prev_alloc && next_alloc) {
//        return bp;
//    }
//
//        // Case 2
//    else if (prev_alloc && !next_alloc) {
//        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
//        PUT(HDRP(bp), PACK(size, 0));
//        PUT(FTRP(bp), PACK(size, 0));
//    }
//
//        // Case 3
//    else if (!prev_alloc && next_alloc) {
//        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
//        PUT(FTRP(bp), PACK(size, 0));
//        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//        bp = PREV_BLKP(bp);
//    }
//
//        // Case 4
//    else if (!prev_alloc && !next_alloc) {
//        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
//        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
//        bp = PREV_BLKP(bp);
//    }
//
//    next_fit_addr = bp;
//    return bp;
//}
//
//static void *extend_heap(size_t words) {
//    void *bp;
//    size_t size;
//
//    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
//    bp = mem_sbrk(size);
//    if ((long) bp == -1) {
//        return NULL;
//    }
//
//
//    PUT(HDRP(bp), PACK(size, 0)); // free block header
//    PUT(FTRP(bp), PACK(size, 0)); // free block footer
//    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header -> 에필로그 헤더를 뒤로 밈.
//
//    // Coalesce 구현
//    return coalesce(bp);
//}
//
//static void *first_fit(size_t asize) {
//    void *bp;
//
//    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
//        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
//            return bp;
//        }
//    }
//
//    return NULL;
//}
//
//static void *next_fit(size_t asize) {
//    void *bp;
//
//    for (bp = next_fit_addr; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
//        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
//            next_fit_addr = bp;
//            return bp;
//        }
//    }
//
//    if (next_fit_addr != heap_listp) {
//        bp = heap_listp;
//        while (bp != next_fit_addr) {
//            if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(HDRP(bp)))) {
//                next_fit_addr = bp;
//                return bp;
//            }
//            bp = NEXT_BLKP(bp);
//        }
//    }
//
//
//    return NULL;
//}
//
//static void *find_fit(size_t asize) {
////    return first_fit(asize); // 54
//    return next_fit(asize); // 83
//}
//
//static void place(void *bp, size_t asize) {
//    size_t csize = GET_SIZE(HDRP(bp));
//
//    if ((csize - asize) >= 2 * DSIZE) {
//        PUT(HDRP(bp), PACK(asize, 1));
//        PUT(FTRP(bp), PACK(asize, 1));
//        bp = NEXT_BLKP(bp);
//        PUT(HDRP(bp), PACK(csize - asize, 0));
//        PUT(FTRP(bp), PACK(csize - asize, 0));
//    } else {
//        PUT(HDRP(bp), PACK(csize, 1));
//        PUT(FTRP(bp), PACK(csize, 1));
//    }
//}
//
///*
// * mm_malloc - Allocate a block by incrementing the brk pointer.
// *     Always allocate a block whose size is a multiple of the alignment.
// */
//void *mm_malloc(size_t size) {
//    size_t asize; // adjusted block size
//    size_t extendsize; // amount to extend heap if no fit
//    void *bp;
//
//    if (size == 0) {
//        return NULL;
//    }
//
//    // header(4) + footer(4) + payload => multiples of DSIZE
//    if (size <= DSIZE) {
//        asize = 2 * DSIZE;
//    } else {
//        // 7을 더한 후 DSIZE 로 나눠서 몫만 챙김 -> payload 가 들어갈 수 있는 DSIZE 의 배수 중 최소값을 구함
//        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
//    }
//
//    bp = find_fit(asize);
//
//    if (bp != NULL) {
//        place(bp, asize);
//        return bp;
//    }
//
//    extendsize = MAX(asize, CHUNKSIZE);
//
//    bp = extend_heap(extendsize / WSIZE);
//
//    if (bp == NULL) {
//        return NULL;
//    }
//
//    place(bp, asize);
//    return bp;
//
//}
//
///*
// * mm_free - Freeing a block does nothing.
// */
//void mm_free(void *bp) {
//    size_t size = GET_SIZE(HDRP(bp));
//
//    PUT(HDRP(bp), PACK(size, 0));
//    PUT(FTRP(bp), PACK(size, 0));
//    coalesce(bp);
//}
//
///*
// * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
// */
//void *mm_realloc(void *ptr, size_t size) {
//    void *oldptr = ptr;
//    void *newptr;
//    size_t copySize;
//
//    newptr = mm_malloc(size); // 새로운 영역을 동적할당 하여 newptr 에 반환
//    if (newptr == NULL)
//        return NULL;
//    copySize = GET_SIZE(HDRP(oldptr));
//    if (size < copySize) // realloc 을 하여 동적할당 사이즈가 줄어들었을 경우
//        copySize = size;
//    memcpy(newptr, oldptr, copySize); // 이전 ptr 에 있던 데이터를 새로 할당한 ptr 에 옮겨준다.
//    mm_free(oldptr); // 이전 ptr 은 할당 해제
//    return newptr;
//}
//
///*
// * mm_init - initialize the malloc package.
// */
//int mm_init(void) {
//    // sbrk 를 호출하여 메모리 할당
//    // 이 때 초기값으로 Alignment padding, Prologue header/footer, Epilogue header 를 만들어주기 때문에 4 word 를 할당해준다.
//    heap_listp = mem_sbrk(4 * WSIZE);
//
//    if (heap_listp == (void *) -1) {
//        return -1;
//    }
//
//    PUT(heap_listp, 0); // Alignment padding
//    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // Prologue header
//    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // Prologue footer
//    PUT(heap_listp + (3 * WSIZE), PACK(0, 1)); // Epilogue header
//    heap_listp += (2 * WSIZE);
//
//    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) { // 할당이 안 되면(?)
//        return -1;
//    }
//    next_fit_addr = heap_listp; // next_fit 구현을 위해 next_fit target 초기화
//    return 0;
//}


///*
// * mm-naive.c - The fastest, least memory-efficient malloc package.
// *
// * In this naive approach, a block is allocated by simply incrementing
// * the brk pointer.  A block is pure payload. There are no headers or
// * footers.  Blocks are never coalesced or reused. Realloc is
// * implemented directly using mm_malloc and mm_free.
// *
// * NOTE TO STUDENTS: Replace this header comment with your own header
// * comment that gives a high level description of your solution.
// */
//#include <stdio.h>
//#include <stdlib.h>
//#include <assert.h>
//#include <unistd.h>
//#include <string.h>
//
//#include "mm.h"
//#include "memlib.h"
//
//
//team_t team = {
//        /* Team name */
//        "ateam",
//        /* First member's full name */
//        "Harry Bovik",
//        /* First member's email address */
//        "bovik@cs.cmu.edu",
//        /* Second member's full name (leave blank if none) */
//        "",
//        /* Second member's email address (leave blank if none) */
//        ""
//};
//
//#define WSIZE 4 // word size
//#define DSIZE 8 // double word size
//#define CHUNKSIZE (1 << 12) // 페이지 증가 요청 크기. 연산 결과는 4kb 에 해당한다.
//
//#define MAX(x, y) ((x) > (y) ? (x) : (y))
//
///* single word (4) or double word (8) alignment */
//// word == 4 byte, double word align 정책을 적용함
//#define ALIGNMENT 8
//
///* rounds up to the nearest multiple of ALIGNMENT */
//// 입력받은 size 보다 큰 8의 배수 중 가장 작은 숫자를 통해, 8 의 배수 align 을 맞춤
//#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)
//
//// typedef __SIZE_TYPE__ size_t; 라는 정의는 size_t 타입이 특정 플랫폼이나 컴파일러에 따라 정의되는 내장 타입 __SIZE_TYPE__로 설정되어 있다는 것을 의미한다.
//// __SIZE_TYPE__은 일반적으로 메모리 주소의 크기에 맞춰져 있으며, 이는 플랫폼이 32비트인지 64비트인지에 따라 달라지게 된다.
//// 32bit 운영체제 기준으로 __SIZE_TYPE__ 은 4 byte 의 크기를 가지게 된다.
//#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
//
//// Header 에 들어갈 데이터를 생성 (블록 크기 | 할당 여부)
//#define PACK(size, alloc) ((size) | (alloc))
//
//// 해당 주소에서 데이터를 read
//#define GET(p) (*(unsigned int *)(p))
//
//// 해당 주소에 데이터를 write
//#define PUT(p, val) (*(unsigned int *)(p) = (val))
//
//// header 에 대해서 마지막 7bit 에 대한 not - and 연산을 취하여 (하위 3개 비트를 0 으로 치환해서) 블록의 크기를 반환
//#define GET_SIZE(p) (GET(p) & ~0x7)
//
//// header 에 대해서 마지막 1bit 에 대한 and 연산을 취하여 블록의 할당 여부를 반환
//#define GET_ALLOC(p) (GET(p) & 0x1)
//
//// 블록 포인터인 bp 를 받아 -4 byte 를 하여 header 의 주소를 read (이 때 bp 는 블록 내 payload 의 위치를 가리키는 것으로 보임)
//#define HDRP(bp) ((char *)(bp) - WSIZE)
//
//// 블록 포인터인 bp 를 받아 bp 에서 block size 만큼을 더한 후, block size 단위(여기선 8 byte) 만큼 빼줌
//// 이 때 bp 가 block 의 header 부터가 아닌 payload 부터라고 하면, footer 의 크기인 4 byte 가 아니라 footer + header 의 크기인 8 byte 만큼 빼 주는 것이 이해 됨
//#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)
//
//// 여기서 bp 또한 payload 의 시작 주소를 나타낸다고 볼 수 있다.
//// 1. payload addr - 4 byte 를 하여 header 의 주소를 구함
//// 2. header 에서 block 의 크기를 구함
//// 3. payload addr 에서 block 의 크기만큼 더하여, 다음 블록의 payload 의 주소를 반환
//#define NEXT_BLKP(bp) ((void *)(bp) + GET_SIZE(HDRP(bp)))
//
//#define PREV_BLKP(bp) ((void *)(bp) - GET_SIZE(((void *)(bp) - DSIZE)))
//
//#define PRED_P(bp)  (*(void **)(bp))
//#define SUCC_P(bp)  (*(void **)((bp) + WSIZE))
//
//static void *heap_listp;
//
//static void *coalesce(void *bp);
//
//static void *extend_heap(size_t words);
//
//static void *find_fit(size_t asize);
//
//static void place(void *p, size_t size);
//
//static void list_add(void *p);
//
//static void list_remove(void *p);
//
///*
// * mm_init - initialize the malloc package.
// */
//int mm_init(void) {
//    heap_listp = mem_sbrk(6 * WSIZE);
//
//    if (heap_listp == (void *) -1) {
//        return -1;
//    }
//
//    PUT(heap_listp, 0); // Alignment padding
//    PUT(heap_listp + (1 * WSIZE), PACK(2 * DSIZE, 1)); // Prologue header
//    PUT(heap_listp + (2 * WSIZE), heap_listp + (3 * WSIZE)); // predecessor
//    PUT(heap_listp + (3 * WSIZE), heap_listp + (2 * WSIZE)); // successor
//    PUT(heap_listp + (4 * WSIZE), PACK(2 * DSIZE, 1)); // Prologue footer
//    PUT(heap_listp + (5 * WSIZE), PACK(0, 1)); // Epilogue header
//    heap_listp += 2 * WSIZE; // payload 위치를 +4 로 변경
//
//    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) { // 할당이 안 되면(?)
//        return -1;
//    }
//
//    return 0;
//}
//
//static void *extend_heap(size_t words) {
//    char *bp;
//    size_t size;
//
//    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
//    bp = mem_sbrk(size);
//    if ((long) bp == -1) {
//        return NULL;
//    }
//
//
//    PUT(HDRP(bp), PACK(size, 0)); // free block header
//    PUT(FTRP(bp), PACK(size, 0)); // free block footer
//    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header -> 에필로그 헤더를 뒤로 밈.
//
//    // Coalesce 구현
//    return coalesce(bp);
//}
//
///*
// * mm_malloc - Allocate a block by incrementing the brk pointer.
// *     Always allocate a block whose size is a multiple of the alignment.
// */
//void *mm_malloc(size_t size) {
//    size_t asize; // adjusted block size
//    size_t extendsize; // amount to extend heap if no fit
//    char *bp;
//
//    if (size == 0) {
//        return NULL;
//    }
//
//    // header(4) + footer(4) + payload => multiples of DSIZE
//    if (size <= DSIZE) {
//        asize = 2 * DSIZE;
//    } else {
//        // 7을 더한 후 DSIZE 로 나눠서 몫만 챙김 -> payload 가 들어갈 수 있는 DSIZE 의 배수 중 최소값을 구함
//        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
//    }
//
//    bp = find_fit(asize);
//
//    if (bp != NULL) {
//        place(bp, asize);
//        return bp;
//    }
//
//    extendsize = MAX(asize, CHUNKSIZE);
//
//    bp = extend_heap(extendsize / WSIZE);
//
//    if (bp == NULL) {
//        return NULL;
//    }
//
//    place(bp, asize);
//    return bp;
//}
//
//static void *find_fit(size_t asize) {
//    void *bp;
//
//    for (bp = SUCC_P(heap_listp); !GET_ALLOC(HDRP(bp)); bp = SUCC_P(bp)) {
//        if (asize <= GET_SIZE(HDRP(bp))) {
//            return bp;
//        }
//    }
//
//    return NULL;
//}
//
//static void place(void *bp, size_t asize) {
//    size_t csize = GET_SIZE(HDRP(bp));
//    list_remove(bp);
//
//    if ((csize - asize) >= 2 * DSIZE) {
//        PUT(HDRP(bp), PACK(asize, 1));
//        PUT(FTRP(bp), PACK(asize, 1));
//        bp = NEXT_BLKP(bp);
//        PUT(HDRP(bp), PACK(csize - asize, 0));
//        PUT(FTRP(bp), PACK(csize - asize, 0));
//        list_add(bp);
//    } else {
//        PUT(HDRP(bp), PACK(csize, 1));
//        PUT(FTRP(bp), PACK(csize, 1));
//    }
//}
//
//static void list_add(void *p) {
//    SUCC_P(p) = SUCC_P(heap_listp);
//    PRED_P(p) = heap_listp;
//    PRED_P(SUCC_P(heap_listp)) = p;
//    SUCC_P(heap_listp) = p;
//}
//
//static void list_remove(void *p) {
//    SUCC_P(PRED_P(p)) = SUCC_P(p);
//    PRED_P(SUCC_P(p)) = PRED_P(p);
//}
//
///*
// * mm_free - Freeing a block does nothing.
// */
//void mm_free(void *bp) {
//    size_t size = GET_SIZE(HDRP(bp));
//
//    PUT(HDRP(bp), PACK(size, 0));
//    PUT(FTRP(bp), PACK(size, 0));
//    coalesce(bp);
//}
//
//
//static void *coalesce(void *bp) {
//    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
//    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
//    size_t size = GET_SIZE(HDRP(bp));
//
//    // Case 1
//    // 전, 후 block 이 모두 allocation 되어있으면 연결 불가
//    if (prev_alloc && next_alloc) {
//        list_add(bp);
//        return bp;
//    }
//
//        // Case 2
//    if (prev_alloc && !next_alloc) {
//        list_remove(NEXT_BLKP(bp));
//        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
//        PUT(HDRP(bp), PACK(size, 0));
//        PUT(FTRP(bp), PACK(size, 0));
//    }
//
//        // Case 3
//    else if (!prev_alloc && next_alloc) {
//        list_remove(PREV_BLKP(bp));
//        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
//        PUT(FTRP(bp), PACK(size, 0));
//        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//        bp = PREV_BLKP(bp);
//    }
//
//        // Case 4
//    else if (!prev_alloc && !next_alloc) {
//        list_remove(PREV_BLKP(bp));
//        list_remove(NEXT_BLKP(bp));
//        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
//        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
//        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
//        bp = PREV_BLKP(bp);
//    }
//
//    list_add(bp);
//    return bp;
//}
//
//
///*
// * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
// */
//void *mm_realloc(void *ptr, size_t size) {
//    void *oldptr = ptr;
//    void *newptr;
//    size_t copySize;
//
//    newptr = mm_malloc(size); // 새로운 영역을 동적할당 하여 newptr 에 반환
//    if (newptr == NULL)
//        return NULL;
//    copySize = GET_SIZE(HDRP(oldptr));
//    if (size < copySize) // realloc 을 하여 동적할당 사이즈가 줄어들었을 경우
//        copySize = size;
//    memcpy(newptr, oldptr, copySize); // 이전 ptr 에 있던 데이터를 새로 할당한 ptr 에 옮겨준다.
//    mm_free(oldptr); // 이전 ptr 은 할당 해제
//    return newptr;
//}

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


team_t team = {
        /* Team name */
        "team 10",
        /* First member's full name */
        "Vitalik Buterin",
        /* First member's email address */
        "ethereum@gmail.com",
        /* Second member's full name (leave blank if none) */
        "",
        /* Second member's email address (leave blank if none) */
        ""
};

#define WSIZE 4 // word size
#define DSIZE 8 // double word size
#define CHUNKSIZE (1 << 12) // 페이지 증가 요청 크기. 연산 결과는 4kb 에 해당한다.

#define MAX(x, y) ((x) > (y) ? (x) : (y))

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

// Header 에 들어갈 데이터를 생성 (블록 크기 | 할당 여부)
#define PACK(size, alloc) ((size) | (alloc))

// 해당 주소에서 데이터를 read
#define GET(p) (*(unsigned int *)(p))

// 해당 주소에 데이터를 write
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// header 에 대해서 마지막 7bit 에 대한 not - and 연산을 취하여 (하위 3개 비트를 0 으로 치환해서) 블록의 크기를 반환
#define GET_SIZE(p) (GET(p) & ~0x7)

// header 에 대해서 마지막 1bit 에 대한 and 연산을 취하여 블록의 할당 여부를 반환
#define GET_ALLOC(p) (GET(p) & 0x1)

// 블록 포인터인 bp 를 받아 -4 byte 를 하여 header 의 주소를 read (이 때 bp 는 블록 내 payload 의 위치를 가리키는 것으로 보임)
#define HDRP(bp) ((char *)(bp) - WSIZE)

// 블록 포인터인 bp 를 받아 bp 에서 block size 만큼을 더한 후, block size 단위(여기선 8 byte) 만큼 빼줌
// 이 때 bp 가 block 의 header 부터가 아닌 payload 부터라고 하면, footer 의 크기인 4 byte 가 아니라 footer + header 의 크기인 8 byte 만큼 빼 주는 것이 이해 됨
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

// 여기서 bp 또한 payload 의 시작 주소를 나타낸다고 볼 수 있다.
// 1. payload addr - 4 byte 를 하여 header 의 주소를 구함
// 2. header 에서 block 의 크기를 구함
// 3. payload addr 에서 block 의 크기만큼 더하여, 다음 블록의 payload 의 주소를 반환
#define NEXT_BLKP(bp) ((void *)(bp) + GET_SIZE(HDRP(bp)))

#define PREV_BLKP(bp) ((void *)(bp) - GET_SIZE(((void *)(bp) - DSIZE)))

#define PRED_P(bp)  (*(void **)(bp))
#define SUCC_P(bp)  (*(void **)((bp) + WSIZE))

#define GET_ROOT(idx) (*(void **)(heap_listp + ((idx) * WSIZE)))

#define MAX_SEG 20

static void *heap_listp;

static void *coalesce(void *bp);

static void *extend_heap(size_t words);

static void *find_fit(size_t asize);

static void place(void *p, size_t size);

static void list_add(void *p);

static void list_remove(void *p);

static int get_class(size_t);

/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) {
    heap_listp = mem_sbrk((4 + MAX_SEG) * WSIZE);

    if (heap_listp == (void *) -1) {
        return -1;
    }

    PUT(heap_listp, 0); // Alignment padding
    PUT(heap_listp + (1 * WSIZE), PACK((2 + MAX_SEG) * WSIZE, 1)); // Prologue header
    for (int i = 0; i < MAX_SEG; i++) {
        PUT(heap_listp + (2 + i) * WSIZE, NULL);
    }
    PUT(heap_listp + ((2 + MAX_SEG) * WSIZE), PACK((2 + MAX_SEG) * WSIZE, 1)); // Prologue footer
    PUT(heap_listp + ((3 + MAX_SEG) * WSIZE), PACK(0, 1)); // Epilogue header

    heap_listp += 2 * WSIZE;

    if (extend_heap(CHUNKSIZE / WSIZE) == NULL) {
        return -1;
    }

    return 0;
}

static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    bp = mem_sbrk(size);
    if ((long) bp == -1) {
        return NULL;
    }

    PUT(HDRP(bp), PACK(size, 0)); // free block header
    PUT(FTRP(bp), PACK(size, 0)); // free block footer
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // new epilogue header -> 에필로그 헤더를 뒤로 밈.

    // Coalesce 구현
    return coalesce(bp);
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size) {
    size_t asize; // adjusted block size
    size_t extendsize; // amount to extend heap if no fit
    char *bp;

    if (size == 0) {
        return NULL;
    }

    // header(4) + footer(4) + payload => multiples of DSIZE
    if (size <= DSIZE) {
        asize = 2 * DSIZE;
    } else {
        // 7을 더한 후 DSIZE 로 나눠서 몫만 챙김 -> payload 가 들어갈 수 있는 DSIZE 의 배수 중 최소값을 구함
        asize = DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
    }

    bp = find_fit(asize);

    if (bp != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);

    bp = extend_heap(extendsize / WSIZE);

    if (bp == NULL) {
        return NULL;
    }

    place(bp, asize);
    return bp;
}

static void *find_fit(size_t asize) {
    /*  First-fit search */
    char *bp;
    char *bp_list;
    int class = get_class(asize);

    while (class < MAX_SEG) {
        bp_list = GET_ROOT(class);
        for (bp = bp_list; bp != NULL; bp = SUCC_P(bp)) {
            if (asize <= GET_SIZE(HDRP(bp)))
                return bp;
        }
        class += 1;
    }

    return NULL;
}

static void place(void *bp, size_t asize) {
    size_t csize = GET_SIZE(HDRP(bp));
    list_remove(bp);

    if ((csize - asize) >= 2 * DSIZE) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 0));
        PUT(FTRP(bp), PACK(csize - asize, 0));
        list_add(bp);
    } else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
    }
}

static void list_add(void *p) {
    int class = get_class(GET_SIZE(HDRP(p)));

    SUCC_P(p) = GET_ROOT(class);
    if (GET_ROOT(class) != NULL) {
        PRED_P(GET_ROOT(class)) = p;
    }
    GET_ROOT(class) = p;
    return;
//
//    for (int i = 1; i <= MAX_SEG; i--) {
//        if (size <= 1 << i) {
//            if (GET_ROOT(i) == NULL) {
//                GET_ROOT(i) = p;
//                PRED_P(p) = NULL;
//            } else {
//                SUCC_P(p) = GET_ROOT(i);
//                PRED_P(p) = NULL;
//                PRED_P(GET_ROOT(i)) = p;
//                GET_ROOT(i) = p;
//            }
//        }
//    }
}

static void list_remove(void *p) {
    int class = get_class(GET_SIZE(HDRP(p)));

    if (GET_ROOT(class) == p) {
        GET_ROOT(class) = SUCC_P(p);
    } else {
        SUCC_P(PRED_P(p)) = SUCC_P(p);
        if (SUCC_P(p) != NULL) {
            PRED_P(SUCC_P(p)) = PRED_P(p);
        }
    }
    return;

//    for (int i = 1; i <= MAX_SEG; i--) {
//        if (size <= 1 << i) {
//            if (GET_ROOT(i) == p) {
//                GET_ROOT(i) = SUCC_P(p);
//            } else {
//                SUCC_P(PRED_P(p)) = SUCC_P(p);
//                if (SUCC_P(p) != NULL) {
//                    PRED_P(SUCC_P(p)) = PRED_P(p);
//                }
//            }
//            return;
//        }
//    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *bp) {
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}


static void *coalesce(void *bp) {
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    // Case 1
    // 전, 후 block 이 모두 allocation 되어있으면 연결 불가
    if (prev_alloc && next_alloc) {
        list_add(bp);
        return bp;
    }

        // Case 2
    else if (prev_alloc && !next_alloc) {
        list_remove(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }

        // Case 3
    else if (!prev_alloc && next_alloc) {
        list_remove(PREV_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

        // Case 4
    else if (!prev_alloc && !next_alloc) {
        list_remove(PREV_BLKP(bp));
        list_remove(NEXT_BLKP(bp));
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    list_add(bp);
    return bp;
}


/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size) {
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size); // 새로운 영역을 동적할당 하여 newptr 에 반환
    if (newptr == NULL)
        return NULL;
    copySize = GET_SIZE(HDRP(oldptr));
    if (size < copySize) // realloc 을 하여 동적할당 사이즈가 줄어들었을 경우
        copySize = size;
    memcpy(newptr, oldptr, copySize); // 이전 ptr 에 있던 데이터를 새로 할당한 ptr 에 옮겨준다.
    mm_free(oldptr); // 이전 ptr 은 할당 해제
    return newptr;
}

int get_class(size_t size) {
    size_t class_size = 32;

    if (size < 16) {
        return -1;
    }

    for (int i = 0; i < MAX_SEG; i++) {
        class_size <<= 1;
        if (size < class_size) {
            return i;
        }
    }

    return MAX_SEG - 1;
}
