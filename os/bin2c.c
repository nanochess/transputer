/*
** Convert binary to C format
**
** by Oscar Toledo G.
**
** Creation date: Mar/05/2025.
*/

#include <stdio.h>
#include <stdlib.h>

/*
** Main program
*/
int main(int argc, char *argv[])
{
	FILE *input;
	FILE *output;
	int c;
	int num;

	if (argc < 3) {
		fprintf(stderr, "Usage: bin2c input.bin output.c [name]\n");
		exit(1);
	}
	input = fopen(argv[1], "rb");
	if (input == NULL) {
        fprintf(stderr, "Couldn't open '%s' for input.\n", argv[1]);
		exit(1);
	}
	output = fopen(argv[2], "w");
	if (output == NULL) {
		fprintf(stderr, "Couldn't open '%s' for output.\n", argv[2]);
		exit(1);
	}
    fprintf(output, "unsigned char %s[] = {\n", argc < 4 ? "binary" : argv[3]);
	num = 0;
	while (1) {
		c = fgetc(input);
		if (c == EOF) {
			if ((num & 7) != 0)
				fprintf(output, "\n");
			break;
		}
		if ((num & 7) == 0) {
			fprintf(output, "\t");
		}
		fprintf(output, "0x%02x, ", c);
		if ((num & 7) == 7)
			fprintf(output, "\n");
		num++;
	}
	fprintf(output, "};\n");
	fclose(output);
	fclose(input);
	exit(0);
}

