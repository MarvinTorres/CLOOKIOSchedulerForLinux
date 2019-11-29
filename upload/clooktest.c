#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLENGTH 1000

/*
 * This program reads each line in a 47K file, input.txt, and writes it to 
 * another file, output.txt.
 *
 * postcondition: size(output.txt) == size(input.txt)
 */
int main(void) {
	FILE *f1ptr;
	FILE *f2ptr;
	char line[MAXLENGTH];

	f1ptr = fopen("input.txt", "r");
	f2ptr = fopen("output.txt", "w");

	if (!f1ptr || !f2ptr) {
		fprintf(stderr, "Failed to access input.txt or output.txt\n");
		return -1;
	}
	
	while (fgets(line, 1000, f1ptr) != NULL) {
		fputs(line, f2ptr);
	}

	fclose(f1ptr);
	fclose(f2ptr);

	return 0;
}
