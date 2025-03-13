/*---------------------------------------------------------------------------

  unshrink.c

  Shrinking is a Dynamic Lempel-Ziv-Welch compression algorithm with partial
  clearing.

  ---------------------------------------------------------------------------*/


#define MAX_BITS      13                 /* used in unShrink() */
#define HSIZE         (1 << MAX_BITS)    /* size of global work area */

   union work {
     struct {
       short *Prefix_of;      /* (8194 * sizeof(short)) */
       byte *Suffix_of;       /* also s-f length_nodes (smaller) */
       byte *Stack;           /* also s-f distance_nodes (smaller) */
     } shrink;
   };
   union work area;

#  define prefix_of area.shrink.Prefix_of
#  define suffix_of area.shrink.Suffix_of
#  define stack area.shrink.Stack


/*************************************/
/*  UnShrink Defines, Globals, etc.  */
/*************************************/

/*      MAX_BITS        13   (in unzip.h; defines size of global work area)  */
#define INIT_BITS       9
#define FIRST_ENT       257
#define CLEAR           256
#define GetCode(dest)   READBIT(codesize,dest)

static void partial_clear __((void));   /* local prototype */

int codesize, maxcode, maxcodemax, free_ent;




/*************************/
/*  Function unShrink()  */
/*************************/

void unShrink()
{
    register int code;
    register int stackp;
    int finchar;
    int oldcode;
    int incode;


    bb = 0;
    bk = 0;
    zipeof = 0;

    memset(ventana, 0, 32776);
    prefix_of = ventana;
    suffix_of = ventana + ((HSIZE + 2) * sizeof(short));
    stack = ventana + ((HSIZE + 2) * sizeof(short)) + HSIZE + 2;

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

    /* decompress the file */
    codesize = INIT_BITS;
    maxcode = (1 << codesize) - 1;
    maxcodemax = HSIZE;         /* (1 << MAX_BITS) */
    free_ent = FIRST_ENT;

    code = maxcodemax;
    do {
        prefix_of[code] = -1;
    } while (--code > 255);
/*
    OvdL: -Ox with SCO's 3.2.0 cc gives
    a. warning: overflow in constant multiplication
    b. segmentation fault (core dumped) when using the executable
    for (code = maxcodemax; code > 255; code--)
        prefix_of[code] = -1;
 */

    for (code = 255; code >= 0; code--) {
        prefix_of[code] = 0;
        suffix_of[code] = (byte) code;
    }

    GetCode(oldcode);
    if (zipeof) {
        return;
    }

    finchar = oldcode;

    fputc(finchar, archivo_nuevo);
    calcula_crc(&finchar, 1);

    stackp = HSIZE;

    while (!zipeof) {
        GetCode(code);
        if (zipeof) {
            return;
        }

        while (code == CLEAR) {
            GetCode(code);
            switch (code) {
                case 1:
                    codesize++;
                    if (codesize == MAX_BITS)
                        maxcode = maxcodemax;
                    else
                        maxcode = (1 << codesize) - 1;
                    break;

                case 2:
                    partial_clear();
                    break;
            }

            GetCode(code);
            if (zipeof) {
                return;
            }
        }


        /* special case for KwKwK string */
        incode = code;
        if (prefix_of[code] == -1) {
            stack[--stackp] = (byte) finchar;
            code = oldcode;
        }
        /* generate output characters in reverse order */
        while (code >= FIRST_ENT) {
            if (prefix_of[code] == -1) {
                stack[--stackp] = (byte) finchar;
                code = oldcode;
            } else {
                stack[--stackp] = suffix_of[code];
                code = prefix_of[code];
            }
        }

        finchar = suffix_of[code];
        stack[--stackp] = (byte) finchar;


        /* and put them out in forward order, block copy */
/*
        if ((HSIZE - stackp + outcnt) < OUTBUFSIZ) {
            memcpy(outptr, &stack[stackp], HSIZE - stackp);
            outptr += HSIZE - stackp;
            outcnt += HSIZE - stackp;
            stackp = HSIZE;
        }
        else
*/
        /* output byte by byte if we can't go by blocks */
            calcula_crc(stack + stackp, HSIZE - stackp);
            while (stackp < HSIZE) {
                if(fputc(stack[stackp++], archivo_nuevo) == -1) {
                    color(15);
                    puts(" Error de escritura\r\n");
                    return;
                }
            }

        /* generate new entry */
        code = free_ent;
        if (code < maxcodemax) {
            prefix_of[code] = oldcode;
            suffix_of[code] = (byte) finchar;

            do
                code++;
            while ((code < maxcodemax) && (prefix_of[code] != -1));

            free_ent = code;
        }
        /* remember previous code */
        oldcode = incode;
    }
}



/******************************/
/*  Function partial_clear()  */
/******************************/

static void partial_clear()
{
    register int pr;
    register int cd;

    /* mark all nodes as potentially unused */
    for (cd = FIRST_ENT; cd < free_ent; cd++)
        prefix_of[cd] |= 0x8000;

    /* unmark those that are used by other nodes */
    for (cd = FIRST_ENT; cd < free_ent; cd++) {
        pr = prefix_of[cd] & 0x7fff;    /* reference to another node? */
        if (pr >= FIRST_ENT)    /* flag node as referenced */
            prefix_of[pr] &= 0x7fff;
    }

    /* clear the ones that are still marked */
    for (cd = FIRST_ENT; cd < free_ent; cd++)
        if ((prefix_of[cd] & 0x8000) != 0)
            prefix_of[cd] = -1;

    /* find first cleared node as next free_ent */
    cd = FIRST_ENT;
    while ((cd < maxcodemax) && (prefix_of[cd] != -1))
        cd++;
    free_ent = cd;
}
