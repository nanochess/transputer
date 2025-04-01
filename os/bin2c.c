/*
** Convert binary to C format
**
** by Oscar Toledo G.
**
** Creation date: Mar/05/2025.
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

char base64_encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/*
** Main program
*/
int main(int argc, char *argv[])
{
	FILE *input;
	FILE *output;
	int c;
	int num;
    int decimal;
    int spaces;
    int maximum_bytes;
    int javascript;
    int base64;
    
    decimal = 0;
    spaces = 0;
    maximum_bytes = -1;
    javascript = 0;
    base64 = 0;
	if (argc < 3) {
		fprintf(stderr, "Usage: bin2c [options] input.bin output.c [name]\n");
        fprintf(stderr, "  -d         Use decimal for numbers.\n");
        fprintf(stderr, "  -l value   Maximum of bytes read from input file.\n");
        fprintf(stderr, "  -s         Remove spaces from output file.\n");
        fprintf(stderr, "  -j         Generate array for Javascript.\n");
        fprintf(stderr, "  -b         Generate base64 string for Javascript.\n");
		exit(1);
	}
    while (c < argc && argv[c][0] == '-') {
        if (toupper(argv[c][1]) == 'D') {
            decimal = 1;
        } else if (toupper(argv[c][1]) == 'L') {
            c++;
            if (c >= argc) {
                fprintf(stderr, "Wrong argument for -l\n");
                exit(1);
            }
            maximum_bytes = atoi(argv[c]);
        } else if (toupper(argv[c][1]) == 'S') {
            spaces = 1;
        } else if (toupper(argv[c][1]) == 'J') {
            javascript = 1;
        } else if (toupper(argv[c][1]) == 'B') {
            base64 = 1;
        } else {
            fprintf(stderr, "Wrong argument %s\n", argv[c]);
            exit(1);
        }
        c++;
    }
    if (c >= argc) {
        fprintf(stderr, "Missing input file\n");
        exit(1);
    }
	input = fopen(argv[c], "rb");
	if (input == NULL) {
        fprintf(stderr, "Couldn't open '%s' for input.\n", argv[c]);
		exit(1);
	}
    c++;
    if (c >= argc) {
        fprintf(stderr, "Missing output file\n");
        exit(1);
    }
	output = fopen(argv[c], "w");
	if (output == NULL) {
		fprintf(stderr, "Couldn't open '%s' for output.\n", argv[c]);
		exit(1);
	}
    c++;
    if (javascript && base64) {
        fprintf(output, "var %s = \"", c >= argc ? "binary" : argv[c]);
        while (1) {
            if (maximum_bytes == 0)
                break;
            c = fgetc(input);
            if (c == EOF) {
                break;
            }
            if (maximum_bytes != -1 && maximum_bytes != 0)
                maximum_bytes--;
            num = (c & 0xff) << 16;
            c = fgetc(input);
            if (c == EOF) {
                c = 0;
            }
            if (maximum_bytes != -1 && maximum_bytes != 0)
                maximum_bytes--;
            num |= (c & 0xff) << 8;
            c = fgetc(input);
            if (c == EOF) {
                c = 0;
            }
            if (maximum_bytes != -1 && maximum_bytes != 0)
                maximum_bytes--;
            num |= (c & 0xff);
            fputc(base64_encode[(num >> 18) & 0x3f], output);
            fputc(base64_encode[(num >> 12) & 0x3f], output);
            fputc(base64_encode[(num >> 6) & 0x3f], output);
            fputc(base64_encode[num & 0x3f], output);
        }
        fprintf(output, "\";\n");
    } else {
        if (base64) {
            fprintf(stderr, "base64 isn't supported for C language\n");
            exit(1);
        }
        if (javascript) {
            fprintf(output, "var %s = [\n", c >= argc ? "binary" : argv[c]);
        } else {
            fprintf(output, "unsigned char %s[] = {\n", c >= argc ? "binary" : argv[c]);
        }
        num = 0;
        while (1) {
            c = fgetc(input);
            if (c == EOF || maximum_bytes != -1 && maximum_bytes-- == 0) {
                if ((num & 7) != 0)
                    fprintf(output, "\n");
                break;
            }
            if ((num & 7) == 0) {
                if (spaces) {
                    /* Nothing */
                } else {
                    fprintf(output, "\t");
                }
            }
            if (spaces) {
                if (decimal)
                    fprintf(output, "%d,", c);
                else
                    fprintf(output, "0x%02x,", c);
            } else {
                if (decimal)
                    fprintf(output, "%d, ", c);
                else
                    fprintf(output, "0x%02x, ", c);
            }
            if ((num & 7) == 7)
                fprintf(output, "\n");
            num++;
        }
        if (javascript)
            fprintf(output, "];\n");
        else
            fprintf(output, "};\n");
    }
	fclose(output);
	fclose(input);
	exit(0);
}

