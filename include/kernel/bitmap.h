#if !defined(KERNEL_BITMAP_H)
#define KERNEL_BITMAP_H

typedef unsigned char bitmap_t;

static inline void bitmap_set(bitmap_t * const bitmap,
			      const unsigned long bit)
{
  bitmap_t *target_byte = bitmap + (bit / 8);
  *target_byte |= 1 << (bit % 8);
}

static inline void bitmap_reset(bitmap_t * const bitmap,
				const unsigned long bit)
{
  bitmap_t *target_byte = bitmap + (bit / 8);
  *target_byte &= ~(1 << (bit % 8));
}

static inline int bitmap_test(bitmap_t * const bitmap,
			      const unsigned long bit)
{
  bitmap_t *target_byte = bitmap + (bit / 8);
  return (*target_byte & (1 << (bit % 8)));
}

static inline void bitmap_set_range(bitmap_t * const bitmap,
			     const unsigned long start,
			     const unsigned long finish)
{
  unsigned int i;
  for (i = start; i <= finish; i++)
    bitmap_set(bitmap, i);
}

static inline void bitmap_reset_range(bitmap_t * const bitmap,
			       const unsigned long start,
			       const unsigned long finish)
{
  unsigned int i;
  for (i = start; i <= finish; i++)
    bitmap_reset(bitmap, i);
}

#endif /* !defined(KERNEL_BITMAP_H) */
