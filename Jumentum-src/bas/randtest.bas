REM x=RND(<number>)
REM Returns a pseudorandom number between 0 and <number>-1 if <number> is
REM positive.  If <number> is negative, the value -<number> is used
REM to seed the random number generator. 

DIM h(33)
REPEAT
 PRINT "seed: ";
 INPUT sd
 PRINT "this take a while..."
 x=RND(-sd)
 a=0
 REPEAT
  a=a+1
  l=RND(32768)
  even=even+(1-(l & 1))
  odd=odd+(1 & l)
  h(1+(l/1000))=h(1+(l/1000))+1
 UNTIL a=10000
 FOR i=1 TO 33
  PRINT "bin ",i," # ",h(i)
 NEXT i
 PRINT "evens=",even
 PRINT "odds=",odd
UNTIL 0
