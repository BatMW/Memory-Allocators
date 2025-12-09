#ifndef MEM_STACK_ALLOCATOR
#define MEM_STACK_ALLOCATOR
#include<stdlib.h>
#include<stddef.h>
#include<stdbool.h>
#include<limits.h>
#include<stdint.h>

#define IS_POW2(a) ((a) && (((a) & ((a) - 1))  == 0))
#define ALIGN_UP(x, a)   (((x) + ((a) - 1)) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define IS_ALIGNED(x, a) (((uintptr_t)(x) & ((a) - 1)) == 0)


struct Stack_Frame{
  struct Stack_Frame* previous;
  uintptr_t start;
  size_t size;
};

struct Stack_Allocator{
  uintptr_t base;
  size_t stack_size;
  struct Stack_Frame* top;

};



struct Stack_Allocator stack_allocator_init(void* mem, size_t size){
  struct Stack_Allocator allocator = {};
  if(size <= 0){
    return allocator;
  }

  allocator.base = (uintptr_t)mem;
  allocator.top = NULL;
  if(allocator.base != (uintptr_t)NULL){
    allocator.stack_size = size;
  }
  return allocator;
}

void* stack_allocator_alloc(struct Stack_Allocator* allocator, size_t size){
  if(allocator == NULL || size == 0){
    return NULL;
  }
  if(allocator->top == NULL){
    if(allocator->stack_size < sizeof(struct Stack_Frame) + size){
      return NULL;
    }else{
      struct Stack_Frame* new_top = (struct Stack_Frame*)allocator->base;
      new_top->size = size;
      new_top->previous = allocator->top;
      new_top->start = (uintptr_t)(new_top+1);
      allocator->top = new_top;
      return (void*)allocator->top->start;
    }
  }
  uintptr_t end_of_stack = allocator->base + allocator->stack_size;
  uintptr_t next_frame_position = (uintptr_t)allocator->top + allocator->top->size + sizeof(struct Stack_Frame);
  if((end_of_stack - next_frame_position) < size + sizeof(struct Stack_Frame)){
    return NULL;
  }

  struct Stack_Frame* new_top = (struct Stack_Frame*)(allocator->top + allocator->top->size + sizeof(struct Stack_Frame));
  new_top->size = size;
  new_top->previous = allocator->top;
  new_top->start = (uintptr_t)(new_top+1);
  allocator->top = new_top;
  return (void*)new_top->start;
}

void* stack_allocator_aligned_alloc(struct Stack_Allocator* allocator, size_t size, size_t align) {
  if (allocator == NULL || size == 0 || align == 0) {
    return NULL;
  }

  uintptr_t base = allocator->base;
  uintptr_t end_of_stack = base + allocator->stack_size;

  struct Stack_Frame* new_top = NULL;
  uintptr_t raw_mem = 0;
  uintptr_t aligned_ptr = 0;
  size_t padding = 0;

  if (allocator->top == NULL) {
    // First allocation
    new_top = (struct Stack_Frame*)allocator->base;
    raw_mem = (uintptr_t)(new_top + 1);
    aligned_ptr = ALIGN_UP(raw_mem, align);
    padding = (size_t)((uintptr_t)aligned_ptr - (uintptr_t)raw_mem);

    size_t total_needed = sizeof(struct Stack_Frame) + padding + size;
    if ((size_t)(end_of_stack - (uintptr_t)new_top) < total_needed) {
      return NULL;
    }

    new_top->size = padding + size;
    new_top->previous = NULL;
    new_top->start = aligned_ptr;
    allocator->top = new_top;
    return (void*)aligned_ptr;
  }

  // Subsequent allocations
  uintptr_t next_frame_pos = (uintptr_t)allocator->top + sizeof(struct Stack_Frame) + allocator->top->size;
  new_top = (struct Stack_Frame*)next_frame_pos;
  raw_mem = (uintptr_t)(new_top + 1);
  aligned_ptr = ALIGN_UP(raw_mem, align);
  padding = (size_t)(aligned_ptr - raw_mem);

  size_t total_needed = sizeof(struct Stack_Frame) + padding + size;
  if ((size_t)(end_of_stack - (uintptr_t)new_top) < total_needed) {
    return NULL;
  }

  new_top->size = padding + size;
  new_top->previous = allocator->top;
  new_top->start = aligned_ptr;
  allocator->top = new_top;
  return (void*)aligned_ptr;
}

bool stack_allocator_realloc(struct Stack_Allocator* allocator, void* ptr, size_t new_size){
  if(allocator == NULL || ptr == NULL || allocator->base == 0 || allocator->top == NULL){
    return false;
  }
  uintptr_t p = (uintptr_t)ptr;
  if(p != allocator->top->start ){
    return false;
  }
  if(p + new_size > allocator->base + allocator->stack_size ){
    return false;
  }
  allocator->top->size = new_size;
  return true;
}

bool stack_allocator_free(struct Stack_Allocator* allocator, void* ptr){
  if(allocator == NULL || ptr == NULL || allocator->top == NULL){
    return false;
  }
  uintptr_t p = (uintptr_t)ptr;
  if(p != allocator->top->start){
    return false;
  }
  allocator->top = allocator->top->previous;
  return true;
}

bool stack_allocator_destroy(struct Stack_Allocator* allocator){
  if(allocator == NULL){
    return false;
  }
  if(allocator->base == 0){
    return false;
  }
  allocator->base = 0;
  allocator->stack_size = 0;
  allocator->top = NULL;
  return true;
}
#undef IS_POW2
#undef ALIGN_UP
#undef ALIGN_DOWN
#undef IS_ALIGNED

#endif
