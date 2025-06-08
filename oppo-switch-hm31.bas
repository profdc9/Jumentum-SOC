rem program to control oppo switch hm31
rem note: null modem required between HM31 and microcontroller
br$="<br>"+CHR$(13)+CHR$(10)
LF$ = CHR$(10)
CR$ = CHR$(13)
start: 
PRINT "welcome to OppoSelect"
 PRINT " "
 PRINT " " 
 PRINT "1   DirectTivo   "
 PRINT "2   Playstation 3 "
 PRINT " " 
 PRINT "Please Select Your Input:  ";
SERINIT 1,9600,8,1,0
REPEAT
  wc=WEBCON
  key = INKEY
UNTIL wc<>-1 OR key<>-1
 IF key<>-1 THEN consle:
 w$=WEBREQ$(wc,0)
 IF INSTR(w$,"/doit",1)=0 THEN prform:
 a$=WEBPST$(wc,200)
  code=WEBOUT(wc,0,"postdata="+WEBESC$(a$)+br$)
 code=WEBOUT(wc,0,"Input="+WEBESC$(WEBFRM$(a$,"Input"))+br$)
inpt = VAL(WEBFRM$(a$,"Input"))
GOSUB sndcmd:
   code=WEBOUT(wc,0,"Switch="+WEBESC$(WEBFRM$(a$,"Switch"))+br$)
prform: 
 code=WEBOUT(wc,0,"<form method=post action=""bas/doit"">")
 code=WEBOUT(wc,0,"Input: <input type=text name=""Input"">"+br$)
 code=WEBOUT(wc,0,"<input type=submit name=""Submit"" value=""Change Input""</form>"+br$)
endcon: code=WEBOUT(wc,-1,"")
GOTO start:
consle:
inpt = key - 48
GOSUB sndcmd:
GOTO start:

sndcmd:
   sout$ = SERINP$(1,50,13,10000)
   PRINT inpt
   IF inpt = 1 THEN input1:
   IF inpt = 2 THEN input2:
   PRINT "Error"
   RETURN

input1:
   code = SEROUT(1,"NU1" + CR$)
  sout$ = SERINP$(1,50,13,100000)
  PRINT "Input ", sout$ 
  RETURN

input2:
  code = SEROUT(1,"NU2" + CR$)
 sout$ = SERINP$(1,50,13,100000)
  PRINT "Input ", sout$ 
  RETURN

