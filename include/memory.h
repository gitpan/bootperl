#if !defined(MEMORY_H)
#define MEMORY_H

typedef unsigned int size_t;

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);

#endif /* !defined(MEMORY_H) */
