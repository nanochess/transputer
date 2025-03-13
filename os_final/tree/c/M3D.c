/*
** Modelado en 3 dimensiones.
**
** Portado por Oscar Toledo G.
**
** Creación: 23 de noviembre de 1995. Trasladado desde Pascal.
*/

double abs(), sin(), cos(), arctan(), exp(), ln(), random(), sqrt(), sqr();
int round(), trunc();

#define maxnoise 16
#define piover180 0.017453293
#define maxmaterial 20
#define maxtexture 10

int entrada;
int salida;

#define smooth 1
#define checker 2
#define grit 3
#define marble 4
#define wood 5
#define sheetrock 6
#define particle 7
#define oceanwaves 8
#define poolwaves 9
#define waves 10
#define image 11

#define maxshapetype 9

#define ground 0
#define lamp 1
#define triangle 2
#define paral 3
#define circles 4
#define ring 5
#define sphere 6
#define cone 7
#define cylinder 8

#define maxlamp 10
#define maxtriangle 256
#define maxparal 32
#define maxcircles 32
#define maxring 32
#define maxsphere 128
#define maxcone 32
#define maxcylinder 32

typedef float TDA[3];
typedef int TDIA[3];
typedef float FDA[4];
typedef char NAME[32];
typedef struct {
  NAME mtype;
  NAME textur;
  TDA ambrfl, difrfl, spcrfl, trans;
  float gloss, index;
} MATERIALLIST;
typedef struct {
  int mtlnum, texnum;
} GROUNDLIST;
typedef struct {
  TDA loc, intens;
  float rad, radsqr;
} LAMPLIST;
typedef struct {
  TDA loc, v1, v2, norm;
  float ndotloc;
  int mtlnum, texnum;
} TRIANGLELIST;
typedef TRIANGLELIST PARALLIST;
typedef struct {
  TDA loc, v1, v2, norm;
  float ndotloc, radius;
  int mtlnum, texnum;
} CIRCLELIST;
typedef struct {
  TDA loc, v1, v2, norm;
  float ndotloc, rad1, rad2;
  int mtlnum, texnum;
} RINGLIST;
typedef struct {
  TDA loc;
  float rad, radsqr;
  int mtlnum, texnum;
} SPHERELIST;
typedef struct {
  TDA baseloc, apexloc, u, v, w;
  float baserad, based, apexrad, height, slope, mind, maxd;
  int insnrm;
  int mtlnum, texnum;
} QUADLIST;
typedef QUADLIST CONELIST;
typedef QUADLIST CYLINDERLIST;

struct SPHERETYPE {
  TDA center;
  float radsqr;
} bounsphere;

float asp, small;

int xres, yres;
float oldrand;
int scanxres, scanyres;
float xaspdivfoclen, yaspdivfoclen;
int centerx, centery;
TDA viewvec;

int numberofframes;

TDA loclwgt, reflwgt, tranwgt, minwgt, maxwgt;

int maxdepth;

int lampreflects;
TDA lamprefl;
float disteffect;

TDA onesvec, preccor;

float focallength;
TDA obspos;
float obsrotate, obstilt;
TDA viewdir, viewu, viewv;

TDA horcol, zencol;
int clouds, skyexists;

TDA tile1, tile2;
float tile;

float ocwaveampl, ocwavephase;
float powaveampl, powavephase, powavexpos, powaveypos;
float waveampl, wavephase, wavexpos, waveypos, wavezpos;

int groundexists;

int bounsphrtest;

char buffer[81];
char buf1[10], buf2[10];

NAME mtlname;

int mtlcount;

FDA gu, gv;

TDA delta;

float t1, t2;

TDA intrpoint, temp;
float a, b, c, d, disc, sroot, t;

int hitbounsphere;

float dot, pos1, pos2;
int framenum, lastframenum;

MATERIALLIST matl[maxmaterial];
int objcnt[maxshapetype];
GROUNDLIST gnd;
LAMPLIST lmp[maxlamp];
TRIANGLELIST tri[maxtriangle];
PARALLIST para[maxparal];
CIRCLELIST cir[maxcircles];
RINGLIST rng[maxring];
SPHERELIST sphr[maxsphere];
CONELIST con[maxcone];
CYLINDERLIST cyl[maxcylinder];

int noisematrix[16][16][16];

leelinea() {
  int contador, caract;
  contador = 0;
  while(1) {
    caract = fgetc(entrada);
    if(caract == '\r') continue;
    if(caract == -1) {
      buffer[contador++] = '\n';
      break;
    }
    buffer[contador++] = caract;
    if(caract == '\n') break;
  }
}

leereal(valor, contador)
  float *valor;
  int *contador;
{
  float fraccion, divisor, menos;
  if(buffer[*contador] == '-') {
    menos = -1.0;
    ++*contador;
  } else
    menos = 1.0;
  *valor = 0.0;
  while (buffer[*contador] >= '0' && buffer[*contador] <= '9')
    *valor = *valor * 10.0 + (buffer[(*contador)++] - 48);
  if (buffer[*contador] == '.') {
    ++*contador;
    fraccion = 0.0;
    divisor = 1.0;
    while (buffer[*contador] >= '0' && buffer[*contador] <= '9') {
      fraccion = fraccion * 10.0 + (buffer[(*contador)++] - 48);
      divisor = divisor * 10.0;
    }
    *valor += fraccion / divisor;
  }
  *valor *= menos;
}

loadline() {
  int contador, contador2;
  for(contador = 0; contador < 10; ++contador)
    buf1[contador] = ' ';
  leelinea();
  contador = contador2 = 0;
  while (buffer[contador] == ' ')
    ++contador;
  while (buffer[contador] != ' ' &&
         buffer[contador] != '\n') {
    if(contador2 < 10)
      buf1[contador2++] = buffer[contador];
    ++contador;
  }
}

loadtda(a)
  TDA a;
{
  int contador;

  leelinea();
  contador = 0;
  while(buffer[contador++] != '=') ;
  while(buffer[contador] == ' ') ++contador;
  leereal(a, &contador);
  while(buffer[contador] == ' ') ++contador;
  leereal(a+1, &contador);
  while(buffer[contador] == ' ') ++contador;
  leereal(a+2, &contador);
}

loadreal(a)
  float *a;
{
  int contador;

  leelinea();
  contador = 0;
  while(buffer[contador++] != '=') ;
  while(buffer[contador] == ' ') ++contador;
  leereal(a, &contador);
}

loadinteger(a)
  int *a;
{
  int contador;

  leelinea();
  contador = 0;
  while(buffer[contador++] != '=');
  while(buffer[contador] == ' ') ++contador;
  *a = 0;
  while(buffer[contador] >= '0' && buffer[contador] <= '9')
    *a = *a * 10 + (buffer[contador++] - 48);
}

loadtext(a)
  NAME a;
{
  int contador, contador2;

  for(contador=0; contador<32; ++contador)
    a[contador] = ' ';
  leelinea();
  contador = 0;
  while (buffer[contador++] != '=') ;
  while (buffer[contador] == ' ')
    ++contador;
  contador2 = 0;
  while (buffer[contador] != ' ' && buffer[contador] != '\n')
    a[contador2++] = buffer[contador++];
}

int loadboolean()
{
  int contador;

  leelinea();
  contador = 0;
  while (buffer[contador++] != '=') ;
  while (buffer[contador] == ' ')
    ++contador;
  if (buffer[contador] == 'F' ||
      buffer[contador] == 'f')
    return 0;
  else
    return 1;
}

compara(origen, destino, bytes)
  char *origen, *destino;
  int bytes;
{
  while(bytes--)
    if(*origen++ != *destino++)
      return 0;
  return 1;
}

getmatlnum(mat, matnum)
  NAME mat;
  int *matnum;
{
  int i;
  for (i=0; i<mtlcount; ++i)
    if(compara(matl[i].mtype, mat, 32))
      *matnum = i;
}

gettexnum(tex, texnum)
  NAME tex;
  int *texnum;
{
       if(compara(tex, "SUAVE     ", 10)) *texnum = smooth;
  else if(compara(tex, "ALTERNADO ", 10)) *texnum = checker;
  else if(compara(tex, "ARENA     ", 10)) *texnum = grit;
  else if(compara(tex, "MARMOL    ", 10)) *texnum = marble;
  else if(compara(tex, "MADERA    ", 10)) *texnum = wood;
  else if(compara(tex, "PIEDRA    ", 10)) *texnum = sheetrock;
  else if(compara(tex, "PARTICULA ", 10)) *texnum = particle;
  else if(compara(tex, "ONDASMAR  ", 10)) *texnum = oceanwaves;
  else if(compara(tex, "ONDASAGUA ", 10)) *texnum = poolwaves;
  else if(compara(tex, "ONDAS     ", 10)) *texnum = waves;
  else if(compara(tex, "IMAGEN    ", 10)) *texnum = image;
}

loadrtheader()
{
  int t, cnt;

  xres = yres = scanxres = scanyres = 0;
  numberofframes = cnt = 0;
  while(cnt != 2) {
    loadline();
    if(compara(buf1, "RESOLUCION", 10)) {
      loadinteger(&scanxres);
      loadinteger(&scanyres);
      ++cnt;
    } else if(compara(buf1, "CUADROS   ", 10)) {
      loadinteger(&numberofframes);
      ++cnt;
    }
  }
}

float min(a,b)
  float a,b;
{
  if(a<b) return a;
  else return b;
}

float max(a,b)
  float a,b;
{
  if(a>b) return a;
  else return b;
}

float min3(a,b,c)
  float a,b,c;
{
  return min(min(a,b),c);
}

float max3(a,b,c)
  float a,b,c;
{
  return max(max(a,b),c);
}

float min4(a,b,c,d)
  float a,b,c,d;
{
  return min(min(a,b),min(c,d));
}

float max4(a,b,c,d)
  float a,b,c,d;
{
  return max(max(a,b),max(c,d));
}

vec(r, s, t, a)
  float r, s, t;
  TDA a;
{
  a[0] = r;
  a[1] = s;
  a[2] = t;
}

unvec(a, r, s, t)
  float *r, *s, *t;
  TDA a;
{
  *r = a[0];
  *s = a[1];
  *t = a[2];
}

float vecdot(a, b)
  TDA a,b;
{
  return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];
}

veccross(a, b, c)
  TDA a,b,c;
{
  c[0] = a[1]*b[2] - a[2]*b[1];
  c[1] = a[2]*b[0] - a[0]*b[2];
  c[2] = a[0]*b[1] - a[1]*b[0];
}

float veclen(a)
  TDA a;
{
  return sqrt(sqr(a[0])+sqr(a[1])+sqr(a[2]));
}

vecnormalize(a)
  TDA a;
{
  float dist;

  dist = 1.0 / veclen(a);
  a[0] *= dist;
  a[1] *= dist;
  a[2] *= dist;
}

vecsub(a,b,c)
  TDA a, b, c;
{
  c[0] = a[0] - b[0];
  c[1] = a[1] - b[1];
  c[2] = a[2] - b[2];
}

vecadd(a,b,c)
  TDA a,b,c;
{
  c[0] = a[0] + b[0];
  c[1] = a[1] + b[1];
  c[2] = a[2] + b[2];
}

vecadd3(a, b, c, d)
  TDA a, b, c, d;
{
  d[0] = a[0] + b[0] + c[0];
  d[1] = a[1] + b[1] + c[1];
  d[2] = a[2] + b[2] + c[2];
}

veclincomb(r, a, s, b, c)
  TDA a, b, c;
  float r, s;
{
  c[0] = r*a[0] + s*b[0];
  c[1] = r*a[1] + s*b[1];
  c[2] = r*a[2] + s*b[2];
}

vecscalmult(r, a, b)
  float r;
  TDA a, b;
{
  b[0] = r*a[0];
  b[1] = r*a[1];
  b[2] = r*a[2];
}

vecsmulti(r, a, b)
  float r;
  TDIA a;
  TDA b;
{
  b[0] = r*a[0];
  b[1] = r*a[1];
  b[2] = r*a[2];
}

vecsmint(r, a, b)
  float r;
  TDA a;
  TDIA b;
{
  b[0] = r*a[0];
  b[1] = r*a[1];
  b[2] = r*a[2];
}

vecaddscalmult(r, a, b, c)
  float r;
  TDA a, b, c;
{
  c[0] = r*a[0] + b[0];
  c[1] = r*a[1] + b[1];
  c[2] = r*a[2] + b[2];
}

vecelemmult(r, a, b, c)
  float r;
  TDA a, b, c;
{
  c[0] = r*a[0] * b[0];
  c[1] = r*a[1] * b[1];
  c[2] = r*a[2] * b[2];
}

vecmin(a, b, c)
  TDA a, b, c;
{
  if (a[0] < b[0])
    c[0] = a[0];
  else
    c[0] = b[0];
  if (a[1] < b[1])
    c[1] = a[1];
  else
    c[1] = b[1];
  if (a[2] < b[2])
    c[2] = a[2];
  else
    c[2] = b[2];
}

vecmax(a, b, c)
  TDA a, b, c;
{
  if (a[0] > b[0])
    c[0] = a[0];
  else
    c[0] = b[0];
  if (a[1] > b[1])
    c[1] = a[1];
  else
    c[1] = b[1];
  if (a[2] > b[2])
    c[2] = a[2];
  else
    c[2] = b[2];
}

vecnull(a)
  TDA a;
{
  a[0] = 0.0;
  a[1] = 0.0;
  a[2] = 0.0;
}

clearquadratic(list)
  QUADLIST *list;
{
  vecnull(list->baseloc);
  list->baserad = 0.0;
  list->based = 0.0;
  vecnull(list->apexloc);
  list->apexrad = 0.0;
  vecnull(list->u);
  vecnull(list->v);
  vecnull(list->w);
  list->height = 0.0;
  list->slope = 0.0;
  list->mind = 0.0;
  list->maxd = 0.0;
  list->insnrm = 0;
  list->mtlnum = 0;
  list->texnum = 0;
}

clearstr(str)
  NAME str;
{
  int i;

  for(i=0; i<32; ++i)
    str[i] = ' ';
}

clearmemory()
{
  int i;

  vecnull(loclwgt);
  vecnull(reflwgt);
  vecnull(tranwgt);
  vecnull(minwgt);
  vecnull(maxwgt);
  maxdepth = 0;
  lampreflects = 0;
  vecnull(lamprefl);
  disteffect = 0.0;
  focallength = 0.0;
  vecnull(obspos);
  obsrotate = 0.0;
  obstilt = 0.0;
  vecnull(viewdir);
  vecnull(viewu);
  vecnull(viewv);
  vecnull(horcol);
  vecnull(zencol);
  clouds = 0;
  skyexists = 0;
  vecnull(tile1);
  vecnull(tile2);
  tile = 0.0;
  ocwaveampl = 0.0;
  ocwavephase = 0.0;
  powaveampl = 0.0;
  powavephase = 0.0;
  powavexpos = 0.0;
  powaveypos = 0.0;
  waveampl = 0.0;
  wavephase = 0.0;
  wavexpos = 0.0;
  waveypos = 0.0;
  wavezpos = 0.0;
  groundexists = 0;
  bounsphrtest = 0;
  for(i = 0; i < maxmaterial; ++i) {
    clearstr(matl[i].mtype);
    clearstr(matl[i].textur);
    vecnull(matl[i].ambrfl);
    vecnull(matl[i].difrfl);
    vecnull(matl[i].spcrfl);
    matl[i].gloss = 0.0;
    vecnull(matl[i].trans);
    matl[i].index = 0.0;
  }
  for(i = 0; i < maxshapetype; ++i)
    objcnt[i] = 0;
  gnd.mtlnum = 0;
  gnd.texnum = 0;
  for(i = 0; i < maxlamp; ++i) {
    vecnull(lmp[i].loc);
    lmp[i].rad = 0.0;
    lmp[i].radsqr = 0.0;
    vecnull(lmp[i].intens);
  }
  for(i = 0; i < maxtriangle; ++i) {
    vecnull(tri[i].loc);
    vecnull(tri[i].v1);
    vecnull(tri[i].v2);
    vecnull(tri[i].norm);
    tri[i].ndotloc = 0.0;
    tri[i].mtlnum = 0;
    tri[i].texnum = 0;
  }
  for(i = 0; i < maxparal; ++i) {
    vecnull(para[i].loc);
    vecnull(para[i].v1);
    vecnull(para[i].v2);
    vecnull(para[i].norm);
    para[i].ndotloc = 0.0;
    para[i].mtlnum = 0;
    para[i].texnum = 0;
  }
  for(i = 0; i < maxcircles; ++i) {
    vecnull(cir[i].loc);
    vecnull(cir[i].v1);
    vecnull(cir[i].v2);
    vecnull(cir[i].norm);
    cir[i].ndotloc = 0.0;
    cir[i].radius = 0.0;
    cir[i].mtlnum = 0;
    cir[i].texnum = 0;
  }
  for(i = 0; i < maxring; ++i) {
    vecnull(rng[i].loc);
    vecnull(rng[i].v1);
    vecnull(rng[i].v2);
    vecnull(rng[i].norm);
    rng[i].ndotloc = 0.0;
    rng[i].rad1 = 0.0;
    rng[i].rad2 = 0.0;
    rng[i].mtlnum = 0;
    rng[i].texnum = 0;
  }
  for(i = 0; i < maxsphere; ++i) {
    vecnull(sphr[i].loc);
    sphr[i].rad = 0.0;
    sphr[i].radsqr = 0.0;
    sphr[i].mtlnum = 0;
    sphr[i].texnum = 0;
  }
  for(i = 0; i < maxcone; ++i)
    clearquadratic(&con[i]);
  for(i = 0; i < maxcylinder; ++i)
    clearquadratic(&cyl[i]);
}

float radians(angle)
  float angle;
{
  return angle*piover180;
}

getviewdir(angl, tilt, view, u, v)
  float angl, tilt;
  TDA view, u, v;
{
  float phi, theta, x, y, z;

  phi = radians(angl);
  theta = radians(tilt);
  x = cos(theta) * sin(phi);
  y = cos(theta) * cos(phi);
  z = -sin(theta);
  vec(x, y, z, view);
  x = cos(phi);
  y = -sin(phi);
  z = 0.0;
  vec(x, y, z, u);
  x = sin(theta) * sin(phi);
  y = sin(theta) * cos(phi);
  z = cos(theta);
  vec(x, y, z, v);
}

orientquad(list)
  QUADLIST *list;
{
  TDA temp;
  float rtmp;

  if (list->baserad<0.0 || list->apexrad<0.0) {
    list->baserad = abs(list->baserad);
    list->apexrad = abs(list->apexrad);
    list->insnrm = 1;
  } else
    list->insnrm = 0;
  vecsub(list->apexloc, list->baseloc, list->w);
  list->height = veclen(list->w);
  vecnormalize(list->w);
  list->slope = (list->apexrad - list->baserad) / list->height;
  list->based = -vecdot(list->baseloc, list->w);
  vec(0.0, 0.0, 1.0, temp);
  rtmp = abs(abs(vecdot(temp, list->w)) - 1.0);
  if (rtmp < small)
    vec(0.0, 1.0, 0.0, temp);
  veccross(list->w, temp, list->u);
  veccross(list->u, list->w, list->v);
  vecnormalize(list->u);
  vecnormalize(list->v);
  list->mind = vecdot(list->w, list->baseloc);
  list->maxd = vecdot(list->w, list->apexloc);
  if (list->maxd < list->mind) {
    rtmp = list->maxd;
    list->maxd = list->mind;
    list->mind = rtmp;
  }
}

veccopy(a, b)
  TDA a, b;
{
  b[0] = a[0];
  b[1] = a[1];
  b[2] = a[2];
}

getdataforframe()
{
  float radial, hgt;
  TDA shapeloc, temploc, vec1, vec2, vec3, pt1, pt2, pt3, pt4;
  int mtlnumber, texnumber;

  mtlcount = 0;
  while(1) {
    loadline();
    if(compara(buf1, "AMBIENTE  ", 10)) {
      loadtda(loclwgt);
      loadtda(reflwgt);
      loadtda(tranwgt);
      loadtda(minwgt);
      loadtda(maxwgt);
      loadinteger(&maxdepth);
    } else if(compara(buf1, "LAMPARAS  ", 10)) {
      lampreflects = loadboolean();
      loadtda(lamprefl);
      loadreal(&disteffect);
    } else if(compara(buf1, "OBSERVADOR", 10)) {
      loadreal(&focallength);
      loadtda(obspos);
      loadreal(&obsrotate);
      loadreal(&obstilt);
      getviewdir(obsrotate, obstilt, viewdir, viewu, viewv);
    } else if(compara(buf1, "CIELO     ", 10)) {
      loadtda(horcol);
      loadtda(zencol);
      clouds = loadboolean();
      skyexists = 1;
    } else if(compara(buf1, "MATERIAL  ", 10)) {
      loadtext(matl[mtlcount].mtype);
      loadtext(matl[mtlcount].textur);
      loadtda(matl[mtlcount].ambrfl);
      loadtda(matl[mtlcount].difrfl);
      loadtda(matl[mtlcount].spcrfl);
      loadreal(&matl[mtlcount].gloss);
      loadtda(matl[mtlcount].trans);
      loadreal(&matl[mtlcount].index);
      if(compara(matl[mtlcount].textur, "ALTERNADO ", 10)) {
        loadtda(tile1);
        loadtda(tile2);
        loadreal(&tile);
      } else if(compara(matl[mtlcount].textur, "ONDASMAR  ", 10)) {
        loadreal(&ocwaveampl);
        loadreal(&ocwavephase);
      } else if(compara(matl[mtlcount].textur, "ONDASAGUA ", 10)) {
        loadreal(&powaveampl);
        loadreal(&powavephase);
        loadreal(&powavexpos);
        loadreal(&powaveypos);
      } else if(compara(matl[mtlcount].textur, "ONDAS     ", 10)) {
        loadreal(&waveampl);
        loadreal(&wavephase);
        loadreal(&wavexpos);
        loadreal(&waveypos);
        loadreal(&wavezpos);
      }
      mtlcount++;
    } else if(compara(buf1, "PISO      ", 10)) {
      groundexists = 1;
      objcnt[ground]++;
      loadtext(mtlname);
      getmatlnum(mtlname, &gnd.mtlnum);
      gettexnum(matl[gnd.mtlnum].textur, &gnd.texnum);
    } else if(compara(buf1, "LAMPARA   ", 10)) {
      loadtda(lmp[objcnt[lamp]].loc);
      loadreal(&lmp[objcnt[lamp]].rad);
      lmp[objcnt[lamp]].radsqr = sqr(lmp[objcnt[lamp]].rad);
      loadtda(lmp[objcnt[lamp]].intens);
      objcnt[lamp]++;
    } else if(compara(buf1, "TRIANGULO ", 10)) {
      loadtda(tri[objcnt[triangle]].loc);
      loadtda(tri[objcnt[triangle]].v1);
      loadtda(tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      loadtext(mtlname);
      getmatlnum(mtlname, &tri[objcnt[triangle]].mtlnum);
      gettexnum(matl[tri[objcnt[triangle]].mtlnum].textur,
        &tri[objcnt[triangle]].texnum);
      objcnt[triangle]++;
    } else if(compara(buf1, "PARALELOGR", 10)) {
      loadtda(para[objcnt[paral]].loc);
      loadtda(para[objcnt[paral]].v1);
      loadtda(para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1, para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      loadtext(mtlname);
      getmatlnum(mtlname, &para[objcnt[paral]].mtlnum);
      gettexnum(matl[para[objcnt[paral]].mtlnum].textur,
        &para[objcnt[paral]].texnum);
      objcnt[paral]++;
    } else if(compara(buf1, "CIRCULO   ", 10)) {
      loadtda(cir[objcnt[circles]].loc);
      loadtda(cir[objcnt[circles]].v1);
      vecnormalize(cir[objcnt[circles]].v1);
      loadtda(cir[objcnt[circles]].v2);
      vecnormalize(cir[objcnt[circles]].v2);
      veccross(cir[objcnt[circles]].v1, cir[objcnt[circles]].v2,
        cir[objcnt[circles]].norm);
      vecnormalize(cir[objcnt[circles]].norm);
      cir[objcnt[circles]].ndotloc = vecdot(cir[objcnt[circles]].norm,
        cir[objcnt[circles]].loc);
      loadreal(&cir[objcnt[circles]].radius);
      loadtext(mtlname);
      getmatlnum(mtlname, &cir[objcnt[circles]].mtlnum);
      gettexnum(matl[cir[objcnt[circles]].mtlnum].textur,
        &cir[objcnt[circles]].texnum);
      objcnt[circles]++;
    } else if(compara(buf1, "ANILLO    ", 10)) {
      loadtda(rng[objcnt[ring]].loc);
      loadtda(rng[objcnt[ring]].v1);
      vecnormalize(rng[objcnt[ring]].v1);
      loadtda(rng[objcnt[ring]].v2);
      vecnormalize(rng[objcnt[ring]].v2);
      veccross(rng[objcnt[ring]].v1, rng[objcnt[ring]].v2,
        rng[objcnt[ring]].norm);
      vecnormalize(rng[objcnt[ring]].norm);
      rng[objcnt[ring]].ndotloc = vecdot(rng[objcnt[ring]].norm,
        rng[objcnt[ring]].loc);
      loadreal(&rng[objcnt[ring]].rad1);
      loadreal(&rng[objcnt[ring]].rad2);
      loadtext(mtlname);
      getmatlnum(mtlname, &rng[objcnt[ring]].mtlnum);
      gettexnum(matl[rng[objcnt[ring]].mtlnum].textur,
        &rng[objcnt[ring]].texnum);
      objcnt[ring]++;
    } else if(compara(buf1, "ESFERA    ", 10)) {
      loadtda(sphr[objcnt[sphere]].loc);
      loadreal(&sphr[objcnt[sphere]].rad);
      sphr[objcnt[sphere]].radsqr = sqr(sphr[objcnt[sphere]].rad);
      loadtext(mtlname);
      getmatlnum(mtlname, &sphr[objcnt[sphere]].mtlnum);
      gettexnum(matl[sphr[objcnt[sphere]].mtlnum].textur,
        &sphr[objcnt[sphere]].texnum);
      objcnt[sphere]++;
    } else if(compara(buf1, "CONO      ", 10)) {
      loadtda(con[objcnt[cone]].baseloc);
      loadreal(&con[objcnt[cone]].baserad);
      loadtda(con[objcnt[cone]].apexloc);
      loadreal(&con[objcnt[cone]].apexrad);
      orientquad(&con[objcnt[cone]]);
      loadtext(mtlname);
      getmatlnum(mtlname, &con[objcnt[cone]].mtlnum);
      gettexnum(matl[con[objcnt[cone]].mtlnum].textur,
        &con[objcnt[cone]].texnum);
      objcnt[cone]++;
    } else if(compara(buf1, "CILINDRO  ", 10)) {
      loadtda(cyl[objcnt[cylinder]].baseloc);
      loadreal(&cyl[objcnt[cylinder]].baserad);
      loadtda(cyl[objcnt[cylinder]].apexloc);
      cyl[objcnt[cylinder]].apexrad = cyl[objcnt[cylinder]].baserad;
      orientquad(&cyl[objcnt[cylinder]]);
      loadtext(mtlname);
      getmatlnum(mtlname, &cyl[objcnt[cylinder]].mtlnum);
      gettexnum(matl[cyl[objcnt[cylinder]].mtlnum].textur,
        &cyl[objcnt[cylinder]].texnum);
      objcnt[cylinder]++;
    } else if(compara(buf1, "CAJA      ", 10)) {
      loadtda(shapeloc);
      loadtda(vec1);
      loadtda(vec2);
      loadtda(vec3);
      loadtext(mtlname);
      getmatlnum(mtlname, &mtlnumber);
      gettexnum(matl[mtlnumber].textur, &texnumber);
      veccopy(shapeloc, para[objcnt[paral]].loc);
      veccopy(vec1, para[objcnt[paral]].v1);
      veccopy(vec3, para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1,
        para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].mtlnum = mtlnumber;
      para[objcnt[paral]].texnum = texnumber;
      objcnt[paral]++;
      veccopy(shapeloc, para[objcnt[paral]].loc);
      veccopy(vec3, para[objcnt[paral]].v1);
      veccopy(vec1, para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1, para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      vec(0.0, vec2[1], 0.0, temploc);
      vecadd(temploc, para[objcnt[paral]].loc, para[objcnt[paral]].loc);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].mtlnum = mtlnumber;
      para[objcnt[paral]].texnum = texnumber;
      objcnt[paral]++;
      veccopy(shapeloc, para[objcnt[paral]].loc);
      veccopy(vec3, para[objcnt[paral]].v1);
      veccopy(vec2, para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1, para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].mtlnum = mtlnumber;
      para[objcnt[paral]].texnum = texnumber;
      objcnt[paral]++;
      veccopy(shapeloc, para[objcnt[paral]].loc);
      veccopy(vec2, para[objcnt[paral]].v1);
      veccopy(vec3, para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1, para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      vec(vec1[0], 0.0, 0.0, temploc);
      vecadd(temploc, para[objcnt[paral]].loc,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].mtlnum = mtlnumber;
      para[objcnt[paral]].texnum = texnumber;
      objcnt[paral]++;
      veccopy(shapeloc, para[objcnt[paral]].loc);
      veccopy(vec2, para[objcnt[paral]].v1);
      veccopy(vec1, para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1, para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].mtlnum = mtlnumber;
      para[objcnt[paral]].texnum = texnumber;
      objcnt[paral]++;
      veccopy(shapeloc, para[objcnt[paral]].loc);
      veccopy(vec1, para[objcnt[paral]].v1);
      veccopy(vec2, para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1, para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      vec(0.0, 0.0, vec3[2], temploc);
      vecadd(temploc, para[objcnt[paral]].loc, para[objcnt[paral]].loc);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].mtlnum = mtlnumber;
      para[objcnt[paral]].texnum = texnumber;
      objcnt[paral]++;
    } else if(compara(buf1, "PIRAMIDE  ", 10)) {
      loadtda(shapeloc);
      loadtda(vec1);
      loadtda(vec2);
      loadreal(&hgt);
      loadtext(mtlname);
      getmatlnum(mtlname, &mtlnumber);
      gettexnum(matl[mtlnumber].textur, &texnumber);
      veccopy(shapeloc, para[objcnt[paral]].loc);
      veccopy(vec2, para[objcnt[paral]].v1);
      veccopy(vec1, para[objcnt[paral]].v2);
      veccross(para[objcnt[paral]].v1, para[objcnt[paral]].v2,
        para[objcnt[paral]].norm);
      vecnormalize(para[objcnt[paral]].norm);
      para[objcnt[paral]].ndotloc = vecdot(para[objcnt[paral]].norm,
        para[objcnt[paral]].loc);
      para[objcnt[paral]].mtlnum = mtlnumber;
      para[objcnt[paral]].texnum = texnumber;
      objcnt[paral]++;
      veccopy(shapeloc, tri[objcnt[triangle]].loc);
      veccopy(vec1, tri[objcnt[triangle]].v1);
      vec(0.5*vec1[0], 0.5*vec2[1], hgt, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
      tri[objcnt[triangle]].loc[0] = shapeloc[0] + vec1[0];
      tri[objcnt[triangle]].loc[1] = shapeloc[1] + vec2[1];
      tri[objcnt[triangle]].loc[2] = shapeloc[2];
      vecscalmult(-1.0, vec1, tri[objcnt[triangle]].v1);
      vec(-0.5 * vec1[0], -0.5 * vec2[1], hgt, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
      tri[objcnt[triangle]].loc[0] = shapeloc[0] + vec1[0];
      tri[objcnt[triangle]].loc[1] = shapeloc[1] + vec2[1];
      tri[objcnt[triangle]].loc[2] = shapeloc[2];
      vec(-0.5 * vec1[0], -0.5 * vec2[1], hgt, tri[objcnt[triangle]].v1);
      vecscalmult(-1.0, vec2, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
      veccopy(shapeloc, tri[objcnt[triangle]].loc);
      vec(0.5 * vec1[0], 0.5 * vec2[1], hgt, tri[objcnt[triangle]].v1);
      veccopy(vec2, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
    } else if(compara(buf1, "TETRA     ", 10)) {
      loadtda(shapeloc);
      loadreal(&hgt);
      loadreal(&radial);
      loadtext(mtlname);
      getmatlnum(mtlname, &mtlnumber);
      gettexnum(matl[mtlnumber].textur, &texnumber);
      vec(shapeloc[0], shapeloc[1], shapeloc[2] + hgt, pt1);
      vec(shapeloc[0], shapeloc[1] + radial, shapeloc[2],pt2);
      vec(shapeloc[0] - radial * 0.707, shapeloc[1] - radial * 0.707,
        shapeloc[2], pt3);
      vec(shapeloc[0] + radial * 0.707, shapeloc[1] - radial * 0.707,
        shapeloc[2], pt4);
      veccopy(pt3, tri[objcnt[triangle]].loc);
      vecsub(pt1, pt3, tri[objcnt[triangle]].v1);
      vecsub(pt2, pt3, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
      veccopy(pt2, tri[objcnt[triangle]].loc);
      vecsub(pt1, pt2, tri[objcnt[triangle]].v1);
      vecsub(pt4, pt2, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
      veccopy(pt4, tri[objcnt[triangle]].loc);
      vecsub(pt1, pt4, tri[objcnt[triangle]].v1);
      vecsub(pt3, pt4, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
      veccopy(pt3, tri[objcnt[triangle]].loc);
      vecsub(pt2, pt3, tri[objcnt[triangle]].v1);
      vecsub(pt4, pt3, tri[objcnt[triangle]].v2);
      veccross(tri[objcnt[triangle]].v1, tri[objcnt[triangle]].v2,
        tri[objcnt[triangle]].norm);
      vecnormalize(tri[objcnt[triangle]].norm);
      tri[objcnt[triangle]].ndotloc = vecdot(tri[objcnt[triangle]].norm,
        tri[objcnt[triangle]].loc);
      tri[objcnt[triangle]].mtlnum = mtlnumber;
      tri[objcnt[triangle]].texnum = texnumber;
      objcnt[triangle]++;
    } else if(compara(buf1, "PRUEBAESFE", 10)) {
      bounsphrtest = 1;
    } else if(compara(buf1, "FINCUADRO ", 10))
      break;
  }
}

calcdirofreflray(dir, srfnrm, reflray)
  TDA dir, srfnrm, reflray;
{
  float tmp;

  tmp = -2.0 * vecdot(dir, srfnrm);
  vecaddscalmult(tmp, srfnrm, dir, reflray);
}

vecnegate(a)
  TDA a;
{
  a[0] = -a[0];
  a[1] = -a[1];
  a[2] = -a[2];
}

calcdiroftranray(dir, srfnrm, mtl, tranray)
  TDA dir, srfnrm, tranray;
  int mtl;
{
  float ni, ndotv, nmult, lsint, ndott, nt;
  TDA cosv, sint, temp;

  ni = 1.0;
  vecnegate(dir);
  ndotv = vecdot(srfnrm, dir);
  nt = matl[mtl].index;
  if (ndotv > 0.0)
    nmult = ni/nt;
  else
    nmult = nt;
  vecscalmult(ndotv, srfnrm, cosv);
  vecsub(cosv, dir, temp);
  vecscalmult(nmult, temp, sint);
  lsint = vecdot(sint, sint);
  if (lsint >= 1.0)
    vecnull(tranray);
  else {
    ndott = sqrt(1.0 - lsint);
    if (ndotv < 0.0)
      ndott = -ndott;
    vecscalmult(ndott, srfnrm, temp);
    vecsub(sint, temp, tranray);
  }
}

quadsrfnrm(intrpt, srfnrm, list)
  TDA intrpt, srfnrm;
  QUADLIST *list;
{
  float t;
  TDA projpt;

  t = -(vecdot(intrpt, list->w) + list->based);
  vecaddscalmult(t, list->w, intrpt, projpt);
  vecsub(projpt, list->baseloc, srfnrm);
  vecnormalize(srfnrm);
  vecaddscalmult(-list->slope, list->w, srfnrm, srfnrm);
  vecnormalize(srfnrm);
  if (list->insnrm)
    vecnegate(srfnrm);
}

getsrfnrm(shp, obj, intrpt, srfnrm)
  int shp, obj;
  TDA intrpt, srfnrm;
{
  switch(shp) {
    case ground:
      vec(0.0, 0.0, 1.0, srfnrm);
      break;
    case lamp:
      vecsub(intrpt, lmp[obj].loc, srfnrm);
      vecnormalize(srfnrm);
      break;
    case triangle:
      veccopy(tri[obj].norm, srfnrm);
      break;
    case paral:
      veccopy(para[obj].norm, srfnrm);
      break;
    case circles:
      veccopy(cir[obj].norm, srfnrm);
      break;
    case ring:
      veccopy(rng[obj].norm, srfnrm);
      break;
    case sphere:
      vecsub(intrpt, sphr[obj].loc, srfnrm);
      vecnormalize(srfnrm);
      break;
    case cone:
      quadsrfnrm(intrpt, srfnrm, &con[obj]);
      break;
    case cylinder:
      quadsrfnrm(intrpt, srfnrm, &cyl[obj]);
      break;
  }
}

getintrpt(pt, dir, dist, intrpt)
  TDA pt, dir, intrpt;
  float dist;
{
  vecaddscalmult(dist, dir, pt, intrpt);
}

int evencrossings(sides)
  int sides;
{
  int i, j, crossings;

  crossings = 0;
  for(i = 0; i < sides; i++) {
    j = (i + 1) % sides;
    if ((gv[i] < 0.0 && gv[j] >= 0.0) ||
        (gv[j] < 0.0 && gv[i] >= 0.0)) {
      if (gu[i] >= 0.0 && gu[j] >= 0.0)
        crossings++;
      else {
        if (gu[i] >= 0.0 || gu[j] >= 0.0)
          if ((gu[i] - gv[i] * (gu[j] - gu[i]) /
              (gv[j] - gv[i])) > 0.0)
            ++crossings;
      }
    }
  }
  return (crossings % 2) == 0;
}

setuptriangle(p1, p2, obj)
  int p1, p2, obj;
{
  gu[0] = -delta[p1];
  gv[0] = -delta[p2];
  gu[1] = tri[obj].v1[p1] - delta[p1];
  gv[1] = tri[obj].v1[p2] - delta[p2];
  gu[2] = tri[obj].v2[p1] - delta[p1];
  gv[2] = tri[obj].v2[p2] - delta[p2];
}

setupparallelogram(p1, p2, obj)
  int p1, p2, obj;
{
  gu[0] = -delta[p1];
  gv[0] = -delta[p2];
  gu[1] = para[obj].v1[p1] - delta[p1];
  gv[1] = para[obj].v1[p2] - delta[p2];
  gu[2] = para[obj].v2[p1] + para[obj].v1[p1] - delta[p1];
  gv[2] = para[obj].v2[p2] + para[obj].v1[p2] - delta[p2];
  gu[3] = para[obj].v2[p1] - delta[p1];
  gv[3] = para[obj].v2[p2] - delta[p2];
}

float qintercheck()
{
  if (t1 <= small && t2 <= small)
    return -1.0;
  else {
    if (t1 > t2) {
      if (t2 < small) return t1;
    } else
      if (t1 > small) return t1;
    return t2;
  }
}

float quadshapes(list, pt, dir)
  QUADLIST *list;
  TDA pt, dir;
{
  TDA newpnt, newdir;
  float sqrslope;

  vecsub(pt, list->baseloc, temp);
  newpnt[0] = vecdot(temp, list->u);
  newpnt[1] = vecdot(temp, list->v);
  newpnt[2] = vecdot(temp, list->w);
  newdir[0] = vecdot(dir, list->u);
  newdir[1] = vecdot(dir, list->v);
  newdir[2] = vecdot(dir, list->w);
  sqrslope = sqr(list->slope);
  a = sqr(newdir[0]) + sqr(newdir[1]) - sqr(newdir[2]) * sqrslope;
  b = 2.0 * (newpnt[0] * newdir[0] + newpnt[1] * newdir[1] -
         newdir[2] * (newpnt[2] * sqrslope - list->baserad * list->slope));
  c = sqr(newpnt[0]) + sqr(newpnt[1]) - sqr(newpnt[2] * list->slope +
    list->baserad);
  if (a == 0.0) {
    if (b == 0.0)
      return -1.0;
    t2 = -c / b;
    if (t2 < small)
      return -1.0;
    t1 = -1.0;
  } else {
    disc = sqr(b) - 4.0 * a * c;
    if (disc < 0.0)
      return -1.0;
    sroot = sqrt(disc);
    t = 1.0 / (a * 2.0);
    t1 = (-b - sroot) * t;
    t2 = (-b + sroot) * t;
    if (t1 < 0.0 && t2 < 0.0)
      return -1.0;
    if (t1 > t2) {
      t = t1;
      t1 = t2;
      t2 = t;
    }
  }
  if (t1 > small) {
    getintrpt(pt, dir, t1, intrpoint);
    d = vecdot(list->w, intrpoint);
    if (d >= list->mind && d <= list->maxd)
      return t1;
  }
  if (t2 > small) {
    getintrpt(pt, dir, t2, intrpoint);
    d = vecdot(list->w, intrpoint);
    if (d >= list->mind && d <= list->maxd)
      return t2;
  }
  return -1.0;
}

float intersect(pt, dir, shp, obj)
  TDA pt, dir;
  int shp, obj;
{
  float rad, dot, pos1, pos2;

  switch(shp) {
    case ground:
      if (dir[2] == 0.0)
        return -1.0;
      t = -pt[2] / dir[2];
      if (t > small)
        return t;
      return -1.0;
    case lamp:
      vecsub(lmp[obj].loc, pt, temp);
      b = vecdot(dir, temp) * -2.0;
      c = vecdot(temp, temp) - lmp[obj].radsqr;
      disc = sqr(b) - 4.0 * c;
      if (disc <= 0.0)
        return -1.0;
      sroot = sqrt(disc);
      t1 = (-b - sroot) * 0.5;
      t2 = (-b + sroot) * 0.5;
      return qintercheck();
    case triangle:
      dot = vecdot(tri[obj].norm, dir);
      if (abs(dot) < small)
        return -1.0;
      pos1 = tri[obj].ndotloc;
      pos2 = vecdot(tri[obj].norm, pt);
      t = (pos1 - pos2) / dot;
      getintrpt(pt, dir, t, intrpoint);
      vecsub(intrpoint, tri[obj].loc, delta);
      if (abs(tri[obj].norm[0]) > abs(tri[obj].norm[1]) &&
          abs(tri[obj].norm[0]) > abs(tri[obj].norm[2]))
        setuptriangle(1, 2, obj);
      else if (abs(tri[obj].norm[1]) >= abs(tri[obj].norm[2]))
        setuptriangle(0, 2, obj);
      else
        setuptriangle(0, 1, obj);
      if (evencrossings(3))
        return -1.0;
      else
        return t;
    case paral:
      dot = vecdot(para[obj].norm, dir);
      if (abs(dot) < small)
        return -1.0;
      pos1 = para[obj].ndotloc;
      pos2 = vecdot(para[obj].norm, pt);
      t = (pos1 - pos2) / dot;
      getintrpt(pt, dir, t, intrpoint);
      vecsub(intrpoint, para[obj].loc, delta);
      if (abs(para[obj].norm[0]) > abs(para[obj].norm[1]) &&
          abs(para[obj].norm[0]) > abs(para[obj].norm[2]))
        setupparallelogram(1, 2, obj);
      else if (abs(para[obj].norm[1]) >= abs(para[obj].norm[2]))
        setupparallelogram(0, 2, obj);
      else
        setupparallelogram(0, 1, obj);
      if (evencrossings(4))
        return -1.0;
      else
        return t;
    case circles:
      dot = vecdot(cir[obj].norm, dir);
      if (abs(dot) < small)
        return -1.0;
      pos1 = cir[obj].ndotloc;
      pos2 = vecdot(cir[obj].norm, pt);
      t = (pos1 - pos2) / dot;
      getintrpt(pt, dir, t, intrpoint);
      vecsub(intrpoint, cir[obj].loc, delta);
      rad = sqrt(vecdot(delta, delta));
      if (rad > cir[obj].radius)
        return -1.0;
      else
        return t;
    case ring:
      dot = vecdot(rng[obj].norm, dir);
      if (abs(dot) < small)
        return -1.0;
      pos1 = rng[obj].ndotloc;
      pos2 = vecdot(rng[obj].norm, pt);
      t = (pos1 - pos2) / dot;
      getintrpt(pt, dir, t, intrpoint);
      vecsub(intrpoint, rng[obj].loc, delta);
      rad = sqrt(vecdot(delta, delta));
      if (rad < rng[obj].rad1 || rad > rng[obj].rad2)
        return -1.0;
      else
        return t;
    case sphere:
      vecsub(sphr[obj].loc, pt, temp);
      b = vecdot(dir, temp) * -2.0;
      c = vecdot(temp, temp) - sphr[obj].radsqr;
      disc = sqr(b) - 4.0 * c;
      if (disc <= 0.0)
        return -1.0;
      sroot = sqrt(disc);
      t1 = (-b - sroot) * 0.5;
      t2 = (-b + sroot) * 0.5;
      return qintercheck();
    case cone:
      return quadshapes(&con[obj], pt, dir);
    case cylinder:
      return quadshapes(&cyl[obj], pt, dir);
  }
}

getinitialdir(i, j, dir)
  int i, j;
  TDA dir;
{
  float x, y;
  TDA eyetopixvec;

  x = (i - centerx) * xaspdivfoclen;
  y = (centery - j) * yaspdivfoclen;
  veclincomb(x, viewu, y, viewv, eyetopixvec);
  vecadd(viewvec, eyetopixvec, dir);
  vecnormalize(dir);
}

quadbound(list, minimum, maximum)
  QUADLIST *list;
  TDA minimum, maximum;
{
  TDA qmin, qmax;
  float maxrad;

  vecmin(list->baseloc, list->apexloc, qmin);
  vecmax(list->baseloc, list->apexloc, qmax);
  maxrad = max(list->baserad, list->apexrad);
  minimum[0] = qmin[0] - maxrad;
  minimum[1] = qmin[1] - maxrad;
  minimum[2] = qmin[2] - maxrad;
  maximum[0] = qmax[0] + maxrad;
  maximum[1] = qmax[1] + maxrad;
  maximum[2] = qmax[2] + maxrad;
}

bounboxes(shp, obj, minimum, maximum)
  int shp, obj;
  TDA minimum, maximum;
{
  TDA p2, p3, p4;

  switch(shp) {
    case triangle:
      vecadd(tri[obj].loc, tri[obj].v1, p2);
      vecadd(tri[obj].loc, tri[obj].v2, p3);
      minimum[0] = min3(tri[obj].loc[0], p2[0], p3[0]);
      minimum[1] = min3(tri[obj].loc[1], p2[1], p3[1]);
      minimum[2] = min3(tri[obj].loc[2], p2[2], p3[2]);
      maximum[0] = max3(tri[obj].loc[0], p2[0], p3[0]);
      maximum[1] = max3(tri[obj].loc[1], p2[1], p3[1]);
      maximum[2] = max3(tri[obj].loc[2], p2[2], p3[2]);
      break;
    case paral:
      vecadd(para[obj].loc, para[obj].v1, p2);
      vecadd(para[obj].loc, para[obj].v2, p3);
      vecadd3(para[obj].loc, para[obj].v1, para[obj].v2, p4);
      minimum[0] = min4(para[obj].loc[0], p2[0], p3[0], p4[0]);
      minimum[1] = min4(para[obj].loc[1], p2[1], p3[1], p4[1]);
      minimum[2] = min4(para[obj].loc[2], p2[2], p3[2], p4[2]);
      maximum[0] = max4(para[obj].loc[0], p2[0], p3[0], p4[0]);
      maximum[1] = max4(para[obj].loc[1], p2[1], p3[1], p4[1]);
      maximum[2] = max4(para[obj].loc[2], p2[2], p3[2], p4[2]);
      break;
    case circles:
      vec(-cir[obj].radius, -cir[obj].radius, cir[obj].radius, minimum);
      vec( cir[obj].radius,  cir[obj].radius, cir[obj].radius, maximum);
      vecadd(minimum, cir[obj].loc, minimum);
      vecadd(maximum, cir[obj].loc, maximum);
      break;
    case ring:
      vec(-rng[obj].rad2, -rng[obj].rad2, -rng[obj].rad2, minimum);
      vec( rng[obj].rad2,  rng[obj].rad2,  rng[obj].rad2, maximum);
      vecadd(minimum, rng[obj].loc, minimum);
      vecadd(maximum, rng[obj].loc, maximum);
      break;
    case sphere:
      vec(-sphr[obj].rad, -sphr[obj].rad, -sphr[obj].rad, minimum);
      vec( sphr[obj].rad,  sphr[obj].rad,  sphr[obj].rad, maximum);
      vecadd(minimum, sphr[obj].loc, minimum);
      vecadd(maximum, sphr[obj].loc, maximum);
      break;
    case cone:
      quadbound(&con[obj], minimum, maximum);
      break;
    case cylinder:
      quadbound(&cyl[obj], minimum, maximum);
      break;
  }
}

getminandmaxpoints(minimum, maximum)
  TDA minimum, maximum;
{
  int shapenum, objectnum;
  TDA minpt, maxpt;

  vecnull(minpt);
  vecnull(maxpt);
  vecnull(minimum);
  vecnull(maximum);
  for (shapenum = 2; shapenum < maxshapetype; ++shapenum) {
    for (objectnum = 0; objectnum < objcnt[shapenum]; ++objectnum) {
      bounboxes(shapenum, objectnum, minpt, maxpt);
      if (shapenum == 2 && objectnum == 0) {
        veccopy(minpt, minimum);
        veccopy(maxpt, maximum);
      } else {
        vecmin(minpt, minimum, minimum);
        vecmax(maxpt, maximum, maximum);
      }
    }
  }
  vecsub(minimum, preccor, minimum);
  vecadd(maximum, preccor, maximum);
}

crbounsphr(sp)
  struct SPHERETYPE *sp;
{
  TDA minimum, maximum, temp;

  getminandmaxpoints(minimum, maximum);
  vecsub(maximum, minimum, temp);
  vecscalmult(0.5, temp, temp);
  vecadd(minimum, temp, sp->center);
  sp->radsqr = vecdot(temp, temp);
}

initbounobjects()
{
  if(bounsphrtest) {
    char *a;
    a = &bounsphere;
    crbounsphr(a);
  }
}

dobstest(start, dir)
  TDA start, dir;
{
  vecsub(bounsphere.center, start, temp);
  b = vecdot(dir, temp) * -2.0;
  c = vecdot(temp, temp) - bounsphere.radsqr;
  disc = sqr(b) - 4.0 * c;
  if(disc <= 0.0)
    hitbounsphere = 0;
  else {
    sroot = sqrt(disc);
    t1 = (-b - sroot) * 0.5;
    t2 = (-b + sroot) * 0.5;
    if (t1 <= small && t2 <= small)
      hitbounsphere = 0;
    else
      hitbounsphere = 1;
  }
}

intersecttest(start, dir, shp, obj, dist, objhit)
  TDA start, dir;
  int *shp, *obj, *objhit;
  float *dist;
{
  int shapenum, objectnum;
  float newdist;

  *objhit = 0;
  for (shapenum = 0; shapenum < maxshapetype; ++shapenum) {
    for (objectnum = 0; objectnum < objcnt[shapenum]; ++objectnum) {
      newdist = intersect(start, dir, shapenum, objectnum);
      if(newdist > small) {
        if(*dist == -1.0) {
          *objhit = 1;
          *dist = newdist;
          *shp = shapenum;
          *obj = objectnum;
        } else if(newdist < *dist) {
          *dist = newdist;
          *shp = shapenum;
          *obj = objectnum;
        }
      }
    }
  }
}

igroundtest(start, dir, shp, obj, dist, objhit)
  TDA start, dir;
  int *shp, *obj, *objhit;
  float *dist;
{
  float newdist;

  *objhit = 0;
  newdist = intersect(start, dir, ground, 0);
  if (newdist > small) {
    *objhit = 1;
    *dist = newdist;
    *shp = ground;
    *obj = 0;
  }
}

shootray(start, dir, shp, obj, dist, objhit)
  TDA start, dir;
  int *shp, *obj, *objhit;
  float *dist;
{
  *shp = -1;
  *obj = -1;
  *dist = -1.0;
  *objhit = 0;
  if (!bounsphrtest)
    intersecttest(start, dir, shp, obj, dist, objhit);
  else {
    dobstest(start, dir);
    if (hitbounsphere)
      intersecttest(start, dir, shp, obj, dist, objhit);
    else if (groundexists)
      igroundtest(start, dir, shp, obj, dist, objhit);
  }
}

initrand(seed)
  float seed;
{
  oldrand = seed;
}

float frac(x)
  float x;
{
  return x - trunc(x);
}

int randint(range)
  int range;
{
  oldrand = frac(423.1966*oldrand);
  return trunc(oldrand * range);
}

initnoise()
{
  int x, y, z, i, j, k;

  initrand(0.409842);
  for (x = 0; x < maxnoise; ++x)
    for (y = 0; y < maxnoise; ++y)
      for (z = 0; z < maxnoise; ++z) {
        noisematrix[x][y][z] = randint(12000);
        if(x == maxnoise - 1) i = 0; else i = x;
        if(y == maxnoise - 1) j = 0; else j = y;
        if(z == maxnoise - 1) k = 0; else k = z;
        noisematrix[x][y][z] = noisematrix[i][j][k];
      }
}

int noise(x, y, z)
  float x, y, z;
{
  float ox, oy, oz;
  int ix, iy, iz, p000, p001, p010, p011, p100, p101, p110, p111;
  int p00, p01, p10, p11;
  int p0, p1, d00, d01, d10, d11, d0, d1, d;

  x = abs(x);
  y = abs(y);
  z = abs(z);
  ix = trunc(x) % (maxnoise - 1);
  iy = trunc(y) % (maxnoise - 1);
  iz = trunc(z) % (maxnoise - 1);
  ox = frac(x);
  oy = frac(y);
  oz = frac(z);
  p000 = noisematrix[ix][iy][iz];
  p001 = noisematrix[ix][iy][iz+1];
  p010 = noisematrix[ix][iy+1][iz];
  p011 = noisematrix[ix][iy+1][iz+1];
  p100 = noisematrix[ix+1][iy][iz];
  p101 = noisematrix[ix+1][iy][iz+1];
  p110 = noisematrix[ix+1][iy+1][iz];
  p111 = noisematrix[ix+1][iy+1][iz+1];
  d00 = p100 - p000;
  d01 = p101 - p001;
  d10 = p110 - p010;
  d11 = p111 - p011;
  p00 = (trunc(d00 * ox)) + p000;
  p01 = (trunc(d01 * ox)) + p001;
  p10 = (trunc(d10 * ox)) + p010;
  p11 = (trunc(d11 * ox)) + p011;
  d0 = p10 - p00;
  d1 = p11 - p01;
  p0 = (trunc(d0 * oy)) + p00;
  p1 = (trunc(d1 * oy)) + p01;
  d = p1 - p0;
  return (trunc(d * oz)) + p0;
}

marbletex(pt, rgb)
  TDA pt, rgb;
{
  float i, d, x, y, z;

  unvec(pt, &x, &y, &z);
  x = x * 0.2;
  d = x + 0.0006 * noise(x, y*0.1, z*0.1);
  d = d * (trunc(d) % 25);
  i = 0.5 + 0.05 * abs(d - 10.0 - 20.0 * (trunc(d*0.05)));
  if (i > 1.0)
    i = 1.0;
  vec(i, i, i, rgb);
}

woodtex(pt, rgb)
  TDA pt, rgb;
{
  float i, d, x, y, z;

  unvec(pt, &x, &y, &z);
  x = x * 0.2;
  d = x + 0.0002 * noise(x, y*0.1, z*0.1);
  d = d * (trunc(d) % 25);
  i = 0.7 + 0.05 * abs(d - 10.0 - 20.0 * (trunc(d*0.05)));
  if (i > 1.0)
    i = 1.0;
  vec(i, i, i, rgb);
}

int texturenumbs(shp, obj)
  int shp, obj;
{
  switch(shp) {
    case ground:
      return gnd.texnum;
    case triangle:
      return tri[obj].texnum;
    case paral:
      return para[obj].texnum;
    case circles:
      return cir[obj].texnum;
    case ring:
      return rng[obj].texnum;
    case sphere:
      return sphr[obj].texnum;
    case cone:
      return con[obj].texnum;
    case cylinder:
      return cyl[obj].texnum;
  }
}

int materialnumbs(shp, obj)
  int shp, obj;
{
  switch(shp) {
    case ground:
      return gnd.mtlnum;
    case triangle:
      return tri[obj].mtlnum;
    case paral:
      return para[obj].mtlnum;
    case circles:
      return cir[obj].mtlnum;
    case ring:
      return rng[obj].mtlnum;
    case sphere:
      return sphr[obj].mtlnum;
    case cone:
      return con[obj].mtlnum;
    case cylinder:
      return cyl[obj].mtlnum;
  }
}

texture(intrpt, tex, texturing)
  TDA intrpt;
  int tex;
  TDA texturing;
{
  int x, y, z, rt;
  float lev, r;

  switch(tex) {
    case checker:
      x = round(abs(intrpt[0])*tile);
      y = round(abs(intrpt[1])*tile);
      z = round(abs(intrpt[2])*tile);
      if ((x + y + z) % 2)
        veccopy(tile1, texturing);
      else
        veccopy(tile2, texturing);
      break;
    case grit:
      rt = randint(32767);
      r = rt / 32768.0;
      lev = r * 0.2 + 0.8;
      vec(lev, lev, lev, texturing);
      break;
    case marble:
      marbletex(intrpt, texturing);
      break;
    case wood:
      woodtex(intrpt, texturing);
      break;
    case sheetrock:
      rt = randint(32767);
      r = rt / 32768.0;
      lev = r * 0.1 + 0.9;
      vec(lev, lev, lev, texturing);
      break;
    case particle:
      rt = randint(32767);
      r = rt / 32768.0;
      lev = r * 0.15 + 0.85;
      vec(lev, lev, lev, texturing);
      break;
    case image:
      x = round(intrpt[0]);
      y = round(intrpt[1]);
      texturing[0] = 1.0;
      texturing[1] = 1.0;
      texturing[2] = 1.0;
      break;
  }
}

int objhittransmissive(mtl)
  int mtl;
{
  if(matl[mtl].trans[0] == 0.0 &&
     matl[mtl].trans[1] == 0.0 &&
     matl[mtl].trans[2] == 0.0)
    return 0;
  else
    return 1;
}

getloclcol(shp, obj, dir, intrpt, srfnrm, dist, loclcol)
  int shp, obj;
  TDA dir, intrpt, srfnrm;
  float dist;
  TDA loclcol;
{
  int mtl, tex, src, objhit, hititself, shadshp, shadobj;
  TDA lmpdir, addition, total, spec, diff, temp, shadintrpt, shadsrfnrm;
  TDA colortexture;
  float intensfactor, lamb, c, glint, shaddist, alpha;

  if(shp == lamp) {
    if (disteffect == 0.0)
      intensfactor = 1.0;
    else
      intensfactor = (1.0 - disteffect) + disteffect * (-vecdot(srfnrm, dir) /
        sqrt(dist));
    vecscalmult(intensfactor, lmp[obj].intens, loclcol);
  } else {
    mtl = materialnumbs(shp, obj);
    veccopy(matl[mtl].ambrfl, total);
    for (src = 0; src < objcnt[lamp]; src++) {
      vecsub(lmp[src].loc, intrpt, lmpdir);
      vecnormalize(lmpdir);
      shootray(intrpt, lmpdir, &shadshp, &shadobj, &shaddist, &objhit);
      hititself = objhit && shadshp == lamp && shadobj == src;
      if(!objhit || hititself) {
        lamb = vecdot(srfnrm, lmpdir);
        if(lamb <= 0.0) {
          vecnull(spec);
          vecnull(diff);
        } else {
          vecelemmult(lamb, matl[mtl].difrfl, lmp[src].intens, diff);
          vecsub(lmpdir, dir, temp);
          vecnormalize(temp);
          c = vecdot(srfnrm, temp);
          if (c > 0.5) {
            glint = exp(matl[mtl].gloss * ln(c));
            vecelemmult(glint, matl[mtl].spcrfl, lmp[src].intens, spec);
          } else
            vecnull(spec);
        }
        vecadd(diff, spec, addition);
        vecadd(total, addition, total);
      } else if (shadshp != lamp) {
        mtl = materialnumbs(shadshp, shadobj);
        if (objhittransmissive(mtl)) {
          getintrpt(intrpt, lmpdir, shaddist, shadintrpt);
          getsrfnrm(shadshp, shadobj, shadintrpt, shadsrfnrm);
          alpha = 0.7 * abs(vecdot(lmpdir, shadsrfnrm)) + 0.2;
          vecelemmult(alpha, lmp[src].intens, matl[mtl].trans, addition);
          vecadd(total, addition, total);
        }
        mtl = materialnumbs(shp,obj);
      }
    }
    tex = texturenumbs(shp, obj);
    if (tex == smooth || tex == oceanwaves || tex == poolwaves || tex == waves)
      veccopy(total, loclcol);
    else {
      texture(intrpt, tex, colortexture);
      vecelemmult(1.0, total, colortexture, loclcol);
    }
  }
}

sky(dir, col)
  TDA dir, col;
{
  float small2, sin2, cos2, x2, y2, z2;

  small2 = .001;
  x2 = sqr(dir[0]);
  y2 = sqr(dir[1]);
  z2 = sqr(dir[2]);
  if (z2 == 0)
    z2 = small2;
  sin2 = z2 / (x2 + y2 + z2);
  cos2 = 1.0 - sin2;
  veclincomb(cos2, horcol, sin2, zencol, col);
}

cloudysky(dir, skycol)
  TDA dir, skycol;
{
  float small2 = .001;
  float sin2, cos2;
  float x2, y2, z2;
  TDA col, col1, col2;
  float x, y, z, d, f;

  unvec(dir, &x, &y, &z);
  x2 = sqr(dir[0]);
  y2 = sqr(dir[1]);
  z2 = sqr(dir[2]);
  if(z2 == 0)
    z2 = small2;
  sin2 = z2 / (x2 + y2 + z2);
  cos2 = 1.0 - sin2;
  veclincomb(cos2, horcol, sin2, zencol, col);
  x *= 10.0;
  y *= 10.0;
  z *= 400.0;
  d = z + 0.0006*noise(x, y, z);
  d *= (float)(trunc(d) % 25);
  f = 0.06 * abs(d - 10.0 - 20.0 *(float) trunc(d * 0.05));
  if (f > 1.0)
    f = 1.0;
  vec(col[2], col[2], col[2], col1);
  veccopy(col, col2);
  veclincomb(f, col1, 1.0-f, col2, skycol);
}

comb(a, b, c, d, e, f, col)
  TDA a, b, c, d, e, f, col;
{
  TDA t1, t2, t3;

  vecelemmult(1.0, a, b, t1);
  vecelemmult(1.0, c, d, t2);
  vecelemmult(1.0, e, f, t3);
  vecadd3(t1, t2, t3, col);
}

int wgtmin(totwgt)
  TDA totwgt;
{
  if(totwgt[0] <= minwgt[0] &&
     totwgt[1] <= minwgt[1] &&
     totwgt[2] <= minwgt[2])
    return 1;
  else
    return 0;
}

int materialspecular(shp, mtl)
  int shp, mtl;
{
  if(shp != lamp) {
    if(matl[mtl].spcrfl[0] == 0.0 &&
       matl[mtl].spcrfl[1] == 0.0 &&
       matl[mtl].spcrfl[2] == 0.0)
      return 0;
    else
      return 1;
  }
  return 1;
}

int materialtransmissive(shp, mtl)
  int shp, mtl;
{
  if(shp != lamp) {
    if(matl[mtl].trans[0] == 0.0 &&
       matl[mtl].trans[1] == 0.0 &&
       matl[mtl].trans[2] == 0.0)
      return 0;
    else
      return 1;
  }
  return 0;
}

traceray(start, dir, totwgt, depth, col)
  TDA start, dir, totwgt, col;
  int depth;
{
  TDA loclcol, reflcol, trancol, refldir, trandir, wgt, intrpt, srfnrm;
  int shp, obj, mtl, tex;
  TDA temp;
  int objhit, dummy;
  float dist, f1, f2, f3, f4, ampl, dampen, dampen2;

  shootray(start, dir, &shp, &obj, &dist, &objhit);
  if (objhit) {
    getintrpt(start, dir, dist, intrpt);
    getsrfnrm(shp, obj, intrpt, srfnrm);
    tex = texturenumbs(shp, obj);
    switch(tex) {
      case oceanwaves:
        f1 = sin(radians(intrpt[0] + intrpt[1] + ocwavephase));
        f2 = sin(radians(2.5 * intrpt[0] + intrpt[1] + ocwavephase));
        f3 = sin(radians(intrpt[0] + 1.7 * intrpt[1] + ocwavephase));
        f4 = sin(radians(1.5 * intrpt[0] + 4.1 * intrpt[1] + ocwavephase));
        ampl = ocwaveampl * (f1 + f2 + f3 + f4) * 0.25;
        srfnrm[0] += ampl;
        srfnrm[1] += ampl;
        vecnormalize(srfnrm);
        break;
      case poolwaves:
        dampen = sqrt(sqr(intrpt[0] - powavexpos) + sqr(intrpt[1] - powaveypos));
        dampen2 = dampen * .01;
        if (dampen2 < 1.0) dampen2 = 1.0;
        ampl = powaveampl * sin(radians(2.5 * dampen + powavephase)) / dampen2;
        srfnrm[0] += ampl;
        srfnrm[1] += ampl;
        vecnormalize(srfnrm);
        break;
      case waves:
        dampen = sqrt(sqr(intrpt[0] - wavexpos) +
                      sqr(intrpt[1] - waveypos) +
                      sqr(intrpt[2] - wavezpos));
        dampen2 = dampen * .01;
        if (dampen2 < 1.0) dampen2 = 1.0;
        ampl = waveampl * cos(radians(60.0*dampen+wavephase)) / dampen2;
        srfnrm[0] += ampl;
        srfnrm[1] += ampl;
        srfnrm[2] += ampl;
        vecnormalize(srfnrm);
        break;
    }
    if (shp == lamp && !lampreflects)
      getloclcol(shp, obj, dir, intrpt, srfnrm, dist, col);
    else {
      getloclcol(shp, obj, dir, intrpt, srfnrm, dist, loclcol);
      if (depth == maxdepth || wgtmin(totwgt))
        vecelemmult(1.0, loclcol, loclwgt, col);
      else {
        if (shp != lamp || (shp == lamp && lampreflects)) {
          mtl = materialnumbs(shp, obj);
          if(materialspecular(shp, mtl)) {
            calcdirofreflray(dir, srfnrm, refldir);
            vecelemmult(1.0, totwgt, reflwgt, wgt);
            traceray(intrpt, refldir, wgt, depth+1, reflcol);
            if(shp != lamp) {
              vecsub(onesvec, matl[mtl].trans, temp);
              vecelemmult(1.0, reflcol, temp, reflcol);
            }
          } else
            vecnull(reflcol);
          if(materialtransmissive(shp, mtl)) {
            calcdiroftranray(dir, srfnrm, mtl, trandir);
            shootray(intrpt, trandir, shp, obj, dist, dummy);
            getintrpt(intrpt, trandir, dist, intrpt);
            getsrfnrm(shp, obj, intrpt, srfnrm);
            calcdiroftranray(trandir, srfnrm, mtl, trandir);
            vecelemmult(1.0, totwgt, tranwgt, wgt);
            traceray(intrpt, trandir, wgt, depth+1, trancol);
            vecelemmult(1.0, trancol, matl[mtl].trans, trancol);
          } else
            vecnull(trancol);
        } else {
          vecnull(reflcol);
          vecnull(trancol);
        }
        if (shp == lamp && lampreflects) {
          vecsub(onesvec, lamprefl, temp);
          vecelemmult(1.0, temp, loclcol, loclcol);
          vecelemmult(1.0, lamprefl, reflcol, reflcol);
          vecadd(loclcol, reflcol, col);
        } else
          comb(loclcol, loclwgt, reflcol, reflwgt, trancol, tranwgt, col);
      }
    }
  } else {
    if(skyexists)
      if(clouds)
        cloudysky(dir, col);
      else
        sky(dir, col);
    else
      vecnull(col);
  }
}

precalculation()
{
  float scale;

  xaspdivfoclen = asp/focallength;
  yaspdivfoclen = 1.0/focallength;
  centerx = scanxres / 2;
  centery = scanyres / 2;
  scale = centerx;
  vecscalmult(scale, viewdir, viewvec);
}

initgraphics()
{
  modo_grafico();
}

clearscreen()
{
  initgraphics();
  xres = scanxres;
  yres = scanyres;
  asp = (4.0/3.0)*((float) yres/(float) xres);
}

plot(xp, yp, r, g, b)
  int xp, yp, r, g, b;
{
  dibuja_pixel(xp - centerx + 320, yp - centery + 240, r, g, b);
  fputc(r, salida);
  fputc(g, salida);
  fputc(b, salida);
}

scan()
{
  TDA initialdir, col;
  TDIA colr;
  int xp, yp, x, y;

  precalculation();
  for(yp = 0; yp < scanyres; yp++)
    for(xp = 0; xp < scanxres; xp++) {
      getinitialdir(xp, yp, initialdir);
      traceray(obspos, initialdir, maxwgt, 1, col);
      if(col[0] > 1.0) col[0] = 1.0;
      if(col[1] > 1.0) col[1] = 1.0;
      if(col[2] > 1.0) col[2] = 1.0;
      vecsmint(255.0, col, colr);
      plot(xp, yp, colr[0], colr[1], colr[2]);
    }
}

getscenefile()
{
  int contador, caract;
  char nombre[64];
  char nombre1[64];
  char nombre2[64];

  puts("Introduzca el nombre del archivo -> ");
  lee_linea(nombre, 64);
  strcpy(nombre1, nombre);
  strcat(nombre1, ".M3D");
  strcpy(nombre2, nombre);
  strcat(nombre2, ".PIX");
  entrada = fopen(nombre1, "r");
  salida = fopen(nombre2, "w");
}

strcpy(a, b)
  char *a, *b;
{
  while(*a++ = *b++) ;
}

strcat(a, b)
  char *a, *b;
{
  while (*a) ++a;
  strcpy (a , b);
}

title()
{
  puts("\x1b\x01\x1e\x0c");
}

exitgraphics()
{
  getchar();
  modo_texto();
  puts("\x1b\x01\x0a\x0c");
}

lee_linea(pos, tam) char *pos; int tam; {
  char *ahora;
  int car;
  ahora = pos;
  while(1) {
    car = getchar();
    if(car == 8) {
      if(ahora == pos) continue;
      putchar(8);
      --ahora;
      continue;
    } else if(car == 13) {
      puts("\r\n");
      *ahora = 0;
      return;
    } else {
      if(ahora == pos + tam - 1) continue;
      putchar(car);
      *ahora++ = car;
    }
  }
}

main()
{
  title();
  puts("Dibujo Fotorealista  Port by Oscar Toledo G. 1995\r\n\n");
  small = .1;
  preccor[0] =
  preccor[1] =
  preccor[2] = 1.0;
  onesvec[0] =
  onesvec[1] =
  onesvec[2] = 1.0;
  initnoise();
  getscenefile();
  loadrtheader();
  clearscreen();
  lastframenum = numberofframes;
  fputc(numberofframes, salida);
  fputc(numberofframes >> 8, salida);
  fputc(scanxres, salida);
  fputc(scanxres >> 8, salida);
  fputc(scanyres, salida);
  fputc(scanyres >> 8, salida);
  for(framenum = 1; framenum <= lastframenum; ++framenum) {
    clearmemory();
    getdataforframe();
    initbounobjects();
    scan();
  }
  fclose(entrada);
  fclose(salida);
  exitgraphics();
}
