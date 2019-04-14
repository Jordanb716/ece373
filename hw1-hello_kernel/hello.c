#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");

static int__init hello_init(void){

	printk(KERN_INTO "Hello, kernel\n");
	return 0;

}

static void__exit hello_exit (void){

	printk(KERN_INFO "Goodbye, kernel\n");

}

module_init(hello_init);
module_exit(hello_exit);
