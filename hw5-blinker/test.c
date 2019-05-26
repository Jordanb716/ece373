//==============================================================================
//Kernel Device: test.c
//Author: Jordan Bergmann
//
//Description: For testing myCharDev.c
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#define MAX 10
#define LED_MODE_ON 0xF
#define LED_MODE_OFF 0xE

int main(){

	//Variables
	int fp;
	int initVal;
	int allOff = ((LED_MODE_OFF)|(LED_MODE_OFF<<8)|(LED_MODE_OFF<<16)|(LED_MODE_OFF<<24));
	int zeroTwo = ((LED_MODE_ON)|(LED_MODE_OFF<<8)|(LED_MODE_ON<<16)|(LED_MODE_OFF<<24));

	//Open file
	if(!(fp = open("/dev/led_dev", O_RDWR))){
		printf("Error opening file.\n");
		return -1;
	}

	//Read
	read(fp, &initVal, 0);
	if(initVal < 0)
		printf("Read error.\n");
	printf("Current value: %X\n", initVal);

	//Write 1
	if((write(fp, &zeroTwo, sizeof(zeroTwo))) < 0){
		printf("Write 1 error.\n");
	}

	//Wait
	sleep(1);

	//Write 2
	if((write(fp, &allOff, sizeof(allOff))) < 0){
		printf("Write 2 error.\n");
	}

	//Wait
	sleep(1);

	//Write 3 (return to initial initValue)
	if((write(fp, &initVal, sizeof(initVal))) < 0){
		printf("Write 3 error.\n");
	}

	//Close file
	close(fp);

}