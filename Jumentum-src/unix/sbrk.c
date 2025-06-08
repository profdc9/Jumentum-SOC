
/* sbrk implementation */

#include <stdio.h>
#include <stdlib.h>

#include "malloc.h"

typedef char *caddr_t;

caddr_t _sbrk (int incr);

#define HEAP_SIZE 262144

char  jumentum_memory[HEAP_SIZE];

char *heap_end = NULL;
char *heap_alloc = NULL;

#ifdef USEDLMALLOC
void *_mysbrk (int incr)
#else
caddr_t _sbrk (int incr)
#endif
{
  char *prev_heap_end;

  if (!heap_end) {
    heap_end = jumentum_memory;
    heap_alloc = heap_end + HEAP_SIZE;
    if (!heap_end)
      return -1;
  }

  /*  printf("heap end=%p %p %d\n",heap_end,heap_alloc,incr); */
  prev_heap_end = heap_end;

  if ((heap_end + incr) > heap_alloc)
  {
    /* errno = ENOMEM; */
    return (caddr_t) -1;
  }

  heap_end += incr;

#ifdef USEDLMALLOC
  return (void *) prev_heap_end;
#else
  return (caddr_t) prev_heap_end;
#endif
}

