#include <kernel/kernel.h>
#include <kernel/linkage.h>
#include <kernel/task.h>
#include <kernel/memory.h>
#include <asm/lock.h>
#include <memory.h>

struct task idle_task = {0, 0, 0, "idle", 0, 0, 0, 0, 1, 0, 0, 0};
struct task *current = &idle_task;

extern int video_init;
int debug_phase = 0;

asmlinkage void switch_to(struct task *next, struct task *prev);

void update_status(struct task *next, struct task *prev)
{
  static volatile unsigned short *vidmem = (unsigned short *)0xb8000;
  unsigned int i;
  char msg1[] = "Kernel operational ";
  char msg2[50];

  if (!video_init) return;

  sprintf(msg2, "[%d] %li <%08x> %s", debug_phase, next->pid, (int)current,
	  next->name);

  vidmem[80 * 24 + 0] = 16 + 0x1e00;
  vidmem[80 * 24 + 79] = 17 + 0x1e00;
  for (i = 1; i < 79; i++) {
    vidmem[80 * 24 + i] = ' ' + 0x1700;
  }
  for (i = 0; (msg1[i] != 0) && (i < 50); i++) {
    vidmem[80 * 24 + i + 1] = msg1[i] + 0x1700;
  }
  vidmem[80 * 24 + i + 1] = 179 + 0x1000;
  for (i = 0; (msg2[i] != 0) && (i < 50); i++) {
    vidmem[80 * 24 + i + 42] = msg2[i] + 0x1700;
  }
}

void schedule(void)
{
  struct task *prev = current;
  struct task *next = 0;

  lock_kernel();

  next = prev->run_next;

  if (prev->state != TASK_RUNNING) {
    prev->run_prev->run_next = prev->run_next;
    prev->run_next->run_prev = prev->run_prev;
    prev->run_next = 0;
    prev->run_prev = 0;
  }
  else {
    prev->need_resched = 0;
    prev->timeslice = 100;
  }

  printk(KERN_DEBUG "%x %.10s %lx\n", (unsigned int)next, next->name,
	 (unsigned long)next->run_next);

  /* begin rewrite */
/*   if ((prev != &idle_task) && (prev->run_next != &idle_task)) { */
/*     next = prev->run_next; */
/*   } */
/*   else { */
  if ((prev == &idle_task) || (next == &idle_task)) {
    if (idle_task.run_next != 0) {
      next = idle_task.run_next;
    }
    else {
      next = &idle_task;
    }
  }
  current = next;
  printk(KERN_DEBUG "Schedule: %x, _kernel_lock: %ld, next->0x10: %lx, next->0x14: %lx\n", (unsigned int)current, _kernel_lock, next->esp, next->ss);
  printk(KERN_DEBUG "  next=%lx, prev=%lx\n", (long)next, (long)prev);
  unlock_kernel();
  update_status(next, prev);
  switch_to(next, prev);
  /* end rewrite */
}

struct task *create_process(unsigned long pid, void (*func)(void),
			    char *name, unsigned long stack)
{
  void *new_stack = kalloc(stack); /* 1k stack is plenty */
  void restore(void);
  struct task *t = kalloc(sizeof(struct task));
  struct task_regs *regs = (struct task_regs *)(new_stack + stack
						- sizeof(struct task_regs));
  void *esp = new_stack + stack - sizeof(struct task_regs) - 12;

  lock_kernel();

  *(long *)(esp + 8) = 0; /* this is for restore to pop as current->regs */
  *(long *)(esp + 4) = (long)restore;
  *(long *)(esp + 0) = 0; /* flags for switch_to */

  t->timeslice = 100;
  t->time = 0;
  t->pid = pid;
  t->name = name;
  t->esp = (unsigned long)esp;
  t->ss = 0x18;
  t->regs = regs;
  t->task_next = 0;
  t->task_prev = 0;
  t->run_next = 0;
  t->run_prev = 0;
  t->state = TASK_RUNNING;
  t->queue = 0;
  t->queue_next = 0;
  t->queue_prev = 0;

  if (idle_task.task_next == 0) {
    idle_task.task_next = t;
    idle_task.task_prev = t;
    t->task_next = &idle_task;
    t->task_prev = &idle_task;
  }
  else {
    idle_task.task_prev->task_next = t;
    t->task_prev = idle_task.task_prev;
    idle_task.task_prev = t;
    t->task_next = &idle_task;
  }

  if (idle_task.run_next == 0) {
    printk(KERN_DEBUG "Create process %ld idle_task.run_next == 0\n", pid);
    idle_task.run_next = t;
    idle_task.run_prev = t;
    t->run_next = &idle_task;
    t->run_prev = &idle_task;
  }
  else {
    idle_task.run_prev->run_next = t;
    t->run_prev = idle_task.run_prev;
    idle_task.run_prev = t;
    t->run_next = &idle_task;
    printk(KERN_DEBUG "Create process %ld idle_task.run_next != 0 t->run_next=%lx t=%lx\n", pid, (unsigned long)t->run_next, (unsigned long)t);
  }

  memset(regs, 0, sizeof(struct task_regs));

  regs->ds = 0x18;
  regs->es = 0x18;
  regs->eip = (unsigned long)func;
  regs->cs = 0x10;
  regs->eflags = 9 << 1; /* bit 9: set=if on */

  unlock_kernel();

  return t;
}

void do_block(struct queue *queue)
{ /* also add bottom halves */
  long flags;

  current->state = TASK_INTERRUPTABLE; /* schedule will deal with the
					  run queue */
  current->need_resched = 1;
  current->queue = queue;

  printk(KERN_DEBUG "Blocking %ld on queue %x\n", current->pid, (int)queue);

  if (queue->first == 0) {
    queue->first = current;
  }
  else {
    struct task *first = queue->first;
    if (first->queue_next == 0) {
      first->queue_next = current;
      first->queue_prev = current;
      current->queue_next = first;
      current->queue_prev = first;
    }
    else {
      first->queue_prev->queue_next = current;
      current->queue_prev = first->queue_prev;
      first->queue_prev = current;
      current->queue_next = first;
    }
  }
  _save_flags(&flags);
  printk(KERN_DEBUG "do_block: _kernel_lock=%ld %01ld\n", _kernel_lock,
	 flags & (1 << 9));
}

void wake_up_queue(struct queue *q)
{
  struct task *t, *next;

  lock_kernel();
  t = q->first;

  printk(KERN_DEBUG "Waking all on queue %x\n", (int)q);
  while (t != 0) {
    printk(KERN_DEBUG "Waking %ld on queue %x\n", t->pid, (int)q);
    next = t->queue_next;
    t->queue_prev = 0;
    t->queue_next = 0;
    wake_up(t);
    t = next;
    if (t == q->first)
      break;
  }

  q->first = 0;
  unlock_kernel();
}

void wake_up(struct task *t)
{
  lock_kernel();
  t->state = TASK_RUNNING;
  if (idle_task.run_next == 0) {
    idle_task.run_next = t;
    idle_task.run_prev = t;
    t->run_next = &idle_task;
    t->run_prev = &idle_task;
  }
  else {
    idle_task.run_prev->run_next = t;
    t->run_prev = idle_task.run_prev;
    idle_task.run_prev = t;
    t->run_next = &idle_task;
  }
  unlock_kernel();
}

void show_tasks(void)
{
  struct task *current = &idle_task;
  const char *states[] = {"running", "interruptable", "uninterruptable"};

  printk("Task list:\n");

  do {
    printk("  Task %ld (%08lx): \"%s\" %s next=%08lx\n",
	   current->pid, (unsigned long)current, current->name,
	   states[current->state], (unsigned long)current->task_next);
    current = current->task_next;
  } while (current != &idle_task);
}
