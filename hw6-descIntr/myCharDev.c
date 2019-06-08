//==============================================================================
//Kernel Device: myCharDev.c
//Author: Jordan Bergmann
//
//Description: A basic char device. Now with PCI support!
//==============================================================================

#include <linux/timer.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/timekeeping.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

#define NUMDEVS 1
#define DEVNAME "blinky"
#define NODENAME "ece_led"

#define LED_MODE_ON 0xF
#define LED_MODE_OFF 0xE

//-----Addresses-----
//Receive
#define RCTL 0x00100 //Receive Control
#define RDBAL 0x02800 //Base Address Low
#define RDBAH 0x02804 //Base Address High
#define RDLEN 0x02808 //Length
#define RDH 0x02810 //Head
#define RDT	0x02818 //Tail

//Interrupt
#define ICR 0x000C0 //Interrupt Cause Read
#define ICS 0x000C8 //Interrupt Cause Set
#define IMS 0x000D0 //Interrupt Mask Set
#define IMC 0x000D8 //Interrupt Mask Clear

#define NUM_DESC 0b000000001 //0000000_01 7 0's to fill Zero region, then a 2 for 16 descriptors.

//-----Function prototypes-----
static int chardev_open(struct inode *inode, struct file *file);
static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
static ssize_t chardev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);
static int pci_blinkDriver_probe(struct pci_dev* pdev, const struct pci_device_id* ent);
static void pci_blinkDriver_remove(struct pci_dev* pdev);
void blinkLED(struct timer_list *list);
void initSettings(void);

//-----Structs-----
static struct mydev_dev {
	struct cdev cdev;
	dev_t devNode;
	long led_initial_val;
	struct class* class;
	bool ledZeroIsOn;
} myDev;

static struct myPci{
	struct pci_dev* pdev;
	void* hw_addr;
} myPci;

static struct dRing{
	struct descriptor{
		uint64_t address;
		uint16_t length;

		bool done;
	} desc[16];
};

//-----Variables-----
void* dRing;

static struct file_operations mydev_fops = {
	.owner = THIS_MODULE,
	.open = chardev_open,
	.read = chardev_read,
	.write = chardev_write,
};

static const struct pci_device_id pci_blinkDriverTable[] = {
	{ PCI_DEVICE(0x8086, 0x100e) },
	{},
};

static struct pci_driver pci_blinkDriver = {
	.name = DEVNAME,
	.id_table = pci_blinkDriverTable,
	.probe = pci_blinkDriver_probe,
	.remove = pci_blinkDriver_remove,
};

int allOff = ((LED_MODE_OFF)|(LED_MODE_OFF<<8)|(LED_MODE_OFF<<16)|(LED_MODE_OFF<<24));
int zeroOn = ((LED_MODE_ON)|(LED_MODE_OFF<<8)|(LED_MODE_OFF<<16)|(LED_MODE_OFF<<24));
struct timer_list blinkTimer;
char blinkDriverName[] = DEVNAME;

int blink_rate = 2;
module_param(blink_rate, int, S_IRUSR | S_IWUSR);

//-----Settings to write-----
union RCTL_SET{
	uint32_t set;
	struct bits{
		int R1 : 1;
		int EN : 1;
		int SBP : 1;
		int UPE : 1;
		int MPE : 1;
		int LPE : 1;
		int LBM : 2;
		int RDMTS : 2;
		int R2 : 2;
		int MO : 2;
		int R3 : 1;
		int BAM : 1;
		int BSIZE : 2;
		int VFE : 1;
		int CFIEN : 1;
		int CFI : 1;
		int R4 : 1;
		int DPF : 1;
		int PMCF : 1;
		int R5 : 1;
		int BSEX : 1;
		int SECRC : 1;
		int R6 : 5;
	} bits;
} RCTL_SET;

//========================================
//Functions
void blinkLED(struct timer_list *list){

	if(myDev.ledZeroIsOn == true){
		myDev.ledZeroIsOn = false;
		writel(allOff, myPci.hw_addr + 0x00E00);
	}
	else if(blink_rate != 0){
		myDev.ledZeroIsOn = true;
		writel(zeroOn, myPci.hw_addr + 0x00E00);
	}

	if(blink_rate == 0){
		mod_timer(&blinkTimer, (HZ)+jiffies);
	}
	else
		mod_timer(&blinkTimer, (HZ/blink_rate)+jiffies);

}

void initSettings(void){
	RCTL_SET.bits.R1 = 0;
	RCTL_SET.bits.EN = 1;
	RCTL_SET.bits.SBP = 0;
	RCTL_SET.bits.UPE = 1;
	RCTL_SET.bits.MPE = 1;
	RCTL_SET.bits.LPE = 1;
	RCTL_SET.bits.LBM = 0;
	RCTL_SET.bits.RDMTS = 0;
	RCTL_SET.bits.R2 = 0;
	RCTL_SET.bits.MO = 0;
	RCTL_SET.bits.R3 = 0;
	RCTL_SET.bits.BAM = 1;
	RCTL_SET.bits.BSIZE = 0;
	RCTL_SET.bits.VFE = 0;
	RCTL_SET.bits.CFIEN = 0;
	RCTL_SET.bits.CFI = 0;
	RCTL_SET.bits.R4 = 0;
	RCTL_SET.bits.DPF = 0;
	RCTL_SET.bits.PMCF = 0;
	RCTL_SET.bits.R5 = 0;
	RCTL_SET.bits.BSEX = 0;
	RCTL_SET.bits.SECRC = 0;
	RCTL_SET.bits.R6 = 0;
}

//========================================
//Initialization
int __init chardev_init(void){

	printk(KERN_INFO "Initializing myCharDev module!\n");

	//Allocate memory.
	dRing = kzalloc(sizeof(struct dRing), GFP_DMA);
	if(dRing == NULL){
		printk(KERN_ERR "kzalloc failed!\n");
		goto free_dRing;
	}
	printk(KERN_INFO "dRing:%X\n", dRing);

	//Allocate major/minor numbers.
	if(alloc_chrdev_region(&myDev.devNode, 0, NUMDEVS, "myCharDev")){
		printk(KERN_ERR "alloc_chrdev_region() failed!\n");
		goto unreg_region;
	}

	printk(KERN_INFO "Allocated %d devices at major: %d\n", NUMDEVS, MAJOR(myDev.devNode));

	//Initialize cdev.
	cdev_init(&myDev.cdev, &mydev_fops);
	myDev.cdev.owner = THIS_MODULE;

	//Add cdev.
	if(cdev_add(&myDev.cdev, myDev.devNode, NUMDEVS)){
		printk(KERN_ERR "cdev_add() failed!\n");
		goto del_cdev;
	}

	//Register as pci driver.
	if(pci_register_driver(&pci_blinkDriver)){
		printk(KERN_ERR "PCI registration failed.\n");
		goto unreg_driver;
	}

	//Create dev node
	if((myDev.class = class_create(THIS_MODULE, NODENAME)) == NULL){
		printk(KERN_ERR "class_create failed!\n");
		goto destroy_class;
	}
	if(device_create(myDev.class, NULL, myDev.devNode, NULL, NODENAME) == NULL){
		printk(KERN_ERR "device_create failed!\n");
		goto unreg_dev_create;
	}
	printk(KERN_INFO "Device created!\n");

	//Setup timer.
	timer_setup(&blinkTimer, blinkLED, 0);
	printk(KERN_INFO "Timer setup!\n");

	return 0;

unreg_dev_create:
	device_destroy(myDev.class, myDev.devNode);
destroy_class:
	class_destroy(myDev.class);
unreg_driver:
	pci_unregister_driver(&pci_blinkDriver);
del_cdev:
	cdev_del(&myDev.cdev);
unreg_region:
	unregister_chrdev_region(myDev.devNode, NUMDEVS);
free_dRing:
	kfree(dRing);
	return -1;
}

//========================================
//Exit
void __exit chardev_exit(void){

	//Clean up.
	device_destroy(myDev.class, myDev.devNode);
	class_destroy(myDev.class);
	pci_unregister_driver(&pci_blinkDriver);
	cdev_del(&myDev.cdev);
	unregister_chrdev_region(myDev.devNode, NUMDEVS);

	del_timer_sync(&blinkTimer);

	kfree(dRing);

	printk(KERN_INFO "\n-----myCharDev module unloaded!-----\n\n");
}

//========================================
//Probe
static int pci_blinkDriver_probe(struct pci_dev* pdev, const struct pci_device_id* ent){

	resource_size_t mmio_start, mmio_len;
	unsigned long barMask;

	printk(KERN_INFO "Blink driver pci called.\n");

	//Initialize settings values for later wrties.
	initSettings();

	//Get BAR mask.
	barMask = pci_select_bars(pdev, IORESOURCE_MEM);
	printk(KERN_INFO "Barmask: %lx\n", barMask);

	//Reserve BAR areas.
	if(pci_request_selected_regions(pdev, barMask, blinkDriverName)){
		printk(KERN_ERR "Request selected regions failed.\n");
		pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
	}

	mmio_start = pci_resource_start(pdev, 0);
	mmio_len = pci_resource_len(pdev, 0);

	printk(KERN_INFO "Mmio start: %lx\n", (unsigned long) mmio_start);
	printk(KERN_INFO "Mmio length: %lx\n", (unsigned long) mmio_len);

	if(!(myPci.hw_addr = ioremap(mmio_start, mmio_len))){
		printk(KERN_ERR "Ioremap failed.\n");
		iounmap(myPci.hw_addr);
		pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
	}

	pci_enable_device(pdev);

	//Reset interrupts.
	writel(0xFFFF, myPci.hw_addr + IMC);

	//Setup descriptor
	writel((uint32_t)dRing, myPci.hw_addr + RDBAL); //Set descriptor address low half.
	writel(((uint32_t)dRing >> 32), myPci.hw_addr + RDBAH); //Set descriptor address high half.
	writel(NUM_DESC, myPci.hw_addr + RDLEN); //Set number of descriptors.
	//writel((uint32_t)dRing, myPci.hw_addr + RDH); //Set head pointer to beginning of ring.
	//writel((uint32_t)dRing, myPci.hw_addr + RDT); //Set tail pointer to beginning of ring.
	writel(RCTL_SET.set, myPci.hw_addr + RCTL); //Start reception and set operating parameters.

	//TEST
	printk(KERN_INFO "BAR: %lx\n", myPci.hw_addr);
	printk(KERN_INFO "RDBAL Offset: %lx\n", RDBAL);
	printk(KERN_INFO "Total: %lx\n", myPci.hw_addr + RDBAL);
	printk(KERN_INFO "Wrote: %lx\n\n", (uint32_t)dRing);
	//TEST//TEST
	printk(KERN_INFO "BAR: %lx\n", myPci.hw_addr);
	printk(KERN_INFO "RDBAH Offset: %lx\n", RDBAH);
	printk(KERN_INFO "Total: %lx\n", myPci.hw_addr + RDBAH);
	printk(KERN_INFO "Wrote: %lx\n\n", ((uint32_t)dRing >> 32));
	//TEST//TEST
	printk(KERN_INFO "BAR: %lx\n", myPci.hw_addr);
	printk(KERN_INFO "RDLEN Offset: %lx\n", RDLEN);
	printk(KERN_INFO "Total: %lx\n\n", myPci.hw_addr + RDLEN);
	//TEST//TEST
	printk(KERN_INFO "BAR: %lx\n", myPci.hw_addr);
	printk(KERN_INFO "RCTL Offset: %lx\n", RCTL);
	printk(KERN_INFO "Total: %lx\n\n", myPci.hw_addr + RCTL);
	//TEST

	//Turn on LED0.
	writel(zeroOn, myPci.hw_addr + 0x00E00);

	//Everything seems fine, blinky time.
	//myDev.led_initial_val = readl(myPci.hw_addr + 0x00E00);
	//printk(KERN_INFO "Initial value is: %lx\n", myDev.led_initial_val);

	return 0;
}

//========================================
//Remove
static void pci_blinkDriver_remove(struct pci_dev* pdev){
	iounmap(myPci.hw_addr);
	pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
	pci_disable_device(pdev);
	printk(KERN_INFO "Blinky PCI driver removed.\n");
}

//========================================
//Open
static int chardev_open(struct inode *inode, struct file *file){

	printk(KERN_INFO "File opened!\n");

	//Start timer
	mod_timer(&blinkTimer, (HZ/blink_rate)+jiffies);

	return 0;
}

//========================================
//Read
static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset){

	union body{
		uint16_t whole;
		struct split{
			uint8_t head;
			uint8_t tail;
		} split;
	} body;

	uint32_t temp;

	if(*offset >= sizeof(int)){
		return 0;
	}

	//Make sure our user wasn't bad...
	if (!buf) {
		return -EINVAL; //Invalid input.
	}

	//Get head and tail.
	temp = readl(myPci.hw_addr + RDH);
	printk(KERN_INFO "Head: %d\n", temp);
	body.split.head = (uint8_t)temp;
	temp = readl(myPci.hw_addr + RDT);
	printk(KERN_INFO "Tail: %d\n", temp);
	body.split.tail = (uint8_t)temp;

	//Send
	if(copy_to_user(buf, &body.whole, sizeof(body.whole))) {
		return -EFAULT; //Send error.
	}

	*offset += len;

	printk(KERN_INFO "User got from us %d\n", body.whole);

	return sizeof(int);
}

//========================================
//Write
static ssize_t chardev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset){

	int val;

	//Make sure our user wasn't bad...
	if (!buf) {
		return -EINVAL; //Invalid input.
	}

	//Copy from the user-provided buffer
	if (copy_from_user(&val, buf, len)) {
		/* uh-oh... */
		return -EFAULT;
	}

	/* print what userspace gave us */
	printk(KERN_INFO "Userspace wrote \"%d\" to us\n", val);

	//Sanity check input.
	if(val >= 0){
		blink_rate = val;
	}
	else{
		return -EINVAL; //Invalid input.
	}

	return len;

}

MODULE_AUTHOR("Jordan Bergmann");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.3");
module_init(chardev_init);
module_exit(chardev_exit);