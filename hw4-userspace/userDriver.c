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

#define LED_MODE_ON 0xF
#define LED_MODE_OFF 0xE

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
	ledAddr = (uint32_t*)(deviceAddr + (OFFSET/4));
	printf("Full BAR:	%X\n", deviceAddr);
	printf("LEDCTL:		%X\n", ledAddr);

	//Read current value and print.
	ledInit = *ledAddr;
	printf("Initial value:	%X\n", *ledAddr);

	//Turn LEDs off.
	*ledAddr = (memFileLED_MODE_OFF)|(memFileLED_MODE_OFF<<8)|(memFileLED_MODE_OFF<<16)|(memFileLED_MODE_OFF<<24);
	printf("Off value:	%X\n", *ledAddr);
	sleep(1);

	//Turn LED on.
	*ledAddr = (memFileLED_MODE_ON)|(memFileLED_MODE_ON<<8)|(memFileLED_MODE_ON<<16)|(memFileLED_MODE_ON<<24);
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