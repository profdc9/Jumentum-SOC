
/* sbrk implementation */

typedef char *caddr_t;

#define STACK_CLEARANCE 0x100

caddr_t _sbrk (int incr);
register char *stack_ptr asm ("sp");
extern unsigned long __end_of_text__;
char *heap_end;

void outstrhex(const char *blah, unsigned long x);

#ifdef USEDLMALLOC
void *_mysbrk (int incr)
#else
caddr_t _sbrk (int incr)
#endif
{
  extern char end asm ("end");  /* Defined by the linker.  */
  char *prev_heap_end;

  //deboutstrhex("heap_end=",heap_end);
  //deboutstrhex("_end=",&end);
  //deboutstrhex("stack_ptr=",stack_ptr);

  if (!heap_end)
    heap_end = & end;

  prev_heap_end = heap_end;

  if ((heap_end + incr + STACK_CLEARANCE) > stack_ptr)
  {
    /* errno = ENOMEM; */
    return (caddr_t) -1;
  }

  heap_end += incr;

//  outstrhex("prev_heap_end=",prev_heap_end);
#ifdef USEDLMALLOC
  return (void *) prev_heap_end;
#else
  return (caddr_t) prev_heap_end;
#endif
}

