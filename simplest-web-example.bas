 SETPIN 28,1
start: wc=WEBCON
 IF wc=-1 THEN start:
 code=WEBOUT(wc,-1,"Value on ADC 1="+STR$(INADC(1)))
 GOTO start:

