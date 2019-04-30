//==============================================================================
//Kernel Device: test.c
//Author: Jordan Bergmann
//
//Description: For testing myCharDev.c
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void main(){

	//Variables
	FILE* fp;
	char c;

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 1 failed\n");
	}
	printf("Current value: %d\n", fgetc(fp));
	fclose(fp);

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 2 failed\n");
	}
	fprintf(fp, "%d", 0);
	fclose(fp);

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 3 failed\n");
	}
	printf("New value: %d\n", fgetc(fp));
	fclose(fp);

	sleep(2);

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 4 failed\n");
	}
	fprintf(fp, "%d", 1);
	fclose(fp);

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 5 failed\n");
	}
	printf("New value: %d\n", fgetc(fp));
	fclose(fp);

	sleep(2);

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 6 failed\n");
	}
	fprintf(fp, "%d", 2);
	fclose(fp);

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 7 failed\n");
	}
	printf("New value: %d\n", fgetc(fp));
	fclose(fp);

}