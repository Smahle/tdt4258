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

static uint8_t interrupt_value;

static int __init Driver_init(void)
{
 interrupt_value = 0;

 // fetch device num, see pg. 45 in ldd (cp. 3)
 if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE alloc_chrdev_regeion!\n");
   return -1;
 }

 // setup I/O ports:
 if (!request_mem_region((unsigned long)GPIO_PC_MODEL, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_PC_MODEL\n");
   return -1;
 }

 if (!request_mem_region((unsigned long)GPIO_PC_DOUT, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_PC_DOUT\n");
   return -1;
 }

 if (!request_mem_region((unsigned long)GPIO_EXTIPSELL, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_EXTIPSELL\n");
   return -1;
 }

 if (!request_mem_region((unsigned long)GPIO_EXTIRISE, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_EXTIRISE\n");
   return -1;
 }
 if (!request_mem_region((unsigned long)GPIO_EXTIFALL, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_EXTIFALL\n");
   return -1;
 }

 if (!request_mem_region((unsigned long)GPIO_IEN, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_IEN\n");
   return -1;
 }
 if (!request_mem_region((unsigned long)GPIO_IF, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_IF\n");
   return -1;
 }
 if (!request_mem_region((unsigned long)GPIO_IFC, 4, DEVICE_NAME)) {
   printk(KERN_INFO "FAILURE TO ALLOCATE: GPIO_IFC\n");
   return -1;
 }

 // set up GPIO registers!
 iowrite32(0x33333333, GPIO_PC_MODEL); /* set pins 0-7 to input */
 iowrite32(0xff,       GPIO_PC_DOUT); /* internal pullup ... */
 iowrite32(0x22222222, GPIO_EXTIPSELL);

 // set up GPIO interrupts!
 if (request_irq(17, (irq_handler_t) interrupt_handler, 0, DEVICE_NAME, &gpio_cdev) || // EVEN
     request_irq(18, (irq_handler_t) interrupt_handler, 0, DEVICE_NAME, &gpio_cdev)) { // ODD
   printk(KERN_INFO "FAILURE request_irq!\n");
   return -1;
 }

 iowrite32(0xff,       GPIO_EXTIRISE);
 iowrite32(0xff,       GPIO_EXTIFALL);
 iowrite32(0xff,       GPIO_IEN);
 iowrite32(0xff,       GPIO_IFC);
	
 cdev_init(&gpio_cdev, &gpio_fops);
 gpio_cdev.owner = THIS_MODULE;
 gpio_cdev.ops = &gpio_fops;

 if (cdev_add(&gpio_cdev, dev_num,1)) {
   printk(KERN_INFO "FAILURE cdev_add\n");
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

static void __exit Driver_cleanup(void)
{
	
  // release I/O ports
  release_mem_region((unsigned long)GPIO_PC_MODEL, 4);
  release_mem_region((unsigned long)GPIO_PC_DOUT, 4);
  release_mem_region((unsigned long)GPIO_EXTIPSELL, 4);
  release_mem_region((unsigned long)GPIO_EXTIRISE, 4);
  release_mem_region((unsigned long)GPIO_EXTIFALL, 4);
  release_mem_region((unsigned long)GPIO_IEN, 4);
  release_mem_region((unsigned long)GPIO_IF, 4);
  release_mem_region((unsigned long)GPIO_IFC, 4);

  // remove irq handlers
  free_irq(17, &gpio_cdev);
  free_irq(18, &gpio_cdev);

  // destroy device driver
  device_destroy(cl, dev_num);
  class_destroy(cl);
  cdev_del(&gpio_cdev);

  unregister_chrdev_region(dev_num, 1);
  
  printk(KERN_INFO "A small module with short life...\n");	
	
}


irqreturn_t interrupt_handler(int irq, void *dev_id, struct pt_regs *regs) {
  interrupt_value = ioread8(GPIO_IF) & ~ioread8(GPIO_PC_DIN);

  // interrupts are marked as handled
  iowrite32(0xff, GPIO_IFC);
  if (async_queue)
    kill_fasync(&async_queue, SIGIO, POLL_IN);

  printk(KERN_INFO "Interrupt successfully handled (0x%02x).\n", interrupt_value);
  return IRQ_HANDLED;
}


static ssize_t gpio_read(struct file* filp, char __user* buff, size_t count, loff_t* offp) {
  if (copy_to_user(buff, &interrupt_value, 1)) {
    printk(KERN_INFO "READ FAILURE\n");
    return 0;
  }
  return 1;
}

static ssize_t gpio_write(struct file* filp, const char __user* buff, size_t count,  loff_t* offp) {
  printk(KERN_INFO "GPIO WRITTEN?\n");
  return 0;
}

static int gpio_open(struct inode* inode, struct file* filp) {
  printk(KERN_INFO "GPIO OPENED?\n");
  return 0;
}

static int gpio_release(struct inode* inode, struct file* filp) {
  printk(KERN_INFO "GPIO RELEASED?\n");
  return 0;
}

static int gpio_fasync(int fd, struct file* filp, int mode) {
  return fasync_helper(fd, filp, mode, &async_queue);
}

module_init(Driver_init);
module_exit(Driver_cleanup);

MODULE_DESCRIPTION("Small module, demo only, not very useful.");
MODULE_LICENSE("GPL");



