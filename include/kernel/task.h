#if !defined(TASK_H)
#define TASK_H

struct task_regs {
  unsigned long ebx; /* 00 */
  unsigned long ecx; /* 04 */
  unsigned long edx; /* 08 */
  unsigned long esi; /* 0c */
  unsigned long edi; /* 10 */
  unsigned long ebp; /* 14 */
  unsigned long eax; /* 18 */
  unsigned long ds; /* 1c */
  unsigned long es; /* 20 */
  unsigned long orig_eax; /* 24 */
  unsigned long eip; /* 28 */
  unsigned long cs; /* 2c */
  unsigned long eflags; /*30 */
  unsigned long oldesp; /* 34 */
  unsigned long oldss; /* 38 */
};

struct task {
  unsigned long timeslice; /* 00 */
  unsigned long time; /* 04 */
  unsigned long pid; /* 08 */
  char *name; /* 0c */
  unsigned long esp; /* 10 */
  unsigned long ss; /* 14 */
  struct task_regs *regs; /* 18 */
  struct task *task_next; /* 1c */
  unsigned int need_resched; /* 20 */
  struct task *task_prev; /* 24 */
  struct task *run_next; /* 28 */
  struct task *run_prev; /* 3c */
  enum {
    TASK_RUNNING, TASK_INTERRUPTABLE, TASK_UNINTERRUPTABLE
  } state; /* 40 */
  struct queue *queue; /* 44 queue that its blocked on */
  struct task *queue_next; /* 48 */
  struct task *queue_prev; /* 4c */
};

struct queue {
  struct task *first;
};
#define QUEUE_INIT {0}
static inline void queue_init(struct queue *q) {
  q->first = 0;
}

extern struct task idle_task;
extern struct task *current;

void schedule(void);
struct task *create_process(unsigned long pid, void (*func)(void),
			    char *name, unsigned long stack);
void wake_up_queue(struct queue *q);
void wake_up(struct task *);
asmlinkage void block(struct queue *q);

#endif /* !defined(TASK_H) */
