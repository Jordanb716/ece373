//==============================================================================
//Kernel Device: test.c
//Author: Jordan Bergmann
//
//Description: For testing myCharDev.c
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAX 10
#define LED_MODE_ON 0xF
#define LED_MODE_OFF 0xE

void main(){

	//Variables
	FILE* fp;
	char c;
	char buf[MAX]; 

	//Read
	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 1 failed\n");
	}
	printf("Current value: %d\n", fgets(buf, MAX, fp));
	fclose(fp);

	//Write 1
	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 2 failed\n");
	}
	fprintf(fp, "%d", (LED_MODE_ON)|(LED_MODE_OFF<<8)|(LED_MODE_ON<<16)|(LED_MODE_OFF<<24));
	fclose(fp);

	sleep(1);

	//Write 2
	if(!(fp = fopen("/dev/myCharDev", "r+"))){
		printf("open 2 failed\n");
	}
	fprintf(fp, "%d", (LED_MODE_OFF)|(LED_MODE_OFF<<8)|(LED_MODE_ON<<16)|(LED_MODE_OFF<<24));
	fclose(fp);

}