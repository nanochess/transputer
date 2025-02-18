/*
 ** Inmos T805 (Transputer) emulator
 **
 ** (c) Copyright 2002-2025 Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Feb/01/2025. Based on my emulator written in 29K assembler (Apr/08/2002)
 ** Revision date: Feb/17/2025. Added support code for the raytracer and Julia sets.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>

/*
 ** The supported machine is a T805 homebrew board made by Oscar Toledo E. circa 1992
 **
 ** It connected to a host machine based on the Zilog Z280 processor.
 **
 ** This emulator replaces both the board and the Z280.
 */

/*
 ** The floating point support assumes the host machine works with IEEE floating point.
 **
 ** IEEE floating point is available almost universally in PC and ARM.
 ** Currently the emulator assumes little-endian order (same as Transputer).
 */

unsigned char memory[0x40000];

unsigned char channel0[0x10000];
int output_channel0;
int offset_channel0;
int bytes_channel0;

unsigned char boot_image[] = {
    0x24, 0xf2, 0x22, 0x20, 0x20, 0x20, 0x80, 0x23,
    0xfc, 0x22, 0xf9, 0x25, 0xf7, 0x29, 0xfc, 0x24,
    0xf2, 0x21, 0xf8, 0x24, 0xf2, 0x21, 0xfc, 0x24,
    0xf2, 0x24, 0xf2, 0xe9, 0x24, 0xf2, 0x24, 0xf2,
    0xea, 0x40, 0x25, 0xf4, 0x27, 0xfa, 0x27, 0xfb,
    0x40, 0xd0, 0x49, 0xd1, 0x70, 0x24, 0xf2, 0xfa,
    0x21, 0xf2, 0x10, 0x4a, 0x22, 0xf1, 0x24, 0xf2,
    0x24, 0x50, 0x24, 0xf2, 0x54, 0x20, 0x20, 0x20,
    0x40, 0xf7, 0x24, 0x9c, 0x2f, 0xff, /*0x60, 0x0c*/
};

/*
 ** Instead of integrating a graphic library, we simply make a framebuffer that
 ** will be saved into a file.
 */
#define X_WIDTH     640
#define Y_HEIGHT    480

int next_image;

unsigned int framebuffer[X_WIDTH * Y_HEIGHT];

/*
 ** Prototypes and variables.
 */
void start_emulation(unsigned int, unsigned int);

int pascal_mode;
int copy_argc;
char **copy_argv;
int next_file;
FILE *input;
FILE *output;

/*
 ** Main program
 */
int main(int argc, char *argv[])
{
    copy_argc = argc;
    copy_argv = argv;
    fprintf(stderr, "\n");
    fprintf(stderr, "Transputer emulator v0.2. Feb/17/2025\n");
    fprintf(stderr, "by Oscar Toledo G. https://nanochess.org/\n\n");
    if (sizeof(float) != 4)
        fprintf(stderr, "WARNING: float size isn't 4.\n");
    if (sizeof(double) != 8)
        fprintf(stderr, "WARNING: double size isn't 8.\n");
    if (argc == 1) {
        fprintf(stderr, "Usage: tem binary.prg [input.pas]\n\n");
        fprintf(stderr, "The use of a Pascal input file triggers a special mode\n");
        fprintf(stderr, "to display the source code on stderr, and the assembler\n");
        fprintf(stderr, "result on stdout.\n\n");
        fprintf(stderr, "The same mode is used to feed scene files into the raytracer.\n\n");
        exit(1);
    }
    if (argc == 2)
        pascal_mode = 0;
    else
        pascal_mode = 1;
/*    printf("Pascal_mode: %d\n", pascal_mode);*/
    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Couldn't open '%s'\n", argv[1]);
        exit(1);
    }
    fseek(input, 0, SEEK_END);
    offset_channel0 = 0;
    bytes_channel0 = ftell(input);
    if (bytes_channel0 > sizeof(channel0)) {
        fprintf(stderr, "Input image bigger than %ld bytes\n", sizeof(channel0));
        exit(1);
    }
    output_channel0 = bytes_channel0;
    fseek(input, 0, SEEK_SET);
    fread(channel0, 1, sizeof(channel0), input);
    fclose(input);
    
    next_file = 2;
    input = NULL;
    boot_image[61] |= (bytes_channel0 >> 12) & 0x0f;
    boot_image[62] |= (bytes_channel0 >>  8) & 0x0f;
    boot_image[63] |= (bytes_channel0 >>  4) & 0x0f;
    boot_image[64] |= (bytes_channel0      ) & 0x0f;
    memcpy(memory + 112, boot_image, sizeof(boot_image));
    start_emulation(0x80000070, 0x80000070);
    
    exit(0);
}

#define MEMORY_MASK  0x0003ffff

#define not_handled(a)  fprintf(stderr, "Unhandled instruction %04x at %08x\n", Oreg, Iptr);return
#define undocumented(a)  fprintf(stderr, "Undocumented instruction %04x at %08x\n", Oreg, Iptr);return

#define read_memory(addr) memory[(addr) & MEMORY_MASK]

#define write_memory(addr, byte) memory[(addr) & MEMORY_MASK] = (byte)

#define read_memory_32(addr) read_memory(addr) | (read_memory((addr) + 1) << 8) | (read_memory((addr) + 2) << 16) | (read_memory((addr) + 3) << 24)

#define write_memory_32(addr, word) memory[(addr) & MEMORY_MASK] = (word) & 0xff; \
memory[((addr) + 1) & MEMORY_MASK] = ((word) >> 8) & 0xff; \
memory[((addr) + 2) & MEMORY_MASK] = ((word) >> 16) & 0xff; \
memory[((addr) + 3) & MEMORY_MASK] = ((word) >> 24) & 0xff;

#define debug_execution(a)  fprintf(stderr, "Iptr=%08x A=%08x B=%08x C=%08x Wptr=%08x\n", Iptr, Areg, Breg, Creg, Wptr)

/*
 ** Handle IN
 */
void handle_input(unsigned int addr, unsigned int channel, unsigned int bytes)
{
    unsigned char *p1;
    unsigned char *p2;
    int c;
    
    /*    fprintf(stderr, "IN addr=%08x channel=%08x bytes=%08x\n", addr, channel, bytes);*/
    if ((int) bytes <= 0)
        return;
    /*        fprintf(stderr, "(available %08x)\n", bytes_channel0);*/
    if (pascal_mode == 0) {
        if (bytes_channel0 == 0) {
            char *p;
            
            if (!fgets((char *) channel0, sizeof(channel0) - 1, stdin))
                exit(1);
            p = (char *) channel0;
            while (*p) {
                if (*p == '\n')
                    *p = '\r';
                p++;
            }
            bytes_channel0 = strlen((char *) channel0);
            offset_channel0 = 0;
        }
    } else {
        if (bytes_channel0 == 0) {
            if (input == NULL) {
                if (next_file < copy_argc) {
                    input = fopen(copy_argv[next_file], "r");
                    if (input == NULL) {
                        fprintf(stderr, "Couldn't open '%s'\n", copy_argv[next_file]);
                        channel0[0] = 0x1a;
                        offset_channel0 = 0;
                        bytes_channel0 = 1;
                    } else {
                        c = fread(channel0, 1, sizeof(channel0), input);
                        fclose(input);
                        input = NULL;
                        p1 = channel0;
                        p2 = channel0;
                        while (p1 < channel0 + c) {
                            if (*p1 == '\r') {
                                p1++;
                                continue;
                            }
                            if (*p1 == '\n')
                                *p2++ = '\r';
                            else
                                *p2++ = *p1;
                            p1++;
                        }
                        offset_channel0 = 0;
                        bytes_channel0 = p2 - channel0;
#if 0
                        {
                            FILE *a;
                            
                            a = fopen("output.bin", "wb");
                            fwrite(channel0, 1, sizeof(channel0), a);
                            fclose(a);
                        }
#endif
                        
                    }
                    next_file++;
                } else {
                    channel0[0] = 0x1a;
                    offset_channel0 = 0;
                    bytes_channel0 = 1;
                }
            }
        }
    }
    if (bytes > bytes_channel0)
        bytes = bytes_channel0;
    while (bytes) {
        write_memory(addr, channel0[offset_channel0]);
        addr++;
        offset_channel0 = (offset_channel0 + 1) & 0xffff;
        bytes_channel0--;
        bytes--;
    }
}

char output_buffer[1024];
char *output_pointer;

char ansi[256];
char *p2 = NULL;

/*
 ** Process output
 */
void process_output(int c)
{
    if (p2 != NULL) {
        *p2++ = c;
        if (isalpha(c)) {  /* End of sequence */
            if (c == 'G') {
                memset(framebuffer, 0, sizeof(framebuffer));
            } else if (c == 'T') {
                FILE *a;
                char output_file[256];
                int c;
                int y;
                int x;
                unsigned char header[54];
                
                next_image++;
                sprintf(output_file, "image%03d.bmp", next_image);
                
                a = fopen(output_file, "wb");
                if (a == NULL) {
                    fprintf(stderr, "Unable to write output file \"%s\"\n", output_file);
                } else {
                    memset(header, 0, sizeof(header));
                    header[0x00] = 'B';     /* Header */
                    header[0x01] = 'M';
                    c = X_WIDTH * Y_HEIGHT * 3 + 54;
                    header[0x02] = c;       /* Complete size of file */
                    header[0x03] = c >> 8;
                    header[0x04] = c >> 16;
                    header[0x05] = c >> 24;
                    c = 54;
                    header[0x0a] = c;       /* Size of header plus palette */
                    c = 40;
                    header[0x0e] = c;       /* Size of header */
                    header[0x12] = X_WIDTH & 0xff;
                    header[0x13] = X_WIDTH >> 8;
                    header[0x16] = Y_HEIGHT & 0xff;
                    header[0x17] = Y_HEIGHT >> 8;
                    header[0x1a] = 0x01;    /* 1 plane */
                    header[0x1c] = 0x18;    /* 24 bits */
                    c = X_WIDTH * Y_HEIGHT * 3;
                    header[0x22] = c;       /* Complete size of file */
                    header[0x23] = c >> 8;
                    header[0x24] = c >> 16;
                    header[0x25] = c >> 24;
                    c = 0x0ec4;             /* 96 dpi */
                    header[0x26] = c;       /* X */
                    header[0x27] = c >> 8;
                    header[0x2a] = c;       /* Y */
                    header[0x2b] = c >> 8;
                    fwrite(header, 1, sizeof(header), a);
                    
                    for (y = Y_HEIGHT - 1; y >= 0; y--) {
                        for (x = 0; x < X_WIDTH; x++) {
                            header[0x00] = framebuffer[y * X_WIDTH + x];
                            header[0x01] = framebuffer[y * X_WIDTH + x] >> 8;
                            header[0x02] = framebuffer[y * X_WIDTH + x] >> 16;
                            fwrite(header, 1, 3, a);
                        }
                    }
                    fclose(a);
                }
            } else if (c == 'P') {
                int x;
                int y;
                int r;
                int g;
                int b;
                
#if 0
                {
                    static int debug;
                    FILE *a;
                    
                    a = fopen("pixel.txt", "a");
                    *p2 = '\0';
                        fprintf(a, "%s\n", ansi);
                    fclose(a);
                }
#endif
                p2 = ansi;
                if (*p2 == '[')
                    p2++;
                x = strtol(p2, &p2, 10);
                if (*p2 == ';')
                    p2++;
                y = strtol(p2, &p2, 10);
                if (*p2 == ';')
                    p2++;
                r = strtol(p2, &p2, 10);
                if (*p2 == ';')
                    p2++;
                g = strtol(p2, &p2, 10);
                if (*p2 == ';')
                    p2++;
                b = strtol(p2, &p2, 10);
                if (x >= 0 && x < X_WIDTH && y >= 0 && y < Y_HEIGHT)
                    framebuffer[y * X_WIDTH + x] = (r << 16) | (g << 8) | b;
            }
            p2 = NULL;
        }
    } else {
        if (c == 0x1b) {   /* ANSI sequence */
            p2 = ansi;
        } else {
            fputc(c, stdout);
        }
    }
}

/*
 ** Handle OUT
 */
void handle_output(unsigned int addr, unsigned int channel, unsigned int bytes)
{
    int c;
    char *p;
    
    if (bytes == 0)
        return;
    if (pascal_mode == 0) {
        while (bytes) {
            c = read_memory(addr);
            addr++;
            bytes--;
            process_output(c);
        }
        return;
    }
    while (bytes) {
        if (output_pointer == NULL)
            output_pointer = output_buffer;
        if (output_pointer >= output_buffer + sizeof(output_buffer)) {
            output_buffer[sizeof(output_buffer) - 1] = '\0';
            fprintf(stderr, "Error: Too much data unprocessed\n");
            for (c = 0; c < sizeof(output_buffer); c++)
                fprintf(stderr, "%02x", output_buffer[c]);
            exit(1);
        }
        c = read_memory(addr);
        *output_pointer++ = c;
        addr++;
        bytes--;
        if (output_pointer[-1] == '\n' || (output_buffer[0] == 0x1b && isalpha(output_pointer[-1]))) {
            c = output_pointer - output_buffer;
            p = output_buffer;
            output_pointer = NULL;
            if (output_buffer[0] == 0x01) {    /* Source code */
                c--;
                while (c--) {
                    if (*p == '\r')
                        fputc('\n', stderr);
                    else
                        fputc(*p, stderr);
                    p++;
                }
            } else if (output_buffer[0] == 0x02) { /* Error */
                c--;
                while (c--) {
                    if (*p == '\r')
                        fputc('\n', stderr);
                    else
                        fputc(*p, stderr);
                    p++;
                }
            } else if (output_buffer[0] == 0x03) { /* End of compilation (error) */
                fprintf(stderr, "Compilation error\n");
                exit(1);
            } else if (output_buffer[0] == 0x04) { /* End of compilation (All good) */
                fprintf(stderr, "Compilation successful!\n");
                exit(0);
            } else {
                while (c--) {
                    process_output(*p++);
                }
            }
        }
    }
}

/*
 ** Instruction table for debugging.
 */
char *instruction_table[] = {
    
    /*
     ** Basic instructions.
     */
    "j", "ldlp", "pfix", "ldnl", "ldc", "ldnlp", "nfix", "ldl",
    "adc", "call", "cj", "ajw", "eqc", "stl", "stnl", "opr",
    
    /*
     ** Basic operations.
     */
    "rev", "lb", "bsub", "endp", "diff", "add", "gcall", "in",
    "prod", "gt", "wsub", "out", "sub", "startp", "outbyte", "outword",
    
    /*
     ** Operations.
     */
    "seterr", "?", "resetch", "csub0", "?", "stopp", "ladd", "stlb",
    "sthf", "norm", "ldiv", "ldpi", "stlf", "xdble", "ldpri", "rem",
    "ret", "lend", "ldtimer", "?", "?", "?", "?", "?",
    "?", "testerr", "testpranal", "tin", "div", "?", "dist", "disc",
    "diss", "lmul", "not", "xor", "bcnt", "lshr", "lshl", "lsum",
    "lsub", "runp", "xword", "sb", "gajw", "savel", "saveh", "wcnt",
    "shr", "shl", "mint", "alt", "altwt", "altend", "and", "enbt",
    "enbc", "enbs", "move", "or", "csngl", "ccnt1", "talt", "ldiff",
    "sthb", "taltwt", "sum", "mul", "sttimer", "stoperr", "cword", "clrhalterr",
    "sethalterr", "testhalterr", "dup", "move2dinit",
    "move2dall", "move2dnonzero", "move2dzero", "?",
    "?", "?", "?", "unpacksn", "?", "?", "?", "?",
    "?", "?", "?", "?", "postnormsn", "roundsn", "?", "?",
    "?", "ldinf", "fmul", "cflerr", "crcword", "crcbyte", "bitcnt", "bitrevword",
    "bitrevnbits", "pop", "timerdisableh", "timerdisablel",
    "timerenableh", "timerenablel", "ldmemstartval", "?",
    "?", "wsubdb", "fpldnldbi", "fpchkerror",
    "fpstnldb", "?", "fpldnlsni", "fpadd",
    "fpstnlsn", "fpsub", "fpldnldb", "fpmul",
    "fpdiv", "?", "fpldnlsn", "fpremfirst",
    "fpremstep", "fpnan", "fpordered", "fpnotfinite",
    "fpgt", "fpeq", "fpi32tor32", "?",
    "fpi32tor64", "?", "fpb32tor64", "?",
    "fptesterr", "fprtoi32", "fpstnli32", "fpldzerosn",
    "fpldzerodb", "fpint", "?", "fpdup", "fprev", "?", "fpldnladddb", "?",
    "fpldnlmuldb", "?", "fpldnladdsn", "fpentry", "fpldnlmulsn", "?", "?", "?",
    "?", "break", "clrj0break", "setj0break", "testj0break", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    
    /*
     ** FPU microcode instructions
     */
    "?", "fpusqrtfirst", "fpusqrtstep", "fpusqrtlast",
    "fpurp", "fpurm", "fpurz", "fpur32tor64",
    "fpur64tor32", "fpuexpdec32", "fpuexpinc32", "fpuabs",
    "?", "fpunoround", "fpuchki32", "fpuchki64",
    "?", "fpudivby2", "fpumulby2", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "fpurn", "fpuseterr", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "fpuclearerr", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
    "?", "?", "?", "?", "?", "?", "?", "?",
};

/*
 ** For getting detailed execution trace.
 */
#define DEBUG       0

void read_memory_fp32(unsigned int addr, float *value)
{
    union {
        unsigned char data[4];
        float f;
    } v;
    v.data[0] = read_memory(addr);
    v.data[1] = read_memory(addr + 1);
    v.data[2] = read_memory(addr + 2);
    v.data[3] = read_memory(addr + 3);
    *value = v.f;
}

void read_memory_fp64(unsigned int addr, double *value)
{
    union {
        unsigned char data[8];
        double d;
    } v;
    v.data[0] = read_memory(addr);
    v.data[1] = read_memory(addr + 1);
    v.data[2] = read_memory(addr + 2);
    v.data[3] = read_memory(addr + 3);
    v.data[4] = read_memory(addr + 4);
    v.data[5] = read_memory(addr + 5);
    v.data[6] = read_memory(addr + 6);
    v.data[7] = read_memory(addr + 7);
    *value = v.d;
}

void write_memory_fp32(unsigned int addr, float value)
{
    union {
        unsigned char data[4];
        float f;
    } v;
    v.f = value;
    write_memory(addr, v.data[0]);
    write_memory(addr + 1, v.data[1]);
    write_memory(addr + 2, v.data[2]);
    write_memory(addr + 3, v.data[3]);
}

void write_memory_fp64(unsigned int addr, double value)
{
    union {
        unsigned char data[8];
        double d;
    } v;
    v.d = value;
    write_memory(addr, v.data[0]);
    write_memory(addr + 1, v.data[1]);
    write_memory(addr + 2, v.data[2]);
    write_memory(addr + 3, v.data[3]);
    write_memory(addr + 4, v.data[4]);
    write_memory(addr + 5, v.data[5]);
    write_memory(addr + 6, v.data[6]);
    write_memory(addr + 7, v.data[7]);
}

/*
 ** Start emulation
 */
void start_emulation(unsigned int Iptr, unsigned int Wptr)
{
    unsigned int Areg = 0;
    unsigned int Breg = 0;
    unsigned int Creg = 0;
    unsigned int Oreg = 0;
    unsigned int transputer_error = 0;
    unsigned int transputer_halterr = 0;
    unsigned int transputer_fperr = 0;
    enum {
        ROUND_MINUS, ROUND_NEAREST, ROUND_POSITIVE, ROUND_ZERO,
    } RoundMode = ROUND_NEAREST;
    unsigned int temp;
    struct {
        int type;
        union {
            float f;
            double d;
        } v;
    } fp[4];
    int byte;
#if DEBUG
    FILE *debug;
#endif
    
    fp[0].type = 0;
    fp[0].v.f = 0.0f;
    fp[1].type = 0;
    fp[1].v.f = 0.0f;
    fp[2].type = 0;
    fp[2].v.f = 0.0f;
    while (1) {
        byte = read_memory(Iptr);
#if DEBUG
        if ((byte & 0xf0) != 0x20 && (byte & 0xf0) != 0x60) {
            debug = fopen("debug.txt", "a");
            if (debug != NULL) {
                if ((byte & 0xf0) != 0xf0)
                    fprintf(debug, "Iptr=%08x A=%08x B=%08x C=%08x Wptr=%08x %s %d\n", Iptr, Areg, Breg, Creg, Wptr, instruction_table[byte >> 4], Oreg | (byte & 0x0f));
                else
                    fprintf(debug, "Iptr=%08x A=%08x B=%08x C=%08x Wptr=%08x %s\n", Iptr, Areg, Breg, Creg, Wptr, instruction_table[16 + (Oreg | (byte & 0x0f))]);
            }
            fclose(debug);
        }
#endif
        Iptr++;
        Oreg |= byte & 0x0f;
        switch (byte & 0xf0) {
            case 0x00:  /* j */
                Iptr += Oreg;
                Oreg = 0;
                break;
            case 0x10:  /* ldlp */
                Creg = Breg;
                Breg = Areg;
                Areg = Wptr + (Oreg << 2);
                Oreg = 0;
                break;
            case 0x20:  /* pfix */
                Oreg <<= 4;
                break;
            case 0x30:  /* ldnl */
                Areg = read_memory_32(Areg + (Oreg << 2));
                Oreg = 0;
                break;
            case 0x40:  /* ldc */
                Creg = Breg;
                Breg = Areg;
                Areg = Oreg;
                Oreg = 0;
                break;
            case 0x50:  /* ldnlp */
                /* Oreg ~= 3; */    /* Inmos says this happens, it isn't true */
                Areg = Areg + (Oreg << 2);
                Oreg = 0;
                break;
            case 0x60:  /* nfix */
                Oreg = ~Oreg << 4;
                break;
            case 0x70:  /* ldl */
                Creg = Breg;
                Breg = Areg;
                Areg = read_memory_32(Wptr + (Oreg << 2));
                Oreg = 0;
                break;
            case 0x80:  /* adc */
                temp = Areg + Oreg;
                /* If operands are same sign but result is different sign */
                if (((Areg ^ Oreg) & 0x80000000) == 0 && ((Areg ^ temp) & 0x80000000) != 0)
                    transputer_error = 1;
                Areg = temp;
                Oreg = 0;
                break;
            case 0x90:  /* call */
                Wptr -= 4;
                write_memory_32(Wptr, Creg);
                Wptr -= 4;
                write_memory_32(Wptr, Breg);
                Wptr -= 4;
                write_memory_32(Wptr, Areg);
                Wptr -= 4;
                write_memory_32(Wptr, Iptr);
                Iptr += Oreg;
                /*                fprintf(stderr, "Jumping to %08x\n", Iptr);*/
                Oreg = 0;
                break;
            case 0xa0:  /* cj */
                if (Areg == 0) {
                    Iptr += Oreg;
                } else {
                    Areg = Breg;
                    Breg = Creg;
                }
                Oreg = 0;
                break;
            case 0xb0:  /* ajw */
                Wptr += Oreg << 2;
                Oreg = 0;
                break;
            case 0xc0:  /* eqc */
                if (Areg == Oreg)
                    Areg = 1;
                else
                    Areg = 0;
                Oreg = 0;
                break;
            case 0xd0:  /* stl */
                write_memory_32(Wptr + (Oreg << 2), Areg);
                Areg = Breg;
                Breg = Creg;
                Oreg = 0;
                break;
            case 0xe0:  /* stnl */
                write_memory_32(Areg + (Oreg << 2), Breg);
                Areg = Creg;
                Oreg = 0;
                break;
            case 0xf0:  /* opr */
                switch (Oreg) {
                    case 0x00:  /* rev */
                        temp = Areg;
                        Areg = Breg;
                        Breg = temp;
                        break;
                    case 0x01:  /* lb */
                        Areg = read_memory(Areg);
                        break;
                    case 0x02:  /* bsub */
                        Areg = Areg + Breg;
                        Breg = Creg;
                        break;
                    case 0x03:  /* endp */
                        not_handled();
                        break;
                    case 0x04:  /* diff */
                        Areg = Breg - Areg;
                        Breg = Creg;
                        break;
                    case 0x05:  /* add */
                        temp = Breg + Areg;
                        /* If operands are same sign but result is different sign */
                        if (((Areg ^ Oreg) & 0x80000000) == 0 && ((Areg ^ temp) & 0x80000000) != 0)
                            transputer_error = 1;
                        Areg = temp;
                        Breg = Creg;
                        break;
                    case 0x06:  /* gcall */
                        temp = Iptr;
                        Iptr = Areg;
                        Areg = temp;
                        break;
                    case 0x07:  /* in */
                        handle_input(Creg, Breg, Areg);
                        break;
                    case 0x08:  /* prod */
                        Areg = Breg * Areg;
                        Breg = Creg;
                        break;
                    case 0x09:  /* gt */
                        /*
                         ** The book Transputer Assembly Language Programming
                         ** says this is unsigned. It isn't.
                         */
                        if ((int) Breg > (int) Areg)    /* Signed comparison */
                            Areg = 1;
                        else
                            Areg = 0;
                        Breg = Creg;
                        break;
                    case 0x0a:  /* wsub */
                        Areg = Areg + Breg * 4;
                        Breg = Creg;
                        break;
                    case 0x0b:  /* out */
                        handle_output(Creg, Breg, Areg);
                        break;
                    case 0x0c:  /* sub */
                        temp = Breg - Areg;
                        /* If operands are different sign but result is different sign than first operand */
                        if (((Areg ^ Oreg) & 0x80000000) != 0 && ((Areg ^ temp) & 0x80000000) != 0)
                            transputer_error = 1;
                        Areg = temp;
                        Breg = Creg;
                        break;
                    case 0x0d:  /* startp */
                        not_handled();
                        break;
                    case 0x12:  /* resetch */
                        temp = read_memory_32(Areg);
                        write_memory_32(Areg, 0x80000000);
                        Areg = temp;
                        break;
                    case 0x0e:  /* outbyte */
                    case 0x0f:  /* outword */
                    case 0x10:  /* seterr */
                    case 0x13:  /* csub0 */
                    case 0x15:  /* stopp */
                        not_handled();
                        break;
                    case 0x16:  /* ladd */
                    case 0x17:  /* stlb */
                        not_handled();
                        break;
                    case 0x18:  /* sthf */
                        /* !!! Do something (^^)! */
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x19:  /* norm */
                        not_handled();
                        break;
                    case 0x1b:  /* ldpi */
                        Areg += Iptr;
                        break;
                    case 0x1c:  /* stlf */
                        /* !!! Do something (^^)! */
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x1a:  /* ldiv */
                    case 0x1d:  /* xdble */
                    case 0x1e:  /* ldpri */
                        not_handled();
                        break;
                    case 0x1f:  /* rem */
                        if (Areg == 0 || Areg == -1 && Breg == 0x80000000) {
                            transputer_error = 1;
                        } else {
                            /* Important: Operation is signed */
                            Areg = (int) Breg % (int) Areg;
                        }
                        /* Sign is the same as dividend. In C this isn't portable */
                        if ((Breg ^ Areg) & 0x80000000)
                            Areg = -Areg;
                        Breg = Creg;
                        break;
                    case 0x20:  /* ret */
                        Iptr = read_memory_32(Wptr);
                        Wptr += 16;
                        break;
                    case 0x21:  /* lend */
                        Breg &= ~3;
                        /*                        fprintf(stderr, "Breg[0] = %08x Breg[1] = %08x\n", read_memory_32(Breg), read_memory_32(Breg + 4));*/
                        temp = read_memory_32(Breg + 4);
                        temp--;
                        write_memory_32(Breg + 4, temp);
                        if (temp != 0) {
                            temp = read_memory_32(Breg);
                            temp++;
                            write_memory_32(Breg, temp);
                            Iptr = Iptr - Areg;
                        }
                        break;
                    case 0x22:  /* ldtimer */
                        not_handled();
                        break;
                    case 0x29:  /* testerr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_error;
                        transputer_error = 0;
                        break;
                    case 0x2a:  /* testpranal */
                        not_handled();
                        break;
                    case 0x2b:  /* tin */
                        not_handled();
                        break;
                    case 0x2c:  /* div */
                        if (Areg == 0 || Areg == -1 && Breg == 0x80000000) {
                            transputer_error = 1;
                        } else {
                            /* Important: Operation is signed */
                            Areg = (int) Breg / (int) Areg;
                        }
                        Breg = Creg;
                        break;
                    case 0x2e:  /* dist */
                        not_handled();
                        break;
                    case 0x2f:  /* disc */
                        not_handled();
                        break;
                    case 0x30:  /* diss */
                        not_handled();
                        break;
                    case 0x32:  /* not */
                        Areg = ~Areg;
                        break;
                    case 0x33:  /* xor */
                        Areg = Breg ^ Areg;
                        Breg = Creg;
                        break;
                    case 0x31:  /* lmul */
                    case 0x34:  /* bcnt */
                    case 0x35:  /* lshr */
                    case 0x36:  /* lshl */
                    case 0x37:  /* lsum */
                    case 0x38:  /* lsub */
                    case 0x39:  /* runp */
                        not_handled();
                        break;
                    case 0x3a:  /* xword */
                        if (Breg < Areg) {
                            Areg = Breg;
                        } else {
                            Areg = Breg - Areg * 2;
                        }
                        Breg = Creg;
                        break;
                    case 0x3b:  /* sb */
                        write_memory(Areg, Breg);
                        Areg = Creg;
                        break;
                    case 0x3c:  /* gajw */
                        Areg &= ~3;
                        Wptr = Areg;
                        break;
                    case 0x3d:  /* savel */
                        not_handled();
                        break;
                    case 0x3e:  /* saveh */
                        not_handled();
                        break;
                    case 0x3f:  /* wcnt */
                        Creg = Breg;
                        Breg = Areg & 3;
                        Areg = Areg >> 2;
                        break;
                    case 0x40:  /* shr */
                        Areg = Breg >> Areg;
                        Breg = Creg;
                        break;
                    case 0x41:  /* shl */
                        Areg = Breg << Areg;
                        Breg = Creg;
                        break;
                    case 0x42:  /* mint */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = 0x80000000;
                        break;
                    case 0x43:  /* alt */
                        not_handled();
                        break;
                    case 0x44:  /* altwt */
                        not_handled();
                        break;
                    case 0x45:  /* altend */
                        not_handled();
                        break;
                    case 0x46:  /* and */
                        Areg = Breg & Areg;
                        Breg = Creg;
                        break;
                    case 0x47:  /* enbt */
                        not_handled();
                        break;
                    case 0x48:  /* enbc */
                        not_handled();
                        break;
                    case 0x49:  /* enbs */
                        not_handled();
                        break;
                    case 0x4a:  /* move */
                        while (Areg) {
                            write_memory(Breg, read_memory(Creg));
                            Creg++;
                            Breg++;
                            Areg--;
                        }
                        break;
                    case 0x4b:  /* or */
                        Areg = Breg | Areg;
                        Breg = Creg;
                        break;
                    case 0x4c:  /* csngl */
                    case 0x4d:  /* ccnt1 */
                    case 0x4e:  /* talt */
                        not_handled();
                        break;
                    case 0x4f:  /* ldiff */
                    case 0x50:  /* sthb */
                        not_handled();
                        break;
                    case 0x51:  /* taltwt */
                        not_handled();
                        break;
                    case 0x52:  /* sum */
                        Areg = Breg + Areg;
                        Breg = Creg;
                        break;
                    case 0x53:  /* mul */
                        Areg = Breg * Areg;
                        Breg = Creg;
                        break;
                    case 0x54:  /* sttimer */
                        /* !!! Do something (^^)! */
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x55:  /* stoperr */
                        not_handled();
                        break;
                    case 0x56:  /* cword */
                    case 0x57:  /* clrhalterr */
                        transputer_halterr = 0;
                        break;
                    case 0x58:  /* sethalterr */
                        transputer_halterr = 1;
                        break;
                    case 0x59:  /* testhalterr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_halterr;
                        break;
                    case 0x5a:  /* dup */
                        Creg = Breg;
                        Breg = Areg;
                        break;
                    case 0x5b:  /* move2dinit */
                        not_handled();
                        break;
                    case 0x5c:  /* move2dall */
                        not_handled();
                        break;
                    case 0x5d:  /* move2dnonzero */
                        not_handled();
                        break;
                    case 0x5e:  /* move2dzero */
                        not_handled();
                        break;
                    case 0x63:  /* unpacksn */
                        not_handled();
                        break;
                    case 0x6c:  /* postnormsn */
                        not_handled();
                        break;
                    case 0x6d:  /* roundsn */
                        not_handled();
                        break;
                    case 0x71:  /* ldinf */
                        not_handled();
                        break;
                    case 0x72:  /* fmul */
                        not_handled();
                        break;
                    case 0x73:  /* cflerr */
                        not_handled();
                        break;
                    case 0x74:  /* crcword */
                        not_handled();
                        break;
                    case 0x75:  /* crcbyte */
                        not_handled();
                        break;
                    case 0x76:  /* bitcnt */
                        not_handled();
                        break;
                    case 0x77:  /* bitrevword */
                        not_handled();
                        break;
                    case 0x78:  /* bitrevnbits */
                        not_handled();
                        break;
                    case 0x79:  /* pop */
                        temp = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        Creg = temp;
                        break;
                    case 0x7a:  /* timerdisableh */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x7b:  /* timerdisablel */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x7c:  /* timerenableh */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x7d:  /* timerenablel */
                        /* !!! Do something (^^)! */
                        break;
                    case 0x7e:  /* ldmemstartval */
                        not_handled();
                        break;
                    case 0x81:  /* wsubdb */
                        Areg = Areg + Breg * 8;
                        Breg = Creg;
                        break;
                    case 0x82:  /* fpldnldbi */
                        Areg = Areg + Breg * 8;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 1;
                        read_memory_fp64(Areg, &fp[0].v.d);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x83:  /* fpchkerr */
                        transputer_error |= transputer_fperr;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x84:  /* fpstnldb */
                        write_memory_fp64(Areg, fp[0].v.d);
                        fp[0] = fp[1];
                        fp[1] = fp[2];
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x86:  /* fpldnlsni */
                        Areg = Areg + Breg * 8;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 0;
                        read_memory_fp32(Areg, &fp[0].v.f);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x87:  /* fpadd */
                        if (fp[0].type == 0)
                            fp[0].v.f = fp[1].v.f + fp[0].v.f;
                        else
                            fp[0].v.d = fp[1].v.d + fp[0].v.d;
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x88:  /* fpstnlsn */
                        write_memory_fp32(Areg, fp[0].v.f);
                        fp[0] = fp[1];
                        fp[1] = fp[2];
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x89:  /* fpsub */
                        if (fp[0].type == 0)
                            fp[0].v.f = fp[1].v.f - fp[0].v.f;
                        else
                            fp[0].v.d = fp[1].v.d - fp[0].v.d;
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8a:  /* fpldnldb */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 1;
                        read_memory_fp64(Areg, &fp[0].v.d);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8b:  /* fpmul */
                        if (fp[0].type == 0)
                            fp[0].v.f = fp[1].v.f * fp[0].v.f;
                        else
                            fp[0].v.d = fp[1].v.d * fp[0].v.d;
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8c:  /* fpdiv */
                        if (fp[0].type == 0)
                            fp[0].v.f = fp[1].v.f / fp[0].v.f;
                        else
                            fp[0].v.d = fp[1].v.d / fp[0].v.d;
                        fp[1] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8e:  /* fpldnlsn */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 0;
                        read_memory_fp32(Areg, &fp[0].v.f);
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x8f:  /* fpremfirst */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x90:  /* fpremstep */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x91:  /* fpnan */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x92:  /* fpordered */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x93:  /* fpnotfinite */
                        not_handled();
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9c:  /* fptesterr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_fperr;
                        transputer_fperr = 0;
                        RoundMode = ROUND_NEAREST;
                       break;
                    case 0x94:  /* fpgt */
                        /* !!! Set transputer_fperr if operands are NaN or Inf */
                        Creg = Breg;
                        Breg = Areg;
                        if (fp[0].type == 0)
                            Areg = (fp[1].v.f > fp[0].v.f) ? 1 : 0;
                        else
                            Areg = (fp[1].v.d > fp[0].v.d) ? 1 : 0;
                        fp[0] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x95:  /* fpeq */
                        /* !!! Set transputer_fperr if operands are NaN or Inf */
                        Creg = Breg;
                        Breg = Areg;
                        if (fp[0].type == 0)
                            Areg = (fp[1].v.f == fp[0].v.f) ? 1 : 0;
                        else
                            Areg = (fp[1].v.d == fp[0].v.d) ? 1 : 0;
                        fp[0] = fp[2];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x96:  /* fpi32tor32 */
                        temp = read_memory_32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 0;
                        fp[0].v.f = (int) temp;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x98:  /* fpi32tor64 */
                        temp = read_memory_32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 1;
                        fp[0].v.d = (int) temp;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9a:  /* fpb32tor64 */
                        temp = read_memory_32(Areg);
                        Areg = Breg;
                        Breg = Creg;
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 1;
                        fp[0].v.d = temp;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9d:  /* fprtoi32 */
                        if (fp[0].type == 0)
                            fp[0].v.f = (int) fp[0].v.f;
                        else
                            fp[0].v.d = (int) fp[0].v.d;
                        /* !!! Validate or set transputer_fperr */
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9e:  /* fpstnli32 */
                        if (fp[0].type == 0) {
                            write_memory_32(Areg, (int) fp[0].v.f);
                        } else {
                            write_memory_32(Areg, (int) fp[0].v.d);
                        }
                        fp[0] = fp[1];
                        fp[1] = fp[2];
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0x9f:  /* fpldzerosn */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 0;
                        fp[0].v.f = 0.0f;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa0:  /* fpldzerodb */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        fp[0].type = 1;
                        fp[0].v.d = 0.0;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa1:  /* fpint */
                        switch (RoundMode) {
                            case ROUND_MINUS:
                                if (fp[0].type == 0) {
                                    fp[0].v.f = floor(fp[0].v.f);
                                } else {
                                    fp[0].v.d = floor(fp[0].v.d);
                                }
                                break;
                            case ROUND_POSITIVE:
                                if (fp[0].type == 0) {
                                    fp[0].v.f = ceil(fp[0].v.f);
                                } else {
                                    fp[0].v.d = ceil(fp[0].v.d);
                                }
                                break;
                            case ROUND_ZERO:
                                if (fp[0].type == 0) {
                                    if (fp[0].v.f < 0) {
                                        fp[0].v.f = -floor(-fp[0].v.f);
                                    } else {
                                        fp[0].v.f = floor(fp[0].v.f);
                                    }
                                } else {
                                    if (fp[0].v.d < 0) {
                                        fp[0].v.d = -floor(-fp[0].v.d);
                                    } else {
                                        fp[0].v.d = floor(fp[0].v.d);
                                    }
                                }
                                break;
                            case ROUND_NEAREST:
                                if (fp[0].type == 0) {
                                    if (fp[0].v.f < 0) {
                                        fp[0].v.f = ceil(fp[0].v.f - 0.5f);
                                    } else {
                                        fp[0].v.f = floor(fp[0].v.f + 0.5f);
                                    }
                                } else {
                                    if (fp[0].v.d < 0) {
                                        fp[0].v.d = ceil(fp[0].v.d - 0.5);
                                    } else {
                                        fp[0].v.d = floor(fp[0].v.d + 0.5);
                                    }
                                }
                                break;
                        }
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa3:  /* fpdup */
                        fp[2] = fp[1];
                        fp[1] = fp[0];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa4:  /* fprev */
                        fp[3] = fp[0];
                        fp[0] = fp[1];
                        fp[1] = fp[3];
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa6:  /* fpldnladddb */
                        fp[3].type = 1;
                        read_memory_fp64(Areg, &fp[3].v.d);
                        fp[0].v.d += fp[3].v.d;
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xa8:  /* fpldnlmuldb */
                        fp[3].type = 1;
                        read_memory_fp64(Areg, &fp[3].v.d);
                        fp[0].v.d *= fp[3].v.d;
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xaa:  /* fpldnladdsn */
                        fp[3].type = 0;
                        read_memory_fp32(Areg, &fp[3].v.f);
                        fp[0].v.f += fp[3].v.f;
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xac:  /* fpldnlmulsn */
                        fp[3].type = 0;
                        read_memory_fp32(Areg, &fp[3].v.f);
                        fp[0].v.f *= fp[3].v.f;
                        Areg = Breg;
                        Breg = Creg;
                        RoundMode = ROUND_NEAREST;
                        break;
                    case 0xab:  /* fpentry */
                        temp = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        switch (temp) {
                            case 0x01:  /* fpusqrtfirst */
                                /* Ignored */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x02:  /* fpusqrtstep */
                                /* Ignored */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x03:  /* fpusqrtlast */
                                if (fp[0].type == 0)
                                    fp[0].v.f = sqrt(fp[0].v.f);
                                else
                                    fp[0].v.d = sqrt(fp[0].v.d);
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x06:  /* fpurz */
                                RoundMode = ROUND_ZERO;
                                break;
                            case 0x22:  /* fpurn */
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x04:  /* fpurp */
                                RoundMode = ROUND_POSITIVE;
                                break;
                            case 0x05:  /* fpurm */
                                RoundMode = ROUND_MINUS;
                                break;
                            case 0x12:  /* fpumulby2 */
                                if (fp[0].type == 0)
                                    fp[0].v.f *= 2.0f;
                                else
                                    fp[0].v.d *= 2.0;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x11:  /* fpudivby2 */
                                if (fp[0].type == 0)
                                    fp[0].v.f /= 2.0f;
                                else
                                    fp[0].v.d /= 2.0;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x0b:  /* fpuabs */
                                if (fp[0].type == 0)
                                    fp[0].v.f = fabs(fp[0].v.f);
                                else
                                    fp[0].v.d = fabs(fp[0].v.d);
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x23:  /* fpuseterr */
                                transputer_fperr = 1;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x9c:  /* fpuclearerr */
                                transputer_fperr = 0;
                                RoundMode = ROUND_NEAREST;
                                break;
                            case 0x0e:  /* fpuchki32 */
                            case 0x0f:  /* fpuchki64 */
                            case 0x07:  /* fpur32tor64 */
                            case 0x08:  /* fpur64tor32 */
                            case 0x0d:  /* fpunoround */
                            case 0x0a:  /* fpuexpinc32 */
                            case 0x09:  /* fpuexpdec32 */
                                fprintf(stderr, "Unhandled fpu instruction %04x (%s) at %08x\n", temp, instruction_table[temp + 272], Iptr);
                                RoundMode = ROUND_NEAREST;
                                return;
                            default:
                                fprintf(stderr, "Undocumented fpu instruction %04x at %08x\n", temp, Iptr);
                                return;
                        }
                        break;
                    case 0xb1:  /* break */
                        not_handled();
                        break;
                    case 0xb2:  /* clrj0break */
                        not_handled();
                        break;
                    case 0xb3:  /* setj0break */
                        not_handled();
                        break;
                    case 0xb4:  /* testj0break */
                        not_handled();
                        break;
                    case 0x17c: /* lddevid */
                        not_handled();
                        break;
                    case 0xff:  /* Exit emulator :) */
                        return;
                    default:
                        undocumented();
                        break;
                }
                Oreg = 0;
                break;
        }
    }
}


