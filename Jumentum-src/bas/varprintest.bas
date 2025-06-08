DIM bar(10,20)
DIM bib(2)
DIM b$(20,10)
DIM c$(56)
h$="asdfasf"
PRINT "h: ";
INPUT h$
gg=1
ggh=3
GOSUB doit:
END

doit: GOSUB doit2:
doit2: FOR i=1 TO 1000
PRINT "x ",i,": ";
INPUT a$
PRINT "a$=",a$
NEXT i
 
