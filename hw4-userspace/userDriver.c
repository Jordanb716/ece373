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

#define LED0_MODE_ON 0x1110
#define LED0_MODE_OFF 0x1111

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

	//Read current value and print.
	ledInit = *ledAddr;
	printf("Current value: %x\n", *ledAddr);

	//Turn LED off.
	*ledAddr = *ledAddr | LED0_MODE_OFF;

	sleep(1);

	//Turn LED on.
	*ledAddr = *ledAddr ^ !LED0_MODE_ON;

	sleep(1);

	//Restore initial value.
	*ledAddr = ledInit;

	//Unmap memory and close file.
	munmap(deviceAddr, LENGTH);
	close(memFile);

	return;

}