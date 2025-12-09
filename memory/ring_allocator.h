#ifndef MEM_RING_ALLOCATOR
#define MEM_RING_ALLOCATOR
#include "stddef.h"
#include "stdbool.h"
#include <stdint.h>

#define IS_POW2(a) ((a) && (((a) & ((a) - 1))  == 0))
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define IS_ALIGNED(x, a) (((uintptr_t)(x) & ((a) - 1)) == 0)

struct Ring_Block_Allocator{
  uintptr_t base;
  size_t head;
  size_t tail;
  size_t block_size;
  size_t nr_blocks;
  bool full;
};


/*
 * Allocates a structure of block_size*nr_blocks bytes to the base pointer.
 * All pointers are NULL if allocation fails.
*/
struct Ring_Block_Allocator ring_block_allocator_init(void* mem, size_t block_size, size_t nr_blocks) {
  struct Ring_Block_Allocator allocator = {0};

  // Preconditions: power-of-two block size, alignment-safe, and overflow-safe
  if ((block_size & (block_size - 1)) != 0) return allocator;
  if (block_size % sizeof(void*) != 0) return allocator;
  if (nr_blocks == 0 || block_size > SIZE_MAX / nr_blocks) return allocator;

  allocator.base = (uintptr_t)mem;
  if(allocator.base == 0){
    return allocator;
  }
  allocator.block_size = block_size;
  allocator.nr_blocks = nr_blocks;

  allocator.head = 0;
  allocator.tail = 0;

  allocator.full = false;

  return allocator;
}

/*
 * Returns a pointer for the next block in the ring.
 * Returns NULL if head has caught up to tail.
*/
void* ring_block_allocator_alloc(struct Ring_Block_Allocator* allocator){
  if(allocator == NULL || allocator->base == 0 || allocator->full == true){
    return NULL;
  }

  uintptr_t ret = allocator->base + (allocator->head * allocator->block_size);
  allocator->head = (allocator->head + 1) & (allocator->nr_blocks - 1);
  allocator->full = (allocator->head == allocator->tail);
  return (void*)ret;

}

/*
 * Marks the block free and moves 'tail' appropriately
*/
void ring_block_allocator_free(struct Ring_Block_Allocator* allocator, void* block_ptr){
  if (!allocator || !allocator->base || !block_ptr) return;

  uintptr_t expected = allocator->base + (allocator->tail * allocator->block_size);
  if ((uintptr_t)block_ptr != expected) {
    return;
  }

  allocator->tail = (allocator->tail + 1) & (allocator->nr_blocks - 1);
  allocator->full = false;
}

/*
 * Sets all pointers to NULL.
*/
void ring_block_allocator_destroy(struct Ring_Block_Allocator* allocator){
  if(allocator == NULL){
    return;
  }else if (allocator->base == 0){
    return;
  }
  allocator->base = 0;
  allocator->tail = 0;
  allocator->head = 0;
  allocator->block_size = 0;
  allocator->nr_blocks = 0;
}

#undef IS_POW2
#undef ALIGN_UP
#undef ALIGN_DOWN
#undef IS_ALIGNED


#endif
