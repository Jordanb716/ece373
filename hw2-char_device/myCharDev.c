//==============================================================================
//Kernel Device: myCharDev.c
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

//Function prototypes
static int chardev_open(struct inode *inode, struct file *file);

//Structs
static struct mydev_dev {
	struct cdev cdev;
	dev_t devNode;
} mydev;

//Variables
static struct file_operations mydev_fops = {
	.owner = THIS_MODULE,
	.open = chardev_open,
};

//Initialization
int __init chardev_init(void)
{
	printk(KERN_INFO "Initializing myCharDev module!\n");

	//Allocate major/minor numbers.
	if(alloc_chrdev_region(&mydev.devNode, 0, NUMDEVS, "myCharDev")){
		printk(KERN_ERR "alloc_chrdev_region() failed!\n");
		return -1;
	}

	printk(KERN_INFO "Allocated %d devices at major: %d\n", NUMDEVS, MAJOR(mydev.devNode));

	//Initialize cdev.
	cdev_init(&mydev.cdev, &mydev_fops);
	mydev.cdev.owner = THIS_MODULE;

	//Add cdev.
	if(cdev_add(&mydev.cdev, mydev.devNode, NUMDEVS)){
		printk(KERN_ERR "cdev_add() failed!\n");
		/* clean up chrdev allocation */
		unregister_chrdev_region(mydev.mydev_node, NUMDEVS);

		return -1;
	}

	return 0;
}

//Exit
void __exit chardev_exit(void)
{
	//Clean up.
	cdev_del(&mydev.cdev);
	unregister_chrdev_region(devNode, NUMDEVS);

	printk(KERN_INFO "myCharDev module unloaded!\n");
}

//Open
static int chardev_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "pop goes the example!\n");

	return 0;
}

MODULE_AUTHOR("Jordan Bergmann");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.2");
module_init(chardev_init);
module_exit(chardev_exit);