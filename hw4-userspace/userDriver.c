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
#define OFFSET 0x00E00

#define LED0_MODE_ON 0xF00
#define LED0_MODE_OFF 0xE00

void main(){

	//Variables
	int memFile;
	int ledInit;
	uint32_t* deviceAddr;
	uint32_t* ledAddr;

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
	ledAddr = (uint32_t*)(deviceAddr + OFFSET);
	printf("Full BAR:	%X\n", deviceAddr);
	printf("LEDCTL:		%X\n", ledAddr);

	//Read current value and print.
	ledInit = *ledAddr;
	printf("Initial value:	%X\n", *ledAddr);

	//Turn LED off.
	*ledAddr = LED0_MODE_OFF;
	printf("Off value:	%X\n", *ledAddr);

	sleep(1);

	//Turn LED on.
	*ledAddr = LED0_MODE_ON;
	printf("On value:	%X\n", *ledAddr);

	sleep(1);

	//Restore initial value.
	*ledAddr = ledInit;
	printf("Restored value:	%X\n", *ledAddr);

	//Unmap memory and close file.
	munmap(deviceAddr, LENGTH);
	close(memFile);

	return;

}