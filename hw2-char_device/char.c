//==============================================================================
//Kernel Device: char.c
//Author: Jordan Bergmann
//
//Description: A basic char device.
//==============================================================================

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#define NUMDEVS 1

static dev_t devNode;

int __init chardev_init(void)
{
	printk(KERN_INFO "Loading module.\n"):

	//Allocate major/minor numbers.
	if(alloc_chrdev_region(&devNode, 0, NUMDEVS, "myCharDev")){
		printk(KERN_ERR "alloc_chrdev_region() failed!\n");
		return -1;
	}

	return 0;
}

void __exit chardev_exit(void)
{
	//Clean up.
	unregister_chrdev_region(devNode, NUMDEVS);

	printk(KERN_INFO "example3 module unloaded!\n");
}

MODULE_AUTHOR("Jordan Bergmann");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(chardev_init);
module_exit(chardev_exit);