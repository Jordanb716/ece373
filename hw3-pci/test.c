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

	if(fp = fopen("/dev/myCharDev", "r+")){
		printf("open 1 failed");
	}
	printf("Current value: %d\n", fgetc(fp));
	fclose(fp);

	if(fp = fopen("/dev/myCharDev", "r+")){
		printf("open 1 failed");
	}
	fprintf(fp, 0);
	fclose(fp);

	if(fp = fopen("/dev/myCharDev", "r+")){
		printf("open 1 failed");
	}
	printf("New value: %d\n", fgetc(fp));
	fclose(fp);

	sleep(2);

	if(fp = fopen("/dev/myCharDev", "r+")){
		printf("open 1 failed");
	}
	fprintf(fp, 1);
	fclose(fp);

	if(fp = fopen("/dev/myCharDev", "r+")){
		printf("open 1 failed");
	}
	printf("New value: %d\n", fgetc(fp));
	fclose(fp);

	sleep(2);

	if(fp = fopen("/dev/myCharDev", "r+")){
		printf("open 1 failed");
	}
	fprintf(fp, 2);
	fclose(fp);

	if(fp = fopen("/dev/myCharDev", "r+")){
		printf("open 1 failed");
	}
	printf("New value: %d\n", fgetc(fp));
	fclose(fp);

}