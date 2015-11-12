/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>

/*
 * template_init - function to insert this module into kernel space
 *
 * This is the first of two exported functions to handle inserting this
 * code into a running kernel
 *
 * Returns 0 if successfull, otherwise -1
 */
static ssize_t gpio_read(struct file* filp, char __user* buff, size_t count, loff_t* offp);
static ssize_t gpio_write(struct file* filp, const char __user* buff, size_t count,  loff_t* offp);
static int gpio_open(struct inode* inode, struct file* filp);
static int gpio_release(struct inode* inode, struct file* filp);
static int gpio_fasync(int fd, struct file* filp, int mode);

static dev_t dev_num;
struct cdev gpio_cdev;
struct fasync_struct* async_queue;
struct class* cl;

static struct file_operations gpio_fops = {
  .owner = THIS_MODULE,
  .read = gpio_read,
  .write = gpio_write,
  .open = gpio_open,
  .release = gpio_release,
  .fasync = gpio_fasync
};


static int __init template_init(void)
{
	int x =	request_mem_region(FIRST, LENGTH, NAME);
	if (x == NULL){
	return -1;

	ioremap_nocache();
	return 0;
	
	 cdev_init(&gpio_cdev, &gpio_fops);
 gpio_cdev.owner = THIS_MODULE;
 gpio_cdev.ops = &gpio_fops;

 if (cdev_add(&gpio_cdev, dev_num,1)) {
   printk(KERN_INFO "FAILURE cdev_add");
   return -1;
 }
 cl = class_create(THIS_MODULE, DEVICE_NAME);
 device_create(cl, NULL, dev_num, NULL, DEVICE_NAME);


 printk(KERN_INFO "Hello World, here is your GPIO-module speaking\n");
 return 0;

}

/*
 * template_cleanup - function to cleanup this module from kernel space
 *
 * This is the second of two exported functions to handle cleanup this
 * code from a running kernel
 */

static void __exit template_cleanup(void)
{
	
	void unregister_chrdev_region(dev_t first, unsigned int count);
	
	 printk("Short life for a small module...\n");

  // remove irq handlers
  free_irq(17, &gpio_cdev);
  free_irq(18, &gpio_cdev);

  // destroy device driver
  device_destroy(cl, dev_num);
  class_destroy(cl);
  cdev_del(&gpio_cdev);
	
}

struct resource *request_region(unsigned long first, unsigned long n, const char *name);






// See I/O Allocation chapter 9 in LDD



module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");



