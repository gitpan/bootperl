#if !defined(KERNEL_MEMORY_H)
#define KERNEL_MEMORY_H

unsigned short page_allocate(void);
unsigned short page_allocate_consecutive(unsigned short count);
void page_release(unsigned short page);
void *kalloc(unsigned int n);
void kfree(void *mem);
void mem_free(void *mem, unsigned int n, int freepages);

static const int page_size = 4096;

static inline void *page_to_addr(unsigned short page)
{ return (void *)(page * page_size); }

static inline unsigned short addr_to_page(void *addr)
{ return (long)addr / page_size; }

#endif /* !defined(KERNEL_MEMORY_H) */
