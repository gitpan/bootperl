#if !defined(ASM_IO_H)
#define ASM_IO_H

/* The file defines functions for accessing ioports.
   Most of the weird code for this taken from Linux 2.2.4 */

#define __SLOW_DOWN_IO "\noutb %%al,$0x80"

/*
 * Talk about misusing macros..
 */
#define __OUT1(s,x) \
extern inline void out##s(unsigned x value, unsigned short port) {

#define __OUT2(s,s1,s2) \
__asm__ __volatile__ ("out" #s " %" s1 "0,%" s2 "1"

#define __OUT(s,s1,x) \
__OUT1(s,x) __OUT2(s,s1,"w") : : "a" (value), "Nd" (port)); } \
__OUT1(s##_p,x) __OUT2(s,s1,"w") __SLOW_DOWN_IO : : "a" (value), "Nd" (port));} \

#define __IN1(t,s) \
extern inline t in##s(unsigned short port) { t _v;

#define __IN2(s,s1,s2) \
__asm__ __volatile__ ("in" #s " %" s2 "1,%" s1 "0"

#define __IN(t,s,s1,i...) \
__IN1(t,s) __IN2(s,s1,"w") : "=a" (_v) : "Nd" (port) ,##i ); return _v; } \
__IN1(t,s##_p) __IN2(s,s1,"w") __SLOW_DOWN_IO : "=a" (_v) : "Nd" (port) ,##i ); return _v; } \

#define __INS(s) \
extern inline void ins##s(unsigned short port, void * addr, unsigned long count) \
{ __asm__ __volatile__ ("cld ; rep ; ins" #s \
: "=D" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }

#define __OUTS(s) \
extern inline void outs##s(unsigned short port, const void * addr, unsigned long count) \
{ __asm__ __volatile__ ("cld ; rep ; outs" #s \
: "=S" (addr), "=c" (count) : "d" (port),"0" (addr),"1" (count)); }

__IN(unsigned char, b,"")
__IN(unsigned short, w,"")
__IN(unsigned int, l,"")

__OUT(b,"b",char)
__OUT(w,"w",short)
__OUT(l,,int)

__INS(b)
__INS(w)
__INS(l)

__OUTS(b)
__OUTS(w)
__OUTS(l)

#endif /* !defined(ASM_IO_H) */
