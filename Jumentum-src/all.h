#ifndef _ALL_H_
#define _ALL_H_

#if USEDLMALLOC
#define malloc dlmalloc
#define free dlfree
#define realloc dlrealloc

void *dlmalloc(size_t);
void dlfree(void *);
void *dlrealloc(void *,size_t);
#endif

#ifdef MYFUNC
#undef isalnum
#undef isalpha
#undef isdigit
#undef isspace
#undef isprint
#undef toupper
#define isdigit myisdigit
#define isspace myisspace
#define isalnum myisalnum
#define isalpha myisalpha
#define isprint myisprint
#define toupper mytoupper
#undef tolower
#define tolower mytolower
#define memmove mymemmove
#define memcpy mymemcpy
#define memset mymemset
#define memcmp mymemcmp
#define strcpy mystrcpy
#define strncpy mystrncpy
#define strcat mystrcat
#define strncat mystrncat
#define strlen mystrlen
#define strchr mystrchr
#define strrchr mystrrchr
#define strcmp mystrcmp
#define strcasecmp mystrcasecmp
#define strncasecmp mystrncasecmp
#define strncmp mystrncmp
#endif

#endif
