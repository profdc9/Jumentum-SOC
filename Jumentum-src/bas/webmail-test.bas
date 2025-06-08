msgto$="testemail@testemail.xxx"
cc$=""
from$="testemail@testemail.xxx"
subj$="it works!"
lf$=CHR$(13)+CHR$(10)
msg$=""
FOR i=1 TO 10
msg$=msg$+STR$(i)+" bottles of beer on the wall"+lf$
NEXT i
code=WEBMAIL(msgto$,cc$,from$,subj$,msg$)
PRINT "return code=",code
