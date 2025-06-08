dfile:
  filno=FDIROP("")
  n=0
rdd2: x$=FDIRRD$(filno)
  n=n+1
  IF (x$="") OR (n=10) THEN df2:
  PRINT n," ",x$
  GOTO rdd2:
df2:
  fil=FDIRCL(filno)
  PRINT "filename: ";
  INPUT fl$
  fil=FOPEN(fl$,"r")
re2: IF FEOF(fil) THEN ce2:
  t$=FREADL$(fil,80)
  PRINT t$;
  GOTO re2:
ce2: code=FCLOSE(fil)
  GOTO dfile:
