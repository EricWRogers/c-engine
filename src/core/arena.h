#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct {
  size_t size;
  void* data;
  bool used;
} BlockInfo;

typedef struct {
  void* data;
  size_t size;
  BlockInfo* _freeList;
} Arena;

extern Arena arena_init(size_t _size);
extern void arena_free(Arena* _arena);
extern void* arena_alloc(Arena* _arena, const size_t _size);
extern void* arena_realloc(Arena* _arena, void* _target, size_t _size);
extern void arena_dealloc(Arena* _arena, void* _target);
extern void _arena_dealloc(Arena* _arena, unsigned int _index);
