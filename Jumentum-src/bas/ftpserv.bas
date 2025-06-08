REM FTP SERVER
usr$="jumentum"
pas$="pass"
lf$=CHR$(13)+CHR$(10)
DIM mos$(12)="Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
waitf:
op=SCACEPT(21,2000)
IF INKEY=32 THEN endprg:
IF op<0 THEN waitf:
PRINT "Connection open"
ll=SCWRITE(op,"220 Jumentum FTP Server"+lf$)
ln=0
auth=0
cwd$="/"
nextln:
r$=SCREAD$(op,200,10)
IF INKEY=32 THEN endprg:
r$=TRIM$(r$)
IF SCISCON(op)=0 THEN endcon:
id=INSTR(r$," ",1)
cmd$=UPPER$(II$(id=0,r$,LEFT$(r$,id-1)))
arg$=II$(id=0,"",MID$(r$,id+1))
ln=ln+1
PRINT ln,": ",r$
IF cmd$="USER" THEN usercm:
IF cmd$="PASS" THEN passwd:
IF cmd$="QUIT" THEN quitc:
IF auth<>2 THEN nocmd:
IF cmd$="PORT" THEN portcd:
IF cmd$="LIST" THEN listc:
IF cmd$="TYPE" THEN typec:
IF cmd$="RETR" THEN retrc:
IF cmd$="STOR" THEN storc:
IF cmd$="DELE" THEN delec:
IF cmd$="RMD" OR cmd$="XRMD" THEN delec:
IF cmd$="MKD" OR cmd$="XMKD" THEN mkdc:
IF cmd$="CWD" THEN cwdc:
IF cmd$="PWD" OR cmd$="XPWD" THEN pwd:
nocmd: ll=SCWRITE(op,"500 Command not understood"+lf$)
GOTO nextln:
endcon: ll=SCCLOSE(op)
GOTO waitf:
endprg: PRINT "ending program"
END

quitc: PRINT "QUIT command"
ll=SCWRITE(op,"200 Goodbye"+lf$)
GOTO endcon:

typec: PRINT "TYPE command"
ll=SCWRITE(op,"200 Type set to "+arg$+lf$)
GOTO nextln:

delec: PRINT "DELE command"
IF cmd$="DELE" THEN delec:
f$=cwd$+"/"+arg$
GOSUB addp:
ll=FDEL(f$)
ll=SCWRITE(op,"200 Deleting file"+lf$)
GOTO nextln:

delec: PRINT "DELE/RMD command"
f$=cwd$+"/"+arg$
GOSUB addp:
ll=FDEL(f$)
ll=SCWRITE(op,"200 Deleting file"+lf$)
GOTO nextln:

mkdc: PRINT "MKD command"
f$=cwd$+"/"+arg$
GOSUB addp:
ll=FMKDIR(f$)
ll=SCWRITE(op,"200 Making directory"+lf$)
GOTO nextln:

usercm: PRINT "USER command: "+arg$
ll=SCWRITE(op,"331 Jumentum login OK send password"+lf$)
IF arg$<>usr$ THEN nextln:
auth=1
GOTO nextln:

passwd: PRINT "PASS command: "+arg$
IF arg$<>pas$ OR auth<>1 THEN badpas:
auth=2
ll=SCWRITE(op,"230 Welcome to Jumentum Web Server"+lf$)
GOTO nextln:
badpas: ll=SCWRITE(op,"230 Invalid password goodbye"+lf$)
GOTO endcon:

pwd: PRINT "PWD command"
ll=SCWRITE(op,"257 """+cwd$+""" is current directory."+lf$)
GOTO nextln:

addp: t1=INSTR(f$,"//",1)
IF t1=0 THEN addp2:
f$=LEFT$(f$,t1)+MID$(f$,t1+2)
GOTO addp:
addp2: t1=INSTR(f$,"/..",1)
IF t1=0 THEN addp3:
t2=1
REPEAT
t3=t2
t2=INSTR(f$,"/",t2)+1
UNTIL t2>=t1
f$=LEFT$(f$,t3-1)+MID$(f$,t1+3)
GOTO addp2:
addp3: RETURN  

cwdc: PRINT "CWD command "+arg$
f$=II$(LEFT$(arg$,1)="/",arg$,cwd$+"/"+arg$)
GOSUB addp:
cwd$=f$
ll=SCWRITE(op,"250 Cmd successful dir "+cwd$+lf$)
GOTO nextln:

portcd: PRINT "PORT command: "+arg$
GOSUB sepcms:
ipa$=STR$(n(1))+"."+STR$(n(2))+"."+STR$(n(3))+"."+STR$(n(4))
ipat=n(5)*256+n(6)
ll=SCWRITE(op,"200 Port Command Successful host "+ipa$+" port "+STR$(ipat)+lf$)
GOTO nextln:

sendf: PRINT "send"
GOSUB openot:
IF ou<0 THEN nocon:
ll=SCCLOSE(ou)
GOTO nextln:

listc: PRINT "LIST command"
ll=SCWRITE(op,"150 Sending Listing"+lf$)
GOSUB openot:
IF ou<0 THEN nocon:
fd=FDIROP(cwd$)
IF fd>=0 THEN list3:
ll=SCWRITE(op,"Could not open directory "+cwd$+lf$)
GOTO list4:
list3: rd$=FDIRRD$(fd)
IF rd$="" THEN listcl:
ms=VAL(MID$(rd$,26,2))
r$=II$(MID$(rd$,15,1)="D","d","-")+"rwxrwxr-x    1 jumentum jumentum"+RIGHT$("         "+TRIM$(MID$(rd$,38,9)),9)+" "+mos$(IIF(ms=0,1,ms))+" "+MID$(rd$,29,2)+"  "+MID$(rd$,32,5)+"   "+MID$(rd$,1,12)
ll=SCWRITE(ou,r$+lf$)
GOTO list3:
listcl: ll=FDIRCL(fd)
list4: ll=SCCLOSE(ou)
ll=SCWRITE(op,"226 Transfer Complete"+lf$)
GOTO nextln:

retrc: PRINT "RETR command"
f$=cwd$+"/"+arg$
GOSUB addp:
fd=FOPEN(f$,"r")
IF fd>=0 THEN retr2:
ll=SCWRITE(op,"500 Could not open file "+arg$+lf$)
GOTO nextln:
retr2: GOSUB openot:
IF ou>=0 THEN retr3:
ll=FCLOSE(fd)
GOTO nocon:
retr3: ll=SCWRITE(op,"150 Sending Binary Mode Data"+lf$)
retr4: IF FEOF(fd) THEN retrcl: 
rd$=FREAD$(fd,2000)
ll=SCWRITE(ou,rd$)
GOTO retr4:
retrcl: ll=FCLOSE(fd)
ll=SCCLOSE(ou)
ll=SCWRITE(op,"226 Transfer Complete"+lf$)
GOTO nextln:

storc: PRINT "STOR command"
f$=cwd$+"/"+arg$
GOSUB addp:
fd=FOPEN(f$,"wa")
IF fd>=0 THEN stor2:
ll=SCWRITE(op,"500 Could not open file "+arg$+lf$)
GOTO nextln:
stor2: GOSUB openot:
IF ou>=0 THEN stor3:
ll=FCLOSE(fd)
GOTO nocon:
stor3: ll=SCWRITE(op,"150 Receiving Binary Mode Data"+lf$)
stor4: rd$=SCREAD$(ou,2000,-1)
ll=FWRITE(fd,rd$)
IF SCISCON(ou) OR rd$<>"" THEN stor4: 
ll=SCCLOSE(ou)
ll=FCLOSE(fd)
ll=SCWRITE(op,"226 Transfer Complete"+lf$)
GOTO nextln:

nocon: ll=SCWRITE(op,"200 Could not connect"+lf$)
GOTO nextln:

sepcms: DIM n(10)
ns=0
sep1: id=INSTR(arg$,",",1)
ns=ns+1
n(ns)=VAL(II$(id=0,arg$,LEFT$(arg$,id-1)))
arg$=II$(id=0,"",MID$(arg$,id+1))
IF id<>0 THEN sep1:
RETURN

openot: 
ou=SCCON(ipa$,ipat,2000)
wt=5000
ope2: wt=wt-1
IF SCISCON(ou)=0 AND wt>0 THEN ope2:
ou=IIF(SCISCON(ou),ou,-1)
PRINT "ip=",ipa$," port=",ipat," con=",ou
RETURN
 
