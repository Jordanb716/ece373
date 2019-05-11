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

void main(){

	//Variables
	int memFile;
	uint32_t* deviceAddr;
	uint32_t* ledAddr;

	//Open memFile.
	memFile = open("/dev/mem", O_RDWR);

	//Map e1000e region.
	deviceAddr = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, memFile, BAR);

	if(deviceAddr == MAP_FAILED){
		printf("Mapping failed!\n");
		munmap(deviceAddr, LENGTH);
		return;
	}

	//Add offset.
	ledAddr = (uint32_t*)(deviceAddr + OFFSET);

	//Read current value and print.
	printf("Current value: %x\n", *ledAddr);

	//Invert LED.
	*ledAddr = *ledAddr ^ 0x0040;

	//Unmap memory and close file.
	munmap(deviceAddr, LENGTH);
	close(memFile);

	return;

}