#ifndef ___ALIGNED_ALLOC__H___
#define ___ALIGNED_ALLOC__H___

// Alignment must be power of 2 (1,2,4,8,16...)
inline void* aligned_malloc(size_t alignment, size_t size) {
    uintptr_t r = (uintptr_t)malloc(size + --alignment + sizeof(uintptr_t));
    uintptr_t t = r + sizeof(uintptr_t);
    uintptr_t o =(t + alignment) & ~(uintptr_t)alignment;
    if (!r) return NULL;
    ((uintptr_t*)o)[-1] = r;
    return (void*)o;
}

inline void aligned_free(void* p) {
    if (!p) return;
    free((void*)(((uintptr_t*)p)[-1]));
}

template <typename T>
inline void mem_fill(T *address, T value, size_t size) {
  for (size_t i = 0; i < size; i++) { address[i] = value; }
}

#endif  // #ifndef ___ALIGNED_ALLOC__H___
