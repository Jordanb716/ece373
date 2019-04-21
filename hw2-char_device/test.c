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

	printf("Current value: %c\n", fgetc(fp));

	fprintf(fp, "50"); //char 2

	printf("New value: %c\n", fgetc(fp));

	fclose(fp);

}