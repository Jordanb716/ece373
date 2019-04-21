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

	printf("1: %d\n", fp);

	printf("Current value: %c\n", fgetc(fp));

	printf("2: %d\n", fp);

	//rewind(fp);

	fprintf(fp, "2"); //char 2

	printf("3: %d\n", fp);

	printf("New value: %c\n", fgetc(fp));

	printf("4: %d\n", fp);

	fclose(fp);

}