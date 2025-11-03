/**
 * @file   mm.c
 * @Author 02335 team
 * @date   September, 2024
 * @brief  Memory management skeleton.
 * 
 */

#include <stdint.h>

#include "mm.h"



/* Proposed data structure elements */

typedef struct header {
  struct header * next;     // Bit 0 is used to indicate free block 
  uint64_t user_block[0];   // Standard trick: Empty array to make sure start of user block is aligned
} BlockHeader;

/* Macros to handle the free flag at bit 0 of the next pointer of header pointed at by p */
#define GET_NEXT(p)    (BlockHeader*) ((uintptr_t)p->next & ~0x1)
#define SET_NEXT(p,n)  ((p)->next = (BlockHeader*) ( ((uintptr_t)(n) & ~0x1) | ((uintptr_t)GET_FREE(p)) ))
#define GET_FREE(p)    (uint8_t) ( (uintptr_t) (p->next) & 0x1 )   /* OK -- do not change */
#define SET_FREE(p,f)  ((p)->next = (BlockHeader*) ( ((uintptr_t)((p)->next) & ~0x1) | ((uintptr_t)(f) & 0x1) ))
#define SIZE(p)        ((size_t)((uintptr_t)GET_NEXT(p) - (uintptr_t)(p + 1)))
#define MIN_SIZE     (8)   // A block should have at least 8 bytes available for the user


static BlockHeader * first = NULL;
static BlockHeader * current = NULL;

/**
 * @name    simple_init
 * @brief   Initialize the block structure within the available memory
 *
 */
void simple_init() {
  uintptr_t aligned_memory_start = (memory_start + 7) & ~0x7;
  uintptr_t aligned_memory_end   = memory_end & ~0x7;
  BlockHeader * last;

  /* Already initalized ? */
  if (first == NULL) {
    /* Check that we have room for at least one free block and an end header */
    if (aligned_memory_start + 2*sizeof(BlockHeader) + MIN_SIZE <= aligned_memory_end) {
      first = (BlockHeader *) aligned_memory_start;
      last  = (BlockHeader *) (aligned_memory_end - sizeof(BlockHeader));

      SET_NEXT(first, last);
      SET_FREE(first, 1);

      SET_NEXT(last, first);
      SET_FREE(last, 0);  /* End block is not free */
    }
    current = first;
  } 
}


/**
 * @name    simple_malloc
 * @brief   Allocate at least size contiguous bytes of memory and return a pointer to the first byte.
 *
 * This function should behave similar to a normal malloc implementation. 
 *
 * @param size_t size Number of bytes to allocate.
 * @retval Pointer to the start of the allocated memory or NULL if not possible.
 *
 */
void* simple_malloc(size_t size) {
  
  if (first == NULL) {
    simple_init();
    if (first == NULL) return NULL;
  }

  size_t aligned_size = (size + 7) & ~0x7;

  /* Search for a free block */
  BlockHeader * search_start = current;
  do {
 
    if (GET_FREE(current)) {

      /* Possibly coalesce consecutive free blocks here */
      BlockHeader *next = GET_NEXT(current);
      while (GET_FREE(next) && next != first) {
        SET_NEXT(current, GET_NEXT(next));
        next = GET_NEXT(current);
      }

      /* Check if free block is large enough */
      if (SIZE(current) >= aligned_size) {
        /* Will the remainder be large enough for a new block? */
        if (SIZE(current) - aligned_size < sizeof(BlockHeader) + MIN_SIZE) {
          /* Use block as is, marking it non-free */
          SET_FREE(current, 0);
          void *next = (void *)(current + 1);
          current = GET_NEXT(current);
          return next;
        }
        BlockHeader *new_block = (BlockHeader *)((uintptr_t)(current + 1) + aligned_size);

        SET_NEXT(new_block, GET_NEXT(current));
        SET_FREE(new_block, 1);      // mark the rest as free
        SET_NEXT(current, new_block);
        SET_FREE(current, 0);        // mark current as used

        void *user_ptr = (void *)(current + 1);
        current = new_block;
        return user_ptr;
      }
    }
    current = GET_NEXT(current);
  } while (current != search_start);
 /* None found */
  return NULL;
}


/**
 * @name    simple_free
 * @brief   Frees previously allocated memory and makes it available for subsequent calls to simple_malloc
 *
 * This function should behave similar to a normal free implementation. 
 *
 * @param void *ptr Pointer to the memory to free.
 *
 */
void simple_free(void * ptr) {
  if (ptr == NULL) return;
  if ((uintptr_t)ptr < memory_start || (uintptr_t)ptr >= memory_end) return;

  BlockHeader *block = (BlockHeader *)ptr - 1;
  if (GET_FREE(block)) {
    /* Block is not in use -- probably an error */
    return;
  }
  SET_FREE(block, 1);

  /* Possibly coalesce consecutive free blocks here */
  BlockHeader *next = GET_NEXT(block);
  if (GET_FREE(next)) {
    /* Merge: skip over next block */
    SET_NEXT(block, GET_NEXT(next));
  }

  BlockHeader *prev = first;
  while (GET_NEXT(prev) != block && GET_NEXT(prev) != first) {
    prev = GET_NEXT(prev);
  }

  if (GET_FREE(prev)) {
    /* Merge previous and current (already possibly merged forward) */
    SET_NEXT(prev, GET_NEXT(block));
    block = prev;  // merged block now starts at prev
  }

  current = block;
}
/* Include test routines */

#include "mm_aux.c"
