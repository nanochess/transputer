/*---------------------------------------------------------------------------

  unreduce.c

  The Reducing algorithm is actually a combination of two distinct algorithms.
  The first algorithm compresses repeated byte sequences, and the second al-
  gorithm takes the compressed stream from the first algorithm and applies a
  probabilistic compression method.

  ---------------------------------------------------------------------------*/

#define READBIT(n,d) {while(bk<(n)){bb|=((ulg)NEXTBYTE)<<bk;bk+=8;}\
(d)=bb&mask_bits[n];bb>>=(n);bk-=(n);}

/**************************************/
/*  UnReduce Defines, Typedefs, etc.  */
/**************************************/

#define DLE    144

typedef byte f_array[64];       /* for followers[256][64] */

static void LoadFollowers __((void));
/* void flush OF((unsigned));      /* routine from inflate.c */



/*******************************/
/*  UnReduce Global Variables  */
/*******************************/

   f_array *followers;     /* shared work space */

byte Slen[256];
int factor;

int L_table[5];

int D_shift[5];
int D_mask[5];

int B_table[256];


/*************************/
/*  Function unReduce()  */
/*************************/

void unReduce()   /* expand probabilistically reduced data */
{
    register int lchar = 0;
    int nchar;
    int ExState = 0;
    int V = 0;
    int Len = 0;
    longint s = tam_sin_comprimir;  /* number of bytes left to decompress */
    unsigned w = 0;      /* position in output window slide[] */
    unsigned u = 1;      /* true if slide[] unflushed */
    int a,b;

    bb = 0;
    bk = 0;
    L_table[0] = 0;
    L_table[1] = 0x7f;
    L_table[2] = 0x3f;
    L_table[3] = 0x1f;
    L_table[4] = 0x0f;

    D_shift[0] = 0;
    D_shift[1] = 0x07;
    D_shift[2] = 0x06;
    D_shift[3] = 0x05;
    D_shift[4] = 0x04;

    D_mask[0] = 0;
    D_mask[1] = 0x01;
    D_mask[2] = 0x03;
    D_mask[3] = 0x07;
    D_mask[4] = 0x0f;

    mask_bits[0] = 0;
    mask_bits[1] = 0x1;
    mask_bits[2] = 0x3;
    mask_bits[3] = 0x7;
    mask_bits[4] = 0xf;
    mask_bits[5] = 0x1f;
    mask_bits[6] = 0x3f;
    mask_bits[7] = 0x7f;
    mask_bits[8] = 0xff;
    mask_bits[9] = 0x1ff;
    mask_bits[10] = 0x3ff;
    mask_bits[11] = 0x7ff;
    mask_bits[12] = 0xfff;
    mask_bits[13] = 0x1fff;
    mask_bits[14] = 0x3fff;
    mask_bits[15] = 0x7fff;
    mask_bits[16] = 0xffff;

    a = 0;
    B_table[a++] = 8;
    for(b = 0; b < 2; b++)
      B_table[a++] = 1;
    for(b = 0; b < 2; b++)
      B_table[a++] = 2;
    for(b = 0; b < 4; b++)
      B_table[a++] = 3;
    for(b = 0; b < 8; b++)
      B_table[a++] = 4;
    for(b = 0; b < 16; b++)
      B_table[a++] = 5;
    for(b = 0; b < 32; b++)
      B_table[a++] = 6;
    for(b = 0; b < 64; b++)
      B_table[a++] = 7;
    for(b = 0; b < 127; b++)
      B_table[a++] = 8;

    followers = (f_array *) (ventana + 0x4000);

    factor = eb_central[6] - 1;
    LoadFollowers();

    while (s > 0 /* && (!zipeof) */) {
        if (Slen[lchar] == 0)
            READBIT(8, nchar)   /* ; */
        else {
            READBIT(1, nchar);
            if (nchar != 0)
                READBIT(8, nchar)       /* ; */
            else {
                int follower;
                int bitsneeded = B_table[Slen[lchar]];
                READBIT(bitsneeded, follower);
                nchar = followers[lchar][follower];
            }
        }
        /* expand the resulting byte */
        switch (ExState) {

        case 0:
            if (nchar != DLE) {
                s--;
                ventana[w++] = (byte) nchar;
                if (w == 0x4000) {
                    flush(w);
                    w = u = 0;
                }
            }
            else
                ExState = 1;
            break;

        case 1:
            if (nchar != 0) {
                V = nchar;
                Len = V & L_table[factor];
                if (Len == L_table[factor])
                    ExState = 2;
                else
                    ExState = 3;
            } else {
                s--;
                ventana[w++] = DLE;
                if (w == 0x4000)
                {
                  flush(w);
                  w = u = 0;
                }
                ExState = 0;
            }
            break;

        case 2:{
                Len += nchar;
                ExState = 3;
            }
            break;

        case 3:{
                register unsigned e;
                register unsigned n = Len + 3;
                register unsigned d = w - ((((V >> D_shift[factor]) &
                               D_mask[factor]) << 8) + nchar + 1);

                s -= n;
                do {
                  n -= (e = (e = 0x4000 - ((d &= 0x3fff) > w ? d : w)) > n ?
                        n : e);
                  if (u && w <= d)
                  {
                    memset(ventana + w, 0, e);
                    w += e;
                    d += e;
                  }
                  else
                    if (w - d < e)      /* (assume unsigned comparison) */
                      do {              /* slow to avoid memcpy() overlap */
                        ventana[w++] = ventana[d++];
                      } while (--e);
                    else
                    {
                      memcpy(ventana + w, ventana + d, e);
                      w += e;
                      d += e;
                    }
                  if (w == 0x4000)
                  {
                    flush(w);
                    w = u = 0;
                  }
                } while (n);

                ExState = 0;
            }
            break;
        }

        /* store character for next iteration */
        lchar = nchar;
    }

    /* flush out slide */
    flush(w);
}





/******************************/
/*  Function LoadFollowers()  */
/******************************/

static void LoadFollowers()
{
    register int x;
    register int i;

    for (x = 255; x >= 0; x--) {
        READBIT(6, Slen[x]);
        for (i = 0; (byte) i < Slen[x]; i++) {
            READBIT(8, followers[x][i]);
        }
    }
}
