  code=FMKDIR("md")
  code=FDEL("temp2")
stat: PRINT "stat=",FSTAT$("test5.txt")
  filno=FDIROP("")
  n=0
rddir: x$=FDIRRD$(filno)
  n=n+1
  IF (x$="") OR (n=10) THEN start:
  PRINT n," ",x$
  GOTO rddir:
rd2: code=FDIRCL(filno)
  END
start: lf$=CHR$(10) 
  PRINT "opening file"
  filno=FOPEN("test5.txt","w")
  PRINT "write filno=",filno
  code=FSEEK(filno,0,2)
  code=FWRITE(filno,"testit"+lf$)
  PRINT "writing to file=",filno
  PRINT "new position=",FTELL(filno)
  code=FCLOSE(filno)
  PRINT "closing file=",code
  GOSUB readit:
  END

readit:  filno=FOPEN("test5.txt","r")
  PRINT "read filno=",filno
  n=0
  PRINT "feof=",FEOF(filno)
doit: IF FEOF(filno) THEN closef:
  text$=FREADL$(filno,80)
  n=n+1 
  GOTO doit:
closef: code=FCLOSE(filno)
  PRINT "closing file=",code," lines=",n
  RETURN


