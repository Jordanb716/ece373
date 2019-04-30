//==============================================================================
//Kernel Device: myCharDev.c
//Author: Jordan Bergmann
//
//Description: A basic char device. Now with PCI support!
//==============================================================================

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pci.h>
#include <linux/netdevice.h>

#define NUMDEVS 1
#define DEVNAME "blinky"

//Function prototypes
static int chardev_open(struct inode *inode, struct file *file);
static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset);
static ssize_t chardev_write(struct file *file, const char __user *buf, size_t len, loff_t *offset);
static int pci_blinkDriver_probe(struct pci_dev* pdev, const struct pci_device_id* ent);
static void pci_blinkDriver_remove(struct pci_dev* pdev);

//Structs
static struct mydev_dev {
	struct cdev cdev;
	dev_t devNode;
	long led_initial_val;
} myDev;

static struct myPci{
	struct pci_dev* pdev;
	void* hw_addr;
} myPci;

//Variables
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

char blinkDriverName[] = DEVNAME;

//========================================
//Initialization
int __init chardev_init(void){

	printk(KERN_INFO "Initializing myCharDev module!\n");

	//Allocate major/minor numbers.
	if(alloc_chrdev_region(&myDev.devNode, 0, NUMDEVS, "myCharDev")){
		printk(KERN_ERR "alloc_chrdev_region() failed!\n");
		return -1;
	}

	printk(KERN_INFO "Allocated %d devices at major: %d\n", NUMDEVS, MAJOR(myDev.devNode));

	//Initialize cdev.
	cdev_init(&myDev.cdev, &mydev_fops);
	myDev.cdev.owner = THIS_MODULE;

	//Add cdev.
	if(cdev_add(&myDev.cdev, myDev.devNode, NUMDEVS)){
		printk(KERN_ERR "cdev_add() failed!\n");
		/* clean up chrdev allocation */
		unregister_chrdev_region(myDev.devNode, NUMDEVS);
		return -1;
	}

	//Register as pci driver.
	if(pci_register_driver(&pci_blinkDriver)){
		printk(KERN_ERR "PCI registration failed.\n");
		pci_unregister_driver(&pci_blinkDriver);
		unregister_chrdev_region(myDev.devNode, NUMDEVS);
		return -1;
	}

	return 0;
}

//========================================
//Exit
void __exit chardev_exit(void){

	//Clean up.
	pci_unregister_driver(&pci_blinkDriver);
	cdev_del(&myDev.cdev);
	unregister_chrdev_region(myDev.devNode, NUMDEVS);

	printk(KERN_INFO "myCharDev module unloaded!\n");
}

//========================================
//Probe
static int pci_blinkDriver_probe(struct pci_dev* pdev, const struct pci_device_id* ent){

	resource_size_t mmio_start, mmio_len;
	unsigned long barMask;

	printk(KERN_INFO "Blink driver pci called.\n");

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

	//Everything seems fine, blinky time.
	myDev.led_initial_val = readl(myPci.hw_addr + 0xE00);
	printk(KERN_INFO "Initial value is: %lx\n", myDev.led_initial_val);

	return 0;
}

//========================================
//Remove
static void pci_blinkDriver_remove(struct pci_dev* pdev){
	iounmap(myPci.hw_addr);
	pci_release_selected_regions(pdev, pci_select_bars(pdev, IORESOURCE_MEM));
	printk(KERN_INFO "Blinky PCI driver removed.\n");
}

//========================================
//Open
static int chardev_open(struct inode *inode, struct file *file){

	printk(KERN_INFO "pop goes the example!\n");

	return 0;
}

//========================================
//Read
static ssize_t chardev_read(struct file *file, char __user *buf, size_t len, loff_t *offset){

	int val = readl(myPci.hw_addr + 0xE00);

	if(*offset >= sizeof(int)){
		return 0;
	}

	//Make sure our user wasn't bad...
	if (!buf) {
		return -EINVAL; //Invalid input.
	}

	//Send
	if(copy_to_user(buf, &val, sizeof(int))) {
		return -EFAULT; //Send error.
	}

	*offset += len;

	printk(KERN_INFO "User got from us %d\n", val);
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
	printk(KERN_INFO "Userspace wrote \"%d\" to us\n", &val);

	//Write to PCI
	writel(val, myPci.hw_addr + 0xE00);

	printk(KERN_INFO "Sent \"%d\" to device\n", &val);

	return len;

}

MODULE_AUTHOR("Jordan Bergmann");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");
module_init(chardev_init);
module_exit(chardev_exit);