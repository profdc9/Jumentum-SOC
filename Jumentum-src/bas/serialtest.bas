SERINIT 1,38400,8,1,0
go: a$=SERINP$(1,100,13,1000000)
 b$="a$="""+TRIM$(a$)+""""+CHR$(13)+CHR$(10)
 code=SEROUT(1,b$)
 IF INKEY<>32 THEN go:
 