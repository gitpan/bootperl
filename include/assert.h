/*
 *	ISO C Standard: 4.2 DIAGNOSTICS	<assert.h>
 */

#ifdef	_ASSERT_H
# undef	_ASSERT_H
# undef	assert
#endif /* assert.h	*/

#define	_ASSERT_H	1

#ifdef __STRING
# undef __STRING
#endif
#define __STRING(expr) #expr

/* void assert (int expression);

   If NDEBUG is defined, do nothing.
   If not, and EXPRESSION is zero, print an error message and abort.  */

#ifdef	NDEBUG
# define assert(expr)		((void) 0)
#else
/* This prints an "Assertion failed" message and aborts.  */
extern void __assert_fail (__const char *__assertion,
			   __const char *__file,
			   unsigned int __line,
			   __const char *__function)
     /*__attribute__ ((__noreturn__))*/;

# define assert(expr)							      \
  ((void) ((expr) ? 0 :							      \
	   (__assert_fail (__STRING(expr),				      \
			   __FILE__, __LINE__, __ASSERT_FUNCTION), 0)))

# ifdef __GNUC__
#  if __GNUC__ > 2 || (__GNUC__ == 2 \
	               && __GNUC_MINOR__ >= (defined __cplusplus ? 6 : 4))
#   define __ASSERT_FUNCTION	__PRETTY_FUNCTION__
#  else
#   define __ASSERT_FUNCTION	((__const char *) 0)
#  endif
# else
#  if defined __STDC_VERSION__ && __STDC_VERSION__ >= 199901L
#   define __ASSERT_FUNCTION	__func__
#  else
#   define __ASSERT_FUNCTION	((__const char *) 0)
#  endif
# endif

#endif /* NDEBUG.  */
