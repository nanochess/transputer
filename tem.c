/*
 ** Inmos T805 (Transputer) emulator
 **
 ** (c) Copyright 2002-2025 Oscar Toledo G.
 ** https://nanochess.org/
 **
 ** Creation date: Feb/01/2025. Based on my emulator written in 29K assembler (Apr/08/2002)
 ** Revision date: Feb/17/2025. Added support code for the raytracer and Julia sets.
 */

/*
 ** For getting detailed execution trace.
 */
#define DEBUG       0
#define DEBUG_DISK  1

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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <time.h>

#ifdef WIN32
void ttyinit(int fd)
{
}

void ttyrestore(int fd)
{
}

int getkey(int fd)
{
    if (!_kbhit())
        return 0;
    return _getch();
}
#else
/*
 ** Only POSIX (macOS and Linux)
 */
#include <unistd.h>
#include <termios.h>

struct termios tio_save;

void ttyinit(int fd)
{
    struct termios tio;
    
    tcgetattr(fd,&tio);
    tio_save = tio;
    
    cfmakeraw(&tio);
    tio.c_cc[VMIN] = 0;
    tio.c_cc[VTIME] = 0;
    
    tcsetattr(fd,TCSANOW,&tio);
}

void ttyrestore(int fd)
{
    
    tcsetattr(fd,TCSAFLUSH,&tio_save);
}

/*
 ** Special keys decoded for macOS
 */
int getkey(int fd)
{
    unsigned char buf[1];
    int len;
    
    len = read(fd,buf,1);
    
    if (len > 0) {
        len = buf[0];
        if (len == 127) {   /* Backspace */
            len = 8;
        } else if (len == 27) { /* Esc */
            len = read(fd,buf,1);
            if (len > 0) {
                if (buf[0] == 91) {
                    len = read(fd, buf, 1);
                    if (len > 0) {
                        if (buf[0] == 65) {
                            len = 0x18; /* Up */
                        } else if (buf[0] == 66) {
                            len = 0x12; /* Down */
                        } else if (buf[0] == 67) {
                            len = 0x16; /* Right */
                        } else if (buf[0] == 68) {
                            len = 0x14; /* Left */
                        } else if (buf[0] == 49) {
                            len = read(fd, buf, 1);
                            if (len > 0) {
                                if (buf[0] == 53) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 5;    /* F5 */
                                    }
                                } else if (buf[0] == 55) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 6;    /* F6 */
                                    }
                                } else if (buf[0] == 56) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 7;    /* F7 */
                                    }
                                } else if (buf[0] == 57) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 8;    /* F8 */
                                    }
                                }
                            }
                        } else if (buf[0] == 50) {
                            len = read(fd, buf, 1);
                            if (len > 0) {
                                if (buf[0] == 48) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 9;    /* F9 */
                                    }
                                } else if (buf[0] == 49) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 10;   /* F10 */
                                    }
                                } else if (buf[0] == 51) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 11;   /* F11 */
                                    }
                                } else if (buf[0] == 52) {
                                    len = read(fd, buf, 1);
                                    if (len > 0) {
                                        if (buf[0] == 126)
                                            len = 12;   /* F12 */
                                    }
                                }
                            }
                        }
                    }
                } else if (buf[0] == 79) {
                    len = read(fd, buf, 1);
                    if (len > 0) {
                        if (buf[0] == 80) {
                            len = 0x01; /* F1 */
                        } else if (buf[0] == 81) {
                            len = 0x02; /* F2 */
                        } else if (buf[0] == 82) {
                            len = 0x03; /* F3 */
                        } else if (buf[0] == 83) {
                            len = 0x04; /* F4 */
                        }
                    }
                }
            } else {
                len = 27;
            }
        }
    }
    
    return len;
}
#endif

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

int emulator_mode;
int copy_argc;
char **copy_argv;
int next_file;
FILE *input;
FILE *output;
char *disk_name;
FILE *disk;
char *harddisk_name;
FILE *harddisk;
char *cdrom_name;
FILE *cdrom;

unsigned char ram_disk[524288] = {
    0x24, 0x0e, 0x53, 0x4f, 0x4d, 0x33, 0x32, 0x20,
    0x76, 0x31, 0x2e, 0x30, 0xa7, 0xf1, 0xd9, 0x2a,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
    0x04, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x44, 0x69, 0x73, 0x63, 0x6f, 0x20, 0x52, 0x41,
    0x4d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
    0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0x12, 0x34,
    0xfd, 0xff, 0xff, 0xff};

FILE *debug;

/*
 ** Main program
 */
int main(int argc, char *argv[])
{
    int c;
    
    copy_argc = argc;
    copy_argv = argv;
    fprintf(stderr, "\n");
    fprintf(stderr, "Transputer emulator v0.2. Feb/22/2025\n");
    fprintf(stderr, "by Oscar Toledo G. https://nanochess.org/\n\n");
    if (sizeof(float) != 4)
        fprintf(stderr, "WARNING: float size isn't 4.\n");
    if (sizeof(double) != 8)
        fprintf(stderr, "WARNING: double size isn't 8.\n");
    if (argc == 1) {
        fprintf(stderr, "Usage: tem binary.prg [program.pas] [>program.len]\n");
        fprintf(stderr, "       tem m3d.prg [scene.m3d]\n");
        fprintf(stderr, "       tem -cc tc.cmg\n");
        fprintf(stderr, "       tem -os maestro.cmg disk.img\n\n");
        fprintf(stderr, "       tem -os2 maestro.cmg disk.img harddisk.img [cdrom.iso]\n\n");
        fprintf(stderr, "The use of a Pascal input file triggers a special mode\n");
        fprintf(stderr, "to display the source code on stderr, and the assembler\n");
        fprintf(stderr, "result on stdout.\n\n");
        fprintf(stderr, "The same mode is used to feed scene files into the raytracer.\n\n");
        fprintf(stderr, "The -cc mode is for compiling C files.\n\n");
        fprintf(stderr, "The -os mode is for executing my transputer operating system.\n\n");
        fprintf(stderr, "The -os2 mode is for executing my improved transputer operating system.\n\n");
        exit(1);
    }
    if (strcmp(argv[1], "-cc") == 0 || strcmp(argv[1], "-CC") == 0) {   /* C compiler */
        emulator_mode = 3;
        c = 2;
    } else if (strcmp(argv[1], "-os") == 0 || strcmp(argv[1], "-OS") == 0) {   /* Operating system mode */
        if (argc != 4) {
            fprintf(stderr, "Not enough argumentos for -os mode\n");
            exit(1);
        }
        disk_name = argv[3];
        disk = fopen(disk_name, "r+b");
        if (disk == NULL) {
            fprintf(stderr, "Couldn't open the disk image file '%s'\n", argv[3]);
            exit(1);
        }
        emulator_mode = 2;
        c = 2;
        ttyinit(0);
        fflush(stdout);
    } else if (strcmp(argv[1], "-os2") == 0 || strcmp(argv[1], "-OS2") == 0) {   /* Operating system mode */
        if (argc < 5) {
            fprintf(stderr, "Not enough argumentos for -os2 mode\n");
            exit(1);
        }
        disk_name = argv[3];
        disk = fopen(disk_name, "r+b");
        if (disk == NULL) {
            fprintf(stderr, "Couldn't open the disk image file '%s'\n", argv[3]);
            exit(1);
        }
        harddisk_name = argv[4];
        harddisk = fopen(harddisk_name, "r+b");
        if (harddisk == NULL) {
            fprintf(stderr, "Couldn't open the hard disk image file '%s'\n", argv[4]);
            exit(1);
        }
        if (argc > 5) {
            cdrom_name = argv[5];
            cdrom = fopen(cdrom_name, "rb");
            if (cdrom == NULL) {
                fprintf(stderr, "Couldn't open the CD-ROM image file '%s'\n", argv[5]);
                exit(1);
            }
        }
        emulator_mode = 4;
        c = 2;
        ttyinit(0);
        fflush(stdout);
    } else {
        if (argc == 2)
            emulator_mode = 0;
        else
            emulator_mode = 1;
        c = 1;
    }
    /*    printf("emulator_mode: %d\n", emulator_mode);*/
    input = fopen(argv[c], "rb");
    if (input == NULL) {
        fprintf(stderr, "Couldn't open '%s'\n", argv[c]);
        exit(1);
    }
    fseek(input, 0, SEEK_END);
    offset_channel0 = 0;
    bytes_channel0 = ftell(input);
    if (bytes_channel0 > sizeof(channel0)) {
        fprintf(stderr, "Boot image bigger than %ld bytes\n", sizeof(channel0));
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
    start_emulation(0x80000070, 1 | 0x80000070 + ((sizeof(boot_image) + 3) & ~3));
    
    exit(0);
}

/*
 ** Emit a debug message
 */
void debug_message(char *data)
{
    debug = fopen("debug.txt", "a");
    if (debug != NULL) {
        fprintf(debug, "%s", data);
        fclose(debug);
    }
}

#define MEMORY_MASK  0x0003ffff

#define not_handled(a)  fprintf(stderr, "Unhandled instruction %04x at %08x\n", Oreg, Iptr);return
#define undocumented(a)  fprintf(stderr, "Undocumented instruction %04x at %08x\n", Oreg, Iptr);return

#define read_memory(addr) memory[(addr) & MEMORY_MASK]

#define write_memory(addr, byte) memory[(addr) & MEMORY_MASK] = (byte)

#define read_memory_32(addr) (read_memory(addr) | (read_memory((addr) + 1) << 8) | (read_memory((addr) + 2) << 16) | (read_memory((addr) + 3) << 24))

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
    
#if 0
    fprintf(stderr, "IN addr=%08x channel=%08x bytes=%08x\n", addr, channel, bytes);
#endif
    if ((int) bytes <= 0)
        return;
    /*        fprintf(stderr, "(available %08x)\n", bytes_channel0);*/
    if (emulator_mode == 0) {   /* Simple mode */
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
    } else if (emulator_mode == 1) {    /* Pascal mode */
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
    } else if (emulator_mode == 2 || emulator_mode == 4) {    /* OS mode */
        //        fprintf(stderr, "Available data: $%08x\n", bytes_channel0);
    } else if (emulator_mode == 3) {    /* C compiler mode */
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

char output_buffer[2048];
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

int cursor_row = 1;
int cursor_column = 1;

/*
 ** Extract pseudo-hexadecimal
 */
int extract_hex(char *p)
{
    int c;
    int v;
    
    c = tolower(*p) - '0';
    v = c << 4;
    p++;
    c = tolower(*p) - '0';
    return v | c;
}

/*
 ** Handle OUT
 */
void handle_output(unsigned int addr, unsigned int channel, unsigned int bytes)
{
    int c;
    char *p;
    
#if 0
    fprintf(stderr, "OUT addr=%08x channel=%08x bytes=%08x\n", addr, channel, bytes);
#endif
    if (bytes == 0)
        return;
    if (emulator_mode == 0) {   /* Normal mode */
        while (bytes) {
            c = read_memory(addr);
            addr++;
            bytes--;
            process_output(c);
        }
    } else if (emulator_mode == 1) {    /* Pascal mode */
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
    } else if (emulator_mode == 2 || emulator_mode == 4) {    /* OS mode */
        int v;
        int v2;
        
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
        }
        if (output_pointer - output_buffer < 2)
            return;
        if (memcmp(output_buffer, "01", 2) == 0) {  /* Code 01: Read sector */
            unsigned int sector;
            
            if (emulator_mode == 2)
                c = 8;
            else
                c = 12;
            if (output_pointer - output_buffer < c)
                return;
#if DEBUG_DISK
            debug_message("Read sector: ");
            output_buffer[c] = '\n';
            output_buffer[c + 1] = 0;
            debug_message(output_buffer);
#endif
            if (emulator_mode == 2) {   /* Only access to floppy disk */
                sector = extract_hex(output_buffer + 2) * 36;    /* Track */
                sector += extract_hex(output_buffer + 4) * 18;    /* Head */
                sector += extract_hex(output_buffer + 6) - 1;    /* Sector */
                c = 0;
            } else {
                sector = extract_hex(output_buffer + 4) << 24;
                sector |= extract_hex(output_buffer + 6) << 16;
                sector |= extract_hex(output_buffer + 8) << 8;
                sector |= extract_hex(output_buffer + 10);
                c = extract_hex(output_buffer + 2); /* Unit */
            }
            if (c == 0) {   /* Floppy disk */
                if (sector >= (unsigned int) 2880) {    /* Not so many sectors */
                    channel0[0] = 1; /* All bad */
                    memset(&channel0[1], 0xfc, 512);
                } else {
                    fseek(disk, sector * 512, SEEK_SET);
                    fread(&channel0[1], 1, 512, disk);
                    channel0[0] = 0;  /* All good */
                }
            } else if (c == 1) {    /* RAM disk */
                if (sector >= (unsigned int) 1024) {    /* Not so many sectors */
                    channel0[0] = 1; /* All bad */
                    memset(&channel0[1], 0xfc, 512);
                } else {
                    memcpy(&channel0[1], &ram_disk[sector * 512], 512);
                }
            } else if (c == 2) {    /* Hard disk drive */
                if (harddisk == NULL || sector >= (unsigned int) (40 * 1048576 / 512)) {    /* Not so many sectors */
                    channel0[0] = 1; /* All bad */
                    memset(&channel0[1], 0xfc, 512);
                } else {
                    fseek(harddisk, sector * 512, SEEK_SET);
                    fread(&channel0[1], 1, 512, harddisk);
                    channel0[0] = 0;  /* All good */
                }
            } else if (c == 3) {    /* CD-ROM */
                if (cdrom == NULL || sector >= (unsigned int) (700 * 1048576 / 512)) {    /* Not so many sectors */
                    channel0[0] = 1; /* All bad */
                    memset(&channel0[1], 0xfc, 512);
                } else {
                    fseek(cdrom, sector * 512, SEEK_SET);
                    fread(&channel0[1], 1, 512, cdrom);
                    channel0[0] = 0;  /* All good */
                }
            } else {
                channel0[0] = 1; /* All bad */
                memset(&channel0[1], 0xfc, 512);
            }
            offset_channel0 = 0;
            bytes_channel0 = 513;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "02", 2) == 0) {  /* Code 02: Write sector */
            unsigned int sector;
            
            if (emulator_mode == 2)
                c = 1032;
            else
                c = 1036;
            if (output_pointer - output_buffer < c)
                return;
            if (emulator_mode == 2) {
                sector = extract_hex(output_buffer + 2) * 36;    /* Track */
                sector += extract_hex(output_buffer + 4) * 18;    /* Head */
                sector += extract_hex(output_buffer + 6) - 1;    /* Sector */
                c = 0;
            } else {
                sector = extract_hex(output_buffer + 4) << 24;
                sector |= extract_hex(output_buffer + 6) << 16;
                sector |= extract_hex(output_buffer + 8) << 8;
                sector |= extract_hex(output_buffer + 10);
                c = extract_hex(output_buffer + 2); /* Unit */
            }
            /*            fprintf(stderr, "Writing disk sector %d\n", c);*/
            if (c == 0) {   /* Floppy disk drive */
                if (sector >= (unsigned int) 2880) {    /* Not so many sectors */
                    channel0[0] = 1; /* All bad */
                } else {
                    char *p;
                    unsigned char *p2;
                    
                    fseek(disk, sector * 512, SEEK_SET);
                    if (emulator_mode == 2)
                        p = output_buffer + 8;
                    else
                        p = output_buffer + 12;
                    p2 = (unsigned char *) &output_buffer[0];
                    do {
                        *p2++ = extract_hex(p);
                        p += 2;
                    } while (p2 < (unsigned char *) output_buffer + 512);
                    fwrite(output_buffer, 1, 512, disk);
                    channel0[0] = 0;  /* All good */
                }
            } else if (c == 1) {    /* RAM disk */
                if (sector >= (unsigned int) 1024) {    /* Not so many sectors */
                    channel0[0] = 1; /* All bad */
                } else {
                    char *p;
                    unsigned char *p2;
                    
                    p = output_buffer + 12;
                    p2 = (unsigned char *) &ram_disk[sector * 512];
                    do {
                        *p2++ = extract_hex(p);
                        p += 2;
                    } while (p < output_buffer + 1036);
                    channel0[0] = 0;  /* All good */
                }
            } else if (c == 2) {    /* Hard disk drive */
                if (sector >= (unsigned int) (40 * 1048576 / 512)) {    /* Not so many sectors */
                    channel0[0] = 1; /* All bad */
                } else {
                    char *p;
                    unsigned char *p2;
                    
                    fseek(harddisk, sector * 512, SEEK_SET);
                    p = output_buffer + 12;
                    p2 = (unsigned char *) &output_buffer[0];
                    do {
                        *p2++ = extract_hex(p);
                        p += 2;
                    } while (p2 < (unsigned char *) output_buffer + 512);
                    fwrite(output_buffer, 1, 512, harddisk);
                    channel0[0] = 0;  /* All good */
                }
            } else if (c == 3) {    /* CD-ROM drive */
                channel0[0] = 1; /* All bad */
            } else {
                channel0[0] = 1; /* All bad */
            }
#if DEBUG_DISK
            debug_message("Write sector: ");
            output_buffer[8] = '\n';
            output_buffer[9] = 0;
            debug_message(output_buffer);
#endif
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "03", 2) == 0) {  /* Code 03: Format track (floppy disk) */
            if (output_pointer - output_buffer < 6)
                return;
            /* Code / Track / Head */
            channel0[0] = 0; /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "04", 2) == 0) {  /* Code 04: Update screen */
            char color[8] = {0, 4, 2, 6, 1, 5, 3, 7};
            
            if (output_pointer - output_buffer < 323)
                return;
#if DEBUG
            debug_message("Screen update: ");
            output_buffer[323] = '\n';
            output_buffer[324] = 0;
            debug_message(output_buffer);
#endif
#if 1
            fprintf(stdout, "\x1b[%d;%df", output_buffer[2] + 1, 1);
            c = -1;
            p = output_buffer + 3;
            while (p < output_buffer + 323) {
                v = extract_hex(p);
                v2 = extract_hex(p + 2);
                if (v2 != c) {
                    int something = 0;
                    
                    fprintf(stdout, "\x1b[");
                    if ((c & 0x88) != (v2 & 0x88)) {
                        fprintf(stdout, "0");
                        something = 1;
                        c = ~v2;
                    }
                    if (v2 & 8) {
                        if (something != 0)
                            fprintf(stdout, ";");
                        fprintf(stdout, "1");
                        something = 1;
                    }
                    if (v2 & 0x80) {
                        if (something != 0)
                            fprintf(stdout, ";");
                        fprintf(stdout, "5");
                        something = 1;
                    }
                    if ((c & 7) != (v2 & 7)) {
                        if (something != 0)
                            fprintf(stdout, ";");
                        fprintf(stdout, "%d", 30 + (color[v2 & 7]));
                        something = 1;
                    }
                    if ((c & 0x70) != (v2 & 0x70)) {
                        if (something != 0)
                            fprintf(stdout, ";");
                        fprintf(stdout, "%d", 40 + color[(v2 & 0x70) / 16]);
                        something = 1;
                    }
                    fprintf(stdout, "m");
                    
                    c = v2;
                }
                fputc(v, stdout);
                p += 4;
            }
            fprintf(stdout, "\x1b[%d;%dH", cursor_row, cursor_column);
            fflush(stdout);
#endif
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "05", 2) == 0) {  /* Code 05: Pressed key */
            int c;
            
            if (output_pointer - output_buffer < 2)
                return;
            channel0[0] = getkey(0);
#if DEBUG
            debug_message("Pressed key: ");
            output_buffer[2] = ' ';
            output_buffer[3] = channel0[0] ? channel0[0] : '#';
            output_buffer[4] = '\n';
            output_buffer[5] = '\0';
            debug_message(output_buffer);
#endif
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "06", 2) == 0) {  /* Code 06: Get time */
            time_t current_time;
            struct tm *time_data;
            
            if (output_pointer - output_buffer < 2)
                return;
#if DEBUG
            debug_message("Get time\n");
#endif
            time(&current_time);
            time_data = localtime(&current_time);
            channel0[0] = time_data->tm_sec;
            channel0[1] = time_data->tm_min;
            channel0[2] = time_data->tm_hour;
            channel0[3] = time_data->tm_mday;
            channel0[4] = time_data->tm_mon + 1;
            channel0[5] = time_data->tm_year;
            offset_channel0 = 0;
            bytes_channel0 = 6;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "07", 2) == 0) {  /* Code 07: Cursor position */
            int c;
            
            if (output_pointer - output_buffer < 6)
                return;
#if DEBUG
            debug_message("Cursor position: ");
            output_buffer[6] = '\n';
            output_buffer[7] = 0;
            debug_message(output_buffer);
#endif
            c = extract_hex(output_buffer + 2) | (extract_hex(output_buffer + 4) << 8);
            cursor_row = c / 80 + 1;
            cursor_column = c % 80 + 1;
            fprintf(stdout, "\x1b[%d;%dH", cursor_row, cursor_column);
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "08", 2) == 0) {  /* Code 08: Cursor shape */
            if (output_pointer - output_buffer < 6)
                return;
#if DEBUG
            debug_message("Cursor shape: ");
            output_buffer[6] = '\n';
            output_buffer[7] = 0;
            debug_message(output_buffer);
#endif
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "09", 2) == 0) {  /* Code 09: Printer */
            if (output_pointer - output_buffer < 4)
                return;
            /* Code / Byte for printer */
            channel0[0] = 0; /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "18", 2) == 0) {  /* Code 18: Read serial port */
            if (output_pointer - output_buffer < 2)
                return;
            channel0[0] = 0; /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "19", 2) == 0) {  /* Code 19: Write serial port */
            if (output_pointer - output_buffer < 4)
                return;
            /* Code / Byte for printer */
            channel0[0] = 0; /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "1:", 2) == 0) {  /* Code 1a: Read tape */
            if (output_pointer - output_buffer < 2)
                return;
            channel0[0] = 0; /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "1;", 2) == 0) {  /* Code 1b: Write tape */
            if (output_pointer - output_buffer < 4)
                return;
            /* Code / Byte for printer */
            channel0[0] = 0; /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        if (memcmp(output_buffer, "1<", 2) == 0) {  /* Code 1c: Rewind tape */
            if (output_pointer - output_buffer < 2)
                return;
            channel0[0] = 0; /* All good */
            offset_channel0 = 0;
            bytes_channel0 = 1;
            output_pointer = output_buffer;
            return;
        }
        fprintf(stderr, "Code %02x%02x detected\n", output_buffer[0], output_buffer[1]);
        exit(1);
    } else if (emulator_mode == 3) {    /* C compiler */
        int v;
        int v2;
        static FILE *list[10];
        int c;
        
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
        }
        if (output_pointer - output_buffer < 1)
            return;
        if (output_buffer[0] == 28) {   /* fopen */
            char filename[256];
            char mode[256];
            char *p;
            char *p2;
            
            p = (char *) &output_buffer[1];
            p2 = filename;
            while (1) {
                if (p >= output_pointer)    /* Incomplete, wait for more data */
                    return;
                if (*p == '\0') {
                    p++;
                    break;
                }
                if (p2 < filename + sizeof(filename) - 1)
                    *p2++ = *p;
                p++;
            }
            *p2 = '\0';
            p2 = mode;
            while (1) {
                if (p >= output_pointer)    /* Incomplete, wait for more data */
                    return;
                if (*p == '\0') {
                    p++;
                    break;
                }
                if (p2 < mode + sizeof(mode) - 1)
                    *p2++ = *p;
                p++;
            }
            *p2 = '\0';
            /*          fprintf(stderr, "Opening '%s' as '%s'\n", filename, mode);*/
            c = 1;
            while (c < 10) {
                if (list[c] == NULL) {
                    break;
                }
                c++;
            }
            if (c == 10) {
                c = 0;
            } else {
                list[c] = fopen(filename, mode);
                if (list[c] == NULL) {
                    c = 0;
                }
            }
            channel0[0] = c;
            channel0[1] = c >> 8;
            offset_channel0 = 0;
            bytes_channel0 = 2;
            output_pointer = output_buffer;
            return;
        }
        if (output_buffer[0] == 29) {   /* fclose */
            if (output_pointer < output_buffer + 2)
                return;
            c = output_buffer[1];
            if (c < 0 || c > 9) {
                c = 0;
            } else {
                if (list[c] != NULL) {
                    fclose(list[c]);
                    list[c] = NULL;
                }
                c = 0;
            }
            channel0[0] = c;
            channel0[1] = c >> 8;
            offset_channel0 = 0;
            bytes_channel0 = 2;
            output_pointer = output_buffer;
            return;
        }
        if (output_buffer[0] == 30) {   /* fputc */
            if (output_pointer < output_buffer + 3)
                return;
            c = output_buffer[1];
            if (c < 0 || c > 9) {
                c = EOF;
            } else {
                if (list[c] != NULL)
                    c = fputc(output_buffer[2], list[c]);
                else
                    c = EOF;
            }
            if (c == EOF)
                c = -1;
            channel0[0] = c;
            channel0[1] = c >> 8;
            offset_channel0 = 0;
            bytes_channel0 = 2;
            output_pointer = output_buffer;
            return;
        }
        if (output_buffer[0] == 31) {   /* fgetc */
            if (output_pointer < output_buffer + 2)
                return;
            c = output_buffer[1];
            if (c < 0 || c > 9) {
                c = 0;
            } else {
                if (list[c] != NULL) {
                    c = fgetc(list[c]);
                    if (c == EOF)
                        c = -1;
                } else {
                    c = 0;
                }
            }
            channel0[0] = c;
            channel0[1] = c >> 8;
            offset_channel0 = 0;
            bytes_channel0 = 2;
            output_pointer = output_buffer;
            return;
        }
        c = 0;
        while (c < output_pointer - output_buffer) {
            if (output_buffer[c] != '\r')
                fputc(output_buffer[c], stdout);
            c++;
        }
        output_pointer = output_buffer;
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

#define NotProcess_p    0x80000000

#define SaveRegisters() \
write_memory_32(0x8000002c, transputer_WdescReg); \
if (transputer_WdescReg != (NotProcess_p | 1)) { \
write_memory_32(0x80000030, Iptr); \
write_memory_32(0x80000034, Areg); \
write_memory_32(0x80000038, Breg); \
write_memory_32(0x8000003c, Creg); \
write_memory_32(0x80000040, transputer_StatusReg); \
}

#define RestoreRegisters() \
temp = read_memory_32(0x8000002c);\
UpdateWdescReg(temp);\
if (transputer_WdescReg != (NotProcess_p | 1)) { \
Iptr = read_memory_32(0x80000030);\
Areg = read_memory_32(0x80000034);\
Breg = read_memory_32(0x80000038);\
Creg = read_memory_32(0x8000003c);\
transputer_StatusReg = read_memory_32(0x80000040);\
}

#define Enqueue(ProcPtr, Fptr, Bptr) \
if (*(Fptr) == NotProcess_p) {\
*(Fptr) = ProcPtr;\
} else { \
write_memory_32(*(Bptr) - 8, ProcPtr);\
}       \
*(Bptr) = ProcPtr;

#define UpdateWdescReg(NewWdescReg) \
transputer_WdescReg = NewWdescReg; \
Wptr = transputer_WdescReg & ~3; \
transputer_priority = transputer_WdescReg & 1;

#define Dequeue(Level) \
UpdateWdescReg(transputer_FPtrReg##Level | Level); \
if (transputer_FPtrReg##Level == transputer_BPtrReg##Level) \
transputer_FPtrReg##Level = NotProcess_p; \
else \
transputer_FPtrReg##Level = read_memory_32(transputer_FPtrReg##Level - 8);

#define Run(ProcDesc) \
if (transputer_priority == 0) { \
Enqueue(ProcDesc & ~3, ProcDesc & 1 ? &transputer_FPtrReg1 : &transputer_FPtrReg0, ProcDesc & 1 ? &transputer_BPtrReg1 : &transputer_BPtrReg0); \
} else { \
if ((ProcDesc & 1) == 0) { \
SaveRegisters(); \
UpdateWdescReg(ProcDesc); \
transputer_StatusReg &= ErrorFlag | HaltOnErrorBit; \
ActivateProcess(); \
} else if (Wptr == NotProcess_p) { \
UpdateWdescReg(ProcDesc); \
ActivateProcess(); \
} else {\
Enqueue(ProcDesc & ~3, &transputer_FPtrReg1, &transputer_BPtrReg1); \
} \
}

#define ActivateProcess() \
Oreg = 0;\
Iptr = read_memory_32(Wptr - 4);

#define GotoSNPBit  0x02
#define ErrorFlag   0x80000000
#define HaltOnErrorBit 0x80

/*
 ** Channel input/output routines for local messaging.
 ** This is only a quick&dirty hack, and it doesn't follows the transputer hardware,
 */
#define channel_input(address, channel, bytes) \
if ((channel & 0xffffffe0) == 0x80000000) { \
handle_input(address, channel, bytes); \
} else { \
unsigned int procDesc = read_memory_32(channel); \
if (procDesc == NotProcess_p) { \
/*            fprintf(stderr, "IN: No data. Waiting as process $%08x in channel $%08x\n", transputer_WdescReg, channel);*/ \
write_memory_32(channel, transputer_WdescReg); \
write_memory_32(Wptr - 4, Iptr); \
write_memory_32(Wptr - 12, address); \
transputer_StatusReg |= GotoSNPBit; \
} else {\
unsigned int procPtr = procDesc & ~3;\
unsigned int source_address = read_memory_32(procPtr - 12);\
while(bytes--) {\
write_memory(address, read_memory(source_address));\
address++;\
source_address++;\
}\
write_memory_32(channel, NotProcess_p); \
/*            fprintf(stderr, "IN: Fulfilled. Jumping to process $%08x in channel $%08x\n", procDesc, channel);*/ \
Run(procDesc); \
}\
}

#define channel_output(address, channel, bytes) \
if ((channel & 0xffffffe0) == 0x80000000) { \
handle_output(address, channel, bytes); \
} else { \
unsigned int procDesc = read_memory_32(channel); \
if (procDesc == NotProcess_p) { \
/*fprintf(stderr, "OUT: No one to receive. Waiting as process $%08x in channel $%08x\n", transputer_WdescReg, channel);*/ \
write_memory_32(channel, transputer_WdescReg); \
write_memory_32(Wptr - 4, Iptr); \
write_memory_32(Wptr - 12, address); \
transputer_StatusReg |= GotoSNPBit; \
} else {\
unsigned int procPtr = procDesc & ~3;\
unsigned int target_address = read_memory_32(procPtr - 12);\
while (bytes--) { \
write_memory(target_address, read_memory(address));\
address++;\
target_address++;\
}\
write_memory_32(channel, NotProcess_p); \
/*fprintf(stderr, "OUT: Fulfilled. Jumping to process $%08x in channel $%08x\n", procDesc, channel);*/ \
Run(procDesc); \
}\
}

unsigned int transputer_ClockReg0 = 0;
unsigned int transputer_ClockReg1 = 0;
unsigned int local_count = 0;

/*
 ** Count timers
 */
void count_timers(void)
{
    local_count += 30; /* 30 ns per byte */
    if (local_count >= 1000) {   /* 1 microsecond */
        local_count -= 1000;
        transputer_ClockReg0++;
        if ((transputer_ClockReg0 & 0x3f) == 0) {
            transputer_ClockReg1++; /* One tick each 64 microseconds */
        }
    }
}

/*
 ** Start emulation
 */
void start_emulation(unsigned int Iptr, unsigned int WptrDesc)
{
    unsigned int Areg = 0;
    unsigned int Breg = 0;
    unsigned int Creg = 0;
    unsigned int Oreg = 0;
    unsigned int Wptr = 0;
    unsigned int transputer_error = 0;
    unsigned int transputer_halterr = 0;
    unsigned int transputer_fperr = 0;
    unsigned int transputer_priority = 0;   /* 0 for high-priority, 1 for low-priority */
    unsigned int transputer_FPtrReg0 = 0x80000000;
    unsigned int transputer_BPtrReg0 = 0x80000000;
    unsigned int transputer_CPtrReg0 = 0x80000000;
    unsigned int transputer_FPtrReg1 = 0x80000000;
    unsigned int transputer_BPtrReg1 = 0x80000000;
    unsigned int transputer_CPtrReg1 = 0x80000000;
    unsigned int transputer_WdescReg = 0;
    unsigned int transputer_StatusReg = 0;
    enum {
        ROUND_MINUS, ROUND_NEAREST, ROUND_POSITIVE, ROUND_ZERO,
    } RoundMode = ROUND_NEAREST;
    unsigned int temp;
    unsigned int temp2;
    struct {
        int type;
        union {
            float f;
            double d;
        } v;
    } fp[4];
    int byte;
    unsigned char word_buffer[4];
    int c;
    int completed;
    
    UpdateWdescReg(WptrDesc);
    write_memory_32(0x80000024, 0x80000000);
    write_memory_32(0x80000028, 0x80000000);
    fp[0].type = 0;
    fp[0].v.f = 0.0f;
    fp[1].type = 0;
    fp[1].v.f = 0.0f;
    fp[2].type = 0;
    fp[2].v.f = 0.0f;
    completed = 0;
    while (1) {
        count_timers();
        if (completed) {
            if ((transputer_StatusReg & GotoSNPBit) != 0) {
                transputer_StatusReg &= ~GotoSNPBit;
#if DEBUG
                debug_message("Trying process change\n");
#endif
                
                if (transputer_priority == 0) {
                    if (transputer_FPtrReg0 != NotProcess_p) {
#if DEBUG
                        debug_message("Process change: Next process in high-priority queue\n");
#endif
                        Dequeue(0);
                        ActivateProcess();
                    } else {
                        RestoreRegisters();
                        if (Wptr == NotProcess_p && transputer_FPtrReg1 != NotProcess_p) {
#if DEBUG
                            debug_message("Process change: Next process in low-priority queue\n");
#endif
                            Dequeue(1);
                            ActivateProcess();
                        } else {
#if DEBUG
                            debug_message("Process change: Registers restored from low-priority\n");
#endif
                        }
                    }
                } else if (transputer_FPtrReg1 != NotProcess_p) {
#if DEBUG
                    debug_message("Process change: Next process in low-priority queue\n");
#endif
                    Dequeue(1);
                    ActivateProcess();
                } else {
/*
                    temp = NotProcess_p | 1;
                    UpdateWdescReg(temp);
 */
                    while (1) {
                        if ((temp = read_memory_32(0x80000024)) != NotProcess_p && (int) (read_memory_32(temp - 20) - transputer_ClockReg0) < 0) {
#if DEBUG
                            {
                                char buffer[256];
                                
                                sprintf(buffer, "Process change: Timer (at %d, ClockReg0=%d)\n", read_memory_32(temp - 20), transputer_ClockReg0);
                                debug_message(buffer);
                            }
#endif
                            write_memory_32(0x80000024, read_memory_32(temp - 16));  /* Remove timer from list */
                            Run(temp);
                            break;
                        }
                        if ((temp = read_memory_32(0x80000028)) != NotProcess_p && (int) (read_memory_32(temp - 20) - transputer_ClockReg1) < 0) {
#if DEBUG
                            {
                                char buffer[256];
                                
                                sprintf(buffer, "Process change: Timer (at %d, ClockReg0=%d)\n", read_memory_32(temp - 20), transputer_ClockReg1);
                                debug_message(buffer);
                            }
#endif
                            write_memory_32(0x80000028, read_memory_32(temp - 16));  /* Remove timer from list */
                            Run(temp | 1);
                            break;
                        }
                        if (read_memory_32(0x80000024) == 0x80000000 && read_memory_32(0x80000028) == 0x80000000) {
#if DEBUG
                            debug_message("Process change: No more process nor timers, exiting emulator\n");
#endif
                            {
                                FILE *dump;
                                
                                fprintf(stderr, "Iptr=%08x A=%08x B=%08x C=%08x Wptr=%08x\n", Iptr, Areg, Breg, Creg, Wptr);
                                fprintf(stderr, "FPtrReg0=%08x\n", transputer_FPtrReg0);
                                fprintf(stderr, "FPtrReg1=%08x\n", transputer_FPtrReg1);
                                dump = fopen("memory.bin", "wb");
                                fwrite(memory, 1, sizeof(memory), dump);
                                fclose(dump);
                            }
                            exit(1);
                        }
                        count_timers();
                    }
                }
            } else {
                
                /*
                 ** Handle high-priority timer
                 ** The transputer has a long microcode here to handle timers.
                 ** This is oversimplified.
                 */
                if ((temp = read_memory_32(0x80000024)) != NotProcess_p && (int) (read_memory_32(temp - 20) - transputer_ClockReg0) < 0) {
#if DEBUG
                    {
                        char buffer[256];
                        
                        sprintf(buffer, "Process change: Timer (at %d, ClockReg0=%d)\n", read_memory_32(temp - 20), transputer_ClockReg0);
                        debug_message(buffer);
                    }
#endif
                    write_memory_32(0x80000024, read_memory_32(temp - 16));  /* Remove timer from list */
                    Run(temp);
                } else if ((temp = read_memory_32(0x80000028)) != NotProcess_p && (int) (read_memory_32(temp - 20) - transputer_ClockReg1) < 0) {
#if DEBUG
                    {
                        char buffer[256];
                        
                        sprintf(buffer, "Process change: Timer (at %d, ClockReg0=%d)\n", read_memory_32(temp - 20), transputer_ClockReg1);
                        debug_message(buffer);
                    }
#endif
                    write_memory_32(0x80000028, read_memory_32(temp - 16));  /* Remove timer from list */
                    Run(temp | 1);
                }
            }
        }
        
        byte = read_memory(Iptr);
        Iptr++;
        
#if DEBUG
        if ((byte & 0xf0) != 0x20 && (byte & 0xf0) != 0x60) {   /* Avoid prefixes */
            char buffer[256];
            
            if ((byte & 0xf0) != 0xf0)
                sprintf(buffer, "Iptr=%08x A=%08x B=%08x C=%08x Wptr=%08x %s %d\n", Iptr, Areg, Breg, Creg, Wptr, instruction_table[byte >> 4], Oreg | (byte & 0x0f));
            else
                sprintf(buffer, "Iptr=%08x A=%08x B=%08x C=%08x Wptr=%08x %s\n", Iptr, Areg, Breg, Creg, Wptr, instruction_table[16 + (Oreg | (byte & 0x0f))]);
            debug_message(buffer);
        }
#endif
        Oreg |= byte & 0x0f;
        completed = 1;
        switch (byte & 0xf0) {
            case 0x00:  /* j */
                Iptr += Oreg;
                Oreg = 0;
                
                /*
                 ** j is one of the scheduling points for the transputer
                 ** Other interruptable instructions (not implemented):
                 ** + move
                 ** + talt
                 ** + tin
                 ** + in
                 ** + out
                 ** + dist
                 */
                if (transputer_priority == 1) {
                    write_memory_32(Wptr - 4, Iptr);
                    Enqueue(Wptr, &transputer_FPtrReg1, &transputer_BPtrReg1);
                    transputer_StatusReg |= GotoSNPBit;
                }
                break;
            case 0x10:  /* ldlp */
                Creg = Breg;
                Breg = Areg;
                Areg = Wptr + (Oreg << 2);
                Oreg = 0;
                break;
            case 0x20:  /* pfix */
                Oreg <<= 4;
                completed = 0;
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
                completed = 0;
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
                Areg = Iptr;
                UpdateWdescReg(Wptr | transputer_priority);
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
                UpdateWdescReg(Wptr | transputer_priority);
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
                        
                        /*
                         ** Instructions for handling processes
                         */
                    case 0x2b:  /* tin */
                        if (transputer_priority == 0)   /* High-priority */
                            temp = transputer_ClockReg0;
                        else    /* Low-priority */
                            temp = transputer_ClockReg1;
                        if ((int) Areg - temp > 0) {  /* Should wait this time */
                            write_memory_32(Wptr - 4, Iptr);    /* Current instruction ptr. */
                            write_memory_32(Wptr - 20, Areg);   /* Time for awakening */
                            if (transputer_priority == 0) { /* Link timer */
                                temp2 = 0x80000024;
                            } else {
                                temp2 = 0x80000028;
                            }
                            while (1) {
                                temp = read_memory_32(temp2);
                                if (temp == 0x80000000)
                                    break;
                                if ((int) Areg - read_memory_32(temp - 20) < 0)
                                    break;
                                temp2 = temp - 16;
                            }
                            write_memory_32(Wptr - 16, read_memory_32(temp2));
                            write_memory_32(temp2, Wptr);
                            transputer_StatusReg |= GotoSNPBit;
                            //                            fprintf(stderr, "Forcing process change\n");
                        }
                        break;
                    case 0x0d:  /* startp */
                        write_memory_32(Areg - 4, Iptr + Breg);
                        temp = Areg | transputer_priority;
                        Run(temp);
                        break;
                    case 0x15:  /* stopp */
                        write_memory_32(Wptr - 4, Iptr);
                        transputer_StatusReg |= GotoSNPBit;
                        break;
                    case 0x03:  /* endp */
                        temp = read_memory_32(Areg + 4);
                        if (temp == 1) {
                            Iptr = read_memory_32(Areg);
                            Wptr = Areg;
                        } else {
                            write_memory_32(Areg + 4, temp - 1);
                            transputer_StatusReg |= GotoSNPBit;
                        }
                        break;
                    case 0x39:  /* runp */
                        temp = Areg;
                        Run(temp);
                        break;
                    case 0x3d:  /* savel */
                        write_memory_32(Areg, transputer_FPtrReg1);
                        write_memory_32(Areg + 4, transputer_BPtrReg1);
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x3e:  /* saveh */
                        write_memory_32(Areg, transputer_FPtrReg0);
                        write_memory_32(Areg + 4, transputer_BPtrReg0);
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x50:  /* sthb */
                        transputer_BPtrReg0 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x17:  /* stlb */
                        transputer_BPtrReg1 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x18:  /* sthf */
                        transputer_FPtrReg0 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                    case 0x1c:  /* stlf */
                        transputer_FPtrReg1 = Areg;
                        Areg = Breg;
                        Breg = Creg;
                        break;
                        
                        /*
                         ** Instructions for handling channels
                         */
                    case 0x07:  /* in */
                        channel_input(Creg, Breg, Areg);
                        break;
                    case 0x0e:  /* outbyte */
                        write_memory(Wptr, Areg);
                        Creg = Wptr;
                        Areg = 1;
                        channel_output(Creg, Breg, Areg);
                        break;
                    case 0x0f:  /* outword */
                        write_memory_32(Wptr, Areg);
                        Creg = Wptr;
                        Areg = 4;
                        channel_output(Creg, Breg, Areg);
                        break;
                    case 0x0b:  /* out */
                        channel_output(Creg, Breg, Areg);
                        break;
                        
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
                    case 0x0c:  /* sub */
                        temp = Breg - Areg;
                        /* If operands are different sign but result is different sign than first operand */
                        if (((Areg ^ Oreg) & 0x80000000) != 0 && ((Areg ^ temp) & 0x80000000) != 0)
                            transputer_error = 1;
                        Areg = temp;
                        Breg = Creg;
                        break;
                    case 0x12:  /* resetch */
                        temp = read_memory_32(Areg);
                        write_memory_32(Areg, 0x80000000);
                        Areg = temp;
                        break;
                    case 0x10:  /* seterr */
                    case 0x13:  /* csub0 */
                        not_handled();
                        break;
                    case 0x16:  /* ladd */
                        not_handled();
                        break;
                    case 0x19:  /* norm */
                        not_handled();
                        break;
                    case 0x1b:  /* ldpi */
                        Areg += Iptr;
                        break;
                    case 0x1a:  /* ldiv */
                    case 0x1d:  /* xdble */
                        not_handled();
                        break;
                    case 0x1e:  /* ldpri */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_priority;
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
                        UpdateWdescReg(Wptr | transputer_priority);
                        break;
                    case 0x21:  /* lend */
                        Breg &= ~3;
                        /*                        fprintf(stderr, "Breg[0] = %08x Breg[1] = %08x\n", read_memory_32(Breg), read_memory_32(Breg + 4));*/
                        temp = read_memory_32(Breg + 4);
                        temp--;
                        write_memory_32(Breg + 4, temp);
                        if (temp == 0)
                            break;
                        temp = read_memory_32(Breg);
                        temp++;
                        write_memory_32(Breg, temp);
                        Iptr = Iptr - Areg;
                        if (transputer_priority == 1) {
                            write_memory_32(Wptr - 4, Iptr);
                            Enqueue(Wptr, &transputer_FPtrReg1, &transputer_BPtrReg1);
                            transputer_StatusReg |= GotoSNPBit;
                        }
                        break;
                    case 0x22:  /* ldtimer */
                        Creg = Breg;
                        Breg = Areg;
                        if (transputer_priority == 0)
                            Areg = transputer_ClockReg0;
                        else
                            Areg = transputer_ClockReg1;
                        break;
                    case 0x29:  /* testerr */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = transputer_error;
                        transputer_error = 0;
                        break;
                    case 0x2a:  /* testpranal */
                        Creg = Breg;
                        Breg = Areg;
                        Areg = 0;   /* Analyse pin not asserted */
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
                        UpdateWdescReg(Wptr | transputer_priority);
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
                        c = 32;
                        do {
                            temp = Breg & 0x80000000;
                            Breg = (Breg << 1) | (Areg >> 31);
                            if (temp)
                                Breg ^= Creg;
                            Areg <<= 1;
                        } while (--c) ;
                        Areg = Breg;
                        break;
                    case 0x75:  /* crcbyte */
                        c = 8;
                        do {
                            temp = Breg & 0x80000000;
                            Breg = (Breg << 1) | (Areg >> 31);
                            if (temp)
                                Breg ^= Creg;
                            Areg <<= 1;
                        } while (--c) ;
                        Areg = Breg;
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
