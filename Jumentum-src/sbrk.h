
#ifndef _SBRK_H
#define _SBRK_H

#ifdef USEDLMALLOC
void *_mysbrk (int incr)
#else
caddr_t _sbrk (int incr)
#endif
 
 #endif   /* SBRK_H */
