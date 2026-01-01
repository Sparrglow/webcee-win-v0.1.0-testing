#ifndef WEBCEE_MEMORY_POOL_H
#define WEBCEE_MEMORY_POOL_H

#include <stddef.h>

typedef struct MemoryBlock MemoryBlock;

typedef struct {
    MemoryBlock* first_block;
    MemoryBlock* current_block;
    size_t block_size;         // Default size for new blocks
    size_t position;           // Offset in the current block
} MemoryPool;

// Create a new memory pool
// default_block_size: Size of memory blocks (e.g., 4096)
MemoryPool* memory_pool_create(size_t default_block_size);

// Destroy the memory pool and free all allocated memory
void memory_pool_destroy(MemoryPool* pool);

// Allocate memory from the pool
// Returns a pointer to the allocated memory, or NULL on failure
void* memory_pool_alloc(MemoryPool* pool, size_t size);

// Duplicate a string using memory from the pool
char* memory_pool_strdup(MemoryPool* pool, const char* src);

#endif // WEBCEE_MEMORY_POOL_H
