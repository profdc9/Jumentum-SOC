/* Jumentum-SOC

  Copyright (C) 2007 by Daniel Marks

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
  
  Daniel L. Marks profdc9@gmail.com

*/

/*****************************************************************
*                           Mini BASIC                           *
*                        by Malcolm McLean                       *
*                           version 1.0                          *
*                        (modified by DLM)                       *
*  DLM changes under same permissive license as original McLean  *
*  license                                                       *
*****************************************************************/

/*
License from 
http://www.personal.leeds.ac.uk/~bgy1mm/Minibasic/MiniBasicHome.html

You can do virtually anything you like with this code, including incorporating
it into your own programs, or modifying as the basis for a scripting language
of your own. It would be nice to be acknowledged but I don't insist on it - you
can pretend that you created the program on your own if it makes your boss
happy. The only thing you can't do is restrict my rights in the program in
any way. So any derivative works or enhancements I can use as I see fit.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

#include "all.h"
#include "basic.h"
#include "lpc2000/io.h"
#include "lib.h"
#include "processor.h"
#ifdef WEBENABLED
#include "uip/apps/webserver/httpd.h"
#include "uip/apps/smtp/smtp.h"
#include "uip/apps/socket/socket.h"
#endif
#ifdef INCFATFS
#include "fatfs/ff.h"
#endif
#include "main.h"

typedef signed char int8;
typedef signed short int16;
typedef signed int int32;
typedef unsigned char tokennum;
typedef signed short dimsize;
#define MAXDIMSIZE 32000
typedef signed short linenumbertype;
typedef unsigned short scriptoffset;   /* offset of bytes into script file */

typedef STRINGARRAYPTR (* stringfunc)(VOIDACCEPTSTATE);
typedef scriptoffset (* docommand)(VOIDACCEPTSTATE);
typedef numtype (* numfunc)(VOIDACCEPTSTATE);

#define TOKEN_NONE 0
#define TOKEN_CMD 1
#define TOKEN_CMDCTRL 2
#define TOKEN_NUMFUNC 3
#define TOKEN_STRFUNC 4

typedef struct
{
#ifdef SMALLTOKENS
	char const tokenname[4];
#else
	char const tokenname[7];
#endif
	int8 length;
	int8 tokentp;
	void *func;
} tokenlist;

typedef struct
{
  char           id[IDLENGTH];           /* line label */
  scriptoffset   ofs;                    /* points to offset at start of line */
} LABELOFFSET;

typedef union
{
  numtype       dval;	 	    /* its value if a real */
  STRINGARRAY   *sval;			/* its value if a string (malloced) */
} VARIABLEDATA;

typedef struct
{
  char         id[IDLENGTH];	/* id of variable */
  tokennum     type;		    /* its type, STRID or FLTID */
  VARIABLEDATA d;               /* data in variable */
} VARIABLE;

typedef union
{
  STRINGARRAY   **str;	        /* pointer to string data */
  numtype       *dval;	        /* pointer to real data */
} DIMVARPTR;

typedef struct
{
  char          id[IDLENGTH];	/* id of dimensioned variable */
  tokennum      type;		    /* its type, STRID or FLTID */
  int8          ndims;	        /* number of dimensions */
  dimsize       dim[MAXDIMS];	/* dimensions in x y order */
  DIMVARPTR     d;              /* pointers to string/real data */
} DIMVAR;

typedef union
{
  STRINGARRAY   **sval;			/* pointer to string data */
  numtype       *dval;		    /* pointer to real data */
} LVALUEDATA;

typedef struct			
{
  int           type;			/* type of variable (STRID or FLTID or ERROR) */   
  LVALUEDATA    d;              /* data pointed to by LVALUE */
} LVALUE;

typedef struct
{
  char           id[IDLENGTH];  /* id of control variable */
  scriptoffset   nextoffset;	/* line below FOR to which control passes */
  numtype        toval;			/* terminal value */
  numtype        step;			/* step size */
} FORLOOP;

#ifdef INCFATFS

typedef union
{
  FIL    *fil;
  DIR    *dir;
} FATPTR;

#define FATFSNONEPTR 0
#define FATFSDIRPTR 1
#define FATFSFILPTR 2

typedef unsigned char fatfsptrtype;

typedef struct
{
  FATPTR fptr;
  fatfsptrtype ftype;
} FATPTRTYPE;
#endif

typedef struct _BASICSTATE
{
#define Sidlestate (CST->idlestate)
    int idlestate;
#define Svariables (CST->variables)
	VARIABLE *variables;		/* the script's variables */
#define Snvariables (CST->nvariables)
	int nvariables;				/* number of variables */
#define Sdimvariables (CST->dimvariables)
	DIMVAR *dimvariables;		/* dimensioned arrays */
#define Sndimvariables (CST->ndimvariables)
	int ndimvariables;			/* number of dimensioned arrays */
#define Slabels (CST->labels)
	LABELOFFSET *labels;		/* list of labels */
#define Snlabels (CST->nlabels)
	int nlabels;				/* number of labels in program */
#define Sstring (CST->string)
	const char *string;         /* string we are parsing */
#define Snexttoken (CST->nexttoken)
	const char *nexttoken;      /* pointer to next token */
#define Sscriptstart (CST->scriptstart)
    const char *scriptstart;    /* start of script we are executing */
#define Serrorflag (CST->errorflag)
    int errorflag;              /* set when error in input encountered */
#define Slasterrorflag (CST->lasterrorflag)
	int lasterrorflag;          /* last error that was encountered */
#define Scuroffset (CST->curoffset)
	scriptoffset curoffset;    /* offset of beginning of current line */
#define Scurinputpos (CST->curinputpos)
    int curinputpos;
#define Scurinputmax (CST->curinputmax)
    int curinputmax;
#define Scurinputptr (CST->curinputptr)
	char *curinputptr;
#define Snfors (CST->nfors)
	int nfors;					 /* number of fors on stack */
#define Sngosub (CST->ngosub)
	int ngosub;
#define Snrepeats (CST->nrepeats)
	int nrepeats;
#define Serroroffset (CST->erroroffset)
	scriptoffset erroroffset;   /* line last error occured on */
#define Sonerroroffset (CST->onerroroffset)
	scriptoffset onerroroffset; /* if we set on error then we go to this line */
#ifdef INCFATFS
#define Sfatptrs (CST->fatptrs)
	FATPTRTYPE fatptrs[FATPTRNUM];
#endif
#define Sgosubstack (CST->gosubstack)
	scriptoffset gosubstack[MAXGOSUB];
#define Srepeatstack (CST->repeatstack)
	scriptoffset repeatstack[MAXREPEATSTACK];
#define Sforstack (CST->forstack) 
	FORLOOP forstack[MAXFORS];   /* stack for for loop conrol */
#define Stoken (CST->token)
	tokennum token;             /* current token (lookahead) */
} BASICSTATE;

/* tokens defined */
#define EOS 0 
#define VALUE 1

#define ERROR 2
#define STRID 3
#define FLTID 4
#define DIMFLTID 5
#define DIMSTRID 6
#define LABEL 7

/* error codes (in BASIC script) defined */

#define ERRCODES ((sizeof(errormessages)/sizeof(const char *))-1)

BSTATIC void reporterror(ACCEPTSTATE int lineno);
BSTATIC scriptoffset findline(ACCEPTSTATE char *id);
BSTATIC scriptoffset getlabel(VOIDACCEPTSTATE);
BSTATIC int findlinenumber(ACCEPTSTATE scriptoffset ofs);
BSTATIC scriptoffset getnextline(VOIDACCEPTSTATE);
BSTATIC scriptoffset line(ACCEPTSTATE int noctrl);
BSTATIC scriptoffset doprint(VOIDACCEPTSTATE);
BSTATIC scriptoffset dopoke(VOIDACCEPTSTATE);
BSTATIC scriptoffset doclrscr(VOIDACCEPTSTATE);
BSTATIC scriptoffset doclreol(VOIDACCEPTSTATE);
BSTATIC scriptoffset doend(VOIDACCEPTSTATE);
BSTATIC scriptoffset dooneset(VOIDACCEPTSTATE);
BSTATIC scriptoffset dotwoset(VOIDACCEPTSTATE);
BSTATIC scriptoffset dothreeset(VOIDACCEPTSTATE);
BSTATIC scriptoffset dolet(VOIDACCEPTSTATE);
BSTATIC scriptoffset domidstring(VOIDACCEPTSTATE);
BSTATIC scriptoffset dodim(VOIDACCEPTSTATE);
BSTATIC scriptoffset doonerror(VOIDACCEPTSTATE);
BSTATIC scriptoffset doif(VOIDACCEPTSTATE);
BSTATIC scriptoffset dogoto(VOIDACCEPTSTATE);
BSTATIC scriptoffset doabort(VOIDACCEPTSTATE);
BSTATIC scriptoffset dorepeat(VOIDACCEPTSTATE);
BSTATIC scriptoffset dountil(VOIDACCEPTSTATE);
BSTATIC scriptoffset dogosub(VOIDACCEPTSTATE);
BSTATIC scriptoffset doreturn(VOIDACCEPTSTATE);
BSTATIC scriptoffset doinput(VOIDACCEPTSTATE);
BSTATIC scriptoffset dofor(VOIDACCEPTSTATE);
BSTATIC scriptoffset donext(VOIDACCEPTSTATE);
BSTATIC scriptoffset dodoline(VOIDACCEPTSTATE);
BSTATIC scriptoffset doadstart(VOIDACCEPTSTATE);
BSTATIC scriptoffset doadstop(VOIDACCEPTSTATE);
BSTATIC scriptoffset doserinit(VOIDACCEPTSTATE);
BSTATIC void lvalue(ACCEPTSTATE LVALUE *lv);
BSTATIC numtype expr(VOIDACCEPTSTATE);
BSTATIC numtype term1(VOIDACCEPTSTATE);
BSTATIC numtype term2(VOIDACCEPTSTATE);
BSTATIC numtype term3(VOIDACCEPTSTATE);
BSTATIC numtype factor(VOIDACCEPTSTATE);
BSTATIC numtype instr(VOIDACCEPTSTATE);
BSTATIC numtype binstr(VOIDACCEPTSTATE);
#ifdef USEFLOATS
BSTATIC numtype num_pi(VOIDACCEPTSTATE);
BSTATIC numtype num_sin(VOIDACCEPTSTATE);
BSTATIC numtype num_cos(VOIDACCEPTSTATE);
BSTATIC numtype num_tan(VOIDACCEPTSTATE);
BSTATIC numtype num_log(VOIDACCEPTSTATE);
BSTATIC numtype num_exp(VOIDACCEPTSTATE);
BSTATIC numtype num_pow(VOIDACCEPTSTATE);
BSTATIC numtype num_sqrt(VOIDACCEPTSTATE);
BSTATIC numtype num_asin(VOIDACCEPTSTATE);
BSTATIC numtype num_acos(VOIDACCEPTSTATE);
BSTATIC numtype num_atan(VOIDACCEPTSTATE);
#endif
BSTATIC numtype num_abs(VOIDACCEPTSTATE);
BSTATIC numtype num_inkey(VOIDACCEPTSTATE);
BSTATIC numtype num_peek(VOIDACCEPTSTATE);
BSTATIC numtype num_len(VOIDACCEPTSTATE);
BSTATIC numtype num_ascii(VOIDACCEPTSTATE);
BSTATIC numtype num_err(VOIDACCEPTSTATE);
BSTATIC numtype num_int(VOIDACCEPTSTATE);
BSTATIC numtype num_rnd(VOIDACCEPTSTATE);
BSTATIC numtype num_iif(VOIDACCEPTSTATE);
BSTATIC numtype num_val(VOIDACCEPTSTATE);
BSTATIC numtype num_vallen(VOIDACCEPTSTATE);
BSTATIC numtype num_inadc(VOIDACCEPTSTATE);
BSTATIC numtype num_inplen(VOIDACCEPTSTATE);
BSTATIC numtype num_setidle(VOIDACCEPTSTATE);
BSTATIC numtype num_adrdbuf(VOIDACCEPTSTATE);
BSTATIC numtype num_adwrite(VOIDACCEPTSTATE);
BSTATIC numtype num_timeset(VOIDACCEPTSTATE);
BSTATIC numtype num_adwrlft(VOIDACCEPTSTATE);
BSTATIC numtype num_serout(VOIDACCEPTSTATE);
BSTATIC numtype num_serspi(VOIDACCEPTSTATE);
BSTATIC numtype num_serbng(VOIDACCEPTSTATE);
#ifdef WEBENABLED
BSTATIC numtype webcon(VOIDACCEPTSTATE);
BSTATIC numtype webout(VOIDACCEPTSTATE);
BSTATIC numtype webmail(VOIDACCEPTSTATE);
BSTATIC numtype scwrite(VOIDACCEPTSTATE);
BSTATIC numtype scnewdt(VOIDACCEPTSTATE);
BSTATIC numtype scacept(VOIDACCEPTSTATE);
BSTATIC numtype sccon(VOIDACCEPTSTATE);
BSTATIC numtype sciscon(VOIDACCEPTSTATE);
BSTATIC numtype scclose(VOIDACCEPTSTATE);
#endif
#ifdef INCFATFS
BSTATIC numtype bfclose(VOIDACCEPTSTATE);
BSTATIC numtype bfdel(VOIDACCEPTSTATE);
BSTATIC numtype bfdircl(VOIDACCEPTSTATE);
BSTATIC numtype bfdirop(VOIDACCEPTSTATE);
BSTATIC numtype bfeof(VOIDACCEPTSTATE);
BSTATIC numtype bfmkdir(VOIDACCEPTSTATE);
BSTATIC numtype bfopen(VOIDACCEPTSTATE);
BSTATIC numtype bfseek(VOIDACCEPTSTATE);
BSTATIC numtype bfsync(VOIDACCEPTSTATE);
BSTATIC numtype bftell(VOIDACCEPTSTATE);
BSTATIC numtype bfwrite(VOIDACCEPTSTATE);
#endif
BSTATIC numtype variable(VOIDACCEPTSTATE);
BSTATIC numtype dimvariable(VOIDACCEPTSTATE);

BSTATIC VARIABLE *findvariable(ACCEPTSTATE const char *id, int *nearest);
BSTATIC DIMVAR *finddimvar(ACCEPTSTATE const char *id, int *nearest);
BSTATIC DIMVAR *dimension(ACCEPTSTATE const char *id, int ndims, dimsize *intdims);
BSTATIC void *getdimvar(ACCEPTSTATE DIMVAR *dv, dimsize *intdims);
BSTATIC VARIABLE *addfloat(ACCEPTSTATE const char *id);
BSTATIC VARIABLE *addstring(ACCEPTSTATE const char *id);
BSTATIC VARIABLE *addnewvar(ACCEPTSTATE const char *id);
BSTATIC DIMVAR *adddimvar(ACCEPTSTATE const char *id);
BSTATIC LABELOFFSET *addnewlabel(VOIDACCEPTSTATE);
BSTATIC int nextlargest(int n);

BSTATIC STRINGARRAY *ercstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *stringexpr(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *chrstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *strstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *leftstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *rightstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *midstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *stringstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *hexstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *iistring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *binstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *trimstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *upperstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *confstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *adreadstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *serinpstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *seri2cstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *timegetstring(VOIDACCEPTSTATE);
#ifdef WEBENABLED
BSTATIC STRINGARRAY *webreqstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *webfrmstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *webescstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *webpststring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *screadstring(VOIDACCEPTSTATE);
#endif
#ifdef INCFATFS
BSTATIC STRINGARRAY *bfdirrdstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *bfreadstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *bfreadlstring(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *bfinfostring(FILINFO *finfo);
BSTATIC STRINGARRAY *bfstatstring(VOIDACCEPTSTATE);
#endif
BSTATIC STRINGARRAY *stringdimvar(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *stringvar(VOIDACCEPTSTATE);
BSTATIC STRINGARRAY *stringliteral(VOIDACCEPTSTATE);

BSTATIC STRINGARRAY *createstringarray(int len);
BSTATIC STRINGARRAY *stringarraydup(STRINGARRAY *str);
BSTATIC STRINGARRAY *stringarrayduppart(STRINGARRAY *str, int offset, int len);
BSTATIC STRINGARRAY *stringarraychar(const char *str);
BSTATIC STRINGARRAY *stringarraycharlen(const char *str, int len);
BSTATIC STRINGARRAY *stringarrayconcat(STRINGARRAY *s1, STRINGARRAY *s2);
BSTATIC void         mystrgrablit(STRINGARRAY *dest, const char *src);  /* only used in mystringgrablit */
BSTATIC int          stringarraycmp(STRINGARRAY *s1, STRINGARRAY *s2);
BSTATIC int          strstrarray(STRINGARRAY *str, STRINGARRAY *substr, int offset);
BSTATIC STRINGARRAY *stringarraysub(STRINGARRAY *str, STRINGARRAY *sub, int ind, int len);
BSTATIC STRINGARRAY *stringarrayresize(STRINGARRAY *str, int newlen);
BSTATIC int input_handler(VOIDACCEPTSTATE);

BSTATIC void match(ACCEPTSTATE int tok);
BSTATIC void matchoparen(ACCEPTSTATE int tok1);
BSTATIC int istokenmatch(ACCEPTSTATE int tok);
BSTATIC void seterror(ACCEPTSTATE int errorcode);

BSTATIC tokennum gettoken(VOIDACCEPTSTATE);
/* BSTATIC int tokenlen(ACCEPTSTATE const char *str, tokennum token); */

BSTATIC int isstring(tokennum token);
BSTATIC numtype getvalue(const char *str, int *len);
BSTATIC void getid(ACCEPTSTATE const char *str, char *out, int *len);

BSTATIC char *mystrend(const char *str, char quote);   /* only used in stringliteral */

BSTATIC const char *errormessages[] =
{ 
#define ERR_CLEAR 0
   "No Error",                     /* ERR_CLEAR */
#define ERR_SYNTAX 1
   "Syntax error",                  /* ERR_SYNTAX */
#define ERR_OUTOFMEMORY 2
   "Out of memory",                 /* ERR_OUTOFMEMORY */
#define ERR_IDTOOLONG 3
   "Identifier too long",           /* ERR_IDTOOLONG */
#define ERR_NOSUCHVARIABLE 4
   "No such variable",              /* ERR_NOSUCHVARIABLE */
#define ERR_BADSUBSCRIPT 5
   "Bad subscript",                 /* ERR_BADSUBSCRIPT */
#define ERR_TOOMANYDIMS 6
   "Too many dimensions",           /* ERR_TOOMANYDIMS */
#define ERR_TOOMANYINITS 7
   "Too many initialisers",         /* ERR_TOOMANYINITS */
#define ERR_BADTYPE 8
   "Illegal type",                  /* ERR_BADTYPE */
#define ERR_TOOMANYFORS 9
   "Too many nested fors",          /* ERR_TOOMANYFORS */
#define ERR_NONEXT 10
   "For without next",              /* ERR_NONEXT */
#define ERR_NOFOR 11
   "Next without for",              /* ERR_NOFOR */
#define ERR_DIVIDEBYZERO 12
   "Divide by zero",                /* ERR_DIVIDEBYZERO */
#define ERR_EOF 13
   "End of file",                   /* ERR_EOF */
#define ERR_TYPEMISMATCH 14
   "Type mismatch",                 /* ERR_TYPEMISMATCH */
#define ERR_INPUTTOOLONG 15
   "Input too long",                /* ERR_INPUTTOOLONG */
#define ERR_BADVALUE 16
   "Bad value",                     /* ERR_BADVALUE */
#define ERR_NOTINT 17
   "Not an int",                    /* ERR_NOTINT */
#define ERR_TOOMANYGOSUB 18
   "Too many GOSUB",                /* ERR_TOOMANYGOSUB */
#define ERR_NORETURN 19
   "RETURN without GOSUB",          /* ERR_NORETURN */
#define ERR_TOOMANYREPEAT 20     
   "Too many nested REPEAT",        /* ERR_TOMANYREPEAT */
#define ERR_NOREPEAT 21 
   "UNTIL without REPEAT",          /* ERR_NOREPEAT */
#define ERR_NOLABEL 22 
   "Label not found",               /* ERR_NOLABEL */
#ifdef USEFLOATS
#define ERR_NEGLOG 23
   "Negative LOG",                  /* ERR_NEGLOG */
#define ERR_NEGSQRT 24
   "Negative SQRT",                 /* ERR_NEGSQRT */
#define ERR_BADSINCOS 25
   "SIN or COS out of range",       /* ERR_BADSINCOS */
#endif
    0 };

#define TOKENBASE 130

#define QUOTE '\"'
#define BITAND '&'
#define OPAREN '('
#define CPAREN ')'
#define MULT '*'
#define PLUS '+'
#define COMMA ','
#define MINUS '-'
#define DIV '/'
#define SEMICOLON ';'
#define LESS '<'
#define EQUALS '='
#define GREATER '>'
#define BITOR '|'
#define BITNOT '~'
#define BITXOR '^'
#define SHORTPRINT '?'

tokenlist const tl[] = {
#define ABORT (TOKENBASE+1)
    { "ABORT",  -5, TOKEN_CMDCTRL, doabort },
#define ABS (ABORT+1)
	{ "ABS",    -3, TOKEN_NUMFUNC, num_abs },
#ifdef USEFLOATS
#define ACOS (ABS+1)
	{ "ACOS",   -4, TOKEN_NUMFUNC, num_acos },
#define ADRDBUF (ACOS+1)
#else
#define ADRDBUF (ABS+1)
#endif
	{ "ADRDBUF", -7, TOKEN_NUMFUNC, num_adrdbuf },
#define ADREADSTRING (ADRDBUF+1)
    { "ADREAD$", 7,  TOKEN_STRFUNC, adreadstring },
#define ADSTART (ADREADSTRING+1)
	{ "ADSTART", -7, TOKEN_CMD, doadstart },
#define ADSTOP (ADSTART+1)
	{ "ADSTOP", -6, TOKEN_CMD, doadstop },
#define ADTRIG (ADSTOP+1)
	{ "ADTRIG", -6, TOKEN_CMD, dothreeset },
#define ADWRITE (ADTRIG+1)
	{ "ADWRITE", -7, TOKEN_NUMFUNC, num_adwrite },
#define ADWRLFT (ADWRITE+1)
	{ "ADWRLFT", -7, TOKEN_NUMFUNC, num_adwrlft },
#define AND (ADWRLFT+1)
	{ "AND",   -3, TOKEN_NONE, NULL },
#define ASCII (AND+1)
	{ "ASCII",  -5, TOKEN_NUMFUNC, num_ascii },
#ifdef USEFLOATS
#define ASIN (ASCII+1)
	{ "ASIN",  -4, TOKEN_NUMFUNC, num_asin },
#define ATAN (ASIN+1)
	{ "ATAN",   -4, TOKEN_NUMFUNC, num_atan },
#define BINSTRING (ATAN+1)
#else
#define BINSTRING (ASCII+1)
#endif
    { "BIN$",  4, TOKEN_STRFUNC,  binstring },
#define BOLD (BINSTRING+1)
    { "BOLD",  -4, TOKEN_CMD, dooneset },
#define BSTR (BOLD+1)
	{ "BSTR",  -4, TOKEN_NUMFUNC, binstr },
#define CHRSTRING (BSTR+1)
	{ "CHR$",   4, TOKEN_STRFUNC, chrstring },
#define CLREOL (CHRSTRING+1)
    { "CLREOL", -6, TOKEN_CMD, doclreol },
#define CLRSCR (CLREOL+1)
    { "CLRSCR", -6, TOKEN_CMD, doclrscr },
#define CONFSTRING (CLRSCR+1)
    { "CONF$",   5, TOKEN_STRFUNC, confstring },
#ifdef USEFLOATS
#define COS (CONFSTRING+1)
	{ "COS",  -3, TOKEN_NUMFUNC, num_cos },
#define DIM (COS+1)
#else
#define DIM (CONFSTRING+1)
#endif
	{ "DIM",  -3, TOKEN_CMD,  dodim },
#define DOLINE (DIM+1)
	{ "DOLINE", -6, TOKEN_CMDCTRL, dodoline },
#define ELSE (DOLINE+1)
    { "ELSE", -4, TOKEN_NONE, NULL },
#define END (ELSE+1)
    { "END",  -3, TOKEN_CMDCTRL,  doend },
#define ERCSTRING (END+1)
    { "ERC$",  4, TOKEN_STRFUNC, ercstring },
#define ERR (ERCSTRING+1)
    { "ERR",  -3, TOKEN_NUMFUNC, num_err },
#ifdef USEFLOATS
#define EXP (ERR+1)
    { "EXP",  -3, TOKEN_NUMFUNC, num_exp },
#define EXPAFTER (EXP+1)
#else
#define EXPAFTER (ERR+1)
#endif
#ifdef INCFATFS
#define FCLOSE (EXPAFTER)
	{ "FCLOSE", -6, TOKEN_NUMFUNC,  bfclose },
#define FDEL (FCLOSE+1)
	{ "FDEL",   -4, TOKEN_NUMFUNC,  bfdel },
#define FDIRCL (FDEL+1)
	{ "FDIRCL", -6, TOKEN_NUMFUNC,  bfdircl },
#define FDIROP (FDIRCL+1)
	{ "FDIROP", -6, TOKEN_NUMFUNC,  bfdirop },
#define FDIRRDSTRING (FDIROP+1)
	{ "FDIRRD$", 7, TOKEN_STRFUNC, bfdirrdstring },
#define FEOF (FDIRRDSTRING+1)
	{ "FEOF",  -4, TOKEN_NUMFUNC,  bfeof },
#define FMKDIR (FEOF+1) 
	{ "FMKDIR", -6, TOKEN_NUMFUNC, bfmkdir },
#define FOPEN (FMKDIR+1)
    { "FOPEN",  -5, TOKEN_NUMFUNC, bfopen },
#define FOR (FOPEN+1)
    { "FOR",   -3, TOKEN_CMDCTRL, dofor },
#define FREADSTRING (FOR+1)
	{ "FREAD$", 6, TOKEN_STRFUNC, bfreadstring },
#define FREADLSTRING (FREADSTRING+1)
	{ "FREADL$", 7, TOKEN_STRFUNC, bfreadlstring },
#define FSEEK (FREADLSTRING+1)
	{ "FSEEK",  -5, TOKEN_NUMFUNC,  bfseek },
#define FSTATSTRING (FSEEK+1)
    { "FSTAT$", 6, TOKEN_STRFUNC, bfstatstring },
#define FSYNC (FSTATSTRING+1)
	{ "FSYNC",  -5, TOKEN_NUMFUNC,  bfsync },
#define FTELL (FSYNC+1)
    { "FTELL",  -5, TOKEN_NUMFUNC,  bftell },
#define FWRITE (FTELL+1)
	{ "FWRITE", -6, TOKEN_NUMFUNC,  bfwrite },
#define FORAFTER (FWRITE+1)
#else
#define FOR (EXPAFTER)
	{ "FOR",   -3, TOKEN_CMDCTRL, dofor },
#define FORAFTER (FOR+1)
#endif
#define GOSUB (FORAFTER)
	{ "GOSUB",  -5, TOKEN_CMDCTRL, dogosub },
#define GOTO (GOSUB+1)
	{ "GOTO",  -4, TOKEN_CMDCTRL,  dogoto },
#define HEXSTRING (GOTO+1)
	{ "HEX$",  4, TOKEN_STRFUNC, hexstring },
#define IF (HEXSTRING+1)
	{ "IF",   -2, TOKEN_CMDCTRL, doif },
#define IISTRING (IF+1)
    { "II$",   3, TOKEN_STRFUNC, iistring },
#define IIF (IISTRING+1)
    { "IIF",   -3, TOKEN_NUMFUNC,  num_iif },
#define INADC (IIF+1)
    { "INADC",  -5, TOKEN_NUMFUNC, num_inadc },
#define IND (INADC+1)
    { "IND",   -3, TOKEN_NUMFUNC, num_inadc },
#define INKEY (IND+1)
	{ "INKEY", -5, TOKEN_NUMFUNC, num_inkey },
#define INPLEN (INKEY+1)
    { "INPLEN", -6, TOKEN_NUMFUNC, num_inplen },
#define INPUT (INPLEN+1)
	{ "INPUT",  -5, TOKEN_NONE, NULL },
#define INSTR (INPUT+1)
	{ "INSTR",  -5, TOKEN_NUMFUNC, instr },
#define INT (INSTR+1)
	{ "INT",   -3, TOKEN_NUMFUNC, num_int },
#define LEFTSTRING (INT+1)
	{ "LEFT$",  5, TOKEN_STRFUNC, leftstring },
#define LEN (LEFTSTRING+1)
	{ "LEN",  -3, TOKEN_NUMFUNC, num_len },
#define LET (LEN+1)
	{ "LET",  -3, TOKEN_CMD, dolet },
#define LOCATE (LET+1)
    { "LOCATE", -6, TOKEN_CMD, dotwoset },
#ifdef USEFLOATS
#define LOG (LOCATE+1)
	{ "LOG",   -3, TOKEN_NUMFUNC, num_log },
#define LSL (LOG+1)
#else
#define LSL (LOCATE+1)
#endif
    { "LSL",  -3, TOKEN_NONE,  NULL },
#define LSR (LSL+1)
    { "LSR",  -3, TOKEN_NONE, NULL },
#define MIDSTRING (LSR+1)
	{ "MID$", 4, TOKEN_STRFUNC, midstring },
#define MOD (MIDSTRING+1)
	{ "MOD",  -3, TOKEN_NONE,  NULL },
#define NEXT (MOD+1)
	{ "NEXT", -4, TOKEN_CMDCTRL, donext },
#define NOT (NEXT+1)
    { "NOT",  -3, TOKEN_NONE,  NULL },
#define ONERROR (NOT+1)
    { "ONERROR", -7, TOKEN_CMDCTRL,  doonerror },
#define OR (ONERROR+1)
	{ "OR",  -2, TOKEN_NONE,   NULL },
#define OUTD (OR+1)
    { "OUTD",   -4, TOKEN_CMD, dotwoset },
#define OUTDAC (OUTD+1)
    { "OUTDAC",  -6, TOKEN_CMD, dotwoset },
#define PEEK (OUTDAC+1)
    { "PEEK",  -4, TOKEN_NUMFUNC, num_peek },
#ifdef USEFLOATS
#define PI (PEEK+1)
	{ "PI",  -2, TOKEN_NUMFUNC, num_pi },
#define POKE (PI+1)
#else
#define POKE (PEEK+1)
#endif
    { "POKE",   -4, TOKEN_CMD, dopoke },
#ifdef USEFLOATS
#define POW (POKE+1)
	{ "POW",    -3, TOKEN_NUMFUNC, num_pow },
#define PRINT (POW+1)
#else
#define PRINT (POKE+1)
#endif
	{ "PRINT",  -5, TOKEN_CMD, doprint },
#define PWM (PRINT+1)
    { "PWM",    -3, TOKEN_CMD, dothreeset },
#define REM (PWM+1)
	{ "REM",    -3, TOKEN_NONE, NULL },
#define REPEAT (REM+1)
	{ "REPEAT", -6, TOKEN_CMDCTRL, dorepeat },
#define RETURN (REPEAT+1)
	{ "RETURN", -6, TOKEN_CMDCTRL, doreturn },
#define RIGHTSTRING (RETURN+1)
	{ "RIGHT$", 6, TOKEN_STRFUNC, rightstring },
#define RND (RIGHTSTRING+1)
	{ "RND",    -3, TOKEN_NUMFUNC, num_rnd },
#ifdef WEBENABLED 
#define SCACEPT (RND+1)
    { "SCACEPT", -7, TOKEN_NUMFUNC, scacept },
#define SCCLOSE (SCACEPT+1)
    { "SCCLOSE", -7, TOKEN_NUMFUNC, scclose },
#define SCCON (SCCLOSE+1)
    { "SCCON",   -5, TOKEN_NUMFUNC, sccon },
#define SCISCON (SCCON+1)
    { "SCISCON", -7, TOKEN_NUMFUNC, sciscon },
#define SCNEWDT (SCISCON+1)
    { "SCNEWDT", -7, TOKEN_NUMFUNC, scnewdt },
#define SCREADSTRING (SCNEWDT+1)
    { "SCREAD$", 7, TOKEN_STRFUNC, screadstring },
#define SCWRITE (SCREADSTRING+1)
    { "SCWRITE", -7, TOKEN_NUMFUNC, scwrite },
#define SERBNG (SCWRITE+1)
#else
#define SERBNG (RND+1)
#endif
    { "SERBNG", -6, TOKEN_NUMFUNC, num_serbng },
#define SERI2CSTRING (SERBNG+1)
    { "SERI2C$", 7, TOKEN_STRFUNC, seri2cstring },
#define SERINIT (SERI2CSTRING+1)
    { "SERINIT", -7, TOKEN_CMD, doserinit },
#define SERINPSTRING (SERINIT+1)
    { "SERINP$", 7, TOKEN_STRFUNC, serinpstring },
#define SEROUT (SERINPSTRING+1)
    { "SEROUT", -6, TOKEN_NUMFUNC, num_serout },
#define SERSPI (SEROUT+1)
    { "SERSPI", -6, TOKEN_NUMFUNC, num_serspi },
#define SETIDLE (SERSPI+1)
    { "SETIDLE", -7, TOKEN_NUMFUNC, num_setidle },
#define SETPIN (SETIDLE+1)
    { "SETPIN", -6, TOKEN_CMD, dotwoset },
#ifdef USEFLOATS
#define SIN (SETPIN+1)
	{ "SIN",    -3, TOKEN_NUMFUNC, num_sin },
#define SQRT (SIN+1)
	{ "SQRT",  -4, TOKEN_NUMFUNC, num_sqrt },
#define STEP (SQRT+1)
#else
#define STEP (SETPIN+1)
#endif
	{ "STEP",  -4, TOKEN_NONE,  NULL },
#define STRSTRING (STEP+1)
	{ "STR$",  4, TOKEN_STRFUNC, strstring },
#define STRINGSTRING (STRSTRING+1)
	{ "STRING$", 7, TOKEN_STRFUNC, stringstring },
#ifdef USEFLOATS
#define TAN (STRINGSTRING+1)
	{ "TAN",   -3, TOKEN_NUMFUNC, num_tan },
#define THEN (TAN+1)
#else
#define THEN (STRINGSTRING+1)
#endif
	{ "THEN",    -4, TOKEN_NONE, NULL },
#define TIMEGETSTRING (THEN+1)
    { "TIMEGT$",  7, TOKEN_STRFUNC, timegetstring },
#define TIMESET (TIMEGETSTRING+1)
    { "TIMESET", -7, TOKEN_NUMFUNC, num_timeset },
#define TO (TIMESET+1)
	{ "TO",     -2, TOKEN_NONE, NULL },
#define TONE (TO+1)
    { "TONE",  -4, TOKEN_CMD, dothreeset },
#define TRIMSTRING (TONE+1)
    { "TRIM$",  5, TOKEN_STRFUNC, trimstring },
#define UNTIL (TRIMSTRING+1)
    { "UNTIL",  -5, TOKEN_CMDCTRL, dountil },
#define UPPERSTRING (UNTIL+1)
    { "UPPER$",  6, TOKEN_STRFUNC, upperstring },
#define VAL (UPPERSTRING+1)
	{ "VAL",    -3, TOKEN_NUMFUNC, num_val },
#define VALLEN (VAL+1)
	{ "VALLEN", -6, TOKEN_NUMFUNC, num_vallen  },
#define WAIT (VALLEN+1)
    { "WAIT",   -4, TOKEN_CMD, dooneset },
#ifdef WEBENABLED
#define WEBCON (WAIT+1)
    { "WEBCON", -6, TOKEN_NUMFUNC, webcon },
#define WEBESCSTRING (WEBCON+1)
    { "WEBESC$", 7, TOKEN_STRFUNC, webescstring },
#define WEBFRMSTRING (WEBESCSTRING+1)
    { "WEBFRM$", 7, TOKEN_STRFUNC, webfrmstring },
#define WEBMAIL (WEBFRMSTRING+1)
    { "WEBMAIL", -7, TOKEN_NUMFUNC, webmail },
#define WEBOUT (WEBMAIL+1)
    { "WEBOUT", -6, TOKEN_NUMFUNC, webout },
#define WEBPSTSTRING (WEBOUT+1)
    { "WEBPST$", 7, TOKEN_STRFUNC, webpststring },
#define WEBREQSTRING (WEBPSTSTRING+1)
    { "WEBREQ$", 7, TOKEN_STRFUNC, webreqstring }
#endif
};

#define TOKENLISTENTRIES (sizeof(tl)/sizeof(tokenlist))
#define ENDPROGRAMOFFSET ((scriptoffset)((~((scriptoffset)0))))
#define NEXTLINEOFFSET ((scriptoffset)((~((scriptoffset)1))))
#define LINENOTFOUNDOFFSET ((scriptoffset)((~((scriptoffset)2))))

#ifndef MULTIPLE
BSTATIC BASICSTATE bs;
#endif

BSTATIC STRINGARRAY emptystring = { 0, { 0 } };

#define EMPTYSTRING &emptystring

#ifdef USEFLOATS
BSTATIC int integer(numtype x);
#else
#define integer(x) (x)
#endif

#ifdef USEFLOATS
int errno;
int *__errno(void)
{
    return &errno;
}
#endif

#ifdef BASICASSERT
BSTATIC void basicassert(int x, char *reason)
{
	if (x==0) {
		outstr("oops! assert thrown ");
		outstr(reason);
		outstr("\n");
	}
}
#endif

BSTATIC void outnum(numtype x)
{
#ifdef USEFLOATS
    char s[DTOASIZE];
    outstr(mydtoa(s,x));
#else
    char s[ITOASIZE];
	outstr(myltoa(s,x));
#endif
}

BSTATIC int input_handler(VOIDACCEPTSTATE)
{
	int c = inkey();
	if ((c == '\r') || (c == '\n')) {
		Scurinputptr[Scurinputpos] = 0;
		outchar('\n');
		return 1;
	}
	if ((c == '\010') || (c == 127)) {
		if (Scurinputpos > 0) {
			outstr("\010 \010");
			Scurinputpos--;
		}
	} else if ((Scurinputpos < Scurinputmax) && (isprint(c)))
	{
		Scurinputptr[Scurinputpos++] = c;
		outchar(c);
	}
	return 0;
}

/*
  Interpret a BASIC script

  Params: script - the script to run
          in - input stream
		  out - output stream
		  err - error stream 
  Returns: 0 on success, 1 on error condition.
*/
int basic(const char *script)
{
#ifdef MULTIPLE
  BASICSTATE bs;
  BASICSTATE *ps = &bs;
#endif

  if( setup(PASSSTATE script) == -1 )
    return 1;
  while(!mainloop(VOIDPASSSTATE));
  cleanup(VOIDPASSSTATE);
  return (Serrorflag != 0);
}

/*
  Main loop.  Allows one to single step through statements
  Returns: 0 for success, 1 on end program
*/
int mainloop(VOIDACCEPTSTATE)
{
	scriptoffset nextoffset;

    Sstring = Snexttoken = &Sscriptstart[Scuroffset];
	Serrorflag = 0;

	nextoffset = line(PASSSTATE 0 );
	if(Serrorflag)
	{
	  Serroroffset = Scuroffset;
	  Slasterrorflag = Serrorflag;
	  if ((Sonerroroffset == ENDPROGRAMOFFSET) || (Serrorflag == ERR_OUTOFMEMORY)) {
		reporterror(PASSSTATE findlinenumber(PASSSTATE Scuroffset));
		return 1;
	  } else {
		nextoffset = Sonerroroffset;
		Sonerroroffset = ENDPROGRAMOFFSET;
	  }
	}

	if(nextoffset == ENDPROGRAMOFFSET)
	  return 1;

	if(nextoffset == NEXTLINEOFFSET)
	{
      Scuroffset = getnextline(VOIDPASSSTATE);
	  if (Sscriptstart[Scuroffset] == 0)
		return 1;
    }
	else
    {
	  if(nextoffset == LINENOTFOUNDOFFSET)
	  {
        outstrint("line not found from: ", findlinenumber(PASSSTATE Scuroffset));
		return 1;
	  }
      Scuroffset = nextoffset;
    }
	return (Scuroffset == ENDPROGRAMOFFSET);
}

/* add a new label */

BSTATIC LABELOFFSET *addnewlabel(VOIDACCEPTSTATE)
{
  LABELOFFSET *lo;
  int nl = Snlabels + 1;

  if (nextlargest(Snlabels) != nextlargest(nl)) {
	lo = realloc(Slabels, nextlargest(nl) * sizeof(LABELOFFSET));
	if (!lo) {
		return NULL;
	}
	Slabels = lo;
  }
  Snlabels = nl;
  return &Slabels[nl-1];
}

/*
  Sets up all our globals, including the list of lines.
  Params: script - the script passed by the user
  Returns: 0 on success, -1 on failure
*/
int setup(ACCEPTSTATE const char *script)
{
  int i, j;
  LABELOFFSET *lo;
  char id[IDLENGTH];
  
  Sscriptstart = script;
  Slabels = NULL;
  Snlabels = 0;
  
  while (*script)
  {
	if (*script == '\n') script++;
    script = skipspaceslf(script);
	if (isalpha(*script)) {
		getid(PASSSTATE script, id, &j);
		if ((j < IDLENGTH) && (id[j-1] == ':')) {	
			if ((lo = addnewlabel(VOIDPASSSTATE)) == NULL) {
				outstr("Out of memory\n");
				if (Slabels) free(Slabels);
				return -1;
			}
			strcpy(lo->id, id);
			lo->ofs = (scriptoffset)(script - Sscriptstart);
	    }
	}
	while ((*script != 0) && (*script != '\n')) script++;
  }
  for (i=0;i<Snlabels;i++) {
	for (j=i+1;j<Snlabels;j++) {
		if (strcasecmp(Slabels[i].id,Slabels[j].id) > 0) {
			LABELOFFSET l;
			memcpy(&l, &Slabels[i], sizeof(LABELOFFSET));
			memcpy(&Slabels[i], &Slabels[j], sizeof(LABELOFFSET));
			memcpy(&Slabels[j], &l, sizeof(LABELOFFSET));
/*			LABELOFFSET l = Slabels[i];
			Slabels[i] = Slabels[j];
			Slabels[j] = l; */
		}
	}
  }
  Snvariables = 0;
  Svariables = 0;
  Sidlestate = 0;

  Sdimvariables = 0;
  Sndimvariables = 0;

  Scurinputmax = 80;
  Scurinputptr = NULL;
  Snfors = Snrepeats = Sngosub = 0;
  Slasterrorflag = 0;
  Sonerroroffset = Serroroffset = ENDPROGRAMOFFSET;
  Scuroffset = 0;

#ifdef INCFATFS
  for (i=0;i<FATPTRNUM;i++) {
	Sfatptrs[i].fptr.fil = NULL;
	Sfatptrs[i].ftype = FATFSNONEPTR;
  }
#endif
  return 0;
}

/* add a new label */

int idlestate(VOIDACCEPTSTATE)
{
  return Sidlestate;
}

BSTATIC int computedimsize(DIMVAR *dv)
{
	int ii, size = 1;
    for(ii=0;ii<dv->ndims;ii++)
	  size *= dv->dim[ii];
	return size;
}

/*
  frees all the memory we have allocated
*/
void cleanup(VOIDACCEPTSTATE)
{
  int i;
  int ii;
  int size;

#ifdef INCFATFS
  for (i=0;i<FATPTRNUM;i++) {
	if (Sfatptrs[i].fptr.fil != NULL) {
		if (Sfatptrs[i].ftype == FATFSFILPTR) {
			f_close(Sfatptrs[i].fptr.fil);
		}
		free(Sfatptrs[i].fptr.fil);
		Sfatptrs[i].fptr.fil = NULL;
		Sfatptrs[i].ftype = FATFSNONEPTR;
	}
  }
#endif
#ifdef WEBENABLED
  socket_free();
#endif

  for(i=0;i<Snvariables;i++) {
      VARIABLE *v = &Svariables[i];
	  if(v->type == STRID) {
		  void *s = v->d.sval;
		  if (s) free(s);
	  }
  }
  if(Svariables)
	  free(Svariables);

  for(i=0;i<Sndimvariables;i++)
  {
    DIMVAR *d = &Sdimvariables[i];
    if(d->type == STRID)
	{
	  void *v = d->d.str;
	  if(v)
	  {
		size = computedimsize(d);
		for(ii=0;ii<size;ii++) {
		  void *v2 = d->d.str[ii];
		  if (v2) free(v2);
		}
		free(v);
	  }
	}
	else {
	  void *v = d->d.dval;
	  if (v) free(v);
	}
  }

  if(Scurinputptr)
	free(Scurinputptr);
  
  if(Sdimvariables)
	free(Sdimvariables);
 
  if(Slabels)
	free(Slabels);
 
}

/*
  set the errorflag.
  Params: errorcode - the error.
  Notes: ignores error cascades
*/
#if STATICSETERROR
BSTATIC void seterrorsyntaxerror(VOIDACCEPTSTATE)
{
	seterror(PASSSTATE ERR_SYNTAX);
}

BSTATIC void seterroroutofmemory(VOIDACCEPTSTATE)
{
	seterror(PASSSTATE ERR_OUTOFMEMORY);
}
#else
#define seterrorsyntaxerror(x) seterror(PASSSTATE ERR_SYNTAX)
#define seterroroutofmemory(x) seterror(PASSSTATE ERR_OUTOFMEMORY)
#endif

BSTATIC void seterror(ACCEPTSTATE int errorcode)
{
  if(Serrorflag == 0 || errorcode == 0)
	Serrorflag = errorcode;
}


BSTATIC int findlinenumber(ACCEPTSTATE scriptoffset ofs)
{
	int lineno = 1;
	const char *string = Sscriptstart;
	const char *of = Sscriptstart+ofs;
	for (;;) {
		if (of<=string) return lineno;
		string = strchr(string,'\n');
		if (string == NULL) break;
		string++;
		lineno++;
	}
	return lineno;
}

/*
  error report function.
  for reporting errors in the user's script.
  checks the global errorflag.
  writes to fperr.
  Params: lineno - the line on which the error occurred
*/
BSTATIC void reporterror(ACCEPTSTATE int lineno)
{
  outstr(((Serrorflag >= 0) && (Serrorflag < ERRCODES)) ? errormessages[Serrorflag] :
		"Unknown error");
  ASSERTDEBUG(basicassert(Serrorflag != 0,"ERR_CLEAR"));
  outstrint(" line # ",lineno);
}

/*
  binary search for a label
  Params: id - label to find
  Returns: offset of the label, or -1 on fail.
*/
BSTATIC scriptoffset findline(ACCEPTSTATE char *id)
{
  int high;
  int low;
  int mid;
  int cmp;

  low = 0;
  high = Snlabels-1;
  if (Slabels == NULL)
		return LINENOTFOUNDOFFSET;
  while(low <= high)
  {
    mid = (high + low)/2;
	cmp = strcasecmp(Slabels[mid].id, id);
	if (cmp > 0)
	  high = mid - 1;
	else if (cmp < 0)
	  low = mid + 1;
	else return Slabels[mid].ofs;
  }
  return LINENOTFOUNDOFFSET;
}

/*
  Get the offset of a label from the current line
*/
BSTATIC scriptoffset getlabel(VOIDACCEPTSTATE)
{
  int len;
  scriptoffset ofs;
  char id[IDLENGTH];
  
  if (Stoken != LABEL) {
	seterrorsyntaxerror(VOIDPASSSTATE);
	return LINENOTFOUNDOFFSET;
  }
  getid(PASSSTATE Sstring, id, &len);
  match(PASSSTATE LABEL); 
  ofs = findline(PASSSTATE id);
  if (ofs == LINENOTFOUNDOFFSET)
	seterror(PASSSTATE ERR_NOLABEL);
  return ofs;
}

/*
  Parse a line. High level parse function
*/
BSTATIC scriptoffset line(ACCEPTSTATE int noctrl)
{
  scriptoffset answer = NEXTLINEOFFSET;

  Stoken = gettoken(VOIDPASSSTATE );
  istokenmatch(PASSSTATE LABEL);

  switch(Stoken)
  {
	case SHORTPRINT:
	  match(PASSSTATE SHORTPRINT);
	  answer = doprint(VOIDPASSSTATE);
	  break;
	case INPUT:
	  if (!noctrl)
	  {
			if (Scurinputptr == NULL) {
				Scurinputptr = malloc(Scurinputmax+1);
				if (!Scurinputptr) {
					seterroroutofmemory(VOIDPASSSTATE);
					return answer;
				}
				Scurinputpos = 0;
			}
			if (input_handler(VOIDPASSSTATE)) {
				answer=doinput(VOIDPASSSTATE);
				free(Scurinputptr);
				Scurinputptr = NULL;
				break;
			}
			return Scuroffset;
	  }
	  seterrorsyntaxerror(VOIDPASSSTATE);
	  return answer;
	case REM:
	  match(PASSSTATE REM);
	  return answer;
	  break;
	case MIDSTRING:
	  answer = domidstring(VOIDPASSSTATE);
	  break;
	default:
       if ((Stoken >= (TOKENBASE+1)) && (Stoken <= (TOKENBASE + TOKENLISTENTRIES))) {
			const tokenlist *t = &tl[Stoken-(TOKENBASE+1)];
			if ((t->tokentp == TOKEN_CMD) || ((t->tokentp == TOKEN_CMDCTRL) && (!noctrl)))
				answer = ((docommand)(t->func))(VOIDPASSSTATE);
			else seterrorsyntaxerror(VOIDPASSSTATE);
	   } else {
			if ((Stoken == STRID) || (Stoken == FLTID) || (Stoken == DIMSTRID) || (Stoken == DIMFLTID))
				dolet(VOIDPASSSTATE);
		}
	  break;
  }

  if(Stoken != EOS)
  {
	const char *str = skipspaceslf(Sstring);
	if(*str != '\n') {
	  seterrorsyntaxerror(VOIDPASSSTATE);
	}
  }
  return answer;
}

BSTATIC scriptoffset doclrscr(VOIDACCEPTSTATE)
{ 
  match(PASSSTATE CLRSCR); 
  clrscr();
  return NEXTLINEOFFSET;
}

BSTATIC scriptoffset doclreol(VOIDACCEPTSTATE)
{ 
  match(PASSSTATE CLREOL); 
  clreol();
  return NEXTLINEOFFSET;
}

BSTATIC scriptoffset doend(VOIDACCEPTSTATE)
{ 
  match(PASSSTATE END); 
  return ENDPROGRAMOFFSET;
}

BSTATIC scriptoffset doadstop(VOIDACCEPTSTATE)
{ 
  match(PASSSTATE ADSTOP); 
  adc_shutdown();
  return NEXTLINEOFFSET;
}

BSTATIC scriptoffset doadstart(VOIDACCEPTSTATE)
{
  int inadcchans, outdacchans, clkdiv, indiv, outdiv;
  
  match(PASSSTATE ADSTART);
  inadcchans = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  outdacchans = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  clkdiv = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  indiv = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  outdiv = integer(expr(VOIDPASSSTATE ));
  adc_init(inadcchans, outdacchans, clkdiv, indiv, outdiv);
  return NEXTLINEOFFSET;
}

BSTATIC scriptoffset doserinit(VOIDACCEPTSTATE)
{
  int port, baud, databits, stopbits, parity;
  
  match(PASSSTATE SERINIT);
  port = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  baud = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  databits = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  stopbits = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  parity = integer(expr(VOIDPASSSTATE ));
  serial_init(port, baud, databits, stopbits, parity);
  return NEXTLINEOFFSET;
}

/*
  the ONERROR statement
*/
BSTATIC scriptoffset doonerror(VOIDACCEPTSTATE)
{
  scriptoffset of;
  match(PASSSTATE ONERROR);
  if (Stoken == LABEL) {
	of = getlabel( VOIDPASSSTATE ) ;
	if (!Serrorflag) Sonerroroffset = of;
  } else Sonerroroffset = ENDPROGRAMOFFSET;
  return NEXTLINEOFFSET;
}

/*
  ONE parameter statement for BOLD/WAIT
*/
BSTATIC scriptoffset dooneset(VOIDACCEPTSTATE)
{
  tokennum t = Stoken;
  int prm;
  
  match(PASSSTATE t);
  prm = integer(expr(VOIDPASSSTATE ));
  
  if (!Serrorflag) {
	switch(t)
	{
		case BOLD: 
			if (prm) highvideo();
				else lowvideo();
		   break;
		case WAIT:
			tone(1000, prm, 100);
			break;
	}
  }
  return NEXTLINEOFFSET;
}

/*
  two parameter statements
*/
BSTATIC scriptoffset dotwoset(VOIDACCEPTSTATE)
{
  tokennum t = Stoken;
  int adr, dat;
  
  match(PASSSTATE t);
  adr = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  dat = integer(expr(VOIDPASSSTATE ));
  if (!Serrorflag) {
	switch (t)
	{
	    case LOCATE: gotoxy(dat,adr); break;
		case SETPIN: setpin(adr,dat); break;
		case OUTD:   outd(adr,dat); break;
		case OUTDAC: outdac(adr,dat); break;
	}
  }
  return NEXTLINEOFFSET;
}

BSTATIC scriptoffset dothreeset(VOIDACCEPTSTATE)
{
  tokennum t = Stoken;
  int adr, dat1, dat2;
  
  match(PASSSTATE t);
  adr = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  dat1 = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  dat2 = integer(expr(VOIDPASSSTATE ));
  if (!Serrorflag) {
	switch (t)
	{
		case PWM:    pwm(adr,dat1,dat2); break;
		case TONE:   tone(adr,dat1,dat2); break;
		case ADTRIG: adc_trig(adr,dat1,dat2); break;
	}
  }
  return NEXTLINEOFFSET;
}

/*
  the POKE statement
*/
BSTATIC scriptoffset dopoke(VOIDACCEPTSTATE)
{
  unsigned int adr, dat;
  
  match(PASSSTATE POKE);
  adr = integer(expr(VOIDPASSSTATE ));
  match(PASSSTATE COMMA);
  dat = integer(expr(VOIDPASSSTATE ));
#ifdef PEEKPOKE
  if (!Serrorflag) *((unsigned int *)adr) = dat;
#endif
  return NEXTLINEOFFSET;
}

/*
  the PRINT statement
*/
BSTATIC scriptoffset doprint(VOIDACCEPTSTATE)
{
  istokenmatch(PASSSTATE PRINT);
  for (;;)
  {
    if(isstring(Stoken))
	{
	  STRINGARRAY *str;
      str = stringexpr(VOIDPASSSTATE);
	  if(str)
	  {
	    int i;
		for (i=0;i<str->len;i++)
			outchar(STRINGOF(str)[i]);
        free(str);
	  }
	}
	else
	{
	  numtype x = expr(VOIDPASSSTATE );
	  outnum(x);
	}
	if (!istokenmatch(PASSSTATE COMMA))
		break;
  }
  if(!istokenmatch(PASSSTATE SEMICOLON)) 
	outchar('\n');
  return NEXTLINEOFFSET;
}

/*
  the MIDSTRING statement
*/
BSTATIC scriptoffset domidstring(VOIDACCEPTSTATE)
{
  LVALUE lv;
  STRINGARRAY *temp, *new;
  int ind, len;

  matchoparen(PASSSTATE MIDSTRING);
  lvalue(PASSSTATE &lv);
  match(PASSSTATE COMMA);
  ind = integer( expr(VOIDPASSSTATE) );
  if (istokenmatch(PASSSTATE COMMA)) {
	len = integer( expr(VOIDPASSSTATE) );
  } else len = -1;
  match(PASSSTATE CPAREN);
  match(PASSSTATE EQUALS);
  temp = stringexpr(VOIDPASSSTATE);
  if (temp == NULL) {
	   seterroroutofmemory(VOIDPASSSTATE);
	   return NEXTLINEOFFSET;
  }
  if (lv.type == STRID)  {
	if (*lv.d.sval == NULL) {
		*lv.d.sval = createstringarray(0);
		if (*lv.d.sval == NULL) 
			seterroroutofmemory(VOIDPASSSTATE);
	}
	if (*lv.d.sval != NULL) {
		new = stringarraysub(*lv.d.sval, temp, ind-1, len);
		if (new == NULL)
			seterroroutofmemory(VOIDPASSSTATE);
		else
			*lv.d.sval = new;
	}
  } else
	 seterror(PASSSTATE ERR_BADTYPE);
  free(temp);
  return NEXTLINEOFFSET;
}

/*
  the LET statement
*/
BSTATIC scriptoffset dolet(VOIDACCEPTSTATE)
{
  LVALUE lv;
  STRINGARRAY *temp, *newstr;

  istokenmatch(PASSSTATE LET);
  lvalue(PASSSTATE &lv);
  match(PASSSTATE EQUALS);
  switch(lv.type)
  {
    case FLTID:
	  *lv.d.dval = expr(VOIDPASSSTATE );
	  break;
    case STRID:
	  temp = *lv.d.sval;
	  newstr = stringexpr(VOIDPASSSTATE);
	  *lv.d.sval = newstr ? newstr : createstringarray(0);
	  if(temp)
		free(temp);
	  break;
	default:
	  break;
  }
  return NEXTLINEOFFSET;
}

/*
  the DIM statement
*/
BSTATIC scriptoffset dodim(VOIDACCEPTSTATE)
{
  int ndims = 0;
  numtype dims[MAXDIMS+1];
  dimsize intdims[MAXDIMS+1];  
  char name[IDLENGTH];
  int len;
  DIMVAR *dimvar;
  void *v;
  int i;
  int size;

  match(PASSSTATE DIM);

  switch(Stoken)
  {
    case DIMFLTID:
	case DIMSTRID:
      getid(PASSSTATE Sstring, name, &len);
	  match(PASSSTATE Stoken);
	  dims[ndims++] = expr(VOIDPASSSTATE );
	  while(istokenmatch(PASSSTATE COMMA))
	  {
	    dims[ndims++] = expr(VOIDPASSSTATE );
		if(ndims > MAXDIMS)
		{
		  seterror(PASSSTATE ERR_TOOMANYDIMS);
		  return NEXTLINEOFFSET;
		}
	  } 
	  match(PASSSTATE CPAREN);
	  for(i=0;i<ndims;i++)
	  {
	    if(dims[i] < 0 || dims[i] > MAXDIMSIZE || dims[i] != (int) dims[i])
		{
		  seterror(PASSSTATE ERR_BADSUBSCRIPT);
		  return NEXTLINEOFFSET;
		}
		intdims[i] = ((dimsize)dims[i]);
	  }
	  dimvar=dimension(PASSSTATE name, ndims, intdims);
	  break;
	default:
	    seterrorsyntaxerror(VOIDPASSSTATE); 
	    return NEXTLINEOFFSET;
  }
  if(dimvar == 0)
  {
	/* out of memory */
	seterroroutofmemory(VOIDPASSSTATE );
	return NEXTLINEOFFSET;
  }

  if(istokenmatch(PASSSTATE EQUALS))
  {
	size = computedimsize(dimvar);
	switch(dimvar->type)
	{
      case FLTID:
		i = 0;
	    dimvar->d.dval[i++] = expr(VOIDPASSSTATE );
		while(Stoken == COMMA && i < size)
		{
		  match(PASSSTATE COMMA);
		  dimvar->d.dval[i++] = expr(VOIDPASSSTATE );
		  if(Serrorflag)
			break;
		}
		break;
	  case STRID:
		i = 0;
		v = dimvar->d.str[i];
		if (v) free (v);
		dimvar->d.str[i++] = stringexpr(VOIDPASSSTATE);

		while(Stoken == COMMA && i < size)
		{
		  match(PASSSTATE COMMA);
		  v = dimvar->d.str[i];
		  if (v) free(v);
		  dimvar->d.str[i++] = stringexpr(VOIDPASSSTATE);
		  if(Serrorflag)
			break;
		}
		break;
	}
	
	if(Stoken == COMMA)
	  seterror(PASSSTATE ERR_TOOMANYINITS);
  }
  return NEXTLINEOFFSET;
}

/*
  the IF statement.
  if jump taken, returns new line no, else returns 0
*/
BSTATIC scriptoffset doif(VOIDACCEPTSTATE)
{
  int condition;
  scriptoffset jumpthen, jumpelse = NEXTLINEOFFSET;

  match(PASSSTATE IF);
  condition = integer ( expr(VOIDPASSSTATE) );
  match(PASSSTATE THEN);
  jumpthen = getlabel( VOIDPASSSTATE );
  if (istokenmatch(PASSSTATE ELSE))
	 jumpelse = getlabel( VOIDPASSSTATE );
  return condition != 0 ? jumpthen : jumpelse;
}

BSTATIC scriptoffset dorepeat(VOIDACCEPTSTATE)
{
  match(PASSSTATE REPEAT);
  if (Snrepeats >= MAXREPEATSTACK) {
		seterror(PASSSTATE ERR_TOOMANYREPEAT);
		return ENDPROGRAMOFFSET;
  }
  Srepeatstack[Snrepeats++] = getnextline(VOIDPASSSTATE);
  return NEXTLINEOFFSET;
}

BSTATIC scriptoffset dountil(VOIDACCEPTSTATE)
{
  int boolex;
  
  match(PASSSTATE UNTIL);
  boolex = integer( expr(VOIDPASSSTATE ) );
  if (Snrepeats <= 0) {
	seterror(PASSSTATE ERR_NOREPEAT);
	return ENDPROGRAMOFFSET;
  }
  if (boolex) {
	Snrepeats--;
	return NEXTLINEOFFSET;
  }
  return Srepeatstack[Snrepeats-1];
}

/*
  the ABORT statement
  eliminates all GOSUB/FOR/REPEAT stacks 
*/
BSTATIC scriptoffset doabort(VOIDACCEPTSTATE)
{
  match(PASSSTATE ABORT);
  Snfors = Sngosub = Snrepeats = 0;
  return getlabel( VOIDPASSSTATE );
}

/*
  the GOTO statement
  returns new script offset
*/
BSTATIC scriptoffset dogoto(VOIDACCEPTSTATE)
{
  match(PASSSTATE GOTO);
  return getlabel( VOIDPASSSTATE ) ;
}

BSTATIC scriptoffset dogosub(VOIDACCEPTSTATE)
{
  scriptoffset tooffset;

  match(PASSSTATE GOSUB);
  tooffset = getlabel( VOIDPASSSTATE ) ;
  if (Sngosub >= MAXGOSUB) {
	  seterror(PASSSTATE ERR_TOOMANYGOSUB);
	  return ENDPROGRAMOFFSET;
  }
  Sgosubstack[Sngosub++] = getnextline(VOIDPASSSTATE);
  return tooffset;
}

BSTATIC scriptoffset doreturn(VOIDACCEPTSTATE)
{
  match(PASSSTATE RETURN);
  if (Sngosub <= 0) {
	  seterror(PASSSTATE ERR_NORETURN);
	  return ENDPROGRAMOFFSET;
  }
  return Sgosubstack[--Sngosub];
}

/*
  The FOR statement.

  Pushes the for stack.
  Returns line to jump to, or -1 to end program

*/
BSTATIC scriptoffset dofor(VOIDACCEPTSTATE)
{
  LVALUE lv;
  char id[IDLENGTH];
  char nextid[IDLENGTH];
  int len;
  numtype initval;
  numtype toval;
  numtype stepval;
  scriptoffset nextoffset;

  match(PASSSTATE FOR);
  getid(PASSSTATE Sstring, id, &len);

  lvalue(PASSSTATE &lv);
  if(lv.type != FLTID)
  {
    seterror(PASSSTATE ERR_BADTYPE);
	return ENDPROGRAMOFFSET;
  }
  match(PASSSTATE EQUALS);
  initval = expr(VOIDPASSSTATE );
  match(PASSSTATE TO);
  toval = expr(VOIDPASSSTATE );
  if (istokenmatch(PASSSTATE STEP))
	stepval = expr(VOIDPASSSTATE );
  else
    stepval = ((numtype)1);

  *lv.d.dval = initval;

  if(Snfors > MAXFORS - 1)
  {
	seterror(PASSSTATE ERR_TOOMANYFORS);
	return ENDPROGRAMOFFSET;
  }
  if(((stepval < 0) && (initval < toval)) || ((stepval > 0) && (initval > toval)))
  {
	const char *savestring = Sstring;
	for (;;)
	{
	  Scuroffset = getnextline(VOIDPASSSTATE);
	  Sstring = &Sscriptstart[Scuroffset];
	  if (*Sstring == 0) break;
      Serrorflag = 0;
	  Stoken = gettoken(VOIDPASSSTATE );
	  istokenmatch(PASSSTATE LABEL);
	  if (istokenmatch(PASSSTATE NEXT))
	  {
		if(Stoken == FLTID || Stoken == DIMFLTID)
		{
          getid(PASSSTATE Sstring, nextid, &len);
		  if(!strcasecmp(id, nextid))
		  {
			nextoffset = getnextline(VOIDPASSSTATE);
			Sstring = savestring;
			Stoken = gettoken(VOIDPASSSTATE );
			return nextoffset;
		  }
		}
	  } 
	} 
	seterror(PASSSTATE ERR_NONEXT);
	return ENDPROGRAMOFFSET;
  }
  else
  {
	FORLOOP *f = &Sforstack[Snfors];
	strcpy(f->id, id);
	f->nextoffset = getnextline(VOIDPASSSTATE);
	f->step = stepval;
	f->toval = toval;
	Snfors++;
    return NEXTLINEOFFSET;
  }
}

/*
  the NEXT statement
  updates the counting index, and returns line to jump to
*/
BSTATIC scriptoffset donext(VOIDACCEPTSTATE)
{
  char id[IDLENGTH];
  int len;
  LVALUE lv;
  FORLOOP *f;

  match(PASSSTATE NEXT);

  if(Snfors)
  {
    getid(PASSSTATE Sstring, id, &len);
    lvalue(PASSSTATE &lv);
	if(lv.type != FLTID)
	{
      seterror(PASSSTATE ERR_BADTYPE);
	  return ENDPROGRAMOFFSET;
	}
	f = &Sforstack[Snfors-1];
    if(strcasecmp(id, f->id)) {
		seterror(PASSSTATE ERR_NOFOR);
		return ENDPROGRAMOFFSET;
	} 
    *lv.d.dval += f->step;
	if( (f->step < 0 && *lv.d.dval < f->toval) ||
		(f->step > 0 && *lv.d.dval > f->toval) )
	{
	  Snfors--;
	  return NEXTLINEOFFSET;
	}
	else
	{
      return f->nextoffset;
	}
  }
  else
  {
    seterror(PASSSTATE ERR_NOFOR);
	return ENDPROGRAMOFFSET;
  }
}

typedef struct {
	STRINGARRAY *tokstr;
	int curlen;
	int iserr;
} dolinetokstat;

#define DOLINEINCSIZE 64

BSTATIC void adddolinetoken(int chv, void *v)
{
	dolinetokstat *dolinestat = (dolinetokstat *)v;
	if (dolinestat->iserr)
		return;
	if (dolinestat->curlen == dolinestat->tokstr->len) {
		STRINGARRAY *temp = stringarrayresize(dolinestat->tokstr,dolinestat->tokstr->len + DOLINEINCSIZE);
		if (temp != NULL) {
			dolinestat->tokstr = temp;
		} else {
			dolinestat->iserr=1; 
			return;
		}
	}
	STRINGOF(dolinestat->tokstr)[dolinestat->curlen++] = chv;
}

BSTATIC scriptoffset dodoline(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	LVALUE lv;
	numtype answer;
	tokstat ts;
	dolinetokstat dolinestat;
	scriptoffset saveScuroffset, saveSerroroffset, saveSonerroroffset;
	const char *saveSstring, *saveSnexttoken, *saveSscriptstart;
	char *end;

    match(PASSSTATE DOLINE);
	
	str = stringexpr(VOIDPASSSTATE);

	if (str == NULL) {
		seterroroutofmemory(VOIDPASSSTATE);
		return NEXTLINEOFFSET;
	}
	if (Serrorflag != 0) {
		free(str);
		return NEXTLINEOFFSET;
	}
	dolinestat.tokstr = createstringarray(0);
	if (dolinestat.tokstr == NULL) {
		free(str);
		seterroroutofmemory(VOIDPASSSTATE);
		return NEXTLINEOFFSET;
	}
	dolinestat.iserr = 0;
	dolinestat.curlen = 0;
	inittokenizestate(&ts);
	end = STRINGOF(str) + str->len;
	tokenizeline(STRINGOF(str), &end, &ts, adddolinetoken, &dolinestat);
	free(str);
	adddolinetoken(0, &dolinestat);
	if (dolinestat.iserr) {
		seterroroutofmemory(VOIDPASSSTATE);
		free(dolinestat.tokstr);
		return NEXTLINEOFFSET;
	}
	str = stringarrayresize(dolinestat.tokstr, dolinestat.curlen);
	if (str == NULL) {
		seterroroutofmemory(VOIDPASSSTATE);
		free(dolinestat.tokstr);
		return NEXTLINEOFFSET;
	}
	dolinestat.tokstr = str;
	saveSstring = Sstring;
	saveSnexttoken = Snexttoken;
	saveScuroffset = Scuroffset;
	saveSerroroffset = Serroroffset;
	saveSonerroroffset = Sonerroroffset;
	saveSscriptstart = Sscriptstart;
	Serrorflag = 0;
	Sscriptstart = Sstring = Snexttoken = STRINGOF(dolinestat.tokstr);
    Sonerroroffset = Serroroffset = ENDPROGRAMOFFSET;
    Scuroffset = 0;
	line(PASSSTATE 1);
	free(dolinestat.tokstr);
	answer = (numtype)Serrorflag;
	if (Serrorflag != ERR_OUTOFMEMORY) Serrorflag = 0;
	Scuroffset = saveScuroffset;
	Serroroffset = saveSerroroffset;
	Sonerroroffset = saveSonerroroffset;
	Snexttoken = saveSnexttoken;
	Sstring = saveSstring;
	Sscriptstart = saveSscriptstart;
	Stoken = gettoken(VOIDPASSSTATE );

    match(PASSSTATE COMMA);
    lvalue(PASSSTATE &lv);
	
	if (lv.type == FLTID)  {
		*lv.d.dval = answer;
	} else {
		seterror(PASSSTATE ERR_BADTYPE);
	}
	return NEXTLINEOFFSET;
}

/*
   INPLEN statement
*/

BSTATIC numtype num_inplen(VOIDACCEPTSTATE)
{
  int len, oldlen = Scurinputmax;
  
  matchoparen(PASSSTATE INPLEN);
  len = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);
  if ((len < 1) || (len > 10000))
  	seterror(PASSSTATE ERR_BADVALUE);
  else Scurinputmax = len;
  if(Scurinputptr) {
	free(Scurinputptr);
	Scurinputptr = NULL;
  }
  return (numtype) oldlen;
}

/*
   SETIDLE statement
*/

BSTATIC numtype num_setidle(VOIDACCEPTSTATE)
{
  int idlestate = Sidlestate, nidlestate;
  
  matchoparen(PASSSTATE SETIDLE);
  nidlestate = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);
  if (Serrorflag == 0) Sidlestate = nidlestate;
  return (numtype) idlestate;
}

/*
  the INPUT statement
*/
BSTATIC scriptoffset doinput(VOIDACCEPTSTATE)
{
  LVALUE lv;

  match(PASSSTATE INPUT);
  lvalue(PASSSTATE &lv);

  if (Scurinputptr == NULL)
     seterror(PASSSTATE ERR_BADVALUE);
  else 
  {
	switch(lv.type)
	{
	case FLTID:
#ifdef USEFLOATS
		*lv.d.dval = mystrtod(Scurinputptr, NULL);
#else
		*lv.d.dval = mystrtol(Scurinputptr, NULL);
#endif
		break;
	case STRID:
		if(*lv.d.sval)
			free(*lv.d.sval);
		*lv.d.sval = stringarraychar(Scurinputptr);
		if(!*lv.d.sval)
			seterroroutofmemory(VOIDPASSSTATE );
		break;
	}
  }
  return NEXTLINEOFFSET;
}

/*
  Get an lvalue from the environment
  Params: lv - structure to fill.
  Notes: missing variables (but not out of range subscripts)
         are added to the variable list.
*/
BSTATIC void lvalue(ACCEPTSTATE LVALUE *lv)
{
  char name[IDLENGTH];
  int len;
  VARIABLE *var;
  DIMVAR *dimvar;
  dimsize index[MAXDIMS];
  void *valptr = 0;
  int type;
  int i;
  
  lv->type = ERROR;
  lv->d.sval = 0;

  switch(Stoken)
  {
    case FLTID:
	  getid(PASSSTATE Sstring, name, &len);
	  match(PASSSTATE FLTID);
	  var = findvariable(PASSSTATE name, NULL);
	  if(!var)
		var = addfloat(PASSSTATE name);
	  if(!var)
	  {
	    seterroroutofmemory(VOIDPASSSTATE );
		return;
	  }
	  lv->type = FLTID;
	  lv->d.dval = &var->d.dval;
	  break;
    case STRID:
	  getid(PASSSTATE Sstring, name, &len);
	  match(PASSSTATE STRID);
	  var = findvariable(PASSSTATE name, NULL);
	  if(!var)
		var = addstring(PASSSTATE name);
	  if(!var)
	  {
	    seterroroutofmemory(VOIDPASSSTATE );
		return;
	  }
	  lv->type = STRID;
	  lv->d.sval = &var->d.sval;
	  break;
	case DIMFLTID:
	case DIMSTRID:
	  type = (Stoken == DIMFLTID) ? FLTID : STRID;
	  getid(PASSSTATE Sstring, name, &len);
	  match(PASSSTATE Stoken);
	  dimvar = finddimvar(PASSSTATE name, NULL);
	  if(dimvar)
	  {
		i=0;
		while (i<dimvar->ndims)
		{
			index[i] = integer( expr(VOIDPASSSTATE ) );
			if (++i != dimvar->ndims)
				match(PASSSTATE COMMA);
		}
		if (Serrorflag == 0)
			valptr = getdimvar(PASSSTATE dimvar, index);
		match(PASSSTATE CPAREN);
	  }
	  else
	  {
	    seterror(PASSSTATE ERR_NOSUCHVARIABLE);
        return;
      }
	  if(valptr)
	  {
		lv->type = type;
	    if(type == FLTID)
	      lv->d.dval = valptr;
	    else if(type == STRID)
	      lv->d.sval = valptr;
		else
		  ASSERTDEBUG(basicassert(0,"bad lv->type"));
	  }
	  break;
	default:
	  seterrorsyntaxerror(VOIDPASSSTATE);
  }
}


/*
  parses an expression
*/
BSTATIC numtype expr(VOIDACCEPTSTATE)
{
  numtype left;
  
  left = term1(VOIDPASSSTATE );
  for (;;)
	{
		numtype right;
		switch(Stoken)
		{
		case AND:
			match(PASSSTATE AND);
			right = term1(VOIDPASSSTATE);
			left = (left != 0) && (right != 0);
			break;
		case OR:
			match(PASSSTATE OR);
			right = term1(VOIDPASSSTATE);
			left = (left != 0) || (right != 0);
			break;
		default:
			return left;
		}
	} 
}

BSTATIC int getrighthelper(ACCEPTSTATE STRINGARRAY *left)
{
	int result = 0;
	STRINGARRAY *right;
	if ((right = stringexpr(VOIDPASSSTATE )) != NULL) {
		result = stringarraycmp(left,right);
		free(right);
	}
	return result;
}

/* parses a >=* term */
BSTATIC numtype term1(VOIDACCEPTSTATE)
{
  numtype left;
  
  if(isstring(Stoken)) {
	left = 0;
	STRINGARRAY *sleft = stringexpr(VOIDPASSSTATE);
	if (sleft != NULL) {
		switch (Stoken)
		{
			case EQUALS:
				match(PASSSTATE EQUALS);
				left = getrighthelper(PASSSTATE sleft) == 0;
				break;
			case GREATER:
				match(PASSSTATE GREATER);
				if(istokenmatch(PASSSTATE EQUALS))
				{
					left = getrighthelper(PASSSTATE sleft) >= 0;
					break;
				} 
				left = getrighthelper(PASSSTATE sleft) > 0;
				break;
			case LESS:
				match(PASSSTATE LESS);
				if(istokenmatch(PASSSTATE EQUALS))
				{
					left = getrighthelper(PASSSTATE sleft) <= 0;
					break;
				}
				if(istokenmatch(PASSSTATE GREATER))
				{
					left = getrighthelper(PASSSTATE sleft) != 0;
					break;
				}
				left = getrighthelper(PASSSTATE sleft) < 0;
				break;
			default:
				seterrorsyntaxerror(VOIDPASSSTATE);
		}
		free(sleft);
	}
	return left;
  }
  
  left = term2(VOIDPASSSTATE );
  {
	switch(Stoken)
		{
			case EQUALS:
				match(PASSSTATE EQUALS);
				left = (left == term2(VOIDPASSSTATE ));
				break;
			case GREATER:
				match(PASSSTATE GREATER);
				if(istokenmatch(PASSSTATE EQUALS))
				{
					left = (left >= term2(VOIDPASSSTATE ));
					break;
				}
				left = (left > term2(VOIDPASSSTATE ));
				break;
			case LESS:
				match(PASSSTATE LESS);
				if(istokenmatch(PASSSTATE EQUALS))
				{
					left = (left <= term2(VOIDPASSSTATE ));
					break;
				}
				else if(istokenmatch(PASSSTATE GREATER))
				{
					left = (left != term2(VOIDPASSSTATE ));
					break;
				}
				left = (left < term2(VOIDPASSSTATE));
				break;
		}
	}
	return left;
}

/*
  parses a term 
*/
BSTATIC numtype term2(VOIDACCEPTSTATE)
{
  numtype left;

  left = term3(VOIDPASSSTATE );
  for (;;)
  {
	numtype right;
    switch(Stoken)
	{
		case PLUS:
			match(PASSSTATE PLUS);
			right = term3(VOIDPASSSTATE );
			left += right;
			break;
		case MINUS:
			match(PASSSTATE MINUS);
			right = term3(VOIDPASSSTATE );
			left -= right;
			break;
		default:
			return left;
	}
  }
}

/*
  parses a term 
*/
BSTATIC numtype term3(VOIDACCEPTSTATE)
{
  numtype left;

  left = factor(VOIDPASSSTATE );
  for (;;)
  {
	numtype right;
    switch(Stoken)
	{
		case MULT:
			match(PASSSTATE MULT);
			right = factor(VOIDPASSSTATE );
			left *= right;
			break;
		case DIV:
			match(PASSSTATE DIV);
			right = factor(VOIDPASSSTATE );
			if(right != ((numtype)0))
				left /= right;
			else
				seterror(PASSSTATE ERR_DIVIDEBYZERO);
			break;
		case MOD:
			match(PASSSTATE MOD);
			right = factor(VOIDPASSSTATE );
			if(right != ((numtype)0))
#ifdef USEFLOATS
				left = fmod(left, right);
#else
				left = left % right;
#endif
			else
				seterror(PASSSTATE ERR_DIVIDEBYZERO);
			break;
		case LSL:
			match(PASSSTATE LSL);
			right = factor(VOIDPASSSTATE );
			left = (numtype) (((unsigned int)left) << ((int)right));
			break;
		case LSR:
			match(PASSSTATE LSR);
			right = factor(VOIDPASSSTATE );
			left = (numtype) (((unsigned int)left) >> ((int)right));
			break;
		case BITOR:
			match(PASSSTATE BITOR);
			right = factor(VOIDPASSSTATE );
			left = (numtype) (((int)left) | ((int)right));
			break;
		case BITAND:
			match(PASSSTATE BITAND);
			right = factor(VOIDPASSSTATE );
			left = (numtype) (((int)left) & ((int)right));
			break;
		case BITXOR:
			match(PASSSTATE BITXOR);
			right = factor(VOIDPASSSTATE );
			left = (numtype) (((int)left) ^ ((int)right));
			break;
		default:
			return left;
	}
  }

}

/*
  parses a factor
*/
BSTATIC numtype factor(VOIDACCEPTSTATE)
{
  numtype answer = 0;
  int len;

  switch(Stoken)
  {
    case OPAREN:
	  match(PASSSTATE OPAREN);
	  answer = expr(VOIDPASSSTATE );
	  match(PASSSTATE CPAREN);
	  break;
	case VALUE:
	  answer = getvalue(Sstring, &len);
	  match(PASSSTATE VALUE);
	  break;
	case NOT:
	  match(PASSSTATE NOT);
	  answer = (numtype) (factor(VOIDPASSSTATE)==0);
	  break;
	case BITNOT:
	  match(PASSSTATE BITNOT);
	  answer = (numtype) (~((int) factor(VOIDPASSSTATE )));
	  break;
	case MINUS:
	  match(PASSSTATE MINUS);
	  answer = -factor(VOIDPASSSTATE );
	  break;
	case FLTID:
	  answer = variable(VOIDPASSSTATE );
	  break;
	case DIMFLTID:
	  answer = dimvariable(VOIDPASSSTATE );
	  break;
	default:
	  if ((Stoken >= (TOKENBASE+1)) && (Stoken <= (TOKENBASE + TOKENLISTENTRIES))) {
		const tokenlist *t = &tl[Stoken-(TOKENBASE+1)];
		if (t->tokentp == TOKEN_NUMFUNC) {
			answer = ((numfunc)(t->func))(VOIDPASSSTATE);
			break;
		} 
	  }
	  if(isstring(Stoken))
		seterror(PASSSTATE ERR_BADTYPE);
	  else
		seterrorsyntaxerror(VOIDPASSSTATE);
	  break;
  }
  return answer;
}

#ifdef USEFLOATS
/* num_ functions */
BSTATIC numtype num_pi(VOIDACCEPTSTATE)
{
	match(PASSSTATE PI);
	return 3.14159265359;
}

BSTATIC numtype num_sin(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE SIN);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	return sin(answer);
}

BSTATIC numtype num_cos(VOIDACCEPTSTATE)
{
	numtype answer;
    matchoparen(PASSSTATE COS);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	return cos(answer);
}

BSTATIC numtype num_tan(VOIDACCEPTSTATE)
{	
	numtype answer;
	matchoparen(PASSSTATE TAN);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	return tan(answer);
}

BSTATIC numtype num_log(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE LOG);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	if(answer > 0)
	   return log(answer);
	seterror(PASSSTATE ERR_NEGLOG);
	return 0;
}

BSTATIC numtype num_exp(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE EXP);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	return exp(answer);
}

BSTATIC numtype num_pow(VOIDACCEPTSTATE)
{
	numtype answer1, answer2;
    matchoparen(PASSSTATE POW);
	answer1 = expr(VOIDPASSSTATE );
	match(PASSSTATE COMMA);
	answer2 = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
    return pow(answer1, answer2);
}

BSTATIC numtype num_sqrt(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE SQRT);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	if(answer >= ((numtype)0))
		return sqrt(answer);
	seterror(PASSSTATE ERR_NEGSQRT);
	return 0;
}

BSTATIC numtype num_asin(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE ASIN);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	if(answer >= -1 && answer <= 1)
	    return asin(answer);
	seterror(PASSSTATE ERR_BADSINCOS);
	return 0;
}

BSTATIC numtype num_acos(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE ACOS);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	if(answer >= ((numtype)(-1)) && answer <= ((numtype)1))
		return acos(answer);
	seterror(PASSSTATE ERR_BADSINCOS);
	return 0;
}

BSTATIC numtype num_atan(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE ATAN);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	return atan(answer);
}
#endif

BSTATIC numtype num_abs(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE ABS);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
#ifdef USEFLOATS
	return  fabs(answer);
#else
	return (answer >= 0) ? answer : -answer;
#endif
}

BSTATIC numtype num_inkey(VOIDACCEPTSTATE)
{
    match(PASSSTATE INKEY);
	return (numtype)inkey();
}

BSTATIC numtype num_adrdbuf(VOIDACCEPTSTATE)
{
    match(PASSSTATE ADRDBUF);
	return (numtype)adc_readlen();
}

BSTATIC numtype num_adwrlft(VOIDACCEPTSTATE)
{
    match(PASSSTATE ADWRLFT);
	return (numtype)dac_wrtleft();
}

BSTATIC numtype num_peek(VOIDACCEPTSTATE)
{
   unsigned int adr;
   
   matchoparen(PASSSTATE PEEK);
   adr = ((unsigned int)expr(VOIDPASSSTATE ));	  
   match(PASSSTATE CPAREN);
#ifdef PEEKPOKE
    if (!Serrorflag)
		return (numtype) (*((unsigned int *)adr));
#endif
   return 0;
}

BSTATIC numtype num_len(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	numtype answer;
	
	matchoparen(PASSSTATE LEN);
	str = stringexpr(VOIDPASSSTATE);
	match(PASSSTATE CPAREN);
	if(str)
	{
	   answer = (numtype)(str->len);
	   free(str);
    } else answer = 0;
	return answer;
}

BSTATIC numtype num_ascii(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	int len;
	numtype answer;

	matchoparen(PASSSTATE ASCII);
	str = stringexpr(VOIDPASSSTATE);
	if (istokenmatch(PASSSTATE COMMA)) {
		len = ((unsigned int)expr(VOIDPASSSTATE )) - 1;
	} else len = 0;
	match(PASSSTATE CPAREN);
	answer = (numtype) -1;
	if (str) {
		if ((len >= 0) && (len < str->len))
			answer = (STRINGOF(str))[len];
		free(str);
	}
	return answer;
}

BSTATIC numtype num_err(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE ERR);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	if (answer != 0) {
		answer = (numtype) findlinenumber( PASSSTATE Serroroffset );
		Serroroffset = ENDPROGRAMOFFSET;
	} else {
		answer = (numtype) Slasterrorflag;
		Slasterrorflag = 0;
    }
	return answer;
}

BSTATIC numtype num_int(VOIDACCEPTSTATE)
{
	numtype answer;
	matchoparen(PASSSTATE INT);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
#ifdef USEFLOATS
	return floor(answer);
#else
	return answer;
#endif
}

BSTATIC numtype num_rnd(VOIDACCEPTSTATE)
{
	numtype answer;

	matchoparen(PASSSTATE RND);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	answer = integer(answer);
#ifdef USEFLOATS
	if (answer > 1)
		answer = floor(myrand()/(MYRAND_MAX + 1.0) * answer);
	else if(answer == 1)
		answer = myrand()/(MYRAND_MAX + 1.0);
	else
	{
		if(answer < 0)
		    mysrand( (unsigned) -answer);
		answer = ((numtype)0);
	}
#else
    if (answer > 1)
		answer = myrand() % answer;
	else if(answer == 1)
		answer = myrand();
	else
	{
	   if(answer < 0)
		    mysrand( (unsigned) -answer);
	   answer = ((numtype)0);
	}
#endif
	return answer;
}

BSTATIC numtype num_iif(VOIDACCEPTSTATE)
{
	int boolex;
	numtype trueval, falseval;
		 
	matchoparen(PASSSTATE IIF);
	boolex = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);
	trueval = expr(VOIDPASSSTATE);
	match(PASSSTATE COMMA);
	falseval = expr(VOIDPASSSTATE);
	match(PASSSTATE CPAREN);
	return (boolex != 0) ? trueval : falseval;
}

BSTATIC numtype num_val(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	numtype answer;
	
	matchoparen(PASSSTATE VAL);
	str = stringexpr(VOIDPASSSTATE);
    match(PASSSTATE CPAREN);
    if(str)
	  {
#ifdef USEFLOATS
	    answer = mystrtod(STRINGOF(str), 0);
#else
	    answer = mystrtol(STRINGOF(str), 0);
#endif
		free(str);
	  } else
		answer = 0;
	return answer;
}

BSTATIC numtype num_vallen(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	const stringchar *end;
	numtype answer;
	
	matchoparen(PASSSTATE VALLEN);
	str = stringexpr(VOIDPASSSTATE);
	match(PASSSTATE CPAREN);
	if(str)
	  {
#ifdef USEFLOATS
	    mystrtod(STRINGOF(str), &end);
#else
		mystrtol(STRINGOF(str), &end);
#endif
		answer = ((numtype)(end - STRINGOF(str)));
		free(str);
	}
	  else
		answer = ((numtype)0);
	return answer;
}

BSTATIC numtype num_adwrite(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	int len;
	numtype answer;
	
	matchoparen(PASSSTATE ADWRITE);
	str = stringexpr(VOIDPASSSTATE);
	match(PASSSTATE CPAREN);
	if(str)
	  {
		len = (str->len/2);
		answer = (numtype) write_dac((unsigned short *)STRINGOF(str),len);
		free(str);
	}
	  else
		answer = ((numtype)0);
	return (numtype)answer;
}

BSTATIC numtype num_timeset(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	int answer;

	matchoparen(PASSSTATE TIMESET);
	str = stringexpr(VOIDPASSSTATE);
	match(PASSSTATE CPAREN);
	if (str) {
		answer = rtcWriteString(STRINGOF(str));
		free(str);
	} else answer = 0;
	return (numtype)answer;
}

BSTATIC numtype num_serout(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	int port;
	numtype answer;
	
	matchoparen(PASSSTATE SEROUT);
	port = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);
	str = stringexpr(VOIDPASSSTATE);
	match(PASSSTATE CPAREN);
	if((str) && (!Serrorflag))
	  {
		answer = (numtype) serial_write(port,(char *)STRINGOF(str),str->len);
		free(str);
	}
	  else
		answer = ((numtype)0);
	return answer;
}

BSTATIC numtype num_serbng(VOIDACCEPTSTATE)
{
	STRINGARRAY *str;
	int port,dly;
	numtype answer;
	
	matchoparen(PASSSTATE SERBNG);
	port = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);
	dly = integer ( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);	
	str = stringexpr(VOIDPASSSTATE);
	match(PASSSTATE CPAREN);
	if((str) && (!Serrorflag)) 
	  {
		answer = (numtype) serbng(port,(unsigned char *)STRINGOF(str),str->len,dly);
		free(str);
	}
	  else
		answer = ((numtype)0);
	return answer;
}

BSTATIC numtype num_serspi(VOIDACCEPTSTATE)
{
	int mosipin, misopin, clkpin, c, dly;
	int answer = 0;

	matchoparen(PASSSTATE SERSPI);
	mosipin = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);
	misopin = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);
	clkpin = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);
	c = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE COMMA);
	dly = integer( expr(VOIDPASSSTATE)  );
	match(PASSSTATE CPAREN);

	if (!Serrorflag) {
		answer = serspi(mosipin,misopin,clkpin,c,dly);
	}
	return ((numtype)answer);
}

BSTATIC numtype num_inadc(VOIDACCEPTSTATE)
{
	numtype answer;
	tokennum t = Stoken;
	matchoparen(PASSSTATE t);
	answer = expr(VOIDPASSSTATE );
	match(PASSSTATE CPAREN);
	if (!Serrorflag) {
		answer = (numtype)((t == IND) ? ind(answer) : inadc(answer));
	}
	return answer;
}

/*
  calculate the BSTR() function.
*/
BSTATIC numtype binstr(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  unsigned long answer = 0;
  int ord;

  matchoparen(PASSSTATE BSTR);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  ord = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);

  if (str != NULL) {
	int i;
	if (ord > 0) {
		if (ord > str->len)
			ord = str->len;
		for (i=ord;i>0;) 
			answer = (answer << 8) + ((unsigned char)STRINGOF(str)[--i]);
	} else {
		ord = -ord;
		if (ord > str->len)
			ord = str->len;
		for (i=0;i<ord;i++) 
			answer = (answer << 8) + ((unsigned char)STRINGOF(str)[i]);
	}
	free(str);
  }
  return (numtype)answer;
}

/*
  calcualte the INSTR() function.
*/
BSTATIC numtype instr(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  STRINGARRAY *substr;
  numtype answer = 0;
  int offset;

  matchoparen(PASSSTATE INSTR);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  substr = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  offset = integer( expr(VOIDPASSSTATE ) );
  offset--;
  match(PASSSTATE CPAREN);

  if ((str != NULL) && (substr != NULL)) 
	answer = strstrarray(str, substr, offset)+1;
 
  if(str)
    free(str);
  if(substr)
    free(substr);

  return answer;
}

/*
  get the value of a scalar variable from string
  matches FLTID
*/
BSTATIC numtype variable(VOIDACCEPTSTATE)
{
  VARIABLE *var;
  char id[IDLENGTH];
  int len;

  getid(PASSSTATE Sstring, id, &len);
  match(PASSSTATE FLTID);
  var = findvariable(PASSSTATE id, NULL);
  if ((var != NULL) && (var->type == FLTID))
    return var->d.dval;
  else
  {
	seterror(PASSSTATE ERR_NOSUCHVARIABLE);
	return ((numtype)0);
  }
}

/*
  get value of a dimensioned variable from string.
  matches DIMFLTID
*/
BSTATIC numtype dimvariable(VOIDACCEPTSTATE)
{
  DIMVAR *dimvar;
  char id[IDLENGTH];
  int len;
  dimsize index[MAXDIMS];
  int i;
  numtype *answer;

  getid(PASSSTATE Sstring, id, &len);
  match(PASSSTATE DIMFLTID);
  dimvar = finddimvar(PASSSTATE id, NULL);
  if(!dimvar)
  {
    seterror(PASSSTATE ERR_NOSUCHVARIABLE);
	return ((numtype)0);
  }

  if(dimvar)
  {
	  i=0;
	  while (i<dimvar->ndims)
	  {
		  index[i] = integer( expr(VOIDPASSSTATE ) );
		  if (++i != dimvar->ndims)
			  match(PASSSTATE COMMA);
	  }
	answer = getdimvar(PASSSTATE dimvar, index);
	match(PASSSTATE CPAREN);
  }

  if(answer)
	return *answer;

  return ((numtype)0);

}

/*
  find a scalar variable invariables list
  Params: id - id to get
  Returns: pointer to that entry, 0 on fail
*/
BSTATIC VARIABLE *findvariable(ACCEPTSTATE const char *id, int *nearest)
{
  int high;
  int low;
  int mid;
  int cmp;

  low = 0;
  high = Snvariables-1;

  while(low <= high)
  {
    mid = (high + low)/2;
	cmp = strcasecmp(Svariables[mid].id, id);
	if (cmp > 0)
	  high = mid - 1;
	else if (cmp < 0)
	  low = mid + 1;
	else {
		if (nearest != NULL) *nearest = mid;
		return &Svariables[mid];
	}
  }
  if (nearest != NULL) *nearest = low;
  return NULL;
}

/*
  get a dimensioned array by name
  Params: id (includes opening parenthesis)
  Returns: pointer to array entry or 0 on fail
*/
BSTATIC DIMVAR *finddimvar(ACCEPTSTATE const char *id, int *nearest)
{
  int high;
  int low;
  int mid;
  int cmp;

  low = 0;
  high = Sndimvariables-1;

  while(low <= high)
  {
    mid = (high + low)/2;
	cmp = strcasecmp(Sdimvariables[mid].id, id);
	if (cmp > 0)
	  high = mid - 1;
	else if (cmp < 0)
	  low = mid + 1;
	else {
		if (nearest != NULL) *nearest = mid;
		return &Sdimvariables[mid];
	}
  }
  if (nearest != NULL) *nearest = low;
  return NULL;
}

/*
  dimension an array.
  Params: id - the id of the array (include leading ()
          ndims - number of dimension (1-5)
		  ... - integers giving dimension size, 
*/
BSTATIC DIMVAR *dimension(ACCEPTSTATE const char *id, int ndims, dimsize *intdims)
{
  DIMVAR *dv;
  int size ;
  int oldsize;
  int i;
  numtype *dtemp;
  STRINGARRAY **stemp;

  if(ndims > MAXDIMS)
	return 0;

  dv = finddimvar(PASSSTATE id, NULL);
  if(!dv)
	dv = adddimvar(PASSSTATE id);
  if(!dv)
  {
    seterroroutofmemory(VOIDPASSSTATE );
	return 0;
  }

  oldsize = dv->ndims ? computedimsize(dv) : 0;

  size = 1;
  for(i=0;i<ndims;i++)
    size *= intdims[i];

  switch(dv->type)
  {
    case FLTID:
      dtemp = realloc(dv->d.dval, size * sizeof(numtype));
      if(dtemp)
        dv->d.dval = dtemp;
	  else
	  {
		seterroroutofmemory(VOIDPASSSTATE );
	    return 0;
	  }
      for(i=oldsize;i<size;i++)
		  dv->d.dval[i] = ((numtype)0);
	  break;
	case STRID:
	  if(dv->d.str)
	  {
	    for(i=size;i<oldsize;i++) {
		  stemp = &dv->d.str[i];
		  if(*stemp)
		  {
			free(*stemp);
		    *stemp = 0;
		  }
		}
	  }
	  stemp = realloc(dv->d.str, size * sizeof(STRINGARRAY *));
	  if(stemp)
	  {
		dv->d.str = stemp;
	    for(i=oldsize;i<size;i++)
		  dv->d.str[i] = 0;
	  }
	  else
	  {
		 for(i=0;i<oldsize;i++) {
	      stemp = &dv->d.str[i];
		  if(*stemp)
		  {
            free(*stemp);
		    *stemp = 0;
		  }
		 }
	    seterroroutofmemory(VOIDPASSSTATE );
		return 0;
	  }
	  break;
	default:
	  ASSERTDEBUG(basicassert(0,"bad dv->type"));
  }

  for(i=0;i<MAXDIMS;i++)
	dv->dim[i] = intdims[i];
  dv->ndims = ndims;

  return dv;
}

/*
  get the address of a dimensioned array element.
  works for both string and real arrays.
  Params: dv - the array's entry in variable list
          ... - integers telling which array element to get
  Returns: the address of that element, 0 on fail
*/ 
BSTATIC void *getdimvar(ACCEPTSTATE DIMVAR *dv, dimsize *index)
{
  int i, ind;
  void *answer = 0;

  for(i=0;i<dv->ndims;i++)
    if(index[i] > dv->dim[i] || index[i] < 1)
	{
	  seterror(PASSSTATE ERR_BADSUBSCRIPT);
	  return 0;
	}
    ind = index[dv->ndims-1]-1;
  for (i=(dv->ndims-1);i>0;) {
	  i--;
	  ind = ind*dv->dim[i] + (index[i]-1);
  }
  if(dv->type == FLTID)
	  answer = &dv->d.dval[ind];
  else if (dv->type == STRID)
	  answer = &dv->d.str[ind]; 
  return answer;
}

#ifndef NEXTLG
#define NEXTLG 1
#endif

#define NG ((int)((1 << NEXTLG)-1))
BSTATIC int nextlargest(int n)
{
   return  ((n+NG)&(~NG));
}

BSTATIC VARIABLE *addnewvar(ACCEPTSTATE const char *id)
{
  VARIABLE *vars, *v;
  int nv = Snvariables + 1, place;

  vars = findvariable(PASSSTATE id, &place);
  if (vars == NULL) {
	if (nextlargest(Snvariables) != nextlargest(nv)) {
		vars = realloc(Svariables, nextlargest(nv) * sizeof(VARIABLE));
		if (!vars) {
			seterroroutofmemory(VOIDPASSSTATE );
			return NULL;
		}
		Svariables = vars;
	}
	v = &Svariables[place];
	memmove(&v[1],v,sizeof(VARIABLE)*(Snvariables-place));
	Snvariables = nv;
	return v;
  }
  return NULL;
}

/*
  add a real variable to our variable list
  Params: id - id of varaible to add.
  Returns: pointer to new entry in table
*/
BSTATIC VARIABLE *addfloat(ACCEPTSTATE const char *id)
{
  VARIABLE *v = addnewvar(PASSSTATE id);

  if (!v)
	  return 0;
  strcpy(v->id, id);
  v->d.dval = ((numtype)0);
  v->type = FLTID;
  return v;
}

/*
  add a string variable to table.
  Params: id - id of variable to get (including trailing $)
  Retruns: pointer to new entry in table, 0 on fail.       
*/
BSTATIC VARIABLE *addstring(ACCEPTSTATE const char *id)
{
  VARIABLE *v = addnewvar(PASSSTATE id);

  if (!v)
	  return 0;
  strcpy(v->id, id);
  v->d.sval = 0;
  v->type = STRID;
  return v;
}

/*
  add a new array to our symbol table.
  Params: id - id of array (include leading ()
  Returns: pointer to new entry, 0 on fail.
*/
BSTATIC DIMVAR *adddimvar(ACCEPTSTATE const char *id)
{
  DIMVAR *vars, *v;
  int nv = Sndimvariables + 1, place;

  vars = finddimvar(PASSSTATE id, &place);
  if (vars == NULL) {
	if (nextlargest(Sndimvariables) != nextlargest(nv)) {
		vars = realloc(Sdimvariables, nextlargest(nv) * sizeof(DIMVAR));
		if (!vars) {
			seterroroutofmemory(VOIDPASSSTATE );
			return NULL;
		}
		Sdimvariables = vars;
	}
	v = &Sdimvariables[place];
	memmove(&v[1],v,sizeof(DIMVAR)*(Sndimvariables-place));
	strcpy(v->id, id);
	v->d.dval = 0;
	v->d.str = 0;
	v->ndims = 0;
	v->type = strchr(id, '$') ? STRID : FLTID;
	Sndimvariables = nv;
	return v;
  }
  return NULL;
}

/*
  high level string parsing function.
  Returns: a malloced pointer, or 0 on error condition.
  caller must free!
*/
BSTATIC STRINGARRAY *stringexpr(VOIDACCEPTSTATE)
{
  STRINGARRAY *left;
  STRINGARRAY *right;
  STRINGARRAY *temp;

  switch(Stoken)
  {
    case DIMSTRID:
	  left = stringarraydup(stringdimvar(VOIDPASSSTATE));
	  break;
    case STRID:
      left = stringarraydup(stringvar(VOIDPASSSTATE));
	  break;
	case QUOTE:
	  left = stringliteral(VOIDPASSSTATE);
	  break;
	default:
	  if ((Stoken >= (TOKENBASE+1)) && (Stoken <= (TOKENBASE + TOKENLISTENTRIES))) {
		const tokenlist *t = &tl[Stoken-(TOKENBASE+1)];
		if (t->tokentp == TOKEN_STRFUNC) {
			left = ((stringfunc)(t->func))(VOIDPASSSTATE);
			break;
		} else seterror(PASSSTATE ERR_TYPEMISMATCH);
	  } else seterrorsyntaxerror(VOIDPASSSTATE);
	  return createstringarray(0);
  }

  if(!left)
  {
    seterroroutofmemory(VOIDPASSSTATE );
    return 0;
  }

  switch(Stoken)
  {
    case PLUS:
	  match(PASSSTATE PLUS);
	  right = stringexpr(VOIDPASSSTATE);
	  if(right)
	  {
	    temp = stringarrayconcat(left, right);
	    free(right);
		if(temp)
		{
		  free(left);
          left = temp;
		}
		else
		  seterroroutofmemory(VOIDPASSSTATE );
	  }
	  else
		seterroroutofmemory(VOIDPASSSTATE );
	  break;
	default:
	  return left;
  }

  return left;
}

/*
  parse the CHR$ token
*/
BSTATIC STRINGARRAY *chrstring(VOIDACCEPTSTATE)
{
  int x;
  STRINGARRAY s, *answer;
  
  matchoparen(PASSSTATE CHRSTRING);
  x = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);

  s.len = 1;
  s.beginstring[0] = x;

  answer = stringarraydup(&s);

  return answer;
}

/*
  parse the BIN$ token
*/
BSTATIC STRINGARRAY *binstring(VOIDACCEPTSTATE)
{
  unsigned long num;
  int nd;
  STRINGARRAY *answer = NULL;

  matchoparen(PASSSTATE BINSTRING);
  num = (unsigned long) expr(VOIDPASSSTATE );
  match(PASSSTATE COMMA);
  nd = integer( expr(VOIDPASSSTATE) );
  match(PASSSTATE CPAREN);

  if ((nd>=1) && (nd<=4)) {
	if ((answer=createstringarray(nd)) != NULL) {
		int i;
		for (i=0;i<nd;i++) {
			STRINGOF(answer)[i]=(num & 0xFF);
			num >>= 8;
		}
	} 
  } else if ((nd>=-4) && (nd<=-1)) {
	nd=-nd;
	if ((answer=createstringarray(nd)) != NULL) {
		while (nd>0) {
			STRINGOF(answer)[--nd]=(num & 0xFF);
			num >>= 8;
		}
	} 
  } else seterror(PASSSTATE ERR_BADVALUE);
  return answer;
}

/*
  parse the II$ token
*/
BSTATIC STRINGARRAY *iistring(VOIDACCEPTSTATE)
{
  int x;
  STRINGARRAY *answer;
  STRINGARRAY *truestr, *falsestr;

  matchoparen(PASSSTATE IISTRING);
  x = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  truestr = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  falsestr = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);

  if ((truestr != NULL) && (falsestr != NULL)) {
		if (x != 0) {
			answer = truestr;
			free(falsestr);
		} else {
			answer = falsestr;
			free(truestr);
		}
  } else {
	if (truestr != NULL) free(truestr);
	if (falsestr != NULL) free(falsestr);
	answer = NULL;
  }
  return answer;
}

/*
  parse the HEX$ token
*/
BSTATIC STRINGARRAY *hexstring(VOIDACCEPTSTATE)
{
  int x;
  STRINGARRAY *answer;
  char p[ITOASIZE];

  matchoparen(PASSSTATE HEXSTRING);
  x = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);

  answer = stringarraychar(myltoahex(p,x));
  return answer;
}

/*
  parse the STR$ token
*/
BSTATIC STRINGARRAY *strstring(VOIDACCEPTSTATE)
{
  numtype x;
  STRINGARRAY *answer;

  matchoparen(PASSSTATE STRSTRING);
  x = expr(VOIDPASSSTATE );
  match(PASSSTATE CPAREN);

#ifdef USEFLOATS
  {
    char s[DTOASIZE];
    answer = stringarraychar(mydtoa(s,x));
  } 
#else
  {
    char s[ITOASIZE];
    answer = stringarraychar(myltoa(s,x));
  }
#endif
  return answer;
}

/*
  parse the ERC$ token
*/
BSTATIC STRINGARRAY *ercstring(VOIDACCEPTSTATE)
{
  int code;
  
  matchoparen(PASSSTATE ERCSTRING);
  code = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);

  if (code<0) code = Slasterrorflag;
  return stringarraychar(((code >= 0) && (code < ERRCODES)) ? errormessages[code] : "NONE");
}

/*
  parse the TRIM$ token
*/
BSTATIC STRINGARRAY *trimstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  STRINGARRAY *answer;
  int x, beg=0;
  
  matchoparen(PASSSTATE TRIMSTRING);
  str = stringexpr(VOIDPASSSTATE);
  if (istokenmatch(PASSSTATE COMMA)) {
	beg = integer( expr(VOIDPASSSTATE) );
  }
  match(PASSSTATE CPAREN);
  
  if(!str)
	return 0;
  if(beg) {
    x = 0;
	while (x<str->len) {
		if (!isspace(STRINGOF(str)[x])) break;
		x++;
	}
	answer = stringarrayduppart(str,x,str->len-x);
  } else {
	x = str->len;
	while (x>0) {
		if (!isspace(STRINGOF(str)[x-1])) break;
		x--;
	}
	answer = stringarrayduppart(str,0,x);
  } 
  free(str);
  return answer;
}

/*
  parse the UPPER$ token
*/
BSTATIC STRINGARRAY *upperstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  STRINGARRAY *answer;
  int x;
  int lwr=0;
  
  matchoparen(PASSSTATE UPPERSTRING);
  str = stringexpr(VOIDPASSSTATE);
  if (istokenmatch(PASSSTATE COMMA)) {
	lwr = integer( expr(VOIDPASSSTATE) );
  }
  match(PASSSTATE CPAREN);
  
  if(!str)
	return 0;
  answer = createstringarray(str->len);
  if (answer != NULL) {
	if (lwr) {
		for (x=0;x<str->len;x++) 
			STRINGOF(answer)[x] = tolower(STRINGOF(str)[x]);
	} else {
		for (x=0;x<str->len;x++) 
			STRINGOF(answer)[x] = toupper(STRINGOF(str)[x]);
	}
  }
  free(str);  
  return answer;
}

/*
  parse the LEFT$ token
*/
BSTATIC STRINGARRAY *confstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  const char *confs;
  int len;
  
  matchoparen(PASSSTATE CONFSTRING);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);
  
  if(!str)
	return 0;
  confs = conf_string(STRINGOF(str), &len);
  free(str);
  if (confs != NULL)
	return stringarraycharlen(confs, len);
  return createstringarray(0);
}

/*
  parse the ADREAD$ token
*/
BSTATIC STRINGARRAY *adreadstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *answer = NULL, *temp;
  int len;
  
  matchoparen(PASSSTATE ADREADSTRING);
  len = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);

  if ((Serrorflag == 0) && (len > 0)) {
	answer = createstringarray(len * 2);
	if (answer != NULL) {
	  len = read_adc((unsigned short *)STRINGOF(answer), len);
	  temp = stringarrayresize(answer, len*2);
	  if (temp != NULL)
			answer = temp;
	}
  }
  return answer;
}

/*
  parse the SERINP$ token
*/
BSTATIC STRINGARRAY *serinpstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *answer = NULL, *temp;
  int port, len, charend, timeout;
  
  matchoparen(PASSSTATE SERINPSTRING);
  port = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  len = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  charend = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  timeout = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);

  if (Serrorflag == 0) {
	if (len < 0) {
		len = serial_read(port, -len, charend, timeout, NULL);
		answer = createstringarray(0);
	} else {
		answer = createstringarray(len);
		if (answer != NULL) {
			len = serial_read(port, len, charend, timeout, (char *)STRINGOF(answer));
			temp = stringarrayresize(answer, len);
			if (temp != NULL)
				answer = temp;
		}
	}
  }
  return answer;
}

/*
  parse the SERI2C$ token
*/

BSTATIC STRINGARRAY *seri2cstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *answer = NULL, *str;
  int sdawritepin, sdareadpin, sclpin, addr, end_trans, numinbytes, errcode;
  
  matchoparen(PASSSTATE SERI2CSTRING);
  sdawritepin = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  sdareadpin = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  sclpin = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  addr = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  end_trans = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  numinbytes = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);
  
  if (str == NULL)
	return 0;
  if (Serrorflag != 0) {
	free(str);
	return 0;
  }
    
  answer = createstringarray(numinbytes);
  if (answer == NULL) {
	free(str);
	return 0;
  }
  memset(STRINGOF(answer),0,numinbytes);
  errcode=seri2c(sdawritepin,sdareadpin,sclpin,addr,end_trans,str->len,STRINGOF(str),
	      numinbytes,STRINGOF(answer));
  if (errcode != 0) {
	 free(answer);
	 answer = createstringarray(0);
  }
  free(str);
  return answer;
 }
  
/*
  parse the TIMEGET$ token
*/
BSTATIC STRINGARRAY *timegetstring(VOIDACCEPTSTATE)
{
  char rtcstring[RTC_STR_LEN];
  match(PASSSTATE TIMEGETSTRING);
  rtcReadString(rtcstring);
  return stringarraychar(rtcstring);
}

/*
  parse the LEFT$ token
*/
BSTATIC STRINGARRAY *leftstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  STRINGARRAY *answer;
  int x;
  
  matchoparen(PASSSTATE LEFTSTRING);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  x = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);
  
  if(!str)
	return 0;

  answer = stringarrayduppart(str,0,x);
  free(str);
  return answer;
}

/*
  parse the RIGHT$ token
*/
BSTATIC STRINGARRAY *rightstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  STRINGARRAY *answer;
  int x;

  matchoparen(PASSSTATE RIGHTSTRING);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  x = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);
  if(!str)
	return 0;

  x = str->len - x;
  answer = stringarrayduppart(str,x < 0 ? 0 : x,str->len);
  free(str);
  return answer;
}

/*
  parse the MID$ token
*/
BSTATIC STRINGARRAY *midstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  STRINGARRAY *answer;
  int x;
  int len = -1;

  matchoparen(PASSSTATE MIDSTRING);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  x = integer( expr(VOIDPASSSTATE ) );
  if (istokenmatch(PASSSTATE COMMA)) {
	len = integer( expr(VOIDPASSSTATE ) );
  }
  match(PASSSTATE CPAREN);

  if(!str)
	return 0;

  if(len == -1)
	len = str->len;
  
  answer = stringarrayduppart(str,x-1,len);
  free(str);
  return answer;
}

/*
  parse the string$ token
*/
BSTATIC STRINGARRAY *stringstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  STRINGARRAY *answer;
  int N;
  int i;

  matchoparen(PASSSTATE STRINGSTRING);
  N = integer( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);

  if(!str)
	return 0;

  if((N < 1) || (str->len < 1))
  {
    free(str);
	answer = createstringarray(0);
	return answer;
  }
  answer = createstringarray( N * str->len);
  if(answer == NULL)
  {
    free(str);
	return 0;
  }
  if (str->len == 1) {
    memset(answer->beginstring,str->beginstring[0],N);
  } else {
	for(i=0; i < N; i++)
		memmove(&answer->beginstring[str->len*i],str->beginstring,str->len);
  }
  free(str);
  return answer;
}

/*
  read a dimensioned string variable from input.
  Returns: pointer to string (not malloced) 
*/
BSTATIC STRINGARRAY *stringdimvar(VOIDACCEPTSTATE)
{
  char id[IDLENGTH];
  int len;
  DIMVAR *dimvar;
  STRINGARRAY **answer;
  dimsize index[MAXDIMS];
  int i;

  getid(PASSSTATE Sstring, id, &len);
  match(PASSSTATE DIMSTRID);
  dimvar = finddimvar(PASSSTATE id, NULL);

  if(dimvar)
  {
	i=0;
	while (i<dimvar->ndims) {
		index[i] = integer( expr(VOIDPASSSTATE ) );
		if (++i != dimvar->ndims)
			match(PASSSTATE COMMA);
	}
	answer = getdimvar(PASSSTATE dimvar, index);
	match(PASSSTATE CPAREN);
  }
  else
	seterror(PASSSTATE ERR_NOSUCHVARIABLE);

  if(!Serrorflag)
	if(*answer)
      return *answer;
	 
  return EMPTYSTRING;
}

/*
  parse a string variable.
  Returns: pointer to string (not malloced) 
*/
BSTATIC STRINGARRAY *stringvar(VOIDACCEPTSTATE)
{
  char id[IDLENGTH];
  int len;
  VARIABLE *var;

  getid(PASSSTATE Sstring, id, &len);
  match(PASSSTATE STRID);
  var = findvariable(PASSSTATE id, NULL);
  if(var)
  {
    if(var->type == STRID)
	  return var->d.sval;
	return EMPTYSTRING;
  }
  seterror(PASSSTATE ERR_NOSUCHVARIABLE);
  return EMPTYSTRING;
}

/*
  parse a string literal
  Returns: malloced string literal
  Notes: newlines aren't allwed in literals, but blind
         concatenation across newlines is. 
*/
BSTATIC STRINGARRAY *stringliteral(VOIDACCEPTSTATE)
{
  int len = 1;
  STRINGARRAY *answer = NULL;
  STRINGARRAY *substr;
  STRINGARRAY *temp;
  char *end;

  while(Stoken == QUOTE)
  {
    Sstring = skipspaces(Sstring);
    end = mystrend(Sstring, '"');
    if(end)
	{
      len = end - Sstring;
      substr = createstringarray(len);
	  if(!substr)
	  {
	    seterroroutofmemory(VOIDPASSSTATE );
	    return answer;
	  }
	  mystrgrablit(substr, Sstring);
	  if(answer)
	  {
		temp = stringarrayconcat(answer, substr);
	    free(substr);
		free(answer);
		answer = temp;
		if(answer == NULL)
		{
	      seterroroutofmemory(VOIDPASSSTATE );
		  return answer;
		}
	  }
	  else
	    answer = substr;
	  Sstring = end;
	  Snexttoken = Sstring+1;
	}
	else
	{
	  seterrorsyntaxerror(VOIDPASSSTATE);
	  return answer;
	}

	match(PASSSTATE QUOTE);
  }
  return answer;
}

#ifdef USEFLOATS
/*
  cast a numtype to an integer, triggering errors if out of range
*/
BSTATIC int integer(numtype x)
{
#if 0
  if( x < INT_MIN || x > INT_MAX )
	seterror(PASSSTATE  ERR_BADVALUE );
  if( x != floor(x) )
	seterror(PASSSTATE  ERR_NOTINT );
#endif
  return (int) x;
}
#endif

/*
  check that we have a token of the passed type 
  (if not set the errorflag)
  Move parser on to next token. Sets token and string.
*/
BSTATIC void match(ACCEPTSTATE int tok)
{
  if(Stoken != tok)
  {
	seterrorsyntaxerror(VOIDPASSSTATE);
	return;
  }
  Sstring = Snexttoken;
  Stoken = gettoken(VOIDPASSSTATE );
  if(Stoken == ERROR)
	seterrorsyntaxerror(VOIDPASSSTATE); 
}

/* shorthand to test for and get a token */
BSTATIC int istokenmatch(ACCEPTSTATE int tok)
{
  if (Stoken == tok) {
		match(PASSSTATE tok);
		return 1;
  }
  return 0;
}

/* shorthand to accept token plus open parenthesis */
BSTATIC void matchoparen(ACCEPTSTATE int tok)
{
   match(PASSSTATE tok);
   match(PASSSTATE OPAREN);   
}

/*
  get the offset of the next line
  Params: str - pointer to parse string
  Returns: line no of next line, 0 if end
  Notes: goes to newline, then finds
         next character or null
*/
BSTATIC scriptoffset getnextline(VOIDACCEPTSTATE)
{
  register const char *loc = &Sscriptstart[Scuroffset];
  while ((*loc != '\n') && (*loc)) loc++;
  if (*loc == '\n') loc++;
  return (scriptoffset)(loc - Sscriptstart);
}

void inituntokenizestate(untokstat *utk, char *str)
{
	if (((unsigned char)*str) == 0xFF)
		utk->s = STRINGOF(EMPTYSTRING);
	else
		utk->s = str;
	utk->toknum = 0;
}

int untokenizecode(untokstat *utk)
{
	int c;
	if (utk->toknum == 0) {
		c = *(utk->s);
		if (c != 0) (utk->s)++;
		if ((c >= (TOKENBASE+1)) && (c <= (TOKENBASE + TOKENLISTENTRIES))) {
			utk->toknum = c;
			utk->charnum = 0;
		}
	}
	if ((utk->toknum >= (TOKENBASE+1)) && (utk->toknum <= (TOKENBASE + TOKENLISTENTRIES))) 
	{
		const tokenlist *t = &tl[utk->toknum-(TOKENBASE+1)];
		c = t->tokenname[utk->charnum++];
		if ((utk->charnum == t->length) || (utk->charnum == -t->length))
			utk->toknum=0;
	}
	return c;
}

void inittokenizestate(tokstat *tk)
{
	tk->notlastspace = tk->inquote = 0;
}

void tokenizeline(char *str, char **end, tokstat *tk, addbasictoken abt, void *v)
{
  const tokenlist *t;
  int high, low, mid, c, ctok, ctokl;

  while (str < *end)
  {
	if (tk->inquote) {
		(*abt)(*str,v);
		if (*str == '\"') {
			if (*(++str) == '\"')
				(*abt)(*str++,v);
			else 
				tk->inquote = 0;
		} else 
			tk->inquote = (*str++ != '\n');
	} else {
		ctok = *str;
		ctokl = 1;
		if (ctok == '\"') {
			tk->inquote = 1;
		} else if (!tk->notlastspace) {
			low = 0;
			high = TOKENLISTENTRIES - 1;
			while (low <= high)
			{
				mid = (high + low)/2;
				t = &tl[mid];
				c = strncasecmp(str, t->tokenname, t->length > 0 ? t->length : -t->length);
				if (c < 0)
					high = mid - 1;
				else if (c > 0)
					low = mid + 1;
				else {
						if (t->length < 0) 
						{
							if (isalnum(str[-t->length]))  {
								low = mid + 1;
							} else {
								ctok = mid+(TOKENBASE+1);
								ctokl = -t->length;
								break;
							}
						} else {
							ctok = mid+(TOKENBASE+1);
							ctokl = t->length;
							break;
						}
					}
				}
			}
		str += ctokl;
		(*abt)(ctok,v);
		tk->notlastspace = isalnum(ctok);
	}
  }
  *end = str;
}

/*
  get a token from the string
  Params: str - string to read token from
  Notes: ignores white space between tokens
*/
BSTATIC tokennum gettoken(VOIDACCEPTSTATE)
{
  const char *str = skipspaceslf(Sstring);
  if (*str==0) {
	  Snexttoken = str;
	  return EOS;
  }
  if(isdigit(*str)) {
#ifdef USEFLOATS
	if ((str[0] == '0') && ((str[1] == 'x') || (str[1] == 'X'))) {
		do {
			str++;
		} while (isalnum(*str));
	} else {
		do {
			str++;
		} while (((*str >= '0') && (*str <= '9')) || (*str == '.'));
		if ((*str == 'e') || (*str == 'E')) {
			str++;
			if ((*str == '+') || (*str == '-')) str++;
			while ((*str >= '0') && (*str <= '9')) str++;
		}
	}
#else
    do {
		str++;
	} while (isalnum(*str));
#endif
	Snexttoken = str;
    return VALUE;
  }
  if(isalpha(*str))
  {
    do {
	   str++;
	} while(isalnum(*str));
	switch(*str)
	{
	  case ':':
	    Snexttoken = str+1;
	    return LABEL;
	  case '$':
	    if (str[1] == '(') {
			Snexttoken = str+2;
			return DIMSTRID;
		} else {
			Snexttoken = str+1;
			return STRID;
		}
	  case '(':
		Snexttoken = str+1;
		return DIMFLTID;
	  default:
		Snexttoken = str;
		return FLTID;
	}
  }
  Snexttoken = str+1;
  return (*str);
}

/*
  test if a token represents a string expression
  Params: token - token to test
  Returns: 1 if a string, else 0
*/
BSTATIC int isstring(tokennum token)
{
  if(token == STRID || token == QUOTE || token == DIMSTRID)
	return 1;
  if ((token >= (TOKENBASE+1)) && (token <= (TOKENBASE + TOKENLISTENTRIES)))
		return (tl[token-(TOKENBASE+1)].tokentp == TOKEN_STRFUNC);
  return 0;
}

/*
  get a numerical value from the parse string
  Params: str - the string to search
          len - return pinter for no chars read
  Retuns: the value of the string.
*/
BSTATIC numtype getvalue(const char *str, int *len)
{
  numtype answer;
  const char *end;

#ifdef USEFLOATS
  answer = mystrtod(str, &end);
#else
  answer = mystrtol(str, &end);
#endif
  *len = end - str;
  ASSERTDEBUG(basicassert(end != str,"getvalue")); 
  return answer;
}


/*
  getid - get an id from the parse string:
  Params: str - string to search
          out - id output [IDLENGTH chars max ]
		  len - return pointer for id length
  Notes: triggers an error if id > (IDLENGTH-1) chars
         the id includes the $ and ( qualifiers.
*/
BSTATIC void getid(ACCEPTSTATE const char *str, char *out, int *len)
{
  int nread = 0;
  str = skipspaces(str);
  ASSERTDEBUG(basicassert(isalpha(*str),"getid"));
  while(isalnum(*str))
  {
	if(nread < (IDLENGTH-1))
	  out[nread++] = *str++;
	else
	{
      seterror(PASSSTATE ERR_IDTOOLONG);
	  break;
	}
  }
  if(*str == ':')
  {
	if(nread < (IDLENGTH-1))
	  out[nread++] = *str++;
	else
	 seterror(PASSSTATE ERR_IDTOOLONG);
  } else {
	if(*str == '$')
	{
		if(nread < (IDLENGTH-1))
			out[nread++] = *str++;
		else
		seterror(PASSSTATE ERR_IDTOOLONG);
	}
	if(*str == '(')
	{
		if(nread < (IDLENGTH-1))
			out[nread++] = *str++;
		else
		seterror(PASSSTATE ERR_IDTOOLONG);
	}
  }
  out[nread] = 0;
  *len = nread;
}

/*
  grab a literal from the parse string.
  Params: dest - destination string
          src - source string
  Notes: strings are in quotes, numtype quotes the escape
*/
BSTATIC void mystrgrablit(STRINGARRAY *s, const char *src)
{
  stringchar *dest = STRINGOF(s);
  
  ASSERTDEBUG(basicassert(*src == '"',"mystrgrablit"));
  src++;
    
  while(*src)
  {
	if(*src == '"')
	{
	  if(src[1] == '"')
	  {
		*dest++ = *src;
	    src++;
	    src++;
	  }
	  else
		break;
	}
	else
     *dest++ = *src++;
  }
  
  s->len = dest - STRINGOF(s);
  STRINGOF(s)[s->len] = 0;
}

/*
  find where a source string literal ends
  Params: src - string to check (must point to quote)
          quote - character to use for quotation
  Returns: pointer to quote which ends string
  Notes: quotes escape quotes
*/
BSTATIC char *mystrend(const char *str, char quote)
{
  ASSERTDEBUG(basicassert(*str == quote,"mystrend"));
  str++;

  while(*str)
  {
    while(*str != quote)
	{
	  if(*str == '\n' || *str == 0)
		return 0;
	  str++;
	}
    if(str[1] == quote)
	  str += 2;
	else
	  break;
  }

  return (char *) (*str? str : 0);
}

BSTATIC STRINGARRAY *createstringarray(int len)
{
	return stringarrayresize(NULL, len);
}


BSTATIC STRINGARRAY *stringarrayresize(STRINGARRAY *str, int newlen)
{
	if (newlen > MAXSTRINGLEN)
		return NULL;
	str = (STRINGARRAY *)realloc(str,LENOFSTRINGARRAY(newlen));
	if (str != NULL) {
		str->len = newlen;
		str->beginstring[newlen] = 0;
	}
	return str;
}

/*
  concatenate two strings
  Params: str - firsts string
          cat - second string
  Returns: malloced string.
*/
BSTATIC STRINGARRAY *stringarrayconcat(STRINGARRAY *s1, STRINGARRAY *s2)
{
  STRINGARRAY *answer;

  answer = createstringarray(((long)s1->len) + ((long)s2->len));
  if (answer == NULL)
		return NULL;
  memmove(answer->beginstring, s1->beginstring, s1->len);
  memmove(&answer->beginstring[s1->len], s2->beginstring, s2->len);
  return answer;
}

BSTATIC int stringarraycmp(STRINGARRAY *s1, STRINGARRAY *s2)
{
  int lencmp = s1->len > s2->len ? s2->len : s1->len;
  int cmp = memcmp(s1->beginstring,s2->beginstring,lencmp);
  if (cmp != 0)
	return cmp;
  if (s1->len == s2->len)
	return 0;
  return (s1->len > s2->len ? 1 : -1);
}

BSTATIC int strstrarray(STRINGARRAY *str, STRINGARRAY *substr, int offset)
{
   int i=offset > 0 ? offset : 0, endof = str->len - substr->len, j;
   while (i<=endof)
   {
	  j=0;
	  while (j<substr->len) {
		  if (str->beginstring[i+j] != substr->beginstring[j])
			break;
		 j++;
	  }
	  if (j == substr->len)
		return i;
	  i++;
   }
   return -1;
}

BSTATIC STRINGARRAY *stringarraydup(STRINGARRAY *str)
{
	STRINGARRAY *s;
	s = createstringarray(str->len);
	if (s != NULL) 
		memmove(s->beginstring,str->beginstring,str->len);
	return s;
}

BSTATIC STRINGARRAY *stringarraysub(STRINGARRAY *str, STRINGARRAY *sub, int ind, int len)
{
	int newlen;
	
	if ((len < 0) || (len > sub->len)) len = sub->len;
	if (ind < 0) ind = 0;
	if (ind > str->len) ind = str->len;
	newlen = len + ind;
	if (newlen > str->len) {
		str = stringarrayresize(str,newlen);
		if (str == NULL)
			return str;
	}
	memmove(&str->beginstring[ind],sub->beginstring,len);
	return str;
}

BSTATIC STRINGARRAY *stringarraycharlen(const char *str, int len)
{
	STRINGARRAY *s = createstringarray(len);
	if (s != NULL)
		memmove(s->beginstring, str, len);
	return s;
}

BSTATIC STRINGARRAY *stringarraychar(const char *str)
{
	return stringarraycharlen(str, strlen(str));
}

BSTATIC STRINGARRAY *stringarrayduppart(STRINGARRAY *str, int offset, int len)
{
	STRINGARRAY *s;
	int end;

	if (offset < 0)
		offset = 0;
	if (offset > str->len)
		offset = str->len;
	end = offset + len;
	if (end < offset)
		end = offset;
	if (end > str->len)
		end = str->len;
	end = end - offset;
	s = createstringarray(end);
	if (s != NULL) {
		if (end > 0)
			memmove(s->beginstring, &str->beginstring[offset], end);
	}
	return s;
}

BSTATIC void strcatnprint(char *d, const char *s, int len)
{
	while (*d)
	{
		d++;
		len--;
	}
	while ((*s) && (len>0)) {
		if ((*s>=' ')&&(*s <= '~')) {
			*d++ = *s;
			len--;
		}
		s++;
	}
	*d = '\0';
}

int basic_debug_interface(ACCEPTSTATE int type, int num, char *var, int varlen, char *data, int datalen)
{
	char t[ITOASIZE > DTOASIZE ? ITOASIZE : DTOASIZE];
	*var = '\0';
	*data = '\0';
	switch (type) {
		case 0: switch (num) {
				case 0:	strcatnprint(var,"Cur Line #",varlen);
					    strcatnprint(data,myltoa(t,findlinenumber(PASSSTATE Scuroffset)),datalen);
						return 1;
				case 1: strcatnprint(var,"Last Err",varlen);
						strcatnprint(data,((Slasterrorflag > 0) && (Slasterrorflag < ERRCODES)) ? errormessages[Slasterrorflag] :  "NONE",datalen);
						return 1;
				case 2:	strcatnprint(var,"Err Line #",varlen);
					    strcatnprint(data,Serroroffset == ENDPROGRAMOFFSET ? "NONE" : myltoa(t,findlinenumber(PASSSTATE Serroroffset)),datalen);
						return 1;
				case 3:	strcatnprint(var,"On Err #",varlen);
					    strcatnprint(data,Sonerroroffset == ENDPROGRAMOFFSET ? "NONE" : myltoa(t,findlinenumber(PASSSTATE Sonerroroffset)),datalen);
						return 1;
				}
				return 0;
		case 1: if (num >= Sngosub) return 0;
				strcatnprint(var,"Return#",varlen);
				strcatnprint(var,myltoa(t,num+1),varlen);
			    strcatnprint(data,myltoa(t,findlinenumber(PASSSTATE Sgosubstack[num])),datalen);
				return 1;
		case 2: if (num >= Snvariables) return 0;
				{
					VARIABLE *v = &Svariables[num];
					strcatnprint(var,v->id,varlen);
					switch (v->type) {
						case STRID: strcatnprint(data,myltoa(t,v->d.sval == NULL ? 0  : v->d.sval->len),datalen);
									strcatnprint(data," length, string: ",datalen);
									strcatnprint(data,v->d.sval == NULL ? "NULL" : STRINGOF(v->d.sval),datalen);
									break;
						case FLTID:
#ifdef USEFLOATS
									strcatnprint(data,mydtoa(t,v->d.dval),datalen);
#else
									strcatnprint(data,myltoa(t,v->d.dval),datalen);
#endif								
					}
				}
				return 1;
		case 3: if (num >= Sndimvariables) return 0;
				{
					int i;
					DIMVAR *dv = &Sdimvariables[num];
					strcatnprint(var,dv->id,varlen);
					strcatnprint(var,")",varlen);
					switch (dv->type) {
						case STRID: strcatnprint(data,"DIM STRING ",datalen);
									break;
						case FLTID: strcatnprint(data,"DIM NUMS ",datalen);
									break;
					}
					for (i=0;i<dv->ndims;i++) {
						if (i) strcatnprint(data,"x",datalen);
						strcatnprint(data,myltoa(t,dv->dim[i]),datalen);
					}
				}
				return 1;
	}
	return 0;
}

#ifdef WEBENABLED
/*
  calculate the WEBCON() function.
*/
BSTATIC numtype webcon(VOIDACCEPTSTATE)
{
  match(PASSSTATE WEBCON);
  return (numtype) httpd_basic_connection();
}

/*
  calculate the WEBOUT() function.
*/
BSTATIC numtype webout(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  int conn_no, func;
  int answer = -1;

  matchoparen(PASSSTATE WEBOUT);
  conn_no = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  func = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);
  
  if (str) {
	    answer=httpd_relay_html(STRINGOF(str),str->len,conn_no);
		if (func < 0)
			answer=httpd_relay_html(NULL,0,conn_no);
		free(str);
  }
  return (numtype) answer;
}

/*
  calculate the WEBOUT() function.
*/
BSTATIC numtype webmail(VOIDACCEPTSTATE)
{
  STRINGARRAY *to, *cc, *from, *subject, *msg;
  int answer = -1;

  matchoparen(PASSSTATE WEBMAIL);
  to = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  cc = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  from = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  subject = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  msg = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);
  
  if ((to != NULL) && (cc != NULL) && (from != NULL) &&
      (subject != NULL) && (msg != NULL)) {
      answer = smtp_send(STRINGOF(to), STRINGOF(cc), STRINGOF(from),
	                     STRINGOF(subject), STRINGOF(msg), msg->len);
  }
  if (to) free(to);
  if (cc) free(cc);
  if (from) free(from);
  if (subject) free(subject);
  if (msg) free(msg);
  return (numtype) answer;
}

/*
  parse the WEBREQ$ token
*/
BSTATIC STRINGARRAY *webreqstring(VOIDACCEPTSTATE)
{
  int num;
  int nd;
  STRINGARRAY *answer = NULL;
  char *webreq;
  
  matchoparen(PASSSTATE WEBREQSTRING);
  num = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  nd = integer( expr(VOIDPASSSTATE) );
  match(PASSSTATE CPAREN);

  webreq=httpd_basic_webreq(num, nd);
  answer=stringarraychar(webreq != NULL ? webreq : "");
  return answer;
}

/*
  parse the WEBESC$ token
*/
BSTATIC STRINGARRAY *webescstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *var;
  STRINGARRAY *answer = NULL;
  int len;
  
  matchoparen(PASSSTATE WEBESCSTRING);
  var = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE CPAREN);

  if (var != NULL) {
	  len = httpd_escape_form(STRINGOF(var), NULL);
	  answer = createstringarray(len);
	  if (answer != NULL) {
			httpd_escape_form(STRINGOF(var), STRINGOF(answer));
	  } 
	  free(var);
  } else seterroroutofmemory(VOIDPASSSTATE);
  return answer;
}

/*
  parse the WEBFRM$ token
*/
BSTATIC STRINGARRAY *webfrmstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *pst, *var;
  STRINGARRAY *answer = NULL;
  int len;
  
  matchoparen(PASSSTATE WEBFRMSTRING);
  pst = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  var = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE CPAREN);

  if ((pst != NULL) && (var != NULL)) {
	  len = httpd_get_form(STRINGOF(pst), STRINGOF(var), NULL);
	  answer = createstringarray(len);
	  if (answer != NULL) {
			httpd_get_form(STRINGOF(pst), STRINGOF(var), STRINGOF(answer));
	  }
  } else seterroroutofmemory(VOIDPASSSTATE);
  if (pst != NULL) free(pst);
  if (var != NULL) free(var);
  return answer;
}

/*
  parse the WEBPST$ token
*/
BSTATIC STRINGARRAY *webpststring(VOIDACCEPTSTATE)
{
  STRINGARRAY *answer = NULL, *temp;
  int num, len;
  
  matchoparen(PASSSTATE WEBPSTSTRING);
  num = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  len = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);

  if ((Serrorflag == 0) && (len > 0)) {
	answer = createstringarray(len);
	if (answer != NULL) {
	  len = httpd_get_post(STRINGOF(answer), len, num);
	  temp = stringarrayresize(answer, len);
	  if (temp != NULL)
			answer = temp;
	} 
  }
  return answer;
}

BSTATIC numtype scwrite(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  int conn_no;
  int answer = -1;

  matchoparen(PASSSTATE SCWRITE);
  conn_no = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);
  
  if (str) {
	    answer=socket_write(conn_no,STRINGOF(str),str->len);
		free(str);
  }
  return (numtype) answer;
}

BSTATIC numtype scnewdt(VOIDACCEPTSTATE)
{
  int conn_no;

  matchoparen(PASSSTATE SCNEWDT);
  conn_no = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  
  return (numtype) socket_newdata(conn_no);
}

BSTATIC numtype sciscon(VOIDACCEPTSTATE)
{
  int conn_no;

  matchoparen(PASSSTATE SCISCON);
  conn_no = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  
  return (numtype) socket_isconnected(conn_no);
}

BSTATIC numtype scacept(VOIDACCEPTSTATE)
{
  int port,len;

  matchoparen(PASSSTATE SCACEPT);
  port = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  len = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  
  return (numtype) socket_accept(port,len);
}

BSTATIC numtype sccon(VOIDACCEPTSTATE)
{
  STRINGARRAY *str;
  int port, len;
  int answer = -1;

  matchoparen(PASSSTATE SCCON);
  str = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE COMMA);
  port = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  len = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  
  if (str) {
	    answer=socket_connect(STRINGOF(str),port,len);
		free(str);
  }
  return (numtype) answer;
}

BSTATIC numtype scclose(VOIDACCEPTSTATE)
{
  int conn_no;

  matchoparen(PASSSTATE SCCLOSE);
  conn_no = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  
  return (numtype) socket_close(conn_no);
}

BSTATIC STRINGARRAY *screadstring(VOIDACCEPTSTATE)
{
  STRINGARRAY *answer = NULL, *temp;
  int len, conn_no, endchar;
  
  matchoparen(PASSSTATE SCREADSTRING);
  conn_no = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  len = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE COMMA);
  endchar = integer ( expr(VOIDPASSSTATE ) );
  match(PASSSTATE CPAREN);

  if ((Serrorflag == 0) && (len > 0)) {
	answer = createstringarray(len);
	if (answer != NULL) {
	  len = socket_read(conn_no, STRINGOF(answer), len, endchar);
	  if (len < 0) {
			free(answer);
			answer=createstringarray(0);
	  } else {
		temp = stringarrayresize(answer, len);
		if (temp != NULL)
				answer = temp;
	  }
	}
  }
  return answer;
}
#endif

#ifdef INCFATFS
BSTATIC numtype bfclose(VOIDACCEPTSTATE)
{
  int filno, ercode;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FCLOSE);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	return -1;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	return -1;
  ercode = -f_close(fpt->fptr.fil);
  free(fpt->fptr.fil);
  fpt->fptr.fil = NULL;
  fpt->ftype = FATFSNONEPTR;
  return ercode;
}

BSTATIC numtype bfdel(VOIDACCEPTSTATE)
{
  int ercode = -1;
  STRINGARRAY *filename;

  matchoparen(PASSSTATE FDEL);
  filename = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE CPAREN);
  if (filename != NULL) {
	 ercode = -f_unlink(STRINGOF(filename));
	 free(filename);
  } else seterroroutofmemory(VOIDPASSSTATE);
  return ercode;
}

BSTATIC numtype bfdircl(VOIDACCEPTSTATE)
{
  int filno;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FDIRCL);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	return -1;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSDIRPTR) || (fpt->fptr.dir == NULL))
	return -1;
  free(fpt->fptr.dir);
  fpt->fptr.dir = NULL;
  fpt->ftype = FATFSNONEPTR;
  return 0;
}

BSTATIC numtype bfdirop(VOIDACCEPTSTATE)
{
  int filno, ercode = -1;
  STRINGARRAY *filename;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FDIROP);
  filename = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE CPAREN);
  if (filename == NULL)
  {
	seterroroutofmemory(VOIDPASSSTATE);
	goto endfdirop;
  } 
  if (Serrorflag != 0) goto endfdirop;
  for (filno=0;filno<FATPTRNUM;filno++) {
    fpt = &Sfatptrs[filno];
	if (fpt->fptr.dir == NULL) {
		DIR *dir = (DIR *)malloc(sizeof(DIR));
		if (dir == NULL) goto endfdirop;
		fpt->fptr.dir = dir;
		fpt->ftype = FATFSDIRPTR;
		break;
	}
  }
  if (filno == FATPTRNUM) goto endfdirop;
  ercode = -f_opendir(fpt->fptr.dir, STRINGOF(filename));
  if (ercode < 0) {
	free(fpt->fptr.dir);
	fpt->fptr.dir = NULL;
	fpt->ftype = FATFSNONEPTR;
  } else ercode = filno;
endfdirop:
  if (filename != NULL) free(filename);
  return ercode;  
}

BSTATIC numtype bfmkdir(VOIDACCEPTSTATE)
{
  int ercode = -1;
  STRINGARRAY *filename;

  matchoparen(PASSSTATE FMKDIR);
  filename = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE CPAREN);
  if (filename != NULL) {
	 ercode = -f_mkdir(STRINGOF(filename));
	 free(filename);
  } else seterroroutofmemory(VOIDPASSSTATE);
  return ercode;
}

BSTATIC numtype bfopen(VOIDACCEPTSTATE)
{
  int flagvar = 0, filno, ercode = -1;
  stringchar *s;
  STRINGARRAY *filename, *flags;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FOPEN);
  filename = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE COMMA);
  flags = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE CPAREN);
  if ((filename == NULL) || (flags == NULL))
  {
	seterroroutofmemory(VOIDPASSSTATE);
	goto endfopen;
  } 
  if (Serrorflag != 0) goto endfopen;
  s = STRINGOF(flags);
  while (*s) {
	switch (toupper(*s)) {
		case 'R': flagvar |= FA_READ; break;
		case 'W': flagvar |= FA_WRITE; break;
		case 'C': flagvar |= FA_CREATE_NEW; break;
		case 'A': flagvar |= FA_CREATE_ALWAYS; break;
		case 'O': flagvar |= FA_OPEN_ALWAYS; break;
	}
	s++;
  }
  for (filno=0;filno<FATPTRNUM;filno++) {
    fpt = &Sfatptrs[filno];
	if (fpt->fptr.fil == NULL) {
		FIL *fil = (FIL *)malloc(sizeof(FIL));
		if (fil == NULL) goto endfopen;
		fpt->fptr.fil = fil;
		fpt->ftype = FATFSFILPTR;
		break;
	}
  }
  if (filno == FATPTRNUM) goto endfopen;
  ercode = -f_open(fpt->fptr.fil, STRINGOF(filename), flagvar);
  if (ercode < 0) {
	free(fpt->fptr.fil);
	fpt->fptr.fil = NULL;
	fpt->ftype = FATFSNONEPTR;
  } else ercode = filno;
endfopen:
  if (filename != NULL) free(filename);
  if (flags != NULL) free(flags);
  return ercode;  
}

BSTATIC numtype bfseek(VOIDACCEPTSTATE)
{
  int filno, ercode;
  int offset, mode;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FSEEK);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  offset = integer( expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  mode = integer( expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (offset < 0) || (Serrorflag != 0))
	return -1;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	return -1;
  if (mode == 1) {
	 offset += fpt->fptr.fil->fptr;
  } else if (mode == 2) {
	 offset = fpt->fptr.fil->fsize - offset;	
  }
  if (offset < 0) offset = 0;
  ercode = -f_lseek(fpt->fptr.fil, offset);
  return ercode;
}

BSTATIC numtype bfeof(VOIDACCEPTSTATE)
{
  int filno;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FEOF);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	return -1;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	return -1;
  return (fpt->fptr.fil->fptr >= fpt->fptr.fil->fsize);
}

BSTATIC numtype bftell(VOIDACCEPTSTATE)
{
  int filno;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FTELL);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	return -1;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	return -1;
  return fpt->fptr.fil->fptr;
}

BSTATIC numtype bfsync(VOIDACCEPTSTATE)
{
  int filno, ercode, lwr=0;
  FATPTRTYPE *fpt;
  
  matchoparen(PASSSTATE FSYNC);
  filno = integer (expr(VOIDPASSSTATE));
  if (istokenmatch(PASSSTATE COMMA)) {
	lwr = integer( expr(VOIDPASSSTATE) );
  }
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	return -1;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	return -1;
  if (lwr)
		ercode = -f_truncate(fpt->fptr.fil);
  ercode = -f_sync(fpt->fptr.fil);
  return ercode;
}

BSTATIC numtype bfwrite(VOIDACCEPTSTATE)
{
  int filno, ercode = -1;
  FATPTRTYPE *fpt;
  STRINGARRAY *wrbuf;
  unsigned int wrlen;
  
  matchoparen(PASSSTATE FWRITE);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  wrbuf = stringexpr(VOIDPASSSTATE);
  match(PASSSTATE CPAREN);
  if (wrbuf == NULL) {
	seterroroutofmemory(VOIDPASSSTATE);
	goto bfwriteend;
  }
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	goto bfwriteend;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	goto bfwriteend;
  ercode = -f_write(fpt->fptr.fil, STRINGOF(wrbuf), wrbuf->len, &wrlen);
bfwriteend:
  if (wrbuf != NULL) free(wrbuf);
  return ercode;
}

BSTATIC STRINGARRAY *bfdirrdstring(VOIDACCEPTSTATE)
{
  int filno;
  FATPTRTYPE *fpt;
  STRINGARRAY *resp = NULL;
  FILINFO finfo;
  
  matchoparen(PASSSTATE FDIRRDSTRING);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	goto bfdirrdend;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSDIRPTR) || (fpt->fptr.dir == NULL))
	goto bfdirrdend;
  if ((f_readdir(fpt->fptr.dir,&finfo) == FR_OK) && (finfo.fname[0] != 0))
		resp = bfinfostring(&finfo);
bfdirrdend:
  if (resp == NULL)
	resp = createstringarray(0);
  return resp;
}

BSTATIC STRINGARRAY *bfreadstring(VOIDACCEPTSTATE)
{
  int filno, ercode = -1;
  unsigned int len, rdlen;
  FATPTRTYPE *fpt;
  STRINGARRAY *rdbuf = NULL;
  
  matchoparen(PASSSTATE FREADSTRING);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  len = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	goto bfreadend;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	goto bfreadend;
  if ((rdbuf = createstringarray(len)) == NULL) 
	goto bfreadend;
  ercode = -f_read(fpt->fptr.fil, STRINGOF(rdbuf), rdbuf->len, &rdlen);
  if (rdlen <= len) {
	STRINGARRAY *temp = stringarrayresize(rdbuf, rdlen);
	if (temp != NULL) rdbuf = temp;
  }
bfreadend:
  return rdbuf;
}

BSTATIC STRINGARRAY *bfreadlstring(VOIDACCEPTSTATE)
{
  int filno, ercode = -1;
  unsigned int len, rdlen, temp;
  FATPTRTYPE *fpt;
  STRINGARRAY *rdbuf = NULL, *tempstr;
  
  matchoparen(PASSSTATE FREADLSTRING);
  filno = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE COMMA);
  len = integer (expr(VOIDPASSSTATE));
  match(PASSSTATE CPAREN);
  if ((filno < 0) || (filno >= FATPTRNUM) || (Serrorflag != 0))
	goto bfreadend;
  fpt = &Sfatptrs[filno];
  if ((fpt->ftype != FATFSFILPTR) || (fpt->fptr.fil == NULL))
	goto bfreadend;
  if ((rdbuf = createstringarray(len)) == NULL) 
	goto bfreadend;
  rdlen = 0;
  while (rdlen < len) {
	  stringchar c;
	  ercode = -f_read(fpt->fptr.fil, &c, 1, &temp);
	  if ((ercode < 0) || (temp < 1))
		 break;
	  STRINGOF(rdbuf)[rdlen++] = c;
	  if (c == '\n')
		 break;
  }
  tempstr = stringarrayresize(rdbuf, rdlen);
  if (tempstr != NULL) rdbuf = tempstr;
bfreadend:
  return rdbuf;
}

#define FINFOLEN 50

BSTATIC STRINGARRAY *bfinfostring(FILINFO *finfo)
{
    char s[ITOASIZE];
	STRINGARRAY *str = createstringarray(FINFOLEN);
	if (str == NULL) return NULL;
	memset(STRINGOF(str),' ',FINFOLEN);
	mystrcpynonull(&STRINGOF(str)[0],&(finfo->fname[0]),12);
	STRINGOF(str)[14]=(finfo->fattrib & AM_DIR) ? 'D' : '-';
	STRINGOF(str)[15]=(finfo->fattrib & AM_RDO) ? 'R' : '-';
	STRINGOF(str)[16]=(finfo->fattrib & AM_HID) ? 'H' : '-';
	STRINGOF(str)[17]=(finfo->fattrib & AM_SYS) ? 'S' : '-';
	STRINGOF(str)[18]=(finfo->fattrib & AM_ARC) ? 'A' : '-';
	mystrcpynonull(&STRINGOF(str)[20],myltoapad(s,(finfo->fdate >> 9) + 1980,4),4);
	STRINGOF(str)[24]='/';
	mystrcpynonull(&STRINGOF(str)[25],myltoapad(s,(finfo->fdate >> 5) & 15,2),2);
	STRINGOF(str)[27]='/';
	mystrcpynonull(&STRINGOF(str)[28],myltoapad(s,finfo->fdate & 31,2),2);
	mystrcpynonull(&STRINGOF(str)[31],myltoapad(s,(finfo->ftime >> 11),2),2);
	STRINGOF(str)[33]=':';
	mystrcpynonull(&STRINGOF(str)[34],myltoapad(s,(finfo->ftime >> 5) & 63,2),2);
	mystrcpynonull(&STRINGOF(str)[37],myltoa(s,finfo->fsize),9);
	return str;
}

BSTATIC STRINGARRAY *bfstatstring(VOIDACCEPTSTATE)
{
  int ercode;
  STRINGARRAY *filename, *resp = NULL;
  FILINFO finfo;

  matchoparen(PASSSTATE FSTATSTRING);
  filename = stringexpr (VOIDPASSSTATE );
  match(PASSSTATE CPAREN);
  if (filename != NULL) {
	 ercode = f_stat(STRINGOF(filename), &finfo);
	 free(filename);
	 if (ercode == 0) 
		resp = bfinfostring(&finfo);
	 else
		resp = createstringarray(0);
  } else seterroroutofmemory(VOIDPASSSTATE);
  return resp;
}

#endif
