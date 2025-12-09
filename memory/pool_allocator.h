#ifndef MEM_POOL_ALLOCATOR
#define MEM_POOL_ALLOCATOR
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <limits.h>


#define IS_POW2(a) ((a) && (((a) & ((a) - 1))  == 0))
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define IS_ALIGNED(x, a) (((uintptr_t)(x) & ((a) - 1)) == 0)

struct Pool_Memory_Block{
  struct Pool_Memory_Block* next;
};

struct Pool_Allocator{
  size_t block_size;
  size_t nr_blocks;
  struct Pool_Memory_Block* head;
  uintptr_t base;
};

#ifdef DBG
  #include "assert.h"
#endif

/*
** Returns the memory required for nr_blocks.
** Assumes that the memory is aligned to block_size.
** Returns 0 if total size is larger than SIZE_MAX
*/
size_t pool_allocator_calculate_mem_requirements(size_t block_size, size_t nr_blocks){
  if (nr_blocks == 0) return 0;
  if (block_size > SIZE_MAX / nr_blocks) return 0;
  return block_size * nr_blocks;
}

/*
** Initializes a pool allocator.
** mem should be aligned to block_size.
 */
struct Pool_Allocator pool_allocator_init(void* mem, size_t block_size, size_t nr_blocks){
  struct Pool_Allocator allocator = {0};
  if (mem == NULL) return allocator;
  if (block_size < sizeof(struct Pool_Memory_Block)) return allocator;
  if (!IS_POW2(block_size)) return allocator;

  if (nr_blocks != 0 && block_size > SIZE_MAX / nr_blocks) return allocator;
  if(!IS_ALIGNED((mem), block_size)) return allocator;

  allocator.base = (uintptr_t)mem;
  allocator.block_size = block_size;
  allocator.nr_blocks = nr_blocks;
  allocator.head = (struct Pool_Memory_Block*)allocator.base;

  uintptr_t block_iterator = allocator.base;
  struct Pool_Memory_Block* block_ptr = allocator.head;
  for (size_t i = 0; i < nr_blocks - 1; ++i) {
    block_iterator += block_size;
    block_ptr->next = (struct Pool_Memory_Block*)block_iterator;
    block_ptr = block_ptr->next;
  }
  block_ptr->next = NULL;
  return allocator;
}

/*
** Returns a pointer to the next free block, NULL if full.
 */
void* pool_allocator_alloc(struct Pool_Allocator* allocator){
  #ifdef DBG
    assert(allocator != NULL);
    assert(!(allocator->base == NULL));
  #endif
  if(allocator == NULL)return NULL;
  if(allocator->base == (uintptr_t)NULL || allocator->head == NULL)return NULL;
  void* ret = (void*)allocator->head;
  allocator->head = allocator->head->next;
  return ret;
}

/*
** Frees a previously allocated block.
** The pointer must point to the base of the block.
 */
bool pool_allocator_free(struct Pool_Allocator* allocator, void* ptr){
  uintptr_t p = (uintptr_t)ptr;
  #ifdef DBG
    assert(allocator != NULL);
    assert(!(allocator->base == NULL));
    assert(!(p > (allocator->base + (allocator->nr_blocks*allocator->block_size))));
    assert(IS_ALIGNED((p - allocator->base), allocator->block_size));
  #endif
  if(allocator == NULL)return false;
  if(allocator->base == (uintptr_t)NULL)return false;
  if (p < allocator->base || p >= (allocator->base + (allocator->nr_blocks * allocator->block_size))) return false;
  if (!IS_ALIGNED((p - allocator->base), allocator->block_size)) return false;

  struct Pool_Memory_Block* insert = (struct Pool_Memory_Block*)ptr;
  insert->next = allocator->head;
  allocator->head = insert;
  return true;
}

/*
** Zeroise the allocator.
** The memory must be freed by the user.
 */
bool pool_allocator_destroy(struct Pool_Allocator* allocator){
  if(allocator == NULL)return false;
  if(allocator->base == (uintptr_t)NULL)return false;
  allocator->block_size = 0;
  allocator->nr_blocks = 0;
  allocator->head = NULL;
  allocator->base = 0;
  return true;
}

#undef IS_POW2
#undef ALIGN_UP
#undef ALIGN_DOWN
#undef IS_ALIGNED

#endif // MEM_POOL_ALLOCATOR
