/*
 ** Build a disk image file of my transputer operating system
 **
 ** by Oscar Toledo G.
 **
 ** (c) Copyright 1995-2025 Oscar Toledo G.
 **
 ** Creation date: Feb/20/2025.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define SECTOR_SIZE     512

unsigned char *image;
int total_sectors;
int max_blocks;
int block_size;
int directory_sector;
int directory_block;

/*
 ** We need this program to bootstrap my transputer operating system
 ** because we have some missing files, and we need to rebuild these
 ** but it would be hard to rebuild the disk image file each time.
 */

unsigned char bootv1[] = {
    0x24, 0x0e, 0x53, 0x4f, 0x4d, 0x33, 0x32, 0x20,
    0x76, 0x31, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x0b, 0x00, 0x00,
    0x12, 0x00, 0x00, 0x00, 0xa0, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,
    0x53, 0x69, 0x73, 0x74, 0x65, 0x6d, 0x61, 0x5f,
    0x33, 0x32, 0x5f, 0x62, 0x69, 0x74, 0x73, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x68, 0xb0, 0x63, 0x42, 0x21, 0xfb, 0x25, 0xfa,
    0x30, 0xdf, 0x35, 0xde, 0x40, 0x21, 0x10, 0x2a,
    0x9d, 0x7e, 0x21, 0x10, 0x2a, 0x98, 0x21, 0x10,
    0xd0, 0x21, 0x26, 0x49, 0x21, 0xfb, 0xd1, 0x4b,
    0xd2, 0x72, 0x21, 0xa4, 0x70, 0xf1, 0x71, 0xf1,
    0xf4, 0xc0, 0x24, 0xac, 0x70, 0x81, 0xd0, 0x71,
    0x81, 0xd1, 0x72, 0x60, 0x8f, 0xd2, 0x61, 0x09,
    0x21, 0x77, 0x21, 0x2f, 0x8f, 0x22, 0x20, 0x40,
    0x22, 0xfc, 0xd2, 0x21, 0x76, 0x7f, 0xf8, 0xd1,
    0x21, 0x26, 0x43, 0x21, 0xfb, 0x8f, 0x4f, 0x23,
    0xf2, 0x24, 0xf6, 0x25, 0xfa, 0xd0, 0xd3, 0x71,
    0x21, 0x10, 0x26, 0x92, 0x21, 0x10, 0x70, 0x22,
    0x20, 0x40, 0x24, 0xfa, 0x70, 0x22, 0x20, 0x80,
    0xd0, 0x71, 0x81, 0xd1, 0x72, 0x60, 0x8f, 0xd2,
    0x72, 0xc0, 0x61, 0xa3, 0x73, 0x29, 0xb0, 0xf6,
    0x24, 0xf2, 0x24, 0x34, 0x27, 0x47, 0x21, 0xfb,
    0x21, 0x90, 0x24, 0xf2, 0x24, 0x34, 0x21, 0x24,
    0x80, 0x2a, 0x4b, 0x21, 0xfb, 0x94, 0x21, 0x94,
    0x68, 0x00, 0x71, 0xf1, 0xad, 0x71, 0xf1, 0x72,
    0x23, 0xfb, 0x71, 0x81, 0xd1, 0x72, 0x82, 0xd2,
    0x60, 0x00, 0x22, 0xf0, 0x60, 0xbe, 0x45, 0xd0,
    0x10, 0xd1, 0x11, 0x24, 0xf2, 0x24, 0x52, 0x44,
    0xfb, 0x11, 0x24, 0xf2, 0x24, 0x53, 0x41, 0xf7,
    0x70, 0x61, 0xab, 0xb2, 0x22, 0xf0, 0x71, 0x60,
    0x8f, 0xd1, 0x41, 0x71, 0x23, 0xfb, 0x72, 0x22,
    0x44, 0x22, 0xfc, 0x71, 0x81, 0x23, 0xfb, 0x72,
    0x21, 0x42, 0x22, 0xfc, 0x42, 0x21, 0xff, 0x71,
    0x82, 0x23, 0xfb, 0x72, 0x21, 0x42, 0x21, 0xff,
    0x81, 0x71, 0x83, 0x23, 0xfb, 0x71, 0xd2, 0x12,
    0x24, 0xf2, 0x24, 0x52, 0x44, 0xfb, 0x12, 0x24,
    0xf2, 0x24, 0x53, 0x41, 0xf7, 0x22, 0xf0, 0x45,
    0x73, 0x74, 0x65, 0x20, 0x64, 0x69, 0x73, 0x63,
    0x6f, 0x20, 0x6e, 0x6f, 0x20, 0x63, 0x6f, 0x6e,
    0x74, 0x69, 0x65, 0x6e, 0x65, 0x20, 0x65, 0x6c,
    0x20, 0x53, 0x69, 0x73, 0x74, 0x65, 0x6d, 0x61,
    0x20, 0x4f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69,
    0x76, 0x6f, 0x20, 0x4d, 0x65, 0x78, 0x69, 0x63,
    0x61, 0x6e, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x33,
    0x32, 0x20, 0x62, 0x69, 0x74, 0x73, 0x2e, 0x00,
    0x49, 0x6e, 0x73, 0x65, 0x72, 0x74, 0x65, 0x20,
    0x75, 0x6e, 0x20, 0x64, 0x69, 0x73, 0x63, 0x6f,
    0x20, 0x71, 0x75, 0x65, 0x20, 0x63, 0x6f, 0x6e,
    0x74, 0x65, 0x6e, 0x67, 0x61, 0x20, 0x65, 0x6c,
    0x20, 0x73, 0x69, 0x73, 0x74, 0x65, 0x6d, 0x61,
    0x20, 0x6f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69,
    0x76, 0x6f, 0x2c, 0x20, 0x79, 0x20, 0x70, 0x75,
    0x6c, 0x73, 0x65, 0x20, 0x63, 0x75, 0x61, 0x6c,
    0x71, 0x75, 0x69, 0x65, 0x72, 0x20, 0x74, 0x65,
    0x63, 0x6c, 0x61, 0x2e, 0x2e, 0x2e, 0x00, 0x53,
    0x4f, 0x4d, 0x33, 0x32, 0x20, 0x20, 0x20, 0x42,
    0x49, 0x4e, 0x28, 0x63, 0x29, 0x31, 0x39, 0x39,
    0x35, 0x20, 0x4f, 0x73, 0x63, 0x61, 0x72, 0x20,
    0x54, 0x6f, 0x6c, 0x65, 0x64, 0x6f, 0x20, 0x47,
    0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34,
};

unsigned char bootv2[] = {
    0x24, 0x0e, 0x53, 0x4f, 0x4d, 0x33, 0x32, 0x20,
    0x76, 0x31, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x0b, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0xa0, 0x05, 0x00, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x06, 0x00, 0x00, 0x00,
    0x02, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
    0x53, 0x69, 0x73, 0x74, 0x65, 0x6d, 0x61, 0x5f,
    0x33, 0x32, 0x5f, 0x62, 0x69, 0x74, 0x73, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x68, 0xb0, 0x63, 0x42, 0x21, 0xfb, 0x25, 0xfa,
    0x30, 0xdf, 0x35, 0x7f, 0xf8, 0xde, 0x40, 0x21,
    0x10, 0x2a, 0x9d, 0x7e, 0x21, 0x10, 0x2a, 0x98,
    0x21, 0x10, 0xd0, 0x21, 0x26, 0x4e, 0x21, 0xfb,
    0xd1, 0x4b, 0xd2, 0x72, 0x21, 0xa4, 0x70, 0xf1,
    0x71, 0xf1, 0xf4, 0xc0, 0x24, 0xac, 0x70, 0x81,
    0xd0, 0x71, 0x81, 0xd1, 0x72, 0x60, 0x8f, 0xd2,
    0x61, 0x09, 0x21, 0x7f, 0x21, 0x2f, 0x8f, 0x22,
    0x20, 0x40, 0x22, 0xfc, 0xd2, 0x21, 0x7e, 0x7f,
    0xf8, 0xd1, 0x21, 0x26, 0x41, 0x21, 0xfb, 0x8f,
    0x4f, 0x23, 0xf2, 0x24, 0xf6, 0x25, 0xfa, 0xd0,
    0xd3, 0x71, 0x21, 0x10, 0x26, 0x92, 0x21, 0x10,
    0x70, 0x22, 0x20, 0x40, 0x24, 0xfa, 0x70, 0x22,
    0x20, 0x80, 0xd0, 0x71, 0x81, 0xd1, 0x72, 0x60,
    0x8f, 0xd2, 0x72, 0xc0, 0x61, 0xa3, 0x73, 0x29,
    0xb0, 0xf6, 0x24, 0xf2, 0x24, 0x34, 0x27, 0x4c,
    0x21, 0xfb, 0x21, 0x90, 0x24, 0xf2, 0x24, 0x34,
    0x21, 0x24, 0x80, 0x2b, 0x40, 0x21, 0xfb, 0x94,
    0x21, 0x94, 0x69, 0x0e, 0x71, 0xf1, 0xad, 0x71,
    0xf1, 0x72, 0x23, 0xfb, 0x71, 0x81, 0xd1, 0x72,
    0x82, 0xd2, 0x60, 0x00, 0x22, 0xf0, 0x60, 0xbe,
    0x45, 0xd0, 0x10, 0xd1, 0x11, 0x24, 0xf2, 0x24,
    0x52, 0x44, 0xfb, 0x11, 0x24, 0xf2, 0x24, 0x53,
    0x41, 0xf7, 0x70, 0x61, 0xab, 0xb2, 0x22, 0xf0,
    0x71, 0x60, 0x8f, 0xd1, 0x41, 0x71, 0x23, 0xfb,
    0x40, 0x71, 0x81, 0x23, 0xfb, 0x72, 0x21, 0x48,
    0x24, 0xf0, 0x71, 0x82, 0x23, 0xfb, 0x72, 0x21,
    0x40, 0x24, 0xf0, 0x71, 0x83, 0x23, 0xfb, 0x72,
    0x48, 0x24, 0xf0, 0x71, 0x84, 0x23, 0xfb, 0x72,
    0x71, 0x85, 0x23, 0xfb, 0x71, 0xd2, 0x12, 0x24,
    0xf2, 0x24, 0x52, 0x44, 0xfb, 0x12, 0x24, 0xf2,
    0x24, 0x53, 0x41, 0xf7, 0x22, 0xf0, 0x45, 0x73,
    0x74, 0x65, 0x20, 0x64, 0x69, 0x73, 0x63, 0x6f,
    0x20, 0x6e, 0x6f, 0x20, 0x63, 0x6f, 0x6e, 0x74,
    0x69, 0x65, 0x6e, 0x65, 0x20, 0x65, 0x6c, 0x20,
    0x53, 0x69, 0x73, 0x74, 0x65, 0x6d, 0x61, 0x20,
    0x4f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x76,
    0x6f, 0x20, 0x4d, 0x65, 0x78, 0x69, 0x63, 0x61,
    0x6e, 0x6f, 0x20, 0x64, 0x65, 0x20, 0x33, 0x32,
    0x20, 0x62, 0x69, 0x74, 0x73, 0x2e, 0x00, 0x49,
    0x6e, 0x73, 0x65, 0x72, 0x74, 0x65, 0x20, 0x75,
    0x6e, 0x20, 0x64, 0x69, 0x73, 0x63, 0x6f, 0x20,
    0x71, 0x75, 0x65, 0x20, 0x63, 0x6f, 0x6e, 0x74,
    0x65, 0x6e, 0x67, 0x61, 0x20, 0x65, 0x6c, 0x20,
    0x73, 0x69, 0x73, 0x74, 0x65, 0x6d, 0x61, 0x20,
    0x6f, 0x70, 0x65, 0x72, 0x61, 0x74, 0x69, 0x76,
    0x6f, 0x2c, 0x20, 0x79, 0x20, 0x70, 0x75, 0x6c,
    0x73, 0x65, 0x20, 0x63, 0x75, 0x61, 0x6c, 0x71,
    0x75, 0x69, 0x65, 0x72, 0x20, 0x74, 0x65, 0x63,
    0x6c, 0x61, 0x2e, 0x2e, 0x2e, 0x00, 0x53, 0x4f,
    0x4d, 0x2e, 0x33, 0x32, 0x2e, 0x62, 0x69, 0x6e,
    0x00, 0x28, 0x63, 0x29, 0x31, 0x39, 0x39, 0x36,
    0x20, 0x4f, 0x73, 0x63, 0x61, 0x72, 0x20, 0x54,
    0x6f, 0x6c, 0x65, 0x64, 0x6f, 0x00, 0x12, 0x34,
};

char label[32];

/*
 ** This is for the early version of my transputer operating system,
 ** a further improved version used 15 letter freeform names.
 */

/*
 ** Main program
 */
int main(int argc, char *argv[])
{
    FILE *input;
    FILE *output;
    int c;
    unsigned char entry[64];
    char *p;
    char *p1;
    unsigned char *p2;
    unsigned char *next_entry;
    int next_block;
    int block;
    unsigned int length;
    int blocks;
    unsigned char *fat;
    time_t current_time;
    struct tm *time_data;
    int temp;
    int arg;
    int media;
    int version;
    
    fprintf(stderr, "\nbuildboot v0.2. Transputer disk image creator\n");
    fprintf(stderr, "by Oscar Toledo G. https://nanochess.org/\n\n");
    time(&current_time);
    time_data = localtime(&current_time);
    if (argc < 3) {
        fprintf(stderr, "Usage: buildboot [options] disk.img arranque.cmg [...files...]\n\n");
        fprintf(stderr, "disk.img is the final disk image (1.44 mb / 40 mb)\n");
        fprintf(stderr, "arranque.cmg is a 512 byte boot sector.\n");
        fprintf(stderr, "             (you can use a period for creating a\n");
        fprintf(stderr, "             default boot sector)\n");
        fprintf(stderr, "[...files...] are files to be added:\n");
        fprintf(stderr, "  * SOM32.BIN must be the first file (operating system v1).\n");
        fprintf(stderr, "  * SOM.32.bin must be the first file (operating system v2).\n");
        fprintf(stderr, "\n");
        fprintf(stderr, "-fd       Create image for 1.44mb floppy disk (default)\n");
        fprintf(stderr, "-hd       Create image for 40mb hard disk (only with -v2)\n");
        fprintf(stderr, "-v2       Create image for operating system v.2\n");
        fprintf(stderr, "-l name   Label name for the disk\n");
        fprintf(stderr, "\n");
        exit(1);
    }
    
    version = -1;
    media = -1;
    arg = 1;
    while (arg < argc && argv[arg][0] == '-') {
        if (tolower(argv[arg][1]) == 'f' && tolower(argv[arg][2]) == 'd') {
            if (media != -1) {
                fprintf(stderr, "Error: Mixed output options\n");
                break;
            }
            media = 0;
            arg++;
        } else if (tolower(argv[arg][1]) == 'h' && tolower(argv[arg][2]) == 'd') {
            if (media != -1) {
                fprintf(stderr, "Error: Mixed output options\n");
                break;
            }
            media = 1;
            arg++;
        } else if (tolower(argv[arg][1]) == 'v' && tolower(argv[arg][2]) == '2') {
            if (version != -1) {
                fprintf(stderr, "Error: Mixed version options\n");
                break;
            }
            version = 1;
            arg++;
        } else if (tolower(argv[arg][1]) == 'l') {
            arg++;
            if (arg >= argc) {
                fprintf(stderr, "Error: missing label for volume\n");
                exit(1);
            }
            strncpy(label, argv[arg], sizeof(label) - 1);
            arg++;
        } else {
            fprintf(stderr, "Unknown option '%s'\n", argv[c]);
            exit(1);
        }
    }
    if (version == -1)
        version = 0;
    if (media == -1)
        media = 0;
    
    if (media == 0) {
        total_sectors = 80 * 2 * 18;
        if (version == 0) {
            block_size = 18;    /* 9 kb. block size (same size as track) */
            max_blocks = total_sectors / block_size;
            directory_sector = 2;
            next_block = 1;
        } else {
            block_size = 2;
            max_blocks = total_sectors / block_size;
            directory_block = (SECTOR_SIZE + max_blocks * 2 + (block_size * SECTOR_SIZE - 1)) / (block_size * SECTOR_SIZE);
            next_block = directory_block + 1;
        }
    } else {
        total_sectors = 40 * (1048576 / SECTOR_SIZE); /* 40 mb */
        block_size = 8;  /* 4 kb. block size */
        max_blocks = total_sectors / block_size;
        if (max_blocks > 256)
            directory_block = (SECTOR_SIZE + max_blocks * 2 + (block_size * SECTOR_SIZE - 1)) / (block_size * SECTOR_SIZE);
        else
            directory_block = (SECTOR_SIZE + max_blocks + (block_size * SECTOR_SIZE - 1)) / (block_size * SECTOR_SIZE);
        next_block = directory_block + 1;
        if (version == 0) {
            fprintf(stderr, "First version of operating system doesn't support hard disks\n");
            exit(1);
        }
    }
    image = malloc(max_blocks * block_size * SECTOR_SIZE);
    if (image == NULL) {
        fprintf(stderr, "Couldn't allocate memory for imaye file\n");
        exit(1);
    }
    /* My personal filler byte for formatted disks, circa 1989 */
    memset(image, 0xfc, max_blocks * block_size * SECTOR_SIZE);
    
    /*
     ** Create an empty FAT
     **
     ** The disk is divided into 160 allocation blocks (80 tracks * 2 sides)
     */
    fat = &image[1 * SECTOR_SIZE + 0];
    if (max_blocks > 256)
        memset(fat, 0, max_blocks * 2);
    else
        memset(fat, 0, max_blocks);
    

    /*
     ** Put the boot sector (1 sector)
     */
    if (arg + 1 >= argc) {
        fprintf(stderr, "Missing boot sector file in arguments\n");
        exit(1);
    }
    if (argv[arg + 1][0] == '.' && argv[arg + 1][1] == '\0') {
        if (media == 0 && version == 0) {
            memcpy(image, bootv1, 512);
            if (label[0] != '\0')
                memcpy(&image[48], &label[0], 32);
        } else if (media == 0 && version == 1) {
            memcpy(image, bootv2, 512);
            if (label[0] != '\0')
                memcpy(&image[48], &label[0], 32);
        } else {
            image[0] = 0x24;    /* Jump to boot code */
            image[1] = 0x0e;
            memcpy(image + 2, "SOM32 v1.0", 10);
            /* Media type */
            if (media == 0)
                image[16] = 0x00;
            else
                image[16] = 0x01;
            image[17] = 0x00;
            image[18] = 0x00;
            image[19] = 0x00;
            /* Total sectors in disc */
            c = total_sectors;
            image[20] = c;
            image[21] = c >> 8;
            image[22] = c >> 16;
            image[23] = c >> 24;
            /* Sectors per block */
            c = block_size;
            image[24] = c;
            image[25] = 0x00;
            image[26] = 0x00;
            image[27] = 0x00;
            /* Total of blocks in the disk */
            image[28] = max_blocks;
            image[29] = max_blocks >> 8;
            image[30] = max_blocks >> 16;
            image[31] = max_blocks >> 24;
            /* Start sector of File Allocation Table */
            image[32] = 0x01;
            image[33] = 0x00;
            image[34] = 0x00;
            image[35] = 0x00;
            /* Size in sectors of the File Allocation Table */
            c = (max_blocks * 2 + 511) / SECTOR_SIZE;
            image[36] = c;
            image[37] = c >> 8;
            image[38] = c >> 16;
            image[39] = c >> 24;
            /* Size in bytes of each entry in the File Allocation Table */
            if (max_blocks > 256) {
                image[40] = 0x02;
            } else {
                image[40] = 0x01;
            }
            image[41] = 0x00;
            image[42] = 0x00;
            image[43] = 0x00;
            /* Start sector or block of the root directory */
            if (version == 0) {
                c = directory_sector;
            } else {
                c = directory_block;
            }
            image[44] = c;
            image[45] = c >> 8;
            image[46] = c >> 16;
            image[47] = c >> 24;
            if (label[0] != '\0')
                memcpy(&image[48], &label[0], 32);
            else
                strncpy((char *) &image[48], "New_disk", 32);
            image[510] = 0x12;
            image[511] = 0x34;
        }
        /* Serial number */
        image[12] = rand();
        image[13] = rand();
        image[14] = rand();
        image[15] = rand();
    } else {
        input = fopen(argv[arg + 1], "rb");
        if (input == NULL) {
            fprintf(stderr, "Couldn't open '%s' for input\n", argv[2]);
            exit(1);
        }
        fread(&image[0 * SECTOR_SIZE], 1, SECTOR_SIZE, input);
        fclose(input);
    }
    
    if (media == 0) {    /* Floppy disk */
        
        if (version == 0) {
            
            /*
             ** Create an empty directory (10 sectors)
             */
            memset(&image[directory_sector * SECTOR_SIZE], 0, 10 * SECTOR_SIZE);
            next_entry = &image[directory_sector * SECTOR_SIZE];
            fat[0] = 0xfd;      /* Mark boot structures */
        } else {
            for (c = 0; c < directory_block; c++) {
                fat[c * 2] = 0xfd;      /* Mark boot structures */
                fat[c * 2 + 1] = 0xff;
            }
            fat[c * 2] = 0xff;      /* Mark directory */
            fat[c * 2 + 1] = 0xff;
            image[1 * SECTOR_SIZE + 1] = 0xff;      /* Mark directory */
            next_entry = &image[directory_block * block_size * SECTOR_SIZE];
            memset(next_entry, 0, block_size * SECTOR_SIZE);
        }
    } else {    /* Hard disk */
        
        /*
         ** Reset FAT
         */
        memset(fat, 0, max_blocks * 2);
        
        /*
         ** Calculate FAT size
         */
        next_block = directory_block;
        next_entry = &image[directory_block * block_size * SECTOR_SIZE];
        memset(next_entry, 0, block_size * SECTOR_SIZE);
        c = directory_block;
        fat[c * 2] = 0xff;      /* Mark directory */
        fat[c * 2 + 1] = 0xff;
        while (c--) {
            fat[c * 2] = 0xfd;      /* Mark boot structures */
            fat[c * 2 + 1] = 0xff;
        }
    }
    
    /*
     ** Add the files
     */
    for (c = arg + 2; c < argc; c++) {
        fprintf(stderr, "Adding %s... ", argv[c]);
        input = fopen(argv[c], "rb");
        if (input == NULL) {
            fprintf(stderr, "\nCouldn't open '%s' file for adding to the floppy disk image\n", argv[c]);
            exit(1);
        }
        fseek(input, 0, SEEK_END);
        length = (unsigned int) ftell(input);
        fprintf(stderr, "(length %d)\n", length);
        blocks = (length + block_size * SECTOR_SIZE - 1) / (block_size * SECTOR_SIZE);
        if (next_block + blocks > max_blocks) {
            fprintf(stderr, "The file '%s' doesn't fit inside the floppy disk image\n", argv[c]);
            fprintf(stderr, "next_block = %d, block_size = %d, blocks = %d, max_blocks = %d\n", next_block, block_size, blocks, max_blocks);
            exit(1);
        }
        fseek(input, 0, SEEK_SET);
        fread(&image[next_block * block_size * SECTOR_SIZE], 1, length, input);
        fclose(input);
        p = argv[c];
        while (1) {
            p1 = strchr(p, '/');
            if (p1 == NULL)
                break;
            p = p1 + 1;
        }
        if (version == 0) {
            
            /*
             ** Build a directory entry
             **
             ** An 8.3 file entry wasn't a great idea (^^)!
             */
            memset(&entry[0], ' ', 11);
            memset(&entry[11], 0, 21);
            p2 = entry;
            while (*p) {
                if (*p == '.') {
                    p2 = entry + 8;
                } else {
                    if (p2 < entry + 11)
                        *p2++ = toupper(*p);
                }
                p++;
            }
            temp = (time_data->tm_sec / 2) | (time_data->tm_min << 5) | (time_data->tm_hour << 11);
            entry[20] = temp;
            entry[21] = temp >> 8;
            /* Starting the year in 1970 in honor to UNIX */
            temp = (time_data->tm_mday) | ((time_data->tm_mon + 1) << 5) | ((time_data->tm_year - 70) << 9);
            entry[22] = temp;
            entry[23] = temp >> 8;
            /* At least I improved choosing a 32-bit block number */
            entry[24] = next_block;
            /* And of course, 4 GB files should be enough for everyone XD */
            entry[28] = length;
            entry[29] = length >> 8;
            entry[30] = length >> 16;
            entry[31] = length >> 24;
            memcpy(next_entry, entry, 32);
            next_entry += 32;
        } else {
            
            /*
             ** Build a directory entry (up to 31 letters per filename)
             */
            memset(&entry[0], '\0', 64);
            p2 = entry;
            while (*p) {
                if (p2 < entry + 31)
                    *p2++ = *p;
                p++;
            }
            temp = (time_data->tm_sec / 2) | (time_data->tm_min << 5) | (time_data->tm_hour << 11);
            entry[52] = temp;
            entry[53] = temp >> 8;
            /* Starting the year in 1970 in honor to UNIX */
            temp = (time_data->tm_mday) | ((time_data->tm_mon + 1) << 5) | ((time_data->tm_year - 70) << 9);
            entry[54] = temp;
            entry[55] = temp >> 8;
            /* At least I improved choosing a 32-bit block number */
            entry[56] = next_block;
            entry[57] = next_block >> 8;
            entry[58] = next_block >> 16;
            entry[59] = next_block >> 24;
            /* And of course, 4 GB files should be enough for everyone XD */
            entry[60] = length;
            entry[61] = length >> 8;
            entry[62] = length >> 16;
            entry[63] = length >> 24;
            memcpy(next_entry, entry, 64);
            next_entry += 64;
        }
        while (blocks > 1) {
            if (max_blocks > 256) {
                fat[next_block * 2] = next_block + 1;
                fat[next_block * 2 + 1] = (next_block + 1) >> 8;
            } else {
                fat[next_block] = next_block + 1;
            }
            next_block++;
            blocks--;
        }
        if (max_blocks > 256) {
            fat[next_block * 2] = 0xff;
            fat[next_block * 2 + 1] = 0xff;
        } else {
            fat[next_block] = 0xff;
        }
        next_block++;
    }
    
    /*
     ** Write the final disk image
     */
    output = fopen(argv[arg], "wb");
    if (output == NULL) {
        fprintf(stderr, "Couldn't open '%s' for output\n", argv[1]);
        exit(1);
    }
    fwrite(image, 1, max_blocks * block_size * SECTOR_SIZE, output);
    fclose(output);
}
