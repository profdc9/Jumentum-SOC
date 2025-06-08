REM test i2c bus
REM the test is for MCP4275 DAC, 24LC256 EEPROM, and LIS302DL accelerometer

SETPIN 4,4
SETPIN 5,0
SETPIN 6,4
SETPIN 7,0

REM set SCL TO high
OUTD 6,1
REM set SDA TO high
OUTD 4,1

REM GOTO eep:

REM code TO test LIS302DL accelerometer

sladr=0x38

a$=SERI2C$(4,5,6,sladr,1,1,CHR$(0x0F))
PRINT "output=",ASCII(a$)
a$=SERI2C$(4,5,6,sladr,1,1,CHR$(0x20))
PRINT "reg 20=",HEX$(ASCII(a$))
a$=SERI2C$(4,5,6,sladr,1,1,CHR$(0x21))
PRINT "reg 21=",HEX$(ASCII(a$))
a$=SERI2C$(4,5,6,sladr,1,1,CHR$(0x22))
PRINT "reg 22=",HEX$(ASCII(a$))
a$=SERI2C$(4,5,6,sladr,1,0,CHR$(0x20)+CHR$(0x47))

CLRSCR
ct:
a$=SERI2C$(4,5,6,sladr,1,1,CHR$(0x29))
b$=SERI2C$(4,5,6,sladr,1,1,CHR$(0x2B))
c$=SERI2C$(4,5,6,sladr,1,1,CHR$(0x2D))
xac=ASCII(a$)
xac=IIF(xac>127,xac-256,xac)
xac$=RIGHT$("     "+STR$(xac),5)
yac=ASCII(b$)
yac=IIF(yac>127,yac-256,yac)
yac$=RIGHT$("     "+STR$(yac),5)
zac=ASCII(c$)
zac=IIF(zac>127,zac-256,zac)
zac$=RIGHT$("     "+STR$(zac),5)
LOCATE 20,0
PRINT "x=",xac$," y=",yac$," z=",zac$
FOR j=1 TO 1000
NEXT j
IF INKEY<>32 THEN ct:

END

REM program TO test read/writing TO I2C EEPROM 24LC256
REM this may NOT be working yet
eep:

REM write a byte TO a location in memory
sladr=0xA1
a$=SERI2C$(4,5,6,sladr,1,0,CHR$(2)+CHR$(7)+"t")
PRINT "results of write=",ASCII(a$)
REM send extra write command as per data sheet 
REM because no ACK is expected during EEPROM programming
a$=SERI2C$(4,5,6,sladr,-1,0,"")

FOR i=1 TO 10000
NEXT i

FOR i=1 TO 1
REM send command TO read memory location from EEPROM
a$=SERI2C$(4,5,6,sladr,1,2,CHR$(2)+CHR$(7))
PRINT "results of read=",ASCII(a$)," ",ASCII(a$,2)
REM dummy read
a$=SERI2C$(4,5,6,sladr,1,2,"")
PRINT "output 3=",ASCII(a$)," ",ASCII(a$,2)
FOR j=1 TO 10000
NEXT j
NEXT i
END

REM code TO test interface TO MCP4725 I2C DAC

dacr:
   sladr=0xC0
   FOR i=1 TO 4095 STEP 200
     REM send command TO set DAC output
     a$=SERI2C$(4,5,6,sladr,1,0,CHR$((i&0x0F00) LSR 8)+CHR$(i & 0xFF))
     FOR j=1 TO 10000
     NEXT j
     REM send command TO read DAC status
     b$=SERI2C$(4,5,6,sladr,1,3,"")
     PRINT CHR$(13)+CHR$(10)+"status bytes b1=",ASCII(b$,1)," ",ASCII(b$,2)," ",ASCII(b$,3)
   NEXT i
 
