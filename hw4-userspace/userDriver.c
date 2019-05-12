//==============================================================================
//Main: userDriver
//Author: Jordan Bergmann
//
//Description: Blinks e1000 network card LEDs from userspace
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BAR 0xF0000000
#define LENGTH 128000
#define LED_OFFSET 0x00E00
//#define GPRC_OFFSET 0x04074
#define GPRC_OFFSET 0x04010

#define LED_MODE_ON 0xF
#define LED_MODE_OFF 0xE

void main(){

	//Variables
	int memFile;
	int ledInit;
	uint32_t* deviceAddr;
	uint32_t* ledAddr;
	uint32_t* GPRCAddr;
	uint32_t* RCTLAddr;

	//Open memFile.
	memFile = open("/dev/mem", O_RDWR);

	if(memFile == -1){
		printf("Open failed!\n");
		return;
	}

	//Map e1000e region.
	deviceAddr = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, memFile, BAR);

	if(deviceAddr == MAP_FAILED){
		printf("Mapping failed!\n");
		munmap(deviceAddr, LENGTH);
		return;
	}

	//Add offset.
	ledAddr = (uint32_t*)(deviceAddr + (LED_OFFSET/4));
	GPRCAddr = (uint32_t*)(deviceAddr + (GPRC_OFFSET/4));
	printf("Full BAR:	%X\n", deviceAddr);
	printf("LEDCTL:		%X\n", ledAddr);

	//Read current value and print.
	ledInit = *ledAddr;
	printf("Initial value:	%X\n", *ledAddr);

	//Turn on LEDs 0 and 2 for 2 seconds.
	printf("Turning on LEDs 0 and 2...\n");
	*ledAddr = (LED_MODE_ON)|(LED_MODE_OFF<<8)|(LED_MODE_ON<<16)|(LED_MODE_OFF<<24);
	sleep(2);

	//Turn LEDs off.
	printf("Turning LEDs off...\n");
	*ledAddr = (LED_MODE_OFF)|(LED_MODE_OFF<<8)|(LED_MODE_OFF<<16)|(LED_MODE_OFF<<24);
	sleep(2);

	//Loop 5 times turning each LED on for one second.
	printf("Looping LEDs...\n");
	for(int x = 0; x < 5; x++){
		*ledAddr = (LED_MODE_ON)|(LED_MODE_OFF<<8)|(LED_MODE_OFF<<16)|(LED_MODE_OFF<<24);
		sleep(1);

		*ledAddr = (LED_MODE_OFF)|(LED_MODE_ON<<8)|(LED_MODE_OFF<<16)|(LED_MODE_OFF<<24);
		sleep(1);

		*ledAddr = (LED_MODE_OFF)|(LED_MODE_OFF<<8)|(LED_MODE_ON<<16)|(LED_MODE_OFF<<24);
		sleep(1);

		*ledAddr = (LED_MODE_OFF)|(LED_MODE_OFF<<8)|(LED_MODE_OFF<<16)|(LED_MODE_ON<<24);
		sleep(1);
	}

	//Restore initial value.
	*ledAddr = ledInit;
	printf("Restored value:	%X\n", *ledAddr);

	//Print good packets received.
	printf("Good packets received:	%d\n", *GPRCAddr);

	//Unmap memory and close file.
	munmap(deviceAddr, LENGTH);
	close(memFile);

	return;

}