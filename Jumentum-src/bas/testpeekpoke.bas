REM Test PEEK and POKE functionality...
REM
for I=1 TO 10
poke 25000+1073741824,I
print peek(25000+1073741824)
next I

REM Test bit functinality...
loop:
print "Insert value A:"
input A
print "Insert value B:"
input B
print A,"|",B,"=",(A|B)
print A,"&",B,"=",(A&B)
print A,"^",B,"=",(A^B)
print "~",A,"=",(~A)
print "0x",HEX$(A),"| 0x",hex$(B),"= 0x",hex$(A|B)
print "0x",HEX$(A),"& 0x",hex$(B),"= 0x",hex$(A&B)
print "0x",hex$(A),"^ 0x",hex$(B),"= 0x",hex$(A^B)
print "~ 0x",HEX$(A),"= 0x",HEX$(~A)
goto loop:
