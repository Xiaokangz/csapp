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
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4           /* Word and header/footer size (bytes) */
#define DSIZE 8           /* Double word size(bytes) */
#define CHUNKSIZE (1<<12) /*Extend heap by this amount (bytes) */

#define MAX(x, y) ((x) > (y) ? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p) (*(unsigned int *)(p))
#define PUT(p, val) (*(unsigned int *)(p) = (unsigned int)(val))

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char *)(bp) - WSIZE)
#define FTRP(bp) ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp) ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given free block ptr bp, compute the address of the next and previous blocks */
#define NEXT_FREE_BLKP(bp) ((char *)(bp))
#define PREV_FREE_BLKP(bp) ((char *)(bp) + WSIZE)

/* Global variables */
static char *heap_listp = 0; /* Pointer to first block */
static char *segragated_lists = 0; /* Pointer to free segragated lists */

/* Function prototypes for internal helper rountines */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void insert_to_list(char *bp);
static void delete_from_list(char *bp);
static char *find_fit_list(size_t size);

//static void printblock(void *bp)
//static void checkheap(int verbose);
//static void checkblock(void *bp);


/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    if ((heap_listp = mem_sbrk(24 * WSIZE)) == (void *)-1) {
        return -1;
    }

    PUT(heap_listp, NULL);                         // block size <= 16
    PUT(heap_listp + 1 * WSIZE, NULL);           

    PUT(heap_listp + 2 * WSIZE, NULL);             // block size <= 32
    PUT(heap_listp + 3 * WSIZE, NULL);

    PUT(heap_listp + 4 * WSIZE, NULL);             // block size <= 64
    PUT(heap_listp + 5 * WSIZE, NULL);

    PUT(heap_listp + 6 * WSIZE, NULL);             // block size <= 128
    PUT(heap_listp + 7 * WSIZE, NULL);

    PUT(heap_listp + 8 * WSIZE, NULL);             // block size <= 256
    PUT(heap_listp + 9 * WSIZE, NULL);

    PUT(heap_listp + 10 * WSIZE, NULL);            // block size <= 512
    PUT(heap_listp + 11 * WSIZE, NULL);

    PUT(heap_listp + 12 * WSIZE, NULL);            // block size <= 1024
    PUT(heap_listp + 13 * WSIZE, NULL);

    PUT(heap_listp + 14 * WSIZE, NULL);            // block size <= 2048
    PUT(heap_listp + 15 * WSIZE, NULL);

    PUT(heap_listp + 16 * WSIZE, NULL);            // block size < = 4096
    PUT(heap_listp + 17 * WSIZE, NULL);

    PUT(heap_listp + 18 * WSIZE, NULL);            // block size > 4096
    PUT(heap_listp + 19 * WSIZE, NULL);

    PUT(heap_listp + 20 * WSIZE, 0);
    PUT(heap_listp + 21 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 22 * WSIZE, PACK(DSIZE, 1));
    PUT(heap_listp + 23 * WSIZE, PACK(0, 1));
    segragated_lists = heap_listp;
    heap_listp += 22 * WSIZE;
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0) {
        return NULL;
    } else {
        asize = ALIGN(size + DSIZE);
    }

    if ((bp = find_fit(asize)) != NULL) {
        place(bp, asize);
        return bp;
    }

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL) {
        return NULL;
    }
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    PUT(HDRP(ptr), PACK(size, 0));
    PUT(FTRP(ptr), PACK(size, 0));
    coalesce(ptr);
}

/* 
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) 
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc) {
        
    } else if (prev_alloc && !next_alloc) {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        delete_from_list(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    } else if (!prev_alloc && next_alloc) {
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        delete_from_list(PREV_BLKP(bp));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    } else {
        size += (GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(HDRP(NEXT_BLKP(bp))));
        delete_from_list(NEXT_BLKP(bp));
        delete_from_list(PREV_BLKP(bp));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    insert_to_list(bp);
    return bp;
}

static void insert_to_list(char *bp) 
{
    size_t size = GET_SIZE(HDRP(bp));
    char *root = find_fit_list(size);
    char *prev = root;
    char *temp = GET(NEXT_FREE_BLKP(root));
    while (temp != NULL && GET_SIZE(HDRP(temp)) < size) {
        prev = temp;
        temp = GET(NEXT_FREE_BLKP(temp));
    }
    PUT(NEXT_FREE_BLKP(bp), temp);
    PUT(PREV_FREE_BLKP(bp), prev);
    PUT(NEXT_FREE_BLKP(prev), bp);
    if (temp != NULL) {
        PUT(PREV_FREE_BLKP(temp), bp);
    }
}

static void delete_from_list(char *bp)
{
    char *prev = GET(PREV_FREE_BLKP(bp));
    char *next = GET(NEXT_FREE_BLKP(bp));
    PUT(NEXT_FREE_BLKP(prev), next);
    if (next != NULL) {
        PUT(PREV_FREE_BLKP(next), prev);
    }
    PUT(NEXT_FREE_BLKP(bp), NULL);
    PUT(PREV_FREE_BLKP(bp), NULL);
}

static char *find_fit_list(size_t size)
{
    if (size <= 16) {
        return segragated_lists;
    } else if (size <= 32) {
        return segragated_lists + DSIZE;
    } else if (size <= 64) {
        return segragated_lists + 2 * DSIZE;
    } else if (size <= 128) {
        return segragated_lists + 3 * DSIZE;
    } else if (size <= 256) {
        return segragated_lists + 4 * DSIZE;
    } else if (size <= 512) {
        return segragated_lists + 5 * DSIZE;
    } else if (size <= 1024) {
        return segragated_lists + 6 * DSIZE;
    } else if (size <= 2048) {
        return segragated_lists + 7 * DSIZE;
    } else if (size <= 4096) {
        return segragated_lists + 8 * DSIZE;
    } else {
        return segragated_lists + 9 * DSIZE;
    }
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    size_t oldsize;
    void *newptr;

    if (size == 0) {
        mm_free(ptr);
        return 0;
    }

    if (ptr == NULL) {
        return mm_malloc(size);
    }

    oldsize = GET_SIZE(HDRP(ptr));

    size_t asize = ALIGN(size + DSIZE);
    if (asize <= oldsize - 2 * DSIZE) {
        size_t left_size = oldsize - asize;
        PUT(HDRP(ptr), PACK(asize, 1));
        PUT(FTRP(ptr), PACK(asize, 1));
        PUT(HDRP(NEXT_BLKP(ptr)), PACK(left_size, 0));
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(left_size, 0));
        coalesce(NEXT_BLKP(ptr));
    } else if (asize <= oldsize) {
    } else {
        newptr = mm_malloc(size);
        if (!newptr) {
            return 0;
        }

        memcpy(newptr, ptr, size);

        mm_free(ptr);

        return newptr;
    }
    return ptr;
}

/*
 * mm_check - a heap consistency check
 */
/*
int mm_check(int verbose)
{
    checkheap(verbose);
}*/

/* 
 * The remianing routines are internal helper routines
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;
    size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
    if ((bp = mem_sbrk(size)) == (void *)-1) {
        return NULL;
    }
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));
    PUT(NEXT_FREE_BLKP(bp), NULL);
    PUT(PREV_FREE_BLKP(bp), NULL);
    return coalesce(bp);
}

/* 
 * place - Place block of asize bytes at start of free block bp
 *         and splict if remiander would be at least minimum block size
 */
static void place (void *bp, size_t asize)
{
    size_t size = GET_SIZE(HDRP(bp));
    size_t left_size = size - asize;
    delete_from_list(bp);
    if (left_size >= (2 * DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(left_size, 0));
        PUT(FTRP(bp), PACK(left_size, 0));
        insert_to_list(bp);
    } else {
        PUT(HDRP(bp), PACK(size, 1));
        PUT(FTRP(bp), PACK(size, 1));
    }
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize)
{
    char* root;
    char* temp;
    for (root = find_fit_list(asize); root != segragated_lists + 10 * DSIZE; root += DSIZE) {
        temp = GET(NEXT_FREE_BLKP(root));
        while (temp != NULL) {
            if (GET_SIZE(HDRP(temp)) >= asize) {
                return temp;
            }
            temp = GET(NEXT_FREE_BLKP(temp));
        }
    }
    return NULL;
}

/*
static void printblock(void *bp)
{
    size_t hsize, halloc, fsize, falloc;

    checkheap(0);
    hsize = GET_SIZE(HDRP(bp));
    halloc = GET_ALLOC(HDPR(bp));
    fsize = GET_SIZE(FTRP(bp));
    falloc = GET_ALLOC(FTRP(bp));

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p: header: [%ld:%c] footer: [%ld:%c]\n", bp,
        hsize, (halloc ? 'a' : 'f'),
        fsize, (falloc ? 'a' : 'f'));
}*/

/*
static void checkblock(void *bp)
{
    if ((size_t)bp % 8) {
        printf("Error: %p is not doubleword aligned\n", bp);
    }
    if (GET(HDRP(bp)) != GET(FTRP(bp))) {
        printf("Error: header does not match footer\n");
    }
}*/

/* 
 * checkheap - Minimal check of the heap for consistency
 */
/*
static void checkheap(int verbose)
{
    char *bp = heap_listp;

    if (verbose) {
        printf("Heap (%p):\n", heap_listp);
    }

    if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !(GET_ALLOC(HDRP(heap_listp)))) {
        printf("Bad prologue header\n");
    }
    checkblock(heap_listp);

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
        if (verbose) {
            printblock(bp);
        }
        checkblock(bp);
    }

    if (verbose) {
        printblock(bp);
    }
    if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp)))) {
        printf("Bad epilogue header\n");
    }
}*/














