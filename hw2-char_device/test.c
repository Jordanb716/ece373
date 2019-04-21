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

	printf("1");

	fp = fopen("/dev/myCharDev", "r+");

	printf("2");

	printf("Current value: %c\n", fgetc(fp));

	printf("3");

	fprintf(fp, "7");

	printf("New value: %c\n", fgetc(fp));

	fclose(fp);

}