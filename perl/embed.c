/* embed.c
 * a small embedded perl interpreter for learning
 * wrapping isn't turned on for this
 * this should be built the same way as the kernel
 */

#include "EXTERN.h"
#include "perl.h"

static PerlInterpreter *my_perl;

int main(int argc, char **argv, char **env)
{
  int exitstatus;

  PERL_SYS_INIT3(&argc,&argv,&env);

  my_perl = perl_alloc();
  perl_construct(my_perl);
  PL_perl_destruct_level = 0;
  PL_exit_flags |= PERL_EXIT_DESTRUCT_END;
  exitstatus = perl_parse(my_perl, NULL, argc, argv, (char **)NULL);
  if (!exitstatus)
    perl_run(my_perl);

  exitstatus = perl_destruct(my_perl);

  perl_free(my_perl);

  PERL_SYS_TERM();

  exit(exitstatus);
  return exitstatus;
}
