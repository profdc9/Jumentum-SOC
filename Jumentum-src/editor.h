#ifndef _ee_h
#define _ee_h

void editor(void);

#define ESTATIC static
#define EDITORTITLE "Editor"

typedef int (*readfile)(void *v);
typedef int (*writefile)(int c, void *v);

extern readfile rf;
extern void *rf_ptr;
extern writefile wf;
extern void *wf_ptr;

#ifdef LPC2106
#define AMAX  0x8000	/* main buffer size */
#define BMAX  0x800 	/* block size */
#endif
#if defined LPC2148 || defined LPC2378 || defined LPC1768
#define AMAX  0x4000	/* main buffer size */
#define BMAX  0x400 	/* block size */
#endif
#ifdef LPC2119
#define AMAX  0x1800	/* main buffer size */
#define BMAX  0x200 	/* block size */
#endif

#ifndef AMAX 
#define AMAX 0xC000
#endif

#ifndef BMAX
#define BMAX 0x1000
#endif

#endif /* _ee_h */
