/* The main program for perl */
#include <kernel/kernel.h>

#include "EXTERN.h"
#include "perl.h"

static PerlInterpreter *my_perl;

static int do_perl(int argc, char **argv, char **env);

void perl_main(void)
{
  int exitstatus, argc = 3;
  char *argv[] = {
    "vmkernel",
    "-e",
    "print \"Hello World\\n\"",
    0,
  };
  char *env[] = {
    0,
  };

  printk("\n\n");

  printk("-- Starting Perl\n");

  exitstatus = do_perl(argc, argv, env);
  printk("-- Exited with %d\n", exitstatus);

  printk("-- Perl finished\n");
  while (1)
    /* do nothing */;
}

extern int debug_phase;

static int do_perl(int argc, char **argv, char **env)
{
  int exitstatus;

  printk("do_perl(%d, %p, %p)\n", argc, argv, env);

  debug_phase = 1;

  PERL_SYS_INIT3(&argc,&argv,&env);

  my_perl = perl_alloc();
  perl_construct(my_perl);
  PL_perl_destruct_level = 0;
  PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

  debug_phase = 2;

  exitstatus = perl_parse(my_perl, NULL, argc, argv, env);
  if (!exitstatus)
    perl_run(my_perl);

  debug_phase = 3;

  exitstatus = perl_destruct(my_perl);

  perl_free(my_perl);

  PERL_SYS_TERM();

  debug_phase = 4;

  return exitstatus;
}
