{From the book Programming in 3 Dimensions: }
{               3-D Graphics, Ray Tracing, and Animation. Watkins & Sharp. M&T Books}
{Port by Oscar Toledo G.}
{Started Nov/1993}
{Last revision Oct/1995}
{Updated Feb/17/2025 for simpler interface with my transputer emulator}
PROGRAM modelado3d (input, output);

CONST
 maxnoise=16;
 ln10=2.3025851;
 oneoverln10=0.4342945;
 pi=3.1415927;
 piover180=1.7453293e-002;
 piunder180=5.7295780e+001;
 maxmaterial=20;
 maxtexture=10;
 smooth=1;
 checker=2;
 grit=3;
 marble=4;
 wood=5;
 sheetrock=6;
 particle=7;
 oceanwaves=8;
 poolwaves=9;
 waves=10;
 image=11;
 maxshapetype=8;
 ground=0;
 lamp=1;
 triangle=2;
 paral=3;
 circles=4;
 ring=5;
 sphere=6;
 cone=7;
 cylinder=8;
 maxlamp=10;
 maxtriangle=512;
 maxparal=50;
 maxcircles=50;
 maxring=50;
 maxsphere=256;
 maxcone=50;
 maxcylinder=50;

TYPE
 byte=char;
 word=integer;
 dword=integer;
 tda=array[0..2] of real;
 tdia=array[0..2] of integer;
 fda=array[0..3] of real;
 name=array[0..31] of char;
 materiallist=
  RECORD
   mtype:name;
   textur:name;
   ambrfl:tda;
   difrfl:tda;
   spcrfl:tda;
   gloss:real;
   trans:tda;
   index:real
  END;
 groundlist=
  RECORD
   mtlnum,texnum:integer
  END;
 lamplist=
  RECORD
   loc:tda;
   rad,radsqr:real;
   intens:tda
  END;
 trianglelist=
  RECORD
   loc,v1,v2,norm:tda;
   ndotloc:real;
   mtlnum,texnum:integer
  END;
 parallist=
  RECORD
   loc,v1,v2,norm:tda;
   ndotloc:real;
   mtlnum,texnum:integer
  END;
 circlelist=
  RECORD
   loc,v1,v2,norm:tda;
   ndotloc,radius:real;
   mtlnum,texnum:integer
  END;
 ringlist=
  RECORD
   loc,v1,v2,norm:tda;
   ndotloc,rad1,rad2:real;
   mtlnum,texnum:integer
  END;
 spherelist=
  RECORD
   loc:tda;
   rad,radsqr:real;
   mtlnum,texnum:integer
  END;
 quadlist=
  RECORD
   baseloc:tda;
   baserad,based:real;
   apexloc:tda;
   apexrad:real;
   u,v,w:tda;
   height,slope:real;
   mind,maxd:real;
   insnrm:boolean;
   mtlnum,texnum:integer
  END;
 conelist=quadlist;
 cylinderlist=quadlist;
 spheretype=RECORD center:tda; radsqr:real END;
 prismface=RECORD loc,v1,v2,norm:tda; ndotloc:real END;
 prismtype=ARRAY[0..6] OF prismface;
 tetraface=RECORD loc,v1,v2,norm:tda; ndotloc:real END;
 tetratype=ARRAY[0..5] OF tetraface;

VAR
 asp,small:real;

 xres,yres:integer;
 maxxres,maxyres:word;
 maxx,maxy:word;
 oldrand:real;
 scanxres:word;
 scanyres:word;
 xaspdivfoclen:real;
 yaspdivfoclen:real;
 xo,yo:word;
 centerx,centery:word;
 viewvec:tda;

 numberofframes:integer;

 loclwgt:tda;
 reflwgt:tda;
 tranwgt:tda;
 minwgt:tda;
 maxwgt:tda;
 maxdepth:integer;

 lampreflects:boolean;
 lamprefl:tda;
 disteffect:real;

 onesvec,preccor:tda;

 focallength:real;
 obspos:tda;
 obsrotate:real;
 obstilt:real;
 viewdir:tda;
 viewu,viewv:tda;

 horcol:tda;
 zencol:tda;
 clouds:boolean;
 skyexists:boolean;

 tile1:tda;
 tile2:tda;
 tile:real;

 ocwaveampl:real;
 ocwavephase:real;

 powaveampl:real;
 powavephase:real;
 powavexpos:real;
 powaveypos:real;

 waveampl:real;
 wavephase:real;
 wavexpos:real;
 waveypos:real;
 wavezpos:real;

 groundexists:boolean;

 tetraext1,tetraext2,tetraext3,tetraext4:tda;

 bounsphrtest,bounprismtest,bountetratest:boolean;

 buffer:array[0..80] of char;
 buf1,buf2:array[0..9] of char;

 mtlname:name;

 mtlcount:integer;

 gu,gv:fda;

 delta:tda;

 t1,t2:real;

 intrpoint,temp:tda;
 a,b,c,d:real;
 disc,sroot:real;
 t:real;

 bounsphere:spheretype;
 bounprism:prismtype;
 bountetra:tetratype;

 initialray:boolean;

 hitbounsphere:boolean;

 dot:real;
 pos1,pos2:real;
 hitbounprism:boolean;

 hitbountetra:boolean;

 singleframe:boolean;

 framenum,lastframenum:integer;

 matl:array[0..maxmaterial] of materiallist;
 objcnt:array[0..maxshapetype] of integer;
 gnd:groundlist;
 lmp:array[0..maxlamp] of lamplist;
 tri:array[0..maxtriangle] of trianglelist;
 para:array[0..maxparal] of parallist;
 cir:array[0..maxcircles] of circlelist;
 rng:array[0..maxring] of ringlist;
 sphr:array[0..maxsphere] of spherelist;
 con:array[0..maxcone] of conelist;
 cyl:array[0..maxcylinder] of cylinderlist;

 noisematrix:array[0..15,0..15,0..15] of integer;

PROCEDURE leelinea;
VAR contador:INTEGER; caract:CHAR;
BEGIN
 contador:=0;
{ write(chr(27),'[!E');}
 REPEAT
  read(caract);
  IF (caract<>chr(13)) AND
     (caract<>chr(26)) THEN
  BEGIN
   buffer[contador]:=caract;
   contador:=contador+1
  END
 UNTIL (caract=chr(13)) OR (caract=chr(26));
 buffer[contador]:=chr(13);
END;

PROCEDURE leereal(var valor:REAL; var contador:INTEGER);
VAR fraccion,divisor,menos:REAL;
BEGIN
 IF buffer[contador]='-' THEN
 BEGIN
  menos:=-1.0;
  contador:=contador+1
 END
 ELSE menos:=1.0;
 valor:=0.0;
 WHILE (buffer[contador]>='0') AND (buffer[contador]<='9') DO
 BEGIN
  valor:=valor*10.0+(ord(buffer[contador])-48);
  contador:=contador+1
 END;
 IF buffer[contador]='.' THEN
 BEGIN
  contador:=contador+1;
  fraccion:=0.0;
  divisor:=1.0;
  WHILE (buffer[contador]>='0') AND (buffer[contador]<='9') DO
  BEGIN
   fraccion:=fraccion*10.0+(ord(buffer[contador])-48);
   divisor:=divisor*10.0;
   contador:=contador+1
  END;
  valor:=valor+fraccion/divisor;
 END;
 valor:=valor*menos;
END;

PROCEDURE loadline;
VAR contador,contador2:integer;
BEGIN
 FOR contador:=0 TO 9 DO buf1[contador]:=' ';
 leelinea;
 contador:=0;
 contador2:=0;
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 WHILE (buffer[contador]<>' ') AND
       (buffer[contador]<>chr(13)) AND
       (buffer[contador]<>chr(26)) DO
 BEGIN
  IF contador2<10 THEN buf1[contador2]:=buffer[contador];
  contador2:=contador2+1;
  contador:=contador+1
 END;
END;

PROCEDURE loadtda(var a:tda);
VAR contador:integer;
BEGIN
 leelinea;
 contador:=0;
 WHILE buffer[contador]<>'=' DO contador:=contador+1;
 contador:=contador+1;
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 leereal(a[0],contador);
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 leereal(a[1],contador);
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 leereal(a[2],contador);
END;

PROCEDURE loadreal(var a:real);
VAR contador:integer;
BEGIN
 leelinea;
 contador:=0;
 WHILE buffer[contador]<>'=' DO contador:=contador+1;
 contador:=contador+1;
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 leereal(a,contador);
END;

PROCEDURE loadinteger(var a:integer);
VAR contador:integer;
BEGIN
 leelinea;
 contador:=0;
 WHILE buffer[contador]<>'=' DO contador:=contador+1;
 contador:=contador+1;
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 a:=0;
 WHILE (buffer[contador]>='0') AND (buffer[contador]<='9') DO
 BEGIN
  a:=a*10+(ord(buffer[contador])-48);
  contador:=contador+1
 END;
END;

PROCEDURE loadtext(var a:name);
VAR contador,contador2:integer;
BEGIN
 FOR contador:=0 TO 31 DO a[contador]:=' ';
 leelinea;
 contador:=0;
 WHILE buffer[contador]<>'=' DO contador:=contador+1;
 contador:=contador+1;
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 contador2:=0;
 WHILE (buffer[contador]<>' ') AND (buffer[contador]<>chr(13)) DO
 BEGIN
  a[contador2]:=buffer[contador];
  contador:=contador+1;
  contador2:=contador2+1
 END;
END;

FUNCTION loadboolean:boolean;
VAR contador:integer;
BEGIN
 leelinea;
 contador:=0;
 WHILE buffer[contador]<>'=' DO contador:=contador+1;
 contador:=contador+1;
 WHILE buffer[contador]=' ' DO contador:=contador+1;
 IF (buffer[contador]='F') OR
    (buffer[contador]='f') THEN loadboolean:=false
 ELSE loadboolean:=true;
END;

PROCEDURE getmatlnum(var mat:name; var matnum:integer);
VAR i:integer;
BEGIN
 FOR i:=1 TO mtlcount DO
  IF matl[i].mtype=mat THEN matnum:=i;
END;

PROCEDURE gettexnum(var tex:name; var texnum:integer);
VAR oscar:ARRAY[0..9] OF char; contador:integer;
BEGIN
 FOR contador:=0 TO 9 DO oscar[contador]:=tex[contador];
      IF oscar='SUAVE     ' THEN texnum:=smooth
 ELSE IF oscar='ALTERNADO ' THEN texnum:=checker
 ELSE IF oscar='ARENA     ' THEN texnum:=grit
 ELSE IF oscar='MARMOL    ' THEN texnum:=marble
 ELSE IF oscar='MADERA    ' THEN texnum:=wood
 ELSE IF oscar='PIEDRA    ' THEN texnum:=sheetrock
 ELSE IF oscar='PARTICULA ' THEN texnum:=particle
 ELSE IF oscar='ONDASMAR  ' THEN texnum:=oceanwaves
 ELSE IF oscar='ONDASAGUA ' THEN texnum:=poolwaves
 ELSE IF oscar='ONDAS     ' THEN texnum:=waves
 ELSE IF oscar='IMAGEN    ' THEN texnum:=image
END;

PROCEDURE loadrtheader;
VAR t,cnt:INTEGER;
BEGIN
 xres:=0;
 yres:=0;
 scanxres:=0;
 scanyres:=0;
 numberofframes:=0;
 cnt:=0;
 REPEAT
  loadline;
  IF buf1='RESOLUCION' THEN
  BEGIN
   loadinteger(scanxres);
   loadinteger(scanyres);
   cnt:=cnt+1
  END
  ELSE IF buf1='CUADROS   ' THEN
  BEGIN
   loadinteger(numberofframes);
   cnt:=cnt+1
  END;
 UNTIL cnt=2;
END;

FUNCTION min(a,b:real):real;
BEGIN
 IF a<b THEN min:=a
 ELSE min:=b
END;

FUNCTION max(a,b:real):real;
BEGIN
 IF a>b THEN max:=a
 ELSE max:=b
END;

FUNCTION min3(a,b,c:real):real;
BEGIN
 min3:=min(min(a,b),c)
END;

FUNCTION max3(a,b,c:real):real;
VAR t:real;
BEGIN
 max3:=max(max(a,b),c);
END;

FUNCTION min4(a,b,c,d:real):real;
BEGIN
 min4:=min(min(a,b),min(c,d))
END;

FUNCTION max4(a,b,c,d:real):real;
BEGIN
 max4:=max(max(a,b),max(c,d))
END;

PROCEDURE vec(r,s,t:real; var a:tda);
BEGIN
 a[0]:=r;
 a[1]:=s;
 a[2]:=t
END;

PROCEDURE unvec(var a:tda; var r,s,t:real);
BEGIN
 r:=a[0];
 s:=a[1];
 t:=a[2]
END;

FUNCTION vecdot(var a,b:tda):real;
BEGIN
 vecdot:=a[0]*b[0]+a[1]*b[1]+a[2]*b[2]
END;

PROCEDURE veccross(var a,b,c:tda);
BEGIN
 c[0]:=a[1]*b[2]-a[2]*b[1];
 c[1]:=a[2]*b[0]-a[0]*b[2];
 c[2]:=a[0]*b[1]-a[1]*b[0]
END;

FUNCTION veclen(var a:tda):real;
BEGIN
 veclen:=sqrt(sqr(a[0])+sqr(a[1])+sqr(a[2]))
END;

PROCEDURE vecnormalize(var a:tda);
VAR dist:real;
BEGIN
 dist:=1.0/veclen(a);
 a[0]:=a[0]*dist;
 a[1]:=a[1]*dist;
 a[2]:=a[2]*dist
END;

PROCEDURE vecsub(var a,b,c:tda);
BEGIN
 c[0]:=a[0]-b[0];
 c[1]:=a[1]-b[1];
 c[2]:=a[2]-b[2]
END;

PROCEDURE vecadd(var a,b,c:tda);
BEGIN
 c[0]:=a[0]+b[0];
 c[1]:=a[1]+b[1];
 c[2]:=a[2]+b[2]
END;

PROCEDURE vecadd3(var a,b,c,d:tda);
BEGIN
 d[0]:=a[0]+b[0]+c[0];
 d[1]:=a[1]+b[1]+c[1];
 d[2]:=a[2]+b[2]+c[2]
END;

PROCEDURE veclincomb(r:real; var a:tda; s:real; var b,c:tda);
BEGIN
 c[0]:=r*a[0]+s*b[0];
 c[1]:=r*a[1]+s*b[1];
 c[2]:=r*a[2]+s*b[2]
END;

PROCEDURE vecscalmult(r:real; var a,b:tda);
BEGIN
 b[0]:=r*a[0];
 b[1]:=r*a[1];
 b[2]:=r*a[2]
END;

PROCEDURE vecsmulti(r:real; var a:tdia; var b:tda);
BEGIN
 b[0]:=r*a[0];
 b[1]:=r*a[1];
 b[2]:=r*a[2]
END;

PROCEDURE vecsmint(r:real; var a:tda; var b:tdia);
BEGIN
 b[0]:=round(r*a[0]);
 b[1]:=round(r*a[1]);
 b[2]:=round(r*a[2])
END;

PROCEDURE vecaddscalmult(r:real; var a,b,c:tda);
BEGIN
 c[0]:=r*a[0]+b[0];
 c[1]:=r*a[1]+b[1];
 c[2]:=r*a[2]+b[2]
END;

PROCEDURE vecelemmult(r:real; var a,b,c:tda);
BEGIN
 c[0]:=r*a[0]*b[0];
 c[1]:=r*a[1]*b[1];
 c[2]:=r*a[2]*b[2];
END;

PROCEDURE vecmin(var a,b,c:tda);
BEGIN
 IF a[0]<b[0] THEN c[0]:=a[0] ELSE c[0]:=b[0];
 IF a[1]<b[1] THEN c[1]:=a[1] ELSE c[1]:=b[1];
 IF a[2]<b[2] THEN c[2]:=a[2] ELSE c[2]:=b[2];
END;

PROCEDURE vecmax(var a,b,c:tda);
BEGIN
 IF a[0]>b[0] THEN c[0]:=a[0] ELSE c[0]:=b[0];
 IF a[1]>b[1] THEN c[1]:=a[1] ELSE c[1]:=b[1];
 IF a[2]>b[2] THEN c[2]:=a[2] ELSE c[2]:=b[2];
END;

PROCEDURE vecnull(var a:tda);
BEGIN
 a[0]:=0.0;
 a[1]:=0.0;
 a[2]:=0.0
END;

PROCEDURE clearquadratic(var list:quadlist);
BEGIN
 vecnull(list.baseloc);
 list.baserad:=0.0;
 list.based:=0.0;
 vecnull(list.apexloc);
 list.apexrad:=0.0;
 vecnull(list.u);
 vecnull(list.v);
 vecnull(list.w);
 list.height:=0.0;
 list.slope:=0.0;
 list.mind:=0.0;
 list.maxd:=0.0;
 list.insnrm:=false;
 list.mtlnum:=0;
 list.texnum:=0
END;

PROCEDURE clearstr(var str:name);
VAR i:integer;
BEGIN
 FOR i:=0 TO 31 DO
  str[i]:=' ';
END;

PROCEDURE clearmemory;
VAR i:integer;
BEGIN
 vecnull(loclwgt);
 vecnull(reflwgt);
 vecnull(tranwgt);
 vecnull(minwgt);
 vecnull(maxwgt);
 maxdepth:=0;
 lampreflects:=false;
 vecnull(lamprefl);
 disteffect:=0.0;
 focallength:=0.0;
 vecnull(obspos);
 obsrotate:=0.0;
 obstilt:=0.0;
 vecnull(viewdir);
 vecnull(viewu);
 vecnull(viewv);
 vecnull(horcol);
 vecnull(zencol);
 clouds:=false;
 skyexists:=false;
 vecnull(tile1);
 vecnull(tile2);
 tile:=0.0;
 ocwaveampl:=0.0;
 ocwavephase:=0.0;
 powaveampl:=0.0;
 powavephase:=0.0;
 powavexpos:=0.0;
 powaveypos:=0.0;
 waveampl:=0.0;
 wavephase:=0.0;
 wavexpos:=0.0;
 waveypos:=0.0;
 wavezpos:=0.0;
 groundexists:=false;
 vecnull(tetraext1);
 vecnull(tetraext2);
 vecnull(tetraext3);
 vecnull(tetraext4);
 bounsphrtest:=false;
 bounprismtest:=false;
 bountetratest:=false;
 FOR i:=0 TO maxmaterial DO
 BEGIN
  clearstr(matl[i].mtype);
  clearstr(matl[i].textur);
  vecnull(matl[i].ambrfl);
  vecnull(matl[i].difrfl);
  vecnull(matl[i].spcrfl);
  matl[i].gloss:=0.0;
  vecnull(matl[i].trans);
  matl[i].index:=0.0
 END;
 FOR i:=0 TO maxshapetype DO
  objcnt[i]:=0;
 gnd.mtlnum:=0;
 gnd.texnum:=0;
 FOR i:=0 TO maxlamp DO
 BEGIN
  vecnull(lmp[i].loc);
  lmp[i].rad:=0.0;
  lmp[i].radsqr:=0.0;
  vecnull(lmp[i].intens);
 END;
 FOR i:=0 TO maxtriangle DO
 BEGIN
  vecnull(tri[i].loc);
  vecnull(tri[i].v1);
  vecnull(tri[i].v2);
  vecnull(tri[i].norm);
  tri[i].ndotloc:=0.0;
  tri[i].mtlnum:=0;
  tri[i].texnum:=0;
 END;
 FOR i:=0 TO maxparal DO
 BEGIN
  vecnull(para[i].loc);
  vecnull(para[i].v1);
  vecnull(para[i].v2);
  vecnull(para[i].norm);
  para[i].ndotloc:=0.0;
  para[i].mtlnum:=0;
  para[i].texnum:=0;
 END;
 FOR i:=0 TO maxcircles DO
 BEGIN
  vecnull(cir[i].loc);
  vecnull(cir[i].v1);
  vecnull(cir[i].v2);
  vecnull(cir[i].norm);
  cir[i].ndotloc:=0.0;
  cir[i].radius:=0.0;
  cir[i].mtlnum:=0;
  cir[i].texnum:=0;
 END;
 FOR i:=0 TO maxring DO
 BEGIN
  vecnull(rng[i].loc);
  vecnull(rng[i].v1);
  vecnull(rng[i].v2);
  vecnull(rng[i].norm);
  rng[i].ndotloc:=0.0;
  rng[i].rad1:=0.0;
  rng[i].rad2:=0.0;
  rng[i].mtlnum:=0;
  rng[i].texnum:=0;
 END;
 FOR i:=0 TO maxsphere DO
 BEGIN
  vecnull(sphr[i].loc);
  sphr[i].rad:=0.0;
  sphr[i].radsqr:=0.0;
  sphr[i].mtlnum:=0;
  sphr[i].texnum:=0;
 END;
 FOR i:=0 TO maxcone DO
  clearquadratic(con[i]);
 FOR i:=0 TO maxcylinder DO
  clearquadratic(cyl[i]);
END;

FUNCTION radians(angle:real):real;
BEGIN
 radians:=angle*piover180
END;

PROCEDURE getviewdir(angl,tilt:real; var view,u,v:tda);
VAR phi,theta,x,y,z:real;
BEGIN
 phi:=radians(angl);
 theta:=radians(tilt);
 x:=cos(theta)*sin(phi);
 y:=cos(theta)*cos(phi);
 z:=-sin(theta);
 vec(x,y,z,view);
 x:=cos(phi);
 y:=-sin(phi);
 z:=0.0;
 vec(x,y,z,u);
 x:=sin(theta)*sin(phi);
 y:=sin(theta)*cos(phi);
 z:=cos(theta);
 vec(x,y,z,v);
END;

PROCEDURE orientquad(var list:quadlist);
VAR temp:tda; rtmp:real;
BEGIN
 IF (list.baserad<0.0) OR (list.apexrad<0.0) THEN
 BEGIN
  list.baserad:=abs(list.baserad);
  list.apexrad:=abs(list.apexrad);
  list.insnrm:=true;
 END
 ELSE list.insnrm:=false;
 vecsub(list.apexloc,list.baseloc,list.w);
 list.height:=veclen(list.w);
 vecnormalize(list.w);
 list.slope:=(list.apexrad-list.baserad)/list.height;
 list.based:=-vecdot(list.baseloc,list.w);
 vec(0.0,0.0,1.0,temp);
 rtmp:=abs(abs(vecdot(temp,list.w))-1.0);
 IF rtmp<small THEN vec(0.0,1.0,0.0,temp);
 veccross(list.w,temp,list.u);
 veccross(list.u,list.w,list.v);
 vecnormalize(list.u);
 vecnormalize(list.v);
 list.mind:=vecdot(list.w,list.baseloc);
 list.maxd:=vecdot(list.w,list.apexloc);
 IF list.maxd<list.mind THEN
 BEGIN
  rtmp:=list.maxd;
  list.maxd:=list.mind;
  list.mind:=rtmp
 END;
END;

PROCEDURE getdataforframe;
VAR
 radial,hgt:real;
 shapeloc,temploc:tda;
 vec1,vec2,vec3:tda;
 pt1,pt2,pt3,pt4:tda;
 mtlnumber,texnumber:integer;
BEGIN
 mtlcount:=0;
 REPEAT
  loadline;
  IF buf1='AMBIENTE  ' THEN
  BEGIN
   loadtda(loclwgt);
   loadtda(reflwgt);
   loadtda(tranwgt);
   loadtda(minwgt);
   loadtda(maxwgt);
   loadinteger(maxdepth)
  END
  ELSE IF buf1='LAMPARAS  ' THEN
  BEGIN
   lampreflects:=loadboolean;
   loadtda(lamprefl);
   loadreal(disteffect)
  END
  ELSE IF buf1='OBSERVADOR' THEN
  BEGIN
   loadreal(focallength);
   loadtda(obspos);
   loadreal(obsrotate);
   loadreal(obstilt);
   getviewdir(obsrotate,obstilt,viewdir,viewu,viewv);
  END
  ELSE IF buf1='CIELO     ' THEN
  BEGIN
   loadtda(horcol);
   loadtda(zencol);
   clouds:=loadboolean;
   skyexists:=true;
  END
  ELSE IF buf1='MATERIAL  ' THEN
  BEGIN
   mtlcount:=mtlcount+1;
   WITH matl[mtlcount] DO
   BEGIN
    loadtext(mtype);
    loadtext(textur);
    loadtda(ambrfl);
    loadtda(difrfl);
    loadtda(spcrfl);
    loadreal(gloss);
    loadtda(trans);
    loadreal(index)
   END;
   IF matl[mtlcount].textur='ALTERNADO                       ' THEN
   BEGIN
    loadtda(tile1);
    loadtda(tile2);
    loadreal(tile)
   END
   ELSE IF matl[mtlcount].textur='ONDASMAR                        ' THEN
   BEGIN
    loadreal(ocwaveampl);
    loadreal(ocwavephase)
   END
   ELSE IF matl[mtlcount].textur='ONDASAGUA                       ' THEN
   BEGIN
    loadreal(powaveampl);
    loadreal(powavephase);
    loadreal(powavexpos);
    loadreal(powaveypos)
   END
   ELSE IF matl[mtlcount].textur='ONDAS                           ' THEN
   BEGIN
    loadreal(waveampl);
    loadreal(wavephase);
    loadreal(wavexpos);
    loadreal(waveypos);
    loadreal(wavezpos)
   END;
  END
  ELSE IF buf1='PISO      ' THEN
  BEGIN
   groundexists:=true;
   objcnt[ground]:=objcnt[ground]+1;
   loadtext(mtlname);
   getmatlnum(mtlname,gnd.mtlnum);
   gettexnum(matl[gnd.mtlnum].textur,gnd.texnum);
  END
  ELSE IF buf1='LAMPARA   ' THEN
  BEGIN
   objcnt[lamp]:=objcnt[lamp]+1;
   loadtda(lmp[objcnt[lamp]].loc);
   loadreal(lmp[objcnt[lamp]].rad);
   lmp[objcnt[lamp]].radsqr:=sqr(lmp[objcnt[lamp]].rad);
   loadtda(lmp[objcnt[lamp]].intens);
  END
  ELSE IF buf1='TRIANGULO ' THEN
  BEGIN
   objcnt[triangle]:=objcnt[triangle]+1;
   WITH tri[objcnt[triangle]] DO
   BEGIN
    loadtda(loc);
    loadtda(v1);
    loadtda(v2);
    veccross(v1,v2,norm);
    vecnormalize(norm);
    ndotloc:=vecdot(norm,loc);
    loadtext(mtlname);
    getmatlnum(mtlname,mtlnum);
    gettexnum(matl[mtlnum].textur,texnum)
   END
  END
  ELSE IF buf1='PARALELOGR' THEN
  BEGIN
   objcnt[paral]:=objcnt[paral]+1;
   WITH para[objcnt[paral]] DO
   BEGIN
    loadtda(loc);
    loadtda(v1);
    loadtda(v2);
    veccross(v1,v2,norm);
    vecnormalize(norm);
    ndotloc:=vecdot(norm,loc);
    loadtext(mtlname);
    getmatlnum(mtlname,mtlnum);
    gettexnum(matl[mtlnum].textur,texnum)
   END
  END
  ELSE IF buf1='CIRCULO   ' THEN
  BEGIN
   objcnt[circles]:=objcnt[circles]+1;
   WITH cir[objcnt[circles]] DO
   BEGIN
    loadtda(loc);
    loadtda(v1);
    vecnormalize(v1);
    loadtda(v2);
    vecnormalize(v2);
    veccross(v1,v2,norm);
    vecnormalize(norm);
    ndotloc:=vecdot(norm,loc);
    loadreal(radius);
    loadtext(mtlname);
    getmatlnum(mtlname,mtlnum);
    gettexnum(matl[mtlnum].textur,texnum)
   END
  END
  ELSE IF buf1='ANILLO    ' THEN
  BEGIN
   objcnt[ring]:=objcnt[ring]+1;
   WITH rng[objcnt[ring]] DO
   BEGIN
    loadtda(loc);
    loadtda(v1);
    vecnormalize(v1);
    loadtda(v2);
    vecnormalize(v2);
    veccross(v1,v2,norm);
    vecnormalize(norm);
    ndotloc:=vecdot(norm,loc);
    loadreal(rad1);
    loadreal(rad2);
    loadtext(mtlname);
    getmatlnum(mtlname,mtlnum);
    gettexnum(matl[mtlnum].textur,texnum)
   END
  END
  ELSE IF buf1='ESFERA    ' THEN
  BEGIN
   objcnt[sphere]:=objcnt[sphere]+1;
   WITH sphr[objcnt[sphere]] DO
   BEGIN
    loadtda(loc);
    loadreal(rad);
    radsqr:=sqr(rad);
    loadtext(mtlname);
    getmatlnum(mtlname,mtlnum);
    gettexnum(matl[mtlnum].textur,texnum)
   END
  END
  ELSE IF buf1='CONO      ' THEN
  BEGIN
   objcnt[cone]:=objcnt[cone]+1;
   WITH con[objcnt[cone]] DO
   BEGIN
    loadtda(baseloc);
    loadreal(baserad);
    loadtda(apexloc);
    loadreal(apexrad);
    orientquad(con[objcnt[cone]]);
    loadtext(mtlname);
    getmatlnum(mtlname,mtlnum);
    gettexnum(matl[mtlnum].textur,texnum)
   END
  END
  ELSE IF buf1='CILINDRO  ' THEN
  BEGIN
   objcnt[cylinder]:=objcnt[cylinder]+1;
   WITH cyl[objcnt[cylinder]] DO
   BEGIN
    loadtda(baseloc);
    loadreal(baserad);
    loadtda(apexloc);
    apexrad:=baserad;
    orientquad(cyl[objcnt[cylinder]]);
    loadtext(mtlname);
    getmatlnum(mtlname,mtlnum);
    gettexnum(matl[mtlnum].textur,texnum)
   END
  END
  ELSE IF buf1='CAJA      ' THEN
  BEGIN
   loadtda(shapeloc);
   loadtda(vec1);
   loadtda(vec2);
   loadtda(vec3);
   loadtext(mtlname);
   getmatlnum(mtlname,mtlnumber);
   gettexnum(matl[mtlnumber].textur,texnumber);
   objcnt[paral]:=objcnt[paral]+1;
   para[objcnt[paral]].loc:=shapeloc;
   para[objcnt[paral]].v1:=vec1;
   para[objcnt[paral]].v2:=vec3;
   veccross(para[objcnt[paral]].v1,
            para[objcnt[paral]].v2,
            para[objcnt[paral]].norm);
   vecnormalize(para[objcnt[paral]].norm);
   para[objcnt[paral]].ndotloc:=
    vecdot(para[objcnt[paral]].norm,
           para[objcnt[paral]].loc);
   para[objcnt[paral]].mtlnum:=mtlnumber;
   para[objcnt[paral]].texnum:=texnumber;
   objcnt[paral]:=objcnt[paral]+1;
   para[objcnt[paral]].loc:=shapeloc;
   para[objcnt[paral]].v1:=vec3;
   para[objcnt[paral]].v2:=vec1;
   veccross(para[objcnt[paral]].v1,
            para[objcnt[paral]].v2,
            para[objcnt[paral]].norm);
   vecnormalize(para[objcnt[paral]].norm);
   vec(0.0,vec2[1],0.0,temploc);
   vecadd(temploc,para[objcnt[paral]].loc,para[objcnt[paral]].loc);
   para[objcnt[paral]].ndotloc:=
    vecdot(para[objcnt[paral]].norm,
           para[objcnt[paral]].loc);
   para[objcnt[paral]].mtlnum:=mtlnumber;
   para[objcnt[paral]].texnum:=texnumber;
   objcnt[paral]:=objcnt[paral]+1;
   para[objcnt[paral]].loc:=shapeloc;
   para[objcnt[paral]].v1:=vec3;
   para[objcnt[paral]].v2:=vec2;
   veccross(para[objcnt[paral]].v1,
            para[objcnt[paral]].v2,
            para[objcnt[paral]].norm);
   vecnormalize(para[objcnt[paral]].norm);
   para[objcnt[paral]].ndotloc:=
    vecdot(para[objcnt[paral]].norm,
           para[objcnt[paral]].loc);
   para[objcnt[paral]].mtlnum:=mtlnumber;
   para[objcnt[paral]].texnum:=texnumber;
   objcnt[paral]:=objcnt[paral]+1;
   para[objcnt[paral]].loc:=shapeloc;
   para[objcnt[paral]].v1:=vec2;
   para[objcnt[paral]].v2:=vec3;
   veccross(para[objcnt[paral]].v1,
            para[objcnt[paral]].v2,
            para[objcnt[paral]].norm);
   vecnormalize(para[objcnt[paral]].norm);
   vec(vec1[0],0.0,0.0,temploc);
   vecadd(temploc,para[objcnt[paral]].loc,
                  para[objcnt[paral]].loc);
   para[objcnt[paral]].ndotloc:=vecdot(para[objcnt[paral]].norm,
                                       para[objcnt[paral]].loc);
   para[objcnt[paral]].mtlnum:=mtlnumber;
   para[objcnt[paral]].texnum:=texnumber;
   objcnt[paral]:=objcnt[paral]+1;
   para[objcnt[paral]].loc:=shapeloc;
   para[objcnt[paral]].v1:=vec2;
   para[objcnt[paral]].v2:=vec1;
   veccross(para[objcnt[paral]].v1,
            para[objcnt[paral]].v2,
            para[objcnt[paral]].norm);
   vecnormalize(para[objcnt[paral]].norm);
   para[objcnt[paral]].ndotloc:=vecdot(para[objcnt[paral]].norm,
                                       para[objcnt[paral]].loc);
   para[objcnt[paral]].mtlnum:=mtlnumber;
   para[objcnt[paral]].texnum:=texnumber;
   objcnt[paral]:=objcnt[paral]+1;
   para[objcnt[paral]].loc:=shapeloc;
   para[objcnt[paral]].v1:=vec1;
   para[objcnt[paral]].v2:=vec2;
   veccross(para[objcnt[paral]].v1,
            para[objcnt[paral]].v2,
            para[objcnt[paral]].norm);
   vecnormalize(para[objcnt[paral]].norm);
   vec(0.0,0.0,vec3[2],temploc);
   vecadd(temploc,para[objcnt[paral]].loc,
                  para[objcnt[paral]].loc);
   para[objcnt[paral]].ndotloc:=vecdot(para[objcnt[paral]].norm,
                                       para[objcnt[paral]].loc);
   para[objcnt[paral]].mtlnum:=mtlnumber;
   para[objcnt[paral]].texnum:=texnumber;
  END
  ELSE IF buf1='PIRAMIDE  ' THEN
  BEGIN
   loadtda(shapeloc);
   loadtda(vec1);
   loadtda(vec2);
   loadreal(hgt);
   loadtext(mtlname);
   getmatlnum(mtlname,mtlnumber);
   gettexnum(matl[mtlnumber].textur,texnumber);
   objcnt[paral]:=objcnt[paral]+1;
   para[objcnt[paral]].loc:=shapeloc;
   para[objcnt[paral]].v1:=vec2;
   para[objcnt[paral]].v2:=vec1;
   veccross(para[objcnt[paral]].v1,
            para[objcnt[paral]].v2,
            para[objcnt[paral]].norm);
   vecnormalize(para[objcnt[paral]].norm);
   para[objcnt[paral]].ndotloc:=
    vecdot(para[objcnt[paral]].norm,
           para[objcnt[paral]].loc);
   para[objcnt[paral]].mtlnum:=mtlnumber;
   para[objcnt[paral]].texnum:=texnumber;
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc:=shapeloc;
   tri[objcnt[triangle]].v1:=vec1;
   vec(0.5*vec1[0],0.5*vec2[1],hgt,tri[objcnt[triangle]].v2);
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,
           tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc[0]:=shapeloc[0]+vec1[0];
   tri[objcnt[triangle]].loc[1]:=shapeloc[1]+vec2[1];
   tri[objcnt[triangle]].loc[2]:=shapeloc[2];
   vecscalmult(-1.0,vec1,tri[objcnt[triangle]].v1);
   vec(-0.5*vec1[0],-0.5*vec2[1],hgt,tri[objcnt[triangle]].v2);
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,
           tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc[0]:=shapeloc[0]+vec1[0];
   tri[objcnt[triangle]].loc[1]:=shapeloc[1]+vec2[1];
   tri[objcnt[triangle]].loc[2]:=shapeloc[2];
   vec(-0.5*vec1[0],-0.5*vec2[1],hgt,tri[objcnt[triangle]].v1);
   vecscalmult(-1.0,vec2,tri[objcnt[triangle]].v2);
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,
           tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc:=shapeloc;
   vec(0.5*vec1[0],0.5*vec2[1],hgt,tri[objcnt[triangle]].v1);
   tri[objcnt[triangle]].v2:=vec2;
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,
           tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
  END
  ELSE IF buf1='TETRA     ' THEN
  BEGIN
   loadtda(shapeloc);
   loadreal(hgt);
   loadreal(radial);
   loadtext(mtlname);
   getmatlnum(mtlname,mtlnumber);
   gettexnum(matl[mtlnumber].textur,texnumber);
   vec(shapeloc[0],shapeloc[1],shapeloc[2]+hgt,pt1);
   vec(shapeloc[0],shapeloc[1]+radial,shapeloc[2],pt2);
   vec(shapeloc[0]-radial*0.707,shapeloc[1]-radial*0.707,shapeloc[2],pt3);
   vec(shapeloc[0]+radial*0.707,shapeloc[1]-radial*0.707,shapeloc[2],pt4);
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc:=pt3;
   vecsub(pt1,pt3,tri[objcnt[triangle]].v1);
   vecsub(pt2,pt3,tri[objcnt[triangle]].v2);
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc:=pt2;
   vecsub(pt1,pt2,tri[objcnt[triangle]].v1);
   vecsub(pt4,pt2,tri[objcnt[triangle]].v2);
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc:=pt4;
   vecsub(pt1,pt4,tri[objcnt[triangle]].v1);
   vecsub(pt3,pt4,tri[objcnt[triangle]].v2);
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
   objcnt[triangle]:=objcnt[triangle]+1;
   tri[objcnt[triangle]].loc:=pt3;
   vecsub(pt2,pt3,tri[objcnt[triangle]].v1);
   vecsub(pt4,pt3,tri[objcnt[triangle]].v2);
   veccross(tri[objcnt[triangle]].v1,
            tri[objcnt[triangle]].v2,
            tri[objcnt[triangle]].norm);
   vecnormalize(tri[objcnt[triangle]].norm);
   tri[objcnt[triangle]].ndotloc:=
    vecdot(tri[objcnt[triangle]].norm,tri[objcnt[triangle]].loc);
   tri[objcnt[triangle]].mtlnum:=mtlnumber;
   tri[objcnt[triangle]].texnum:=texnumber;
  END
  ELSE IF buf1='EXTENSIONT' THEN
  BEGIN
   loadtda(tetraext1);
   loadtda(tetraext2);
   loadtda(tetraext3);
   loadtda(tetraext4);
   bountetratest:=true;
  END
  ELSE IF buf1='PRUEBAESFE' THEN
   bounsphrtest:=true
  ELSE IF buf1='PRUEBAPRIS' THEN
   bounprismtest:=true
 UNTIL buf1='FINCUADRO ';
END;

PROCEDURE calcdirofreflray(var dir,srfnrm,reflray:tda);
VAR tmp:real;
BEGIN
 tmp:=-2.0*vecdot(dir,srfnrm);
 vecaddscalmult(tmp,srfnrm,dir,reflray);
END;

PROCEDURE vecnegate(var a:tda);
BEGIN
 a[0]:=-a[0];
 a[1]:=-a[1];
 a[2]:=-a[2]
END;

PROCEDURE calcdiroftranray(var dir,srfnrm:tda; mtl:integer; var tranray:tda);
VAR ni,ndotv,nmult,lsint,ndott,nt:real; cosv,sint,temp:tda;
BEGIN
 ni:=1.0;
 vecnegate(dir);
 ndotv:=vecdot(srfnrm,dir);
 nt:=matl[mtl].index;
 IF ndotv>0.0 THEN nmult:=ni/nt
 ELSE nmult:=nt;
 vecscalmult(ndotv,srfnrm,cosv);
 vecsub(cosv,dir,temp);
 vecscalmult(nmult,temp,sint);
 lsint:=vecdot(sint,sint);
 IF lsint>=1.0 THEN vecnull(tranray)
 ELSE
 BEGIN
  ndott:=sqrt(1.0-lsint);
  IF ndotv<0.0 THEN ndott:=-ndott;
  vecscalmult(ndott,srfnrm,temp);
  vecsub(sint,temp,tranray)
 END;
END;

PROCEDURE quadsrfnrm(var intrpt,srfnrm:tda; var list:quadlist);
VAR t:real; projpt:tda;
BEGIN
 t:=-(vecdot(intrpt,list.w)+list.based);
 vecaddscalmult(t,list.w,intrpt,projpt);
 vecsub(projpt,list.baseloc,srfnrm);
 vecnormalize(srfnrm);
 t:=-list.slope;
 vecaddscalmult(t,list.w,srfnrm,srfnrm);
 vecnormalize(srfnrm);
 IF list.insnrm THEN vecnegate(srfnrm);
END;

PROCEDURE getsrfnrm(shp,obj:integer; var intrpt,srfnrm:tda);
BEGIN
 CASE shp OF
 ground: vec(0.0,0.0,1.0,srfnrm);
 lamp:
  BEGIN
   vecsub(intrpt,lmp[obj].loc,srfnrm);
   vecnormalize(srfnrm)
  END;
 triangle: srfnrm:=tri[obj].norm;
 paral: srfnrm:=para[obj].norm;
 circles: srfnrm:=cir[obj].norm;
 ring: srfnrm:=rng[obj].norm;
 sphere:
  BEGIN
   vecsub(intrpt,sphr[obj].loc,srfnrm);
   vecnormalize(srfnrm)
  END;
 cone: quadsrfnrm(intrpt, srfnrm, con[obj]);
 cylinder: quadsrfnrm(intrpt, srfnrm, cyl[obj])
 END;
END;

PROCEDURE getintrpt(var pt,dir:tda; dist:real; var intrpt:tda);
BEGIN
 vecaddscalmult(dist,dir,pt,intrpt)
END;

FUNCTION evencrossings(sides:integer):boolean;
VAR i,j:integer; crossings:integer;
BEGIN
 crossings:=0;
 FOR i:=0 TO sides-1 DO
 BEGIN
  j:=(i+1) MOD sides;
  IF ((gv[i]<0) AND (gv[j]>=0)) OR
     ((gv[j]<0) AND (gv[i]>=0)) THEN
  BEGIN
   IF (gu[i]>=0) AND (gu[j]>=0) THEN crossings:=crossings+1
   ELSE
   BEGIN
    IF (gu[i]>=0) OR (gu[j]>=0) THEN
    BEGIN
     IF (gu[i]-gv[i]*(gu[j]-gu[i])/
         (gv[j]-gv[i]))>0 THEN crossings:=crossings+1
    END;
   END;
  END;
 END;
 IF (crossings MOD 2)=0 THEN evencrossings:=true
 ELSE evencrossings:=false;
END;

PROCEDURE setuptriangle(p1,p2:integer; obj:integer);
BEGIN
 gu[0]:=-delta[p1];
 gv[0]:=-delta[p2];
 gu[1]:=tri[obj].v1[p1]-delta[p1];
 gv[1]:=tri[obj].v1[p2]-delta[p2];
 gu[2]:=tri[obj].v2[p1]-delta[p1];
 gv[2]:=tri[obj].v2[p2]-delta[p2];
END;

PROCEDURE setupparallelogram(p1,p2:integer; obj:integer);
BEGIN
 gu[0]:=-delta[p1];
 gv[0]:=-delta[p2];
 gu[1]:=para[obj].v1[p1]-delta[p1];
 gv[1]:=para[obj].v1[p2]-delta[p2];
 gu[2]:=para[obj].v2[p1]+para[obj].v1[p1]-delta[p1];
 gv[2]:=para[obj].v2[p2]+para[obj].v1[p2]-delta[p2];
 gu[3]:=para[obj].v2[p1]-delta[p1];
 gv[3]:=para[obj].v2[p2]-delta[p2];
END;

FUNCTION qintercheck:real;
BEGIN
 IF (t1<=small) AND (t2<=small) THEN qintercheck:=-1.0
 ELSE
 BEGIN
  if (t1>t2) then
  BEGIN
   if (t2<=small) then t2:=t1
  END
  ELSE IF (t1>small) THEN t2:=t1;
  qintercheck:=t2;
 END;
END;

FUNCTION quadshapes(var list:quadlist; var pt,dir:tda):real;
LABEL 99;
VAR newpnt,newdir:tda; sqrslope:real;
BEGIN
 vecsub(pt,list.baseloc,temp);
 newpnt[0]:=vecdot(temp,list.u);
 newpnt[1]:=vecdot(temp,list.v);
 newpnt[2]:=vecdot(temp,list.w);
 newdir[0]:=vecdot(dir,list.u);
 newdir[1]:=vecdot(dir,list.v);
 newdir[2]:=vecdot(dir,list.w);
 sqrslope:=sqr(list.slope);
 a:=sqr(newdir[0])+sqr(newdir[1])-sqr(newdir[2])*sqrslope;
 b:=2.0*(newpnt[0]*newdir[0]+newpnt[1]*newdir[1]-
         newdir[2]*(newpnt[2]*sqrslope-list.baserad*list.slope));
 c:=sqr(newpnt[0])+sqr(newpnt[1])-sqr(newpnt[2]*list.slope+list.baserad);
 IF a=0 THEN
 BEGIN
  if b=0 THEN
  BEGIN
   quadshapes:=-1.0;
   GOTO 99
  END
  ELSE
  BEGIN
   t2:=-c/b;
   IF t2<small THEN
   BEGIN
    quadshapes:=-1.0;
    GOTO 99
   END
   ELSE t1:=-1.0;
  END;
 END
 ELSE
 BEGIN
  disc:=sqr(b)-4.0*a*c;
  IF disc<0.0 THEN
  BEGIN
   quadshapes:=-1.0;
   GOTO 99
  END
  ELSE
  BEGIN
   sroot:=sqrt(disc);
   t:=1.0/(a*2.0);
   t1:=(-b-sroot)*t;
   t2:=(-b+sroot)*t;
   IF (t1<0.0) AND (t2<0.0) THEN
   BEGIN
    quadshapes:=-1.0;
    GOTO 99
   END
   ELSE
   BEGIN
    IF t1>t2 THEN
    BEGIN
     t:=t1;
     t1:=t2;
     t2:=t;
    END;
   END;
  END;
 END;
 IF t1>small THEN
 BEGIN
  getintrpt(pt,dir,t1,intrpoint);
  d:=vecdot(list.w,intrpoint);
  IF (d>=list.mind) AND (d<=list.maxd) THEN
  BEGIN
   quadshapes:=t1;
   GOTO 99
  END;
 END;
 IF t2>small THEN
 BEGIN
  getintrpt(pt,dir,t2,intrpoint);
  d:=vecdot(list.w,intrpoint);
  IF (d>=list.mind) AND (d<=list.maxd) THEN
  BEGIN
   quadshapes:=t2;
   GOTO 99
  END;
 END;
 quadshapes:=-1.0;
 99:
END;

FUNCTION intersect(var pt,dir:tda; shp,obj:integer):real;
VAR rad,dot:real; pos1,pos2:real;
BEGIN
 CASE shp OF
 ground:
 BEGIN
  IF dir[2]=0.0 THEN intersect:=-1.0
  ELSE
  BEGIN
   t:=-pt[2]/dir[2];
   IF t>small THEN intersect:=t
   else intersect:=-1.0;
  END;
 END;
 lamp:
 BEGIN
  vecsub(lmp[obj].loc,pt,temp);
  b:=vecdot(dir, temp)*(-2.0);
  c:=vecdot(temp, temp)-lmp[obj].radsqr;
  disc:=sqr(b)-4.0*c;
  if disc<=0.0 then intersect:=-1.0
  else
  begin
   sroot:=sqrt(disc);
   t1:=(-b-sroot)*0.5;
   t2:=(-b+sroot)*0.5;
   intersect:=qintercheck;
  END;
 END;
 triangle:
 BEGIN
  dot:=vecdot(tri[obj].norm,dir);
  IF abs(dot)<small THEN intersect:=-1.0
  ELSE
  BEGIN
   pos1:=tri[obj].ndotloc;
   pos2:=vecdot(tri[obj].norm,pt);
   t:=(pos1-pos2)/dot;
   getintrpt(pt,dir,t,intrpoint);
   vecsub(intrpoint,tri[obj].loc,delta);
   IF (abs(tri[obj].norm[0])>abs(tri[obj].norm[1])) AND
      (abs(tri[obj].norm[0])>abs(tri[obj].norm[2])) THEN
    setuptriangle(1,2,obj)
   ELSE
   BEGIN
    IF abs(tri[obj].norm[1])>=abs(tri[obj].norm[2]) THEN
     setuptriangle(0,2,obj)
    ELSE
     setuptriangle(0,1,obj);
   END;
   IF evencrossings(3) THEN intersect:=-1.0
   ELSE intersect:=t;
  END;
 END;
 paral:
 BEGIN
  dot:=vecdot(para[obj].norm,dir);
  IF abs(dot)<small THEN intersect:=-1.0
  ELSE
  BEGIN
   pos1:=para[obj].ndotloc;
   pos2:=vecdot(para[obj].norm,pt);
   t:=(pos1-pos2)/dot;
   getintrpt(pt,dir,t,intrpoint);
   vecsub(intrpoint,para[obj].loc,delta);
   IF (abs(para[obj].norm[0])>abs(para[obj].norm[1])) AND
      (abs(para[obj].norm[0])>abs(para[obj].norm[2])) THEN
    setupparallelogram(1,2,obj)
   ELSE
   BEGIN
    IF abs(para[obj].norm[1])>=abs(para[obj].norm[2]) THEN
     setupparallelogram(0,2,obj)
    ELSE
     setupparallelogram(0,1,obj);
   END;
   IF evencrossings(4) THEN intersect:=-1.0
   ELSE intersect:=t;
  END;
 END;
 circles:
 BEGIN
  dot:=vecdot(cir[obj].norm,dir);
  IF abs(dot)<small THEN intersect:=-1.0
  ELSE
  BEGIN
   pos1:=cir[obj].ndotloc;
   pos2:=vecdot(cir[obj].norm,pt);
   t:=(pos1-pos2)/dot;
   getintrpt(pt,dir,t,intrpoint);
   vecsub(intrpoint,cir[obj].loc,delta);
   rad:=sqrt(vecdot(delta,delta));
   IF rad>cir[obj].radius THEN
    intersect:=-1.0
   ELSE
    intersect:=t;
  END;
 END;
 ring:
 BEGIN
  dot:=vecdot(rng[obj].norm,dir);
  IF abs(dot)<small THEN intersect:=-1.0
  ELSE
  BEGIN
   pos1:=rng[obj].ndotloc;
   pos2:=vecdot(rng[obj].norm,pt);
   t:=(pos1-pos2)/dot;
   getintrpt(pt,dir,t,intrpoint);
   vecsub(intrpoint,rng[obj].loc,delta);
   rad:=sqrt(vecdot(delta,delta));
   IF (rad<rng[obj].rad1) OR (rad>rng[obj].rad2) THEN
    intersect:=-1.0
   ELSE
    intersect:=t;
  END;
 END;
 sphere:
 BEGIN
  vecsub(sphr[obj].loc,pt,temp);
  b:=vecdot(dir,temp)*(-2.0);
  c:=vecdot(temp,temp)-sphr[obj].radsqr;
  disc:=sqr(b)-4.0*c;
  IF disc<=0.0 THEN intersect:=-1.0
  ELSE
  BEGIN
   sroot:=sqrt(disc);
   t1:=(-b-sroot)*0.5;
   t2:=(-b+sroot)*0.5;
   intersect:=qintercheck;
  END;
 END;
 cone: intersect:=quadshapes(con[obj],pt,dir);
 cylinder: intersect:=quadshapes(cyl[obj],pt,dir)
 END;
END;

PROCEDURE getinitialdir(i,j:real; var dir:tda);
VAR x,y:real; eyetopixvec:tda;
BEGIN
 x:=(i-centerx)*xaspdivfoclen;
 y:=(centery-j)*yaspdivfoclen;
 veclincomb(x,viewu,y,viewv,eyetopixvec);
 vecadd(viewvec,eyetopixvec,dir);
 vecnormalize(dir)
END;

PROCEDURE quadbound(var list:quadlist; var minimum,maximum:tda);
VAR qmin,qmax:tda; maxrad:real;
BEGIN
 vecmin(list.baseloc,list.apexloc,qmin);
 vecmax(list.baseloc,list.apexloc,qmax);
 maxrad:=max(list.baserad,list.apexrad);
 minimum[0]:=qmin[0]-maxrad;
 minimum[1]:=qmin[1]-maxrad;
 minimum[2]:=qmin[2]-maxrad;
 maximum[0]:=qmax[0]+maxrad;
 maximum[1]:=qmax[1]+maxrad;
 maximum[2]:=qmax[2]+maxrad;
END;

PROCEDURE bounboxes(shp,obj:integer; var minimum,maximum:tda);
VAR p2,p3,p4:tda;
BEGIN
 CASE shp OF
 triangle:
 BEGIN
  vecadd(tri[obj].loc,tri[obj].v1,p2);
  vecadd(tri[obj].loc,tri[obj].v2,p3);
  minimum[0]:=min3(tri[obj].loc[0],p2[0],p3[0]);
  minimum[1]:=min3(tri[obj].loc[1],p2[1],p3[1]);
  minimum[2]:=min3(tri[obj].loc[2],p2[2],p3[2]);
  maximum[0]:=max3(tri[obj].loc[0],p2[0],p3[0]);
  maximum[1]:=max3(tri[obj].loc[1],p2[1],p3[1]);
  maximum[2]:=max3(tri[obj].loc[2],p2[2],p3[2]);
 END;
 paral:
 BEGIN
  vecadd(para[obj].loc,para[obj].v1,p2);
  vecadd(para[obj].loc,para[obj].v2,p3);
  vecadd3(para[obj].loc,para[obj].v1,para[obj].v2,p4);
  minimum[0]:=min4(para[obj].loc[0],p2[0],p3[0],p4[0]);
  minimum[1]:=min4(para[obj].loc[1],p2[1],p3[1],p4[1]);
  minimum[2]:=min4(para[obj].loc[2],p2[2],p3[2],p4[2]);
  maximum[0]:=max4(para[obj].loc[0],p2[0],p3[0],p4[0]);
  maximum[1]:=max4(para[obj].loc[1],p2[1],p3[1],p4[1]);
  maximum[2]:=max4(para[obj].loc[2],p2[2],p3[2],p4[2]);
 END;
 circles:
 BEGIN
  vec(-cir[obj].radius,-cir[obj].radius,cir[obj].radius,minimum);
  vec(cir[obj].radius,cir[obj].radius,cir[obj].radius,maximum);
  vecadd(minimum,cir[obj].loc,minimum);
  vecadd(maximum,cir[obj].loc,maximum);
 END;
 ring:
 BEGIN
  vec(-rng[obj].rad2,-rng[obj].rad2,-rng[obj].rad2,minimum);
  vec(rng[obj].rad2,rng[obj].rad2,rng[obj].rad2,maximum);
  vecadd(minimum,rng[obj].loc,minimum);
  vecadd(maximum,rng[obj].loc,maximum);
 END;
 sphere:
 BEGIN
  vec(-sphr[obj].rad,-sphr[obj].rad,-sphr[obj].rad,minimum);
  vec(sphr[obj].rad,sphr[obj].rad,sphr[obj].rad,maximum);
  vecadd(minimum,sphr[obj].loc,minimum);
  vecadd(maximum,sphr[obj].loc,maximum);
 END;
 cone: quadbound(con[obj],minimum,maximum);
 cylinder: quadbound(cyl[obj],minimum,maximum)
 END;
END;

PROCEDURE getminandmaxpoints(var minimum,maximum:tda);
VAR shapenum,objectnum:integer; minpt,maxpt:tda;
BEGIN
 vecnull(minpt);
 vecnull(maxpt);
 vecnull(minimum);
 vecnull(maximum);
 FOR shapenum:=2 TO maxshapetype DO
 BEGIN
  FOR objectnum:=1 TO objcnt[shapenum] DO
  BEGIN
   bounboxes(shapenum,objectnum,minpt,maxpt);
   IF (shapenum=2) AND (objectnum=1) THEN
   BEGIN
    minimum:=minpt;
    maximum:=maxpt
   END
   ELSE
   BEGIN
    vecmin(minpt,minimum,minimum);
    vecmax(maxpt,maximum,maximum)
   END;
  END;
 END;
 vecsub(minimum,preccor,minimum);
 vecadd(maximum,preccor,maximum);
END;

PROCEDURE crbounsphr(var sphere:spheretype);
VAR minimum,maximum,temp:tda;
BEGIN
 getminandmaxpoints(minimum,maximum);
 vecsub(maximum,minimum,temp);
 vecscalmult(0.5,temp,temp);
 vecadd(minimum,temp,sphere.center);
 sphere.radsqr:=vecdot(temp,temp);
END;

PROCEDURE crbounprism(var prism:prismtype);
VAR
 i,j:integer;
 dot:array[0..6] of real;
 minimum,maximum:tda;
 vec1,vec2,vec3:tda;
 span:tda;
 prismloc,temploc:tda;
 initdir:tda;
 tempface:prismface;
 tempdot:real;
BEGIN
 getminandmaxpoints(minimum,maximum);
 vecnull(vec1);
 vecnull(vec2);
 vecnull(vec3);
 vecsub(maximum,minimum,span);
 vec(span[0],0.0,0.0,vec1);
 vec(0.0,span[1],0.0,vec2);
 vec(0.0,0.0,span[2],vec3);
 prismloc:=minimum;
 FOR i:=1 TO 6 DO
 BEGIN
  vecnull(prism[i].loc);
  vecnull(prism[i].v1);
  vecnull(prism[i].v2);
  vecnull(prism[i].norm);
  prism[i].ndotloc:=0.0
 END;
 prism[1].loc:=prismloc;
 prism[1].v1:=vec1;
 prism[1].v2:=vec3;
 veccross(prism[1].v1,prism[1].v2,prism[1].norm);
 vecnormalize(prism[1].norm);
 prism[1].ndotloc:=vecdot(prism[1].norm,prism[1].loc);
 prism[2].loc:=prismloc;
 prism[2].v1:=vec3;
 prism[2].v2:=vec1;
 veccross(prism[2].v1,prism[2].v2,prism[2].norm);
 vecnormalize(prism[2].norm);
 vec(0.0,vec2[1],0.0,temploc);
 vecadd(temploc,prism[2].loc,prism[2].loc);
 prism[2].ndotloc:=vecdot(prism[2].norm,prism[2].loc);
 prism[3].loc:=prismloc;
 prism[3].v1:=vec3;
 prism[3].v2:=vec2;
 veccross(prism[3].v1,prism[3].v2,prism[3].norm);
 vecnormalize(prism[3].norm);
 prism[3].ndotloc:=vecdot(prism[3].norm,prism[3].loc);
 prism[4].loc:=prismloc;
 prism[4].v1:=vec2;
 prism[4].v2:=vec3;
 veccross(prism[4].v1,prism[4].v2,prism[4].norm);
 vecnormalize(prism[4].norm);
 vec(vec1[0],0.0,0.0,temploc);
 vecadd(temploc,prism[4].loc,prism[4].loc);
 prism[4].ndotloc:=vecdot(prism[4].norm,prism[4].loc);
 prism[5].loc:=prismloc;
 prism[5].v1:=vec2;
 prism[5].v2:=vec1;
 veccross(prism[5].v1,prism[5].v2,prism[5].norm);
 vecnormalize(prism[5].norm);
 prism[5].ndotloc:=vecdot(prism[5].norm,prism[5].loc);
 prism[6].loc:=prismloc;
 prism[6].v1:=vec1;
 prism[6].v2:=vec2;
 veccross(prism[6].v1,prism[6].v2,prism[6].norm);
 vecnormalize(prism[6].norm);
 vec(0.0,0.0,vec3[2],temploc);
 vecadd(temploc,prism[6].loc,prism[6].loc);
 prism[6].ndotloc:=vecdot(prism[6].norm,prism[6].loc);
 getinitialdir(scanxres/2,scanyres/2,initdir);
 FOR i:=1 TO 6 DO
  dot[i]:=vecdot(initdir,prism[i].norm);
 FOR j:=1 TO 6 DO
 BEGIN
  FOR i:=1 TO 5 DO
  BEGIN
   IF dot[i]>dot[i+1] THEN
   BEGIN
    tempface:=prism[i];
    prism[i]:=prism[i+1];
    prism[i+1]:=tempface;
    tempdot:=dot[i];
    dot[i]:=dot[i+1];
    dot[i+1]:=tempdot
   END;
  END;
 END;
END;

PROCEDURE crbountetra(var tetra:tetratype);
VAR
 i,j:integer;
 dot:array[0..4] OF real;
 tetraloc:tda;
 initdir:tda;
 temp:tda;
 tempface:tetraface;
 tempdot:real;
BEGIN
 vecscalmult(1.1,tetraext1,tetraext1);
 vecscalmult(1.1,tetraext2,tetraext2);
 vecscalmult(1.1,tetraext3,tetraext3);
 vecscalmult(1.1,tetraext4,tetraext4);
 FOR i:=1 TO 4 DO
 BEGIN
  vecnull(tetra[i].loc);
  vecnull(tetra[i].v1);
  vecnull(tetra[i].v2);
  vecnull(tetra[i].norm);
  tetra[i].ndotloc:=0.0;
 END;
 vecadd3(tetraext1,tetraext2,tetraext3,temp);
 vecscalmult(1.0/3.0,temp,tetraloc);
 vecadd(tetraloc,tetraext1,tetra[1].loc);
 vecsub(tetraext2,tetraext1,tetra[1].v1);
 vecsub(tetraext4,tetraext1,tetra[1].v2);
 veccross(tetra[1].v1,tetra[1].v2,tetra[1].norm);
 vecnormalize(tetra[1].norm);
 tetra[1].ndotloc:=vecdot(tetra[1].norm,tetra[1].loc);
 vecadd(tetraloc,tetraext2,tetra[2].loc);
 vecsub(tetraext3,tetraext2,tetra[2].v1);
 vecsub(tetraext4,tetraext2,tetra[2].v2);
 veccross(tetra[2].v1,tetra[2].v2,tetra[2].norm);
 vecnormalize(tetra[2].norm);
 tetra[2].ndotloc:=vecdot(tetra[2].norm,tetra[2].loc);
 vecadd(tetraloc,tetraext3,tetra[3].loc);
 vecsub(tetraext1,tetraext3,tetra[3].v1);
 vecsub(tetraext4,tetraext3,tetra[3].v2);
 veccross(tetra[3].v1,tetra[3].v2,tetra[3].norm);
 vecnormalize(tetra[3].norm);
 tetra[3].ndotloc:=vecdot(tetra[3].norm,tetra[3].loc);
 vecadd(tetraloc,tetraext1,tetra[4].loc);
 vecsub(tetraext3,tetraext1,tetra[4].v1);
 vecsub(tetraext2,tetraext1,tetra[4].v2);
 veccross(tetra[4].v1,tetra[4].v2,tetra[4].norm);
 vecnormalize(tetra[4].norm);
 tetra[4].ndotloc:=vecdot(tetra[4].norm,tetra[4].loc);
 getinitialdir(scanxres/2,scanyres/2,initdir);
 FOR i:=1 TO 4 DO
  dot[i]:=vecdot(initdir,tetra[i].norm);
 FOR j:=1 to 4 DO
 BEGIN
  FOR i:=1 TO 3 DO
  BEGIN
   IF dot[i]>dot[i+1] THEN
   BEGIN
    tempface:=tetra[i];
    tetra[i]:=tetra[i+1];
    tetra[i+1]:=tempface;
    tempdot:=dot[i];
    dot[i]:=dot[i+1];
    dot[i+1]:=tempdot;
   END;
  END;
 END;
END;

PROCEDURE initbounobjects;
BEGIN
 IF bounsphrtest THEN crbounsphr(bounsphere);
 IF bounprismtest THEN crbounprism(bounprism);
 IF bountetratest THEN crbountetra(bountetra);
END;

PROCEDURE setuptri(i,p1,p2:integer);
BEGIN
 gu[0]:=-delta[p1];
 gv[0]:=-delta[p2];
 gu[1]:=bountetra[i].v1[p1]-delta[p1];
 gv[1]:=bountetra[i].v1[p2]-delta[p2];
 gu[2]:=bountetra[i].v2[p1]-delta[p1];
 gv[2]:=bountetra[i].v2[p2]-delta[p2];
END;

PROCEDURE setuppara(i,p1,p2:integer);
BEGIN
 gu[0]:=-delta[p1]; 
 gv[0]:=-delta[p2];
 gu[1]:=bounprism[i].v1[p1]-delta[p1];
 gv[1]:=bounprism[i].v1[p2]-delta[p2];
 gu[2]:=bounprism[i].v2[p1]+bounprism[i].v1[p1]-delta[p1];
 gv[2]:=bounprism[i].v2[p2]+bounprism[i].v1[p2]-delta[p2];
 gu[3]:=bounprism[i].v2[p1]-delta[p1];
 gv[3]:=bounprism[i].v2[p2]-delta[p2];
END;

PROCEDURE dobstest(var start,dir:tda);
BEGIN
 vecsub(bounsphere.center,start,temp);
 b:=vecdot(dir,temp)*(-2.0);
 c:=vecdot(temp,temp)-bounsphere.radsqr;
 disc:=sqr(b)-4.0*c;
 IF disc<=0.0 THEN hitbounsphere:=false
 ELSE
 BEGIN
  sroot:=sqrt(disc);
  t1:=(-b-sroot)*0.5;
  t2:=(-b+sroot)*0.5;
  IF (t1<=small) AND (t2<=small) THEN hitbounsphere:=false
  ELSE hitbounsphere:=true;
 END;
END;

PROCEDURE dobptest(var start,dir:tda);
VAR i,last:integer;
BEGIN
 IF initialray THEN
 BEGIN
  initialray:=false;
  last:=4
 END
 ELSE last:=7;
 i:=1;
 REPEAT
  dot:=vecdot(bounprism[i].norm,dir);
  IF abs(dot)<small THEN hitbounprism:=false
  ELSE
  BEGIN
   pos1:=bounprism[i].ndotloc;
   pos2:=vecdot(bounprism[i].norm,start);
   t:=(pos1-pos2)/dot;
   getintrpt(start,dir,t,intrpoint);
   vecsub(intrpoint,bounprism[i].loc,delta);
   IF (abs(bounprism[i].norm[0])>abs(bounprism[i].norm[1])) AND
      (abs(bounprism[i].norm[0])>abs(bounprism[i].norm[2])) THEN
    setuppara(i,1,2)
   ELSE
   BEGIN
    IF NOT (abs(bounprism[i].norm[1])<abs(bounprism[i].norm[2])) THEN
     setuppara(i,0,2)
    ELSE
     setuppara(i,0,1);
   END;
   IF evencrossings(4) THEN hitbounprism:=false
   ELSE hitbounprism:=true;
  END;
  i:=i+1
 UNTIL (i=last) OR (hitbounprism=true);
END;

PROCEDURE dobttest(var start,dir:tda);
VAR i,last:integer;
BEGIN
 IF initialray THEN
 BEGIN
  initialray:=false;
  last:=4
 END
 ELSE last:=5;
 i:=1;
 REPEAT
  dot:=vecdot(bountetra[i].norm,dir);
  IF abs(dot)<small THEN hitbountetra:=false
  ELSE
  BEGIN
   pos1:=bountetra[i].ndotloc;
   pos2:=vecdot(bountetra[i].norm,start);
   t:=(pos1-pos2)/dot;
   getintrpt(start,dir,t,intrpoint);
   vecsub(intrpoint,bountetra[i].loc,delta);
   IF (abs(bountetra[i].norm[0])>abs(bountetra[i].norm[1])) AND
      (abs(bountetra[i].norm[0])>abs(bountetra[i].norm[2])) THEN
    setuptri(i,1,2)
   ELSE
   BEGIN
    IF NOT (abs(bountetra[i].norm[1])<abs(bountetra[i].norm[2])) THEN
     setuptri(i,0,2)
    ELSE setuptri(i,0,1);
   END;
   IF evencrossings(3) THEN hitbountetra:=false
   ELSE hitbountetra:=true;
  END;
  i:=i+1
 UNTIL (i=last) OR (hitbountetra=true);
END;

PROCEDURE intersecttest(var start,dir:tda;
var shp,obj:integer; var dist:real; var objhit:boolean);
VAR shapenum,objectnum:integer; newdist:real;
BEGIN
 objhit:=false;
 FOR shapenum:=0 TO maxshapetype DO
  FOR objectnum:=1 TO objcnt[shapenum] DO
  BEGIN
   newdist:=intersect(start,dir,shapenum,objectnum);
   IF newdist>small THEN
   BEGIN
    IF dist=-1.0 THEN
    BEGIN
     objhit:=true;
     dist:=newdist;
     shp:=shapenum;
     obj:=objectnum;
    END
    ELSE
    BEGIN
     IF newdist<dist THEN
     BEGIN
      dist:=newdist;
      shp:=shapenum;
      obj:=objectnum;
     END;
    END;
   END;
  END;
END;

PROCEDURE igroundtest(var start,dir:tda;
var shp,obj:integer; var dist:real; var objhit:boolean);
VAR newdist:real;
BEGIN
 objhit:=false;
 newdist:=intersect(start,dir,ground,1);
 IF newdist>small THEN
 BEGIN
  objhit:=true;
  dist:=newdist;
  shp:=ground;
  obj:=1
 END;
END;

PROCEDURE shootray(var start,dir:tda; var shp,obj:integer; var dist:real;
var objhit:boolean);
VAR i:INTEGER;
BEGIN
 shp:=-1;
 obj:=-1;
 dist:=-1.0;
 objhit:=false;
 IF NOT (bounsphrtest OR bounprismtest OR bountetratest) THEN
  intersecttest(start,dir,shp,obj,dist,objhit)
 ELSE IF (bounsphrtest) THEN
 BEGIN
  dobstest(start,dir);
  IF hitbounsphere AND bounprismtest THEN
  BEGIN
   dobptest(start,dir);
   IF hitbounprism THEN intersecttest(start,dir,shp,obj,dist,objhit)
   ELSE IF groundexists THEN igroundtest(start,dir,shp,obj,dist,objhit);
  END
  ELSE IF hitbounsphere AND bountetratest THEN
  BEGIN
   dobttest(start,dir);
   IF hitbountetra THEN intersecttest(start,dir,shp,obj,dist,objhit)
   ELSE IF groundexists THEN igroundtest(start,dir,shp,obj,dist,objhit);
  END
  ELSE IF hitbounsphere THEN intersecttest(start,dir,shp,obj,dist,objhit)
  ELSE IF groundexists THEN igroundtest(start,dir,shp,obj,dist,objhit);
 END
 ELSE IF bounprismtest THEN
 BEGIN
  dobptest(start,dir);
  IF hitbounprism THEN intersecttest(start,dir,shp,obj,dist,objhit)
  ELSE IF groundexists THEN igroundtest(start,dir,shp,obj,dist,objhit);
 END
 ELSE IF bountetratest THEN
 BEGIN
  dobttest(start,dir);
  IF hitbountetra THEN intersecttest(start,dir,shp,obj,dist,objhit)
  ELSE IF groundexists THEN igroundtest(start,dir,shp,obj,dist,objhit);
 END;
END;

PROCEDURE initrand(seed:real);
BEGIN
 oldrand:=seed
END;

FUNCTION frac(x:real):real;
BEGIN
 frac:=x-trunc(x)
END;

FUNCTION randint(range:word):integer;
BEGIN
 oldrand:=frac(423.1966*oldrand);
 randint:=trunc(oldrand*range)
END;

PROCEDURE initnoise;
VAR x,y,z,i,j,k:integer;
BEGIN
 initrand(0.409842);
 FOR x:=0 TO maxnoise-1 DO
  FOR y:=0 TO maxnoise-1 DO
   FOR z:=0 TO maxnoise-1 DO
   BEGIN
    noisematrix[x,y,z]:=randint(12000);
    IF x=maxnoise-1 THEN i:=0 ELSE i:=x;
    IF y=maxnoise-1 THEN j:=0 ELSE j:=y;
    IF z=maxnoise-1 THEN k:=0 ELSE k:=z;
    noisematrix[x,y,z]:=noisematrix[i,j,k];
   END;
END;

FUNCTION noise(x,y,z:real):integer;
VAR
 ox,oy,oz:real;
 ix,iy,iz,p000,p001,p010,p011:integer;
 p100,p101,p110,p111:integer;
 p00,p01,p10,p11:integer;
 p0,p1,d00,d01,d10,d11:integer;
 d0,d1,d:integer;
BEGIN
 x:=abs(x);
 y:=abs(y);
 z:=abs(z);
 ix:=trunc(x) MOD (maxnoise-1);
 iy:=trunc(y) MOD (maxnoise-1);
 iz:=trunc(z) MOD (maxnoise-1);
 ox:=frac(x);
 oy:=frac(y);
 oz:=frac(z);
 p000:=noisematrix[ix,iy,iz];
 p001:=noisematrix[ix,iy,iz+1];
 p010:=noisematrix[ix,iy+1,iz];
 p011:=noisematrix[ix,iy+1,iz+1];
 p100:=noisematrix[ix+1,iy,iz];
 p101:=noisematrix[ix+1,iy,iz+1];
 p110:=noisematrix[ix+1,iy+1,iz];
 p111:=noisematrix[ix+1,iy+1,iz+1];
 d00:=p100-p000;
 d01:=p101-p001;
 d10:=p110-p010;
 d11:=p111-p011;
 p00:=trunc(d00*ox)+p000;
 p01:=trunc(d01*ox)+p001;
 p10:=trunc(d10*ox)+p010;
 p11:=trunc(d11*ox)+p011;
 d0:=p10-p00;
 d1:=p11-p01;
 p0:=trunc(d0*oy)+p00;
 p1:=trunc(d1*oy)+p01;
 d:=p1-p0;
 noise:=trunc(d*oz)+p0;
END;

PROCEDURE marbletex(var pt:tda; var rgb:tda);
VAR i,d,x,y,z:real;
BEGIN
 unvec(pt,x,y,z);
 x:=x*0.2;
 d:=x+0.0006*noise(x,y*0.1,z*0.1);
 d:=d*(trunc(d) MOD 25);
 i:=0.5+0.05*abs(d-10.0-20.0*trunc(d*0.05));
 IF i>1.0 THEN i:=1.0;
 vec(i,i,i,rgb);
END;

PROCEDURE woodtex(var pt:tda; var rgb:tda);
VAR i,d,x,y,z:real;
BEGIN
 unvec(pt,x,y,z);
 x:=x*0.2;
 d:=x+0.0002*noise(x,y*0.1,z*0.1);
 d:=d*(trunc(d) MOD 25);
 i:=0.7+0.05*abs(d-10.0-20.0*trunc(d*0.05));
 IF i>1.0 THEN i:=1.0;
 vec(i,i,i,rgb);
END;

FUNCTION texturenumbs(shp,obj:integer):integer;
BEGIN
 CASE shp OF
 ground: texturenumbs:=gnd.texnum;
 triangle: texturenumbs:=tri[obj].texnum;
 paral: texturenumbs:=para[obj].texnum;
 circles: texturenumbs:=cir[obj].texnum;
 ring: texturenumbs:=rng[obj].texnum;
 sphere: texturenumbs:=sphr[obj].texnum;
 cone: texturenumbs:=con[obj].texnum;
 cylinder: texturenumbs:=cyl[obj].texnum
 END;
END;

FUNCTION materialnumbs(shp,obj:integer):integer;
BEGIN
 CASE shp OF
 ground: materialnumbs:=gnd.mtlnum;
 triangle: materialnumbs:=tri[obj].mtlnum;
 paral: materialnumbs:=para[obj].mtlnum;
 circles: materialnumbs:=cir[obj].mtlnum;
 ring: materialnumbs:=rng[obj].mtlnum;
 sphere: materialnumbs:=sphr[obj].mtlnum;
 cone: materialnumbs:=con[obj].mtlnum;
 cylinder: materialnumbs:=cyl[obj].mtlnum
 END;
END;

PROCEDURE texture(VAR intrpt:tda; tex:integer; var texturing:tda);
VAR
 x,y,z,rt:integer;
 lev,r:real;
 valor:char;
BEGIN
 CASE tex OF
 checker:
  BEGIN
   x:=round(abs(intrpt[0])*tile);
   y:=round(abs(intrpt[1])*tile);
   z:=round(abs(intrpt[2])*tile);
   IF ((x+y+z) MOD 2)=1 THEN texturing:=tile1
   ELSE texturing:=tile2;
  END;
 grit:
  BEGIN
   rt:=randint(32767);
   r:=rt/32768.0;
   lev:=r*0.2+0.8;
   vec(lev,lev,lev,texturing);
  END;
 marble: marbletex(intrpt, texturing);
 wood:   woodtex(intrpt, texturing);
 sheetrock:
  BEGIN
   rt:=randint(32767);
   r:=rt/32768.0;
   lev:=r*0.1+0.9;
   vec(lev,lev,lev,texturing);
  END;
 particle:
  BEGIN
   rt:=randint(32767);
   r:=rt/32768.0;
   lev:=r*0.15+0.85;
   vec(lev,lev,lev,texturing);
  END;
 image:
  BEGIN
   x:=round(intrpt[0]);
   y:=round(intrpt[1]);
{  write(chr(27),'[w',chr(x),chr(x shr 8), chr(y), chr(y shr 8)); }
   write(chr(27),'[w',chr((x and 15) + ord('0')),
                      chr(((x shr 4) and 15) + ord('0')),
                      chr(((x shr 8) and 15) + ord('0')),
                      chr(((x shr 12) and 15) + ord('0')),
                      chr((y and 15) + ord('0')),
                      chr(((y shr 4) and 15) + ord('0')),
                      chr(((y shr 8) and 15) + ord('0')),
                      chr(((y shr 12) and 15) + ord('0')));
   read(valor);
   texturing[0]:=ord(valor)/255.0;
   read(valor);
   texturing[1]:=ord(valor)/255.0;
   read(valor);
   texturing[2]:=ord(valor)/255.0
  END
 END;
END;

FUNCTION objhittransmissive(mtl:integer):boolean;
BEGIN
 IF (matl[mtl].trans[0]=0.0) AND
    (matl[mtl].trans[1]=0.0) AND
    (matl[mtl].trans[2]=0.0) THEN objhittransmissive:=false
 ELSE objhittransmissive:=true;
END;

PROCEDURE getloclcol(shp,obj:integer; var dir,intrpt,srfnrm:tda; dist:real;
var loclcol:tda);
VAR
mtl,tex,src:integer;
objhit,hititself:boolean;
intensfactor:real;
lmpdir,addition,total:tda;
lamb:real;
spec,diff,temp:tda;
cone,glint:real;
shadshp:integer;
shadobj:integer;
shaddist:real;
shadintrpt,shadsrfnrm:tda;
alpha:real;
colortexture:tda;
BEGIN
 IF shp=lamp THEN
 BEGIN
  IF disteffect=0.0 THEN intensfactor:=1.0
  ELSE intensfactor:=(1.0-disteffect)+disteffect*(-vecdot(srfnrm,dir)/
  sqrt(dist));
  vecscalmult(intensfactor,lmp[obj].intens,loclcol);
 END
 ELSE
 BEGIN
  mtl:=materialnumbs(shp,obj);
  total:=matl[mtl].ambrfl;
  FOR src:=1 TO objcnt[lamp] DO
  BEGIN
   vecsub(lmp[src].loc,intrpt,lmpdir);
   vecnormalize(lmpdir);
   shootray(intrpt,lmpdir,shadshp,shadobj,shaddist,objhit);
   IF objhit AND (shadshp=lamp) AND (shadobj=src) THEN hititself:=true
   ELSE hititself:=false;
   IF (NOT objhit) OR hititself THEN
   BEGIN
    lamb:=vecdot(srfnrm,lmpdir);
    IF lamb<=0.0 THEN
    BEGIN
     vecnull(spec);
     vecnull(diff)
    END
    ELSE
    BEGIN
     vecelemmult(lamb,matl[mtl].difrfl,lmp[src].intens,diff);
     vecsub(lmpdir,dir,temp);
     vecnormalize(temp);
     cone:=vecdot(srfnrm,temp);
     IF cone>0.5 THEN
     BEGIN
      glint:=exp(matl[mtl].gloss*ln(cone));
      vecelemmult(glint,matl[mtl].spcrfl,lmp[src].intens,spec)
     END
     ELSE vecnull(spec);
    END;
    vecadd(diff,spec,addition);
    vecadd(total,addition,total);
   END
   ELSE IF shadshp<>lamp THEN
   BEGIN
    mtl:=materialnumbs(shadshp,shadobj);
    IF objhittransmissive(mtl) THEN
    BEGIN
     getintrpt(intrpt,lmpdir,shaddist,shadintrpt);
     getsrfnrm(shadshp,shadobj,shadintrpt,shadsrfnrm);
     alpha:=0.7*abs(vecdot(lmpdir,shadsrfnrm))+0.2;
     vecelemmult(alpha,matl[mtl].trans,lmp[src].intens,addition);
     vecadd(total,addition,total)
    END;
    mtl:=materialnumbs(shp,obj);
   END;
  END;
  tex:=texturenumbs(shp,obj);
  IF (tex=smooth) OR (tex=oceanwaves) OR (tex=poolwaves) OR (tex=waves) THEN
   loclcol:=total
  ELSE
  BEGIN
   texture(intrpt,tex,colortexture);
   vecelemmult(1.0,total,colortexture,loclcol)
  END;
 END;
END;

PROCEDURE sky(var dir,col:tda);
VAR small2,sin2,cos2,x2,y2,z2:real;
BEGIN
 small2:=1e-3;
 x2:=sqr(dir[0]);
 y2:=sqr(dir[1]);
 z2:=sqr(dir[2]);
 IF z2=0 THEN z2:=small2;
 sin2:=z2/(x2+y2+z2);
 cos2:=1.0-sin2;
 veclincomb(cos2,horcol,sin2,zencol,col);
END;

PROCEDURE cloudysky(VAR dir,skycol:tda);
VAR small2,sin2,cos2,x2,y2,z2,x,y,z,d,f:real; col,col1,col2:tda;
BEGIN
 small2:=1e-3;
 unvec(dir,x,y,z);
 x2:=sqr(dir[0]);
 y2:=sqr(dir[1]);
 z2:=sqr(dir[2]);
 IF z2=0 THEN z2:=small2;
 sin2:=z2/(x2+y2+z2);
 cos2:=1.0-sin2;
 veclincomb(cos2,horcol,sin2,zencol,col);
 x:=x*10.0;
 y:=y*10.0;
 z:=z*400.0;
 d:=z+0.0006*noise(x,y,z);
 d:=d*(trunc(d) MOD 25);
 f:=0.06+abs(d-10.0-20.0*trunc(d*0.05));
 IF f>1.0 THEN f:=1.0;
 vec(col[2],col[2],col[2],col1);
 col2:=col;
 veclincomb(f,col1,1.0-f,col2,skycol);
END;

PROCEDURE comb(var a,b,c,d,e,f,col:tda);
VAR t1,t2,t3:tda;
BEGIN
 vecelemmult(1.0,a,b,t1);
 vecelemmult(1.0,c,d,t2);
 vecelemmult(1.0,e,f,t3);
 vecadd3(t1,t2,t3,col)
END;

FUNCTION wgtmin(var totwgt:tda):boolean;
BEGIN
 IF (totwgt[0]<=minwgt[0]) AND
    (totwgt[1]<=minwgt[1]) AND
    (totwgt[2]<=minwgt[2]) THEN wgtmin:=true
 ELSE wgtmin:=false;
END;

FUNCTION materialspecular(shp,mtl:integer):boolean;
BEGIN
 IF shp<>lamp THEN
 BEGIN
  IF (matl[mtl].spcrfl[0]=0.0) AND
     (matl[mtl].spcrfl[1]=0.0) AND
     (matl[mtl].spcrfl[2]=0.0) THEN materialspecular:=false
  ELSE materialspecular:=true;
 END
 ELSE materialspecular:=true;
END;

FUNCTION materialtransmissive(shp,mtl:integer):boolean;
BEGIN
 IF shp<>lamp THEN
 BEGIN
  IF (matl[mtl].trans[0]=0.0) AND
     (matl[mtl].trans[1]=0.0) AND
     (matl[mtl].trans[2]=0.0) THEN materialtransmissive:=false
  ELSE materialtransmissive:=true;
 END
 ELSE materialtransmissive:=false;
END;

PROCEDURE traceray(var start,dir,totwgt:tda; depth:integer; var col:tda);
VAR
loclcol,reflcol,trancol,refldir,trandir,wgt,intrpt,srfnrm:tda;
shp,obj:integer;
dist:real;
mtl,tex:integer;
f1,f2,f3,f4,ampl,dampen,dampen2:real;
temp:tda;
objhit,dummy:boolean;
BEGIN
 shootray(start,dir,shp,obj,dist,objhit);
 IF objhit THEN
 BEGIN
  getintrpt(start,dir,dist,intrpt);
  getsrfnrm(shp,obj,intrpt,srfnrm);
  tex:=texturenumbs(shp,obj);
  CASE tex OF
  oceanwaves:
  BEGIN
   f1:=sin(radians(intrpt[0]+intrpt[1]+ocwavephase));
   f2:=sin(radians(2.5*intrpt[0]+intrpt[1]+ocwavephase));
   f3:=sin(radians(intrpt[0]+1.7*intrpt[1]+ocwavephase));
   f4:=sin(radians(1.5*intrpt[0]+4.1*intrpt[1]+ocwavephase));
   ampl:=ocwaveampl*(f1+f2+f3+f4)*0.25;
   srfnrm[0]:=srfnrm[0]+ampl;
   srfnrm[1]:=srfnrm[1]+ampl;
   vecnormalize(srfnrm)
  END;
  poolwaves:
  BEGIN
   dampen:=sqrt(sqr(intrpt[0]-powavexpos)+sqr(intrpt[1]-powaveypos));
   dampen2:=dampen*1e-2;
   IF dampen2<1.0 THEN dampen2:=1.0;
   ampl:=powaveampl*sin(radians(2.5*dampen+powavephase))/dampen2;
   srfnrm[0]:=srfnrm[0]+ampl;
   srfnrm[1]:=srfnrm[1]+ampl;
   vecnormalize(srfnrm)
  END;
  waves:
  BEGIN
   dampen:=sqrt(sqr(intrpt[0]-wavexpos)+
   sqr(intrpt[1]-waveypos)+
   sqr(intrpt[2]-wavezpos));
   dampen2:=dampen*1e-2;
   IF dampen2<1.0 THEN dampen2:=1.0;
   ampl:=waveampl*cos(radians(60.0*dampen+wavephase))/dampen2;
   srfnrm[0]:=srfnrm[0]+ampl;
   srfnrm[1]:=srfnrm[1]+ampl;
   srfnrm[2]:=srfnrm[2]+ampl;
   vecnormalize(srfnrm)
  END
  END;
  IF (shp=lamp) AND (NOT lampreflects) THEN
   getloclcol(shp,obj,dir,intrpt,srfnrm,dist,col)
  ELSE
  BEGIN
   getloclcol(shp,obj,dir,intrpt,srfnrm,dist,loclcol);
   IF (depth=maxdepth) OR wgtmin(totwgt) THEN
    vecelemmult(1.0,loclcol,loclwgt,col)
   ELSE
   BEGIN
    IF (shp<>lamp) OR ((shp=lamp) AND lampreflects) THEN
    BEGIN
     mtl:=materialnumbs(shp,obj);
     IF materialspecular(shp,mtl) THEN
     BEGIN
      calcdirofreflray(dir,srfnrm,refldir);
      vecelemmult(1.0,totwgt,reflwgt,wgt);
      traceray(intrpt,refldir,wgt,depth+1,reflcol);
      IF shp<>lamp THEN
      BEGIN
       vecsub(onesvec,matl[mtl].trans,temp);
       vecelemmult(1.0,reflcol,temp,reflcol)
      END;
     END
     ELSE vecnull(reflcol);
     IF materialtransmissive(shp,mtl) THEN
     BEGIN
      calcdiroftranray(dir,srfnrm,mtl,trandir);
      shootray(intrpt,trandir,shp,obj,dist,dummy);
      getintrpt(intrpt,trandir,dist,intrpt);
      getsrfnrm(shp,obj,intrpt,srfnrm);
      calcdiroftranray(trandir,srfnrm,mtl,trandir);
      vecelemmult(1.0,totwgt,tranwgt,wgt);
      traceray(intrpt,trandir,wgt,depth+1,trancol);
      vecelemmult(1.0,trancol,matl[mtl].trans,trancol)
     END
     ELSE vecnull(trancol);
    END
    ELSE
    BEGIN
     vecnull(reflcol);
     vecnull(trancol)
    END;
    IF (shp=lamp) AND lampreflects THEN
    BEGIN
     vecsub(onesvec,lamprefl,temp);
     vecelemmult(1.0,temp,loclcol,loclcol);
     vecelemmult(1.0,lamprefl,reflcol,reflcol);
     vecadd(loclcol,reflcol,col);
    END
    ELSE comb(loclcol,loclwgt,reflcol,reflwgt,trancol,tranwgt,col);
   END;
  END;
 END
 ELSE
 BEGIN
  IF skyexists THEN
   IF clouds THEN cloudysky(dir,col) ELSE sky(dir,col)
  ELSE vecnull(col);
 END;
END;

PROCEDURE precalculation;
VAR scale:real;
BEGIN
 xaspdivfoclen:=asp/focallength;
 yaspdivfoclen:=1.0/focallength;
 centerx:=scanxres div 2;
 centery:=scanyres div 2;
 scale:=centerx;
 vecscalmult(scale,viewdir,viewvec);
END;

PROCEDURE initgraphics;
BEGIN
 write(chr(27),'[G')
END;

PROCEDURE clearscreen;
BEGIN
 xres:=scanxres;
 yres:=scanyres;
 asp:=(4/3)*(yres/xres);
END;

PROCEDURE plot(xp,yp,r,g,b:integer);
BEGIN
 write(chr(27),'[',xp:0,';',yp:0,';',r:0,';',g:0,';',b:0,'P');

END;

PROCEDURE scan;
VAR initialdir,col:tda; colr:tdia; xp,yp,x,y:word; inside:boolean;
BEGIN
 precalculation;
 FOR yp:=0 TO scanyres-1 DO
  FOR xp:=0 TO scanxres-1 DO
  BEGIN
   getinitialdir(xp,yp,initialdir);
   initialray:=true;
   traceray(obspos,initialdir,maxwgt,1,col);
   IF col[0]>1.0 THEN col[0]:=1.0;
   IF col[1]>1.0 THEN col[1]:=1.0;
   IF col[2]>1.0 THEN col[2]:=1.0;
   vecsmint(255.0,col,colr);
   plot(xp,yp,colr[0],colr[1],colr[2]);
  END;
END;

PROCEDURE getscenefile;
BEGIN
END;

PROCEDURE exitgraphics;
VAR tecla:char;
BEGIN
 write(chr(27),'[T')
END;

BEGIN
 writeln('Ray tracing');
 writeln;
 writeln('Port by Oscar Toledo G.');
 writeln;
 small:=1e-1;
 preccor[0]:=1.0;
 preccor[1]:=1.0;
 preccor[2]:=1.0;
 onesvec[0]:=1.0;
 onesvec[1]:=1.0;
 onesvec[2]:=1.0;
 initnoise;
 getscenefile;
 loadrtheader;
 clearscreen;
 lastframenum:=numberofframes;
 FOR framenum:=1 TO lastframenum DO
 BEGIN
  initgraphics;
  clearmemory;
  getdataforframe;
  initbounobjects;
  scan;
  exitgraphics;
 END;
END.
