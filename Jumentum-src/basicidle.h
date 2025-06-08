#ifndef _BASICIDLE_H
#define _BASICIDLE_H

int idle (int alwayscheck);
#define BASICIDLE() idle(0)
/* BASICIDLE should be called only from inside a basic program */
#define GENERALIDLE() idle(1)
/* GENERALIDLE should be called everywhere else */

#endif  /* _BASICIDLE_H */