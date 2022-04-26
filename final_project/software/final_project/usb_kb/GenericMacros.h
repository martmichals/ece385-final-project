#ifndef __GENERIC_MACROS_H_
#define __GENERIC_MACROS_H_

/* Generic macros */

/* Word <> two chars. Works both ways */
#define LOBYTE(x) ((char*)(&(x)))[0]
#define HIBYTE(x) ((char*)(&(x)))[1]

/* Bit set/clear */
#define bitset(var, bitno) ((var) |= 1 << (bitno))
#define bitclr(var, bitno) ((var) &= ~(1 << (bitno)))

#endif // __GENERIC_MACROS_H_
