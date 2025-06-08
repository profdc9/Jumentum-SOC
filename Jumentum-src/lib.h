#ifndef _lib_h
#define _lib_h

void initstdout(long x);
int outchar(int ch);
int inchar(void);
int inkey(void);

#define raw_initstdout(x) init_serial_c(console_uart,x,8,1,0)
#define raw_outchar(x) putchar_serial_c(console_uart,x)
#define raw_inchar() waitkey_serial_c(console_uart)
#define raw_inkey() getkey_serial_c(console_uart)

#define cputs(s)  	outstr(s)
#define putch(c)  	outchar(c)
#define getch()     inchar()
#define ttopen()
#define ttclose()

void clreol(void);
void clrscr(void);
void gotoxy(int x, int y);
void highvideo(void);
void lowvideo(void);

#define ITOASIZE 20
#define DTOASIZE 40

void outstrint(const char *s, int x);
void deboutstrint(const char *s, int x);
void deboutstrhex(const char *s, int x);
void deboutstr(const char *s, const char *d, int len);
void outstrhex(const char *s, unsigned int x);
void outstr(const char *x);
int inputstr(char *str, int len);
void outint(int x);

char *myltoa(char *p, long n);
char *myltoapad(char *p, long n, int dig);
char *myltoahex(char *p, unsigned long n);
long mystrtol(const char *str, const char **end);
unsigned long mystrtolhex(const char *str, const char **end);
#ifdef USEFLOATS
char *mydtoa(char *p, double n);
double mystrtod(const char *str, const char **end);
#endif

int myrand(void);
void mysrand(unsigned int seed);
#define MYRAND_MAX 32767

const char *skipspaces(const char *str);
const char *skipspaceslf(const char *str);

char *mystrcpynonull(char *dest, const char *src, size_t n);

#ifdef MYFUNC
int myisalnum(int ch);
int myisalpha(int ch);
int mytoupper(int ch);
int mytolower(int ch);
int myisdigit(int ch);
int myisprint(int ch);
int myisspace(int ch);
int mymemcmp(const void *s1, const void *d2, size_t n);
void *mymemmove(void *s1, const void *s2, size_t n);
void *mymemcpy(void *s1, const void *s2, size_t n);
void *mymemset(void *s1, int ch, size_t n);
char *mystrcpy(char *dest, const char *src);
char *mystrcat(char *dest, const char *src);
size_t mystrlen(const char *s);
const char *mystrchr(const char *s, int ch);
const char *mystrrchr(const char *s, int ch);
int mystrcmp(const char *s1, const char *s2);
int mystrcasecmp(const char *s1, const char *s2);
int mystrncasecmp(const char *s1, const char *s2, size_t n);
char *mystrncpy(char *dest, const char *src, size_t len);
char *mystrncat(char *dest, const char *src, size_t len);
int mystrncmp(const char *s1, const char *s2, size_t n);
#endif

#endif /* _lib_h */
