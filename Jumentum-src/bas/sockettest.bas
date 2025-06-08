REM GOTO waitf:
op=SCCON("128.174.4.87",80,4000)
waitb: cd=SCISCON(op)
REM  PRINT "op=",op," cd=",cd
  IF cd<1 THEN waitb:
  cd=SCWRITE(op,"GET /index.html HTTP/1.0"+CHR$(13)+CHR$(10)+CHR$(13)+CHR$(10))
  PRINT "write=",cd
  ln=0
  bytes=0
waitc: r$=SCREAD$(op,210,-10)
  PRINT r$;
  bytes=bytes+LEN(r$)
  ln=ln+1
  cd=SCISCON(op)
  IF cd=1 OR r$<>"" THEN waitc:
  cd=SCCLOSE(op)
  PRINT ""
  PRINT "lines=",ln
  PRINT "bytes=",bytes
END
waitf:
op=SCACEPT(4100,1000)
PRINT "opened socket=",op
FOR j=1 TO 10000
NEXT j
IF op<0 THEN waitf:
FOR i=1 TO 10000
IF SCISCON(op)<0 THEN q1: 
r$=SCREAD$(op,200,42)
IF LEFT$(r$,1)<>"q" THEN noq1:
q1: i=10000
noq1: IF r$="" THEN nt:
PRINT "len=",LEN(r$)
ll=SCWRITE(op,"echoing:"+r$+CHR$(13)+CHR$(10))
PRINT "scwrite=",ll
nt: NEXT i
ll=SCCLOSE(op)
PRINT "scclose=",ll
END
