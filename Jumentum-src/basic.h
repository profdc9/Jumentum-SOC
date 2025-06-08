#ifndef basic_h
#define basic_h
/*
  Basic header file
  By Daniel Marks
  originally by Malcolm McLean as Minibasic.
*/

//#include "serial.h"

#undef MULTIPLE    /* support reentrancy, adds about 500 bytes to code */

#define IDLENGTH 8        /* maximum length of variable */
#define MAXFORS 8         /* maximum number of nested fors */
#define MAXGOSUB 8        /* maximum number of gosubs */
#define MAXREPEATSTACK 8  /* maximum repeatstack */
#define MAXDIMS 2         /* maximum indices for dimensions */
#define FATPTRNUM 4       /* number of pointers to fat structures */
#define BSTATIC static

#define STDINOUT      /* use stdin/stdout for print/input, otherwise you have to define it */

#ifdef INTONLY
#define numtype int
#undef USEFLOATS
#else
#define numtype double
#define USEFLOATS
#endif

#define PEEKPOKE

#ifdef INCNETWORK
#define WEBENABLED /* add web control commands */
#endif

#define NEXTLG 3   /* controls chunk size for allocation of variables */

#undef BASICASSERT

#ifdef BASICASSERT
#define ASSERTDEBUG(x) x
#else
#define ASSERTDEBUG(x) {}
#endif

typedef unsigned short stringlen;
#define MAXSTRINGLEN 65000
typedef char stringchar;

typedef struct
{
   stringlen  len;
   stringchar beginstring[1];
} STRINGARRAY;

typedef STRINGARRAY *STRINGARRAYPTR;
#define LENOFSTRINGARRAY(x) ((x)+(sizeof(STRINGARRAY)-sizeof(stringchar)+1))
#define STRINGOF(s) (&((s)->beginstring[0]))

typedef void (*addbasictoken)(int chv, void *v);

typedef struct _BASICSTATE *BASICSTATEPTR;

#ifdef MULTIPLE
#define CST (ps)
#define PASSSTATE ps,
#define VOIDPASSSTATE ps
#define ACCEPTSTATE BASICSTATEPTR ps,
#define VOIDACCEPTSTATE BASICSTATEPTR ps
#else
#define CST (&bs)
#define PASSSTATE
#define VOIDPASSSTATE
#define ACCEPTSTATE
#define VOIDACCEPTSTATE void
#endif 

int basic(const char *script);
int mainloop(VOIDACCEPTSTATE);
int setup(ACCEPTSTATE const char *script);
void cleanup(VOIDACCEPTSTATE);
int idlestate(VOIDACCEPTSTATE);

typedef struct 
{
  int inquote;
  int notlastspace;
} tokstat;

typedef struct
{
  unsigned char *s;
  int toknum;
  int charnum;
} untokstat;

void inittokenizestate(tokstat *t);
void tokenizeline(char *str, char **end, tokstat *tk, addbasictoken abt, void *v);
void inituntokenizestate(untokstat *utk, char *str);
int untokenizecode(untokstat *utk);
#define BASIC_DEBUG_TYPES 4
int basic_debug_interface(ACCEPTSTATE int debug_type, int num, char *var, int varlen, char *data, int datalen);

#endif
