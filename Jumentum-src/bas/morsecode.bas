begin: REM morse code
SETPIN 10,4
SETPIN 11,4
SETPIN 12,4
SETPIN 13,4
OUTD 13,0
PRINT "enter morse code: (- dash . dot)"
INPUT a$
FOR i=1 TO LEN(a$)
b$=MID$(a$,i,1)
IF b$<>"-" THEN dot:
OUTD 10,0
OUTD 11,0
TONE 12,250,523
OUTD 10,1
OUTD 11,1
GOTO short:
dot: IF b$<>"." THEN pause:
OUTD 10,0
OUTD 11,0
TONE 12,125,523
OUTD 10,1
OUTD 11,1
GOTO short:
pause: TONE -1,250,5
short: TONE -1,250,5
NEXT i
GOTO begin:
