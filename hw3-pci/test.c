//==============================================================================
//Kernel Device: test.c
//Author: Jordan Bergmann
//
//Description: For testing myCharDev.c
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX 20

void main(){

	//Variables
	FILE* fp;
	char c;
	char buf[MAX]; 

	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 1 failed\n");
	}
	printf("Current value: %d\n", fgets(buf, MAX, fp));
	fclose(fp);



	for(int x = 0; x < 255; x++){
		if(!(fp = fopen("/dev/myCharDev", "r+"))){
			printf("open 2 failed\n");
		}
		fprintf(fp, "%d", x);
		fclose(fp);

		if(!(fp = fopen("/dev/myCharDev", "r+"))){
			printf("open 3 failed\n");
		}
		printf("New value: %d\n", fgets(buf, MAX, fp));
		fclose(fp);

		sleep(1);
	}



}