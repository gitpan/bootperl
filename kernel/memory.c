#include <kernel/kernel.h>
#include <kernel/memory.h>
#include <assert.h>
#include <asm/init.h>
#include <asm/io.h>
#include <asm/lock.h>
#include <assert.h>

struct freepage {
  unsigned short pages;
  struct freepage *next;
};
static struct freepage pages_head = {0, 0};

struct freeblock {
  unsigned int size;
  struct freeblock *next;
};
struct blockheader {
  unsigned int size;
};
static struct freeblock blocks_head = {0, 0};

void printpages(void); void printblocks(void);
void mem_free(void *mem, unsigned int n, int freepages);

void show_memory(void);

long pages;

/* NB: *memory_start is page aligned already */
__initfunc(void kalloc_init(unsigned long *memory_start,
			      unsigned long *memory_end))
{
  /* store the free pages in a linked list stored in the pages themselves */
  /* first free page is at *memory_start */
  pages_head.next = (void *)*memory_start;
  pages_head.next->pages = (*memory_end - *memory_start) / page_size;
  pages_head.next->next = 0;
  printpages(); /* print the initial freepages list */

  /* initialise the structures for the free memory blocks */
  /* nothing to do here */

  pages = *memory_end / page_size;
}

void printpages(void)
{
  struct freepage *p = &pages_head;
  printk(KERN_DEBUG "Current free pages list:\n");

  while (p != 0) {
    printk(KERN_DEBUG "  Start = page %d, length = %d pages, next = page %d\n",
	   addr_to_page(p), p->pages, addr_to_page(p->next));
    p = p->next;
  }
}

void printblocks(void)
{
  struct freeblock *p = &blocks_head;
  printk(KERN_DEBUG "Current free blocks list:\n");

  lock_kernel();
  while (p != 0) {
    printk(KERN_DEBUG "  Start = 0x%x, size = 0x%x, next = 0x%x\n",
	   (unsigned int)p, p->size, (unsigned int)p->next);
    p = p->next;
  }
  unlock_kernel();
}

void show_memory(void)
{
  unsigned long free_memory = 0UL;
  struct freepage *p = &pages_head;
  struct freeblock *b = &blocks_head;

  while (p != 0) {
    free_memory += p->pages * page_size;
    p = p->next;
  }
  while (b != 0) {
    free_memory += b->size;
    b = b->next;
  }

  printk("Free Memory: %ldk/%ldk\n", free_memory / 1024,
	 pages * page_size / 1024);
}

unsigned short page_allocate(void)
{
  unsigned short page_number;

  /* runtime condition check */
  if (pages_head.next == 0)
    panic("Out of pages to allocate");

  /* logic error check */
  assert(pages_head.next->pages != 0);

  page_number = addr_to_page(pages_head.next);
  page_number += pages_head.next->pages - 1;

  if (--pages_head.next->pages == 0)
    pages_head.next = pages_head.next->next;

  return page_number;
}

unsigned short page_allocate_consecutive(unsigned short count)
{
  struct freepage *cur, *prev;
  unsigned short page_number;

  /* parameter check */
  assert(count > 0);

  /* runtime condition check */
  if (pages_head.next == 0)
    panic("Out of pages to allocate");

  /* logic error check */
  assert(pages_head.next->pages != 0);

  prev = &pages_head;
  cur = pages_head.next;
  page_number = addr_to_page(cur);
  while (cur != 0) {
    if (cur->pages >= count) {
      cur->pages -= count;
      page_number += cur->pages;
      if (cur->pages == 0)
	prev->next = cur->next;

      break;
    }

    prev = cur;
    cur = prev->next;
    page_number = addr_to_page(cur);
  }

  if (cur == 0)
    panic("Not enough consecutive pages to complete alloc");

  return page_number;
}

void page_release(unsigned short page)
{
  struct freepage *cur = page_to_addr(page);
  struct freepage *q;
  struct freepage *p;
  unsigned short n = 1;

  /* B1 */
  q = &pages_head;
  
  /* B2 */
  p = q->next;
  while (!((p == 0) || (p > cur))) {
    q = p;
    p = q->next;
  }
  
  /* B3 */
  if (((page + n) == addr_to_page(p)) && (p != 0)) {
    n = n + p->pages;
    cur->next = p->next;
  }
  else {
    cur->next = p;
  }

  /* B4 */
  if ((addr_to_page(q) + q->pages) == page) {
    q->pages = q->pages + n;
    q->next = cur->next;
  }
  else {
    q->next = cur;
    cur->pages = n;
  }
}

void *kalloc(unsigned int n)
{
  struct freeblock *p;
  struct freeblock *q;
  unsigned int k;
  struct blockheader *h;
  unsigned int realsize = n + sizeof(struct blockheader);

  lock_kernel();

  printk(KERN_DEBUG "start of kalloc(%d) freeblock=%x\n", n, sizeof(struct freeblock));
  printblocks();

  /* A1 */
  q = &blocks_head;

  /* A2 */
  p = q->next;
  while ((p == 0) ||
      ( p->size < (realsize + sizeof(struct freeblock)) ) ) {
    if (p == 0) {
      /* allocate a new page to store the data */
      int pages = (realsize + 4095) / 4096;
      printk(KERN_DEBUG "XXX: pages=%d n=%d realsize=%d\n", pages, n, realsize);
      void *mem = page_to_addr(page_allocate_consecutive(pages));
      mem_free(mem, pages * 4096, 0);
      /* reset the search */
      p = &blocks_head;
    }
    q = p;
    p = q->next;
  }

  /* A4 */
  k = p->size - realsize;
  if (k == 0) {
    q->next = p->next;
  }
  else {
    p->size = k;
  }

  h = (void *)p + k;
  h->size = n;

  printk(KERN_DEBUG "kalloc for realsize=%x\n", realsize);
  printblocks();
  printk(KERN_DEBUG "kalloc returns %lx\n", (unsigned long)h + sizeof(struct blockheader));

  unlock_kernel();
  return (void *)h + sizeof(struct blockheader);
}

/* I hate pointer arithmetic... look at all those void * casts... ugghh!!! */

void kfree(void *mem)
{
  struct blockheader *h = mem - sizeof(struct blockheader);
  unsigned int n = h->size + sizeof(struct blockheader);

  mem_free((void *)h, n, 1);
}

/* TODO: free unused pages */
void mem_free(void *mem, unsigned int n, int freepages)
{
  struct freeblock *cur = mem;
  struct freeblock *q;
  struct freeblock *p;

  /* B1 */
  q = &blocks_head;

  /* B2 */
  p = q->next;
  while (!((p == 0) || (p > cur))) {
    q = p;
    p = q->next;
  }

  /* B3 */
  if ((((void *)cur + n) == p) && (p != 0)) {
    n = n + p->size;
    cur->next = p->next;
  }
  else
    cur->next = p;
  
  /* B4 */
  if (((void *)q + q->size) == cur) {
    q->size = q->size + n;
    q->next = cur->next;
  }
  else {
    q->next = cur;
    cur->size = n;
  }

  if (freepages && (cur->size >= page_size)) {
    unsigned long start, c, size;
    /* page align the address into start and adjust our size */
    start = ((unsigned long)cur + page_size - 1) & ~(page_size - 1);
    size = cur->size - (start - (unsigned long)cur);
    /* count how many pages we can free */
    c = size / page_size;
    while (c > 0) {
      page_release(addr_to_page((void *)start));
      start += page_size;
      c--;
    }
  }
}
