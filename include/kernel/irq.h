#if !defined(KERNEL_IRQ_H)
#define KERNEL_IRQ_H

extern void disable_irq(unsigned char irq);
extern void enable_irq(unsigned char irq);
extern int pending_irq(unsigned char irq);
extern void ack_irq(unsigned char irq);

struct irq_handler {
  unsigned char irq;
  void (*handler)(struct irq_handler *h);
  const char *name;
  struct irq_handler *next;
  struct irq_handler *prev;
};

extern void register_irq_handler(struct irq_handler *handler);
#endif /* !defined(KERNEL_IRQ_H) */
