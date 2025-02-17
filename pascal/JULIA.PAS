{From the book Programming in 3 Dimensions: }
{               3-D Graphics, Ray Tracing, and Animation. Watkins & Sharp. M&T Books}
{Port by Oscar Toledo G.}
{Last revision: Feb/24/1994}

PROGRAM uno;

PROCEDURE initgraphics;
BEGIN
 write(chr(27),'[G')
END;

PROCEDURE exitgraphics;
BEGIN
 readln;
 write(chr(27),'[T')
END;

PROCEDURE plot(xp,yp,r,g,b:integer);
BEGIN
 write(chr(27),'[',xp:0,';',yp:0,';',r:0,';',g:0,';',b:0,'P');
END;

PROCEDURE pixel(xp,yp,val:integer);
BEGIN
 plot(xp-1,yp-1,val,val,val);
END;

PROCEDURE Julia (rmin,rmax,imin,imax:real; n,it:integer);
VAR
 rg,ig,cr,ci,zr,zi:real;
 cnt:integer;
 zm:real;
 zrp,zip:real;
 i,j:integer;
BEGIN
 rg:=(rmax-rmin)/n;
 ig:=(imax-imin)/n;
 FOR i:=1 TO n DO
 BEGIN
  FOR j:=1 TO n DO
  BEGIN
   cr:=(j*rg)+rmin;
   ci:=(i*ig)+imin;
   cnt:=0;
   zr:=cr;
   zi:=ci;
   zm:=sqrt(sqr(zr)+sqr(zi));
   IF zm>2.0 THEN pixel(i,j,0)
   ELSE
   BEGIN
    WHILE (cnt<it) AND (zm<2.0) DO
    BEGIN
     zrp:=(sqr(zr)-sqr(zi)+cr);
     zip:=(2*zr*zi+ci);
     zr:=zrp;
     zi:=zip;
     zm:=sqrt(sqr(zr)+sqr(zi));
     cnt:=cnt+1;
    END;
    cnt:=256-cnt;
    IF cnt>127 THEN cnt:=-256+cnt;
    pixel(i,j,cnt);
   END;
  END;
 END;
END;

BEGIN
 initgraphics;
 julia(-0.7235,-0.7160,0.2175,0.2250,512,255);
{ readln;} {things were rendered directly to screen}
 exitgraphics;
END.
