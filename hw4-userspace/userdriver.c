//==============================================================================
//Main: userDriver
//Author: Jordan Bergmann
//
//Description: Blinks e1000 network card LEDs from userspace
//==============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define BAR 0xF0000000
#define LENGTH 128000
#define OFFSET 0x00E00

void main(){
	
	//Variables
	FILE* fp;
	void* deviceAddr;
	void* ledAddr;

	//Open memory.
	fp = open("/dev/mem", O_RDWR);

	//Map e1000e region.
	deviceAddr = mmap(NULL, LENGTH, PROT_READ | PROT_WRITE, MAP_SHARED, fp, BAR);

	if(deviceAddr == MAP_FAILED){
		printf("Mapping failed!\n");
		munmap(deviceAddr, LENGTH);
		return;
	}

	//Add offset.
	ledAddr = deviceAddr + OFFSET;

	//Read current value and print.
	printf("Current value: %x\n", *ledAddr);

	//Invert LED.
	*ledAddr = *ledAddr ^ 0x0040;

	//Unmap Memory and close file.
	munmap(deviceAddr, LENGTH);
	close(fp);

	return;

}