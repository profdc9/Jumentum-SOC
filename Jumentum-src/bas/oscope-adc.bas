 REM oscilloscope
 el$=CHR$(13)+CHR$(10)
 br$="<br>"+el$
 width=256
 height=128
 SETPIN 28,1
 cn=0
REPEAT
 cn=cn+1
 PRINT "waiting for connection no ",cn
REPEAT
 wc=WEBCON
 IF INKEY=32 THEN endprg:
 UNTIL wc<>-1
 f$=WEBREQ$(wc,0)
 IF INSTR(f$,"x.bmp",1)=0 THEN dpage:
 DIM data(width)
 ADTRIG 3,1,500
 ADSTART 0x02,0x01,0xf0,4,1
 h$=STRING$(width*2," ")
 ct=1
 REPEAT
 REPEAT
 UNTIL ADRDBUF>=64
 MID$(h$,ct*2-1)=ADREAD$(64)
 ct=ct+64
 UNTIL ct>width
 ADSTOP
 FOR w=1 TO width
 data(w)=int((BSTR(MID$(h$,w*2-1,2),2) & 0x3FF)*height/1024)
 NEXT w
 code=WEBOUT(wc,0,"Connection: close"+el$+"Content-type: image/bmp"+el$+el$)
 h$=BIN$(0x31be4d42,4)+BIN$(0,4)+BIN$(0x3e0000,4)+BIN$(0x280000,4)
 h$=h$+BIN$(0,2)+BIN$(width,4)+BIN$(height,4)+BIN$(0x10001,4)
 h$=h$+BIN$(0,4)+BIN$(0x3180,4)+BIN$(0,4)+BIN$(0,4)
 h$=h$+BIN$(0,4)+BIN$(0,4)+BIN$(0,4)+BIN$(0x00ff00,4)
 code=WEBOUT(wc,0,h$)
 ws=width/8
 FOR r=0 TO height-1 STEP 64
 h$=STRING$(width/8,CHR$(0xAA))+STRING$(width*4-width/8,CHR$(0))
 h$=h$+h$
 mr=r+63
 lastp=-1
 FOR w=1 TO width
 lastp=IIF(lastp=-1,data(w),lastp)
 a=IIF(lastp<data(w),lastp,data(w))
 a=IIF(a>r,a,r)
 b=IIF(lastp<data(w),data(w),lastp)
 b=IIF(b<mr,b,mr)
 FOR p=a TO b
 v=1+ws*(p-r)+((w-1)/8)
 MID$(h$,v)=CHR$(ASCII(MID$(h$,v,1)) | (128 LSR ((w-1) & 7)))
 nextp: NEXT p
 lastp=data(w)
 nopix: NEXT w
 code=WEBOUT(wc,0,h$)
 NEXT r
 h$=""
endcon: code=WEBOUT(wc,-1,"")
 UNTIL 0
dpage: code=WEBOUT(wc,0,"<h1>Oscilloscope "+STR$(cn)+"</h1>"+br$)
 code=WEBOUT(wc,0,"<META HTTP-EQUIV=""Refresh"" CONTENT=2>")
 code=WEBOUT(wc,0,"<table border=0><tr><td>3.3V</td><td></td><td></td></tr><tr><td></td><td><img src=""/basr/x.bmp"" width="+STR$(width*2)+" height="+STR$(height*2)+"></td><td></td></tr>")
 code=WEBOUT(wc,0,"<tr><td>0V</td><td align=left>Begin</td><td>End</td></tr></table>")
 GOTO endcon:
endprg: PRINT "ending"
 END

