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

#define MAX_BLOCKS  160
#define BLOCK_SIZE  9216
#define DIRECTORY_SECTOR    2

unsigned char image[MAX_BLOCKS * BLOCK_SIZE];	/* 1.44mb file */

/*
** We need this program to bootstrap my transputer operating system
** because we have some missing files, and we need to rebuild these
** but it would be hard to rebuild the disk image file each time.
*/

/*
** This is for the early version of my transputer operating system,
s** a further improved version used 15 letter freeform names.
*/

/*
** Main program
*/
int main(int argc, char *argv[])
{
	FILE *input;
	FILE *output;
	int c;
	unsigned char entry[32];
	char *p;
	unsigned char *p2;
	unsigned char *next_entry;
	int next_block;
	unsigned int length;
	int blocks;
	unsigned char *fat;
	time_t current_time;
	struct tm *time_data;
	int temp;

    fprintf(stderr, "\nbuildboot v0.1. Transputer disk image creator\n");
    fprintf(stderr, "by Oscar Toledo G. https://nanochess.org/\n\n");
    time(&current_time);
	time_data = localtime(&current_time);
	if (argc < 3) {
		fprintf(stderr, "Usage: buildboot disk.img arranque.cmg [...files...]\n\n");
		fprintf(stderr, "disk.img is the final disk image (1.44 mb)\n");
		fprintf(stderr, "arranque.cmg is a 512 byte boot sector.\n");
		fprintf(stderr, "[...files...] are files to be added:\n");
		fprintf(stderr, "  * SOM32.BIN must be the first file (operating system).\n");
		fprintf(stderr, "\n");
		exit(1);
	}

	/* My personal filler byte for formatted disks, circa 1989 */
	memset(image, 0xfc, sizeof(image));

	/*
	** Put the boot sector (1 sector)
	*/
	input = fopen(argv[2], "rb");
	if (input == NULL) {
		fprintf(stderr, "Couldn't open '%s' for input\n", argv[2]);
		exit(1);
	}	
	fread(&image[0 * 512], 1, 512, input);
	fclose(input);

	/*
	** Create an empty FAT (1 sector)
	**
	** The disk is divided into 160 allocation blocks (80 tracks * 2 sides)
	*/
	memset(&image[1 * 512], 0, MAX_BLOCKS);
	image[1 * 512 + 0] = 0xfe;	    /* Mark boot structures */
    image[1 * 512 + 1] = 0xff;      /* Mark directory */
	fat = &image[1 * 512 + 0];

	/*
	** Create an empty directory (10 sectors)
	*/
	memset(&image[DIRECTORY_SECTOR * 512], 0, 10 * 512);

	/*
	** Add the files
	*/
	next_block = (DIRECTORY_SECTOR + 17) / 18;
	next_entry = &image[DIRECTORY_SECTOR * 512];
	for (c = 3; c < argc; c++) {
        
        /*
        ** Build a directory entry
        **
        ** An 8.3 file entry wasn't a great idea (^^)!
        */
		memset(&entry[0], ' ', 11);
		memset(&entry[11], 0, 21);
		p = argv[c];
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
        fprintf(stderr, "Adding %s... ", argv[c]);
		input = fopen(argv[c], "rb");
		if (input == NULL) {
			fprintf(stderr, "\nCouldn't open '%s' file for adding to the floppy disk image\n", argv[c]);
			exit(1);
		}
		fseek(input, 0, SEEK_END);
		length = (unsigned int) ftell(input);
        fprintf(stderr, "(length %d)\n", length);
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
		fseek(input, 0, SEEK_SET);
		blocks = (length + BLOCK_SIZE - 1) / BLOCK_SIZE;
		if (next_block + blocks > MAX_BLOCKS) {
            fprintf(stderr, "The file '%s' doesn't fit inside the floppy disk image\n", argv[c]);
			exit(1);
		}
		fread(&image[next_block * BLOCK_SIZE], 1, length, input);
		fclose(input);
		while (blocks > 1) {
			fat[next_block] = next_block + 1;
			next_block++;
            blocks--;
		}
		fat[next_block] = 0xff;
		next_block++;
		memcpy(next_entry, entry, 32);
		next_entry += 32;
	}

	/*
	** Write the final disk image
	*/
	output = fopen(argv[1], "wb");
	if (output == NULL) {
		fprintf(stderr, "Couldn't open '%s' for output\n", argv[1]);
		exit(1);
	}
	fwrite(image, 1, sizeof(image), output);
	fclose(output);
}
