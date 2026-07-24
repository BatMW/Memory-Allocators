# Memory Allocators

A collection of specialized allocators written in C. Each allocator is designed for a specific allocation pattern rather than as a general-purpose replacement for `malloc()`.

## Features

- No hidden allocations. All allocators except the Page Ring Buffer perform no internal memory allocations. The caller supplies all required memory during initialization, making the library suitable for embedded systems and other environments without an operating system.
- O(1) allocation and deallocation (where applicable).
- Single-header implementations.

## Allocator Implementations
| Allocator        | Free Order | Variable Size | Typical Use                       |
| ---------------- | ---------- | ------------- | --------------------------------- |
| Reset            | Reset all  | y             | Scratch memory                    |
| Stack            | LIFO       | y             | Recursive or nested allocations   |
| Pool             | Any        | n             | Fixed-size objects                |
| Ring             | FIFO       | y             | Queues and streaming              |
| Page Ring Buffer | N/A        | N/A           | High-performance circular buffers |

### Reset Allocator

Also known as an **Arena** or **Bump Allocator**.

Allocations simply advance an offset into a memory region. Individual allocations cannot be freed. Instead, the entire allocator is reset at once.

Useful as temporary scratch memory or for grouping allocations with the same lifetime.

### Stack Allocator

Allocations behave like a stack. Each allocation stores enough information to restore the previous state. `realloc()` can grow the most recent allocation, and `free()` must occur in reverse order of allocation.

Useful when allocation order naturally follows a stack discipline.

### Pool Allocator

Manages a fixed number of equally sized blocks using a free list embedded within unused blocks.

Useful for frequently allocating and freeing objects of the same (or similar) size.

### Ring Allocator

Similar to the Pool Allocator, but blocks must be freed in the same order they were allocated (FIFO).

Useful for producer/consumer workloads and streaming data.

### Page Ring Buffer

Maps three contiguous virtual memory regions to the same physical memory. This allows up to one region's worth of data to be read or written from any position without explicit wraparound or overflow checks.

Implementations are provided for both Linux and Windows.
