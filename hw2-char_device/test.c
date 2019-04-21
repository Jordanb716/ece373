//==============================================================================
//Kernel Device: test.c
//Author: Jordan Bergmann
//
//Description: For testing myCharDev.c
//==============================================================================

#include <stdio.h>
#include <stdlib.h>

void main(){

	//Variables
	FILE* fp;
	char c;

	fp = fopen("/dev/myCharDev", "r+");

	printf("Current value: %d\n", fgetc(fp));

	rewind(fp);

	printf("Current value: %d\n", fgetc(fp));

	rewind(fp);

	fprintf(fp, "2"); //char 2

	rewind(fp);

	printf("New value: %d\n", fgetc(fp));

	fclose(fp);

}