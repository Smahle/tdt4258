/*
 * This is a demo Linux kernel module.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/ioport.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/irqreturn.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <linux/irq.h>
#include <linux/signal.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include "efm32gg.h"

#define DEVICE_NAME "GPIO_PAD"
 
static irqreturn_t interrupt_handler(int irq, void* dev_id, struct pt_regs* regs);
 
 
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

// fetch device num, see pg. 45 in ldd (cp. 3)
 if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE alloc_chrdev_regeion!");
   return -1;
}

 // setup I/O ports:
 if (!request_mem_region(GPIO_PC_MODEL, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_PC_MODEL");
   return -1;
 }
 if (!request_mem_region(GPIO_PC_DOUT, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_PC_DOUT");
   return -1;
 }

 if (!request_mem_region(GPIO_EXTIPSELL, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_EXTIPSELL");
   return -1;
 }

 if (!request_mem_region(GPIO_EXTIFALL, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_EXTIFALL");
   return -1;
 }

 if (!request_mem_region(GPIO_IEN, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_IEN");
   return -1;
 }
 if (!request_mem_region(GPIO_IFC, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_IFC");
   return -1;
 }

 // set up GPIO registers!
 iowrite32(0x33333333, GPIO_PC_MODEL); /* set pins 0-7 to input */
 iowrite32(0xff,       GPIO_PC_DOUT); /* internal pullup ... */
 iowrite32(0x22222222, GPIO_EXTIPSELL);

 // set up GPIO interrupts!
 if (request_irq(17, (irq_handler_t) interrupt_handler, 0, DEVICE_NAME, &gpio_cdev) || // EVEN
     request_irq(18, (irq_handler_t) interrupt_handler, 0, DEVICE_NAME, &gpio_cdev)) { // ODD
   printk(KERN_INFO "FAILURE request_irq!");
   return -1;
 }

 iowrite32(0xff,       GPIO_EXTIFALL);
 iowrite32(0xff,       GPIO_IEN);
 iowrite32(0xff,       GPIO_IFC);
	
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


static ssize_t gpio_read(struct file* filp, char __user* buff, size_t count, loff_t* offp) {
  uint8_t gpio_button_state = ioread8(GPIO_PC_DIN);
  if (copy_to_user(buff, &gpio_button_state, 1)) {
    printk(KERN_INFO "READ FAILURE");
    return 0;
  }
  return 1; // byte length, not error code
}

static ssize_t gpio_write(struct file* filp, const char __user* buff, size_t count,  loff_t* offp) {
  printk(KERN_INFO "GPIO WRITTEN?!");
  return 0;
}

static int gpio_open(struct inode* inode, struct file* filp) {
  printk(KERN_INFO "GPIO OPENED?!");
  return 0;
}

static int gpio_release(struct inode* inode, struct file* filp) {
  printk(KERN_INFO "GPIO RELEASED?!");
  return 0;
}

static int gpio_fasync(int fd, struct file* filp, int mode) {
  // pg 171 cp 09 in ldd
  return fasync_helper(fd, filp, mode, &async_queue);
}

module_init(template_init);
module_exit(template_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");



