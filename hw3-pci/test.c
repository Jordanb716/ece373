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
	fclose(fp);

	fp = fopen("/dev/myCharDev", "r+");
	fprintf(fp, "2"); //char 2
	fclose(fp);

	fp = fopen("/dev/myCharDev", "r+");
	printf("New value: %d\n", fgetc(fp));
	fclose(fp);

	fclose(fp);

}