#ifndef MEM_RESET_ALLOCATOR
#define MEM_RESET_ALLOCATOR
#include <stddef.h>
#include <stdint.h>

#define IS_POW2(a) ((a) && (((a) & ((a) - 1))  == 0))
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define IS_ALIGNED(x, a) (((uintptr_t)(x) & ((a) - 1)) == 0)

struct Reset_Allocator{
  uintptr_t base;
  uintptr_t top;
  size_t size;
  size_t failed_allocs;
};

/*
 * Initialize an allocator with given memory and size.
*/
struct Reset_Allocator reset_allocator_init(void* mem, size_t size){
  struct Reset_Allocator allocator;
  allocator.base = (uintptr_t)mem;
  allocator.top = allocator.base;
  allocator.size = size;
  allocator.failed_allocs = 0;
  return allocator;
}

/*
 * Tries to allocate 'size' bytes of memory.
 * Returns NULL if unsuccessful.
*/
void* reset_allocator_alloc(struct Reset_Allocator* allocator, size_t size){
  if(allocator == NULL || size == 0){
    return NULL;
  }
  uintptr_t limit = allocator->base + allocator->size;
  if (allocator->top > limit || size > (limit - allocator->top)){
    allocator->failed_allocs++;
    return NULL;
  }
  uintptr_t ret = allocator->top;
  allocator->top = allocator->top + size;
  return (void*)ret;
}

/*
 * Tries to allocate 'size' bytes of aligned memory.
 * Returns NULL if unsuccessful.
*/
void* reset_allocator_aligned_alloc(struct Reset_Allocator* allocator, size_t size, size_t align){
  if(!IS_POW2(align)){
    return NULL;
  }
  if(allocator == NULL || size == 0 || allocator->base == (uintptr_t)NULL){
    return NULL;
  }

  uintptr_t aligned_top = ALIGN_UP(allocator->top, align);
  uintptr_t limit = allocator->base + allocator->size;

  if (aligned_top > limit || size > (limit - aligned_top)) {
      allocator->failed_allocs++;
      return NULL;
  }

  uintptr_t ret = aligned_top;
  allocator->top = (aligned_top + size);
  return (void*)ret;
}

/*
 * Resets the 'top' pointer to equal base.
 * No pointers allocated before reset should not be used.
*/
void reset_allocator_reset(struct Reset_Allocator* allocator){
  if(allocator == NULL){
    return;
  }
  allocator->top = allocator->base;
  allocator->failed_allocs = 0;
}

/*
 * Returns the amount of space used
*/
size_t reset_allocator_used(struct Reset_Allocator* allocator) {
    return allocator->top - allocator->base;
}


/*
 * Sets all pointers to NULL.
*/
void reset_allocator_destroy(struct Reset_Allocator* allocator){
  if(allocator == NULL){
    return;
  }else if (allocator->base == (uintptr_t)NULL){
    return;
  }
  allocator->base = (uintptr_t)NULL;
  allocator->top = (uintptr_t)NULL;
  allocator->size = 0;
}


#undef IS_POW2
#undef ALIGN_UP
#undef ALIGN_DOWN
#undef IS_ALIGNED
#endif
