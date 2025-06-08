n=100
DIM a$(n)
FOR j=1 TO 1000
FOR i=1 TO n
a$(i)=STRING$(RND(200),"X")
NEXT i
FOR i=1 TO n
PRINT i," ",LEN(a$(i))
NEXT i
NEXT j
