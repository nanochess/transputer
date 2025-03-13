/*
 ** Extract all the files from a SOM32 drive
 **
 ** (c) Copyright 2025 Oscar Toledo G.
 **
 ** Creation date: Mar/12/2025.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/stat.h>   /* POSIX */

#define SECTOR_SIZE         512

char path[4096];

unsigned char boot[512];
unsigned char *fat;

int sectors_per_block;
int max_blocks;
int fat_sector;
int fat_size;
int fat_entry;

void extract_directory(char *, int);

#define read_32(addr) ((addr)[0] | ((addr)[1] << 8) | ((addr)[2] << 16) | ((addr)[3] << 24))

FILE *input;

/*
 ** Main program
 */
int main(int argc, char *argv[])
{

    if (argc != 2) {
        fprintf(stderr, "Usage: extractimage harddisk.img\n");
        exit(1);
    }
    input = fopen(argv[1], "rb");
    if (input == NULL) {
        fprintf(stderr, "Couldn't open '%s' for input\n", argv[1]);
        exit(1);
    }
    fread(boot, 1, 512, input);
    if (boot[510] != 0x12 && boot[511] != 0x34) {
        fprintf(stderr, "Not a disk with SOM32 format\n");
        exit(1);
    }
    sectors_per_block = read_32(boot + 24);
    max_blocks = read_32(boot + 28);
    fat_sector = read_32(boot + 32);
    fat_size = read_32(boot + 36);
    fat_entry = read_32(boot + 40);
    fat = malloc(fat_size * SECTOR_SIZE);
    if (fat == NULL) {
        fprintf(stderr, "Couldn't allocate FAT\n");
        exit(1);
    }
    fseek(input, fat_sector * SECTOR_SIZE, SEEK_SET);
    fread(fat, 1, fat_size * SECTOR_SIZE, input);
    extract_directory(path, read_32(boot + 44));
    free(fat);
    fclose(input);
    exit(0);
}

/*
 ** Read FAT
 */
int read_fat(int block)
{
    if (fat_entry == 1)
        return fat[block];
    return fat[block * 2] | (fat[block * 2 + 1] << 8);
}

/*
 ** Extract directory
 */
void extract_directory(char *subpath, int block)
{
    unsigned char *dir;
    unsigned char *entry;
    unsigned char *buffer;
    int file_block;
    int file_size;
    int remaining;
    FILE *output;
    
    dir = malloc(sectors_per_block * SECTOR_SIZE * 2);
    if (dir == NULL) {
        fprintf(stderr, "Couldn't allocate directory block\n");
        exit(1);
    }
    buffer = dir + sectors_per_block * SECTOR_SIZE;
    while (1) {
        fseek(input, block * sectors_per_block * SECTOR_SIZE, SEEK_SET);
        fread(dir, 1, sectors_per_block * SECTOR_SIZE, input);
        entry = dir;
        do {
            if (entry[0] == 0) {
                free(dir);
                return;
            }
            if (entry[0] != 0x80) { /* Not deleted */
                strcpy(subpath, (char *) entry);
                if (entry[48] & 0x08) { /* Subdirectory */
                    mkdir(path, 0744);
                    strcat(subpath, "/");
                    fprintf(stderr, "%s\n", path);
                    extract_directory(path + strlen(path), read_32(entry + 56));
                } else {
                    file_block = read_32(entry + 56);
                    file_size = read_32(entry + 60);
                    fprintf(stderr, "%s (%d bytes)...\n", path, file_size);
                    output = fopen(path, "wb");
                    if (output != NULL) {
                        while (file_size) {
                            remaining = (file_size > sectors_per_block * SECTOR_SIZE) ? sectors_per_block * SECTOR_SIZE : file_size;
                            fseek(input, file_block * sectors_per_block * SECTOR_SIZE, SEEK_SET);
                            fread(buffer, 1, remaining, input);
                            fwrite(buffer, 1, remaining, output);
                            file_block = read_fat(file_block);
                            if (file_block == 0xffffffff)
                                break;
                            file_size -= remaining;
                        }
                        fclose(output);
                    }
                }
            }
            entry += 64;
        } while (entry < dir + sectors_per_block * SECTOR_SIZE) ;
        block = read_fat(block);
        if (block == 0xffffffff)
            break;
    }
    free(dir);
}

