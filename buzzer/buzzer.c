#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/printk.h>
#include <linux/cdev.h>
#include "device.h"
#define GPIO_23 (23)

int buzzer_major =   0;
int buzzer_minor =   0;

MODULE_AUTHOR("Brett Lange");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Device driver for an audio buzzer connected to the GPIO 23 pin of a Raspberry Pi 4b.");
MODULE_VERSION("1.0.0");

struct buzzer_dev buzzer_device;
static struct class *dev_class;

/***
 * Open function for the GPIO character device
 */
int buzzer_open(struct inode *inode, struct file *filp)
{
    struct buzzer_dev *dev;
	
	dev = container_of(inode->i_cdev, struct buzzer_dev, cdev);
	filp->private_data = dev;
	
    return 0;
}

/**
 * Release function for the GPIO character device.
 * Nothing is needed to be done in this function.
 */
int buzzer_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/***
 * Write function for the GPIO character device.
 * Turns on the GPIO if buf is 1, turns it off if buf is 0.
 */
ssize_t buzzer_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
    struct buzzer_dev *dev = filp->private_data;
    char local_buf;

    if (len > 1)
    {
        len = 1;
    }
    else if (len < 1) 
    {
        return -EFAULT;
	}

    if (mutex_lock_interruptible(&dev->lock))
    {
		return -ERESTARTSYS;
    }

	if (copy_from_user(&local_buf, buf, len)) 
    {
		mutex_unlock(&dev->lock);
        return -EFAULT;
	}

    dev->gpio_state = (int)local_buf;
    if (dev->gpio_state > 1)
    {
        dev->gpio_state = 1;
    }
    else if (dev->gpio_state < 0)
    {
        dev->gpio_state = 0;
    }

    gpio_set_value(GPIO_23, dev->gpio_state);

    mutex_unlock(&dev->lock);
	
	*off = 0;

    return 1;
}

struct file_operations buzzer_fops = {
    .owner =   THIS_MODULE,
    .write =   buzzer_write,
    .open =    buzzer_open,
    .release = buzzer_release,
};

/**
 * Buzzer initialize module function
 */
int buzzer_init_module(void)
{
    dev_t dev = 0;
    int res = 0;
    int devno = -1;

    res = alloc_chrdev_region(&dev, buzzer_minor, 1, "buzzer");
    if (res < 0)
    {
        printk(KERN_WARNING "Unable to dynamically assign major number. Returned error from alloc_chrdev_region: %d\n", res);
        return res;
    }
    buzzer_major = MAJOR(dev);

    memset(&buzzer_device, 0, sizeof(struct buzzer_dev));
    mutex_init(&buzzer_device.lock);

    devno = MKDEV(buzzer_major, buzzer_minor);
    cdev_init(&buzzer_device.cdev, &buzzer_fops);
    buzzer_device.cdev.owner = THIS_MODULE;
    buzzer_device.cdev.ops = &buzzer_fops;
    res = cdev_add (&buzzer_device.cdev, devno, 1);
    if (res)
    {
        printk(KERN_ERR "Error adding switch cdev. Return from cdev_add: %d", res);
        unregister_chrdev_region(dev, 1);
		return res;
    }

    // Create class struct
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"buzzer_class"))){
        printk(KERN_ERR "Cannot create the struct class\n");
        class_destroy(dev_class);
        return -1;
    }
 
    // Create device
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "buzzer"))){
        printk(KERN_ERR "Cannot create the Device \n");
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
  
    // Check if the GPIO is valid
    if (gpio_is_valid(GPIO_23) == false){
        printk(KERN_ERR "GPIO %d is not valid\n", GPIO_23);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
  
    // Request the GPIO
	res = gpio_request(GPIO_23, "GPIO_23");
    if (res < 0){
        printk(KERN_ERR "ERROR: GPIO %d request\n", GPIO_23);
        gpio_free(GPIO_23);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
	
    // Make the GPIO visible, direction cannot be changed by the user
    res = gpio_export(GPIO_23, false);
    if (res < 0) 
    {
        printk(KERN_ERR "ERROR: GPIO %d export with failure code: %d\n", GPIO_23, res);
        gpio_free(GPIO_23);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
	
	// Configure the GPIO as an input
	res = gpio_direction_output(GPIO_23, 0);
    if (res < 0) 
    {
        printk(KERN_ERR "ERROR: GPIO %d direction failed to set to output. Failure code: %d\n", GPIO_23, res);
        gpio_free(GPIO_23);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
	
    return 0;
}

/**
 * Buzzer cleanup module function
 */
void buzzer_cleanup_module(void)
{
    dev_t devno = MKDEV(buzzer_major, buzzer_minor);
    
	gpio_unexport(GPIO_23);
    gpio_free(GPIO_23);
    device_destroy(dev_class, devno);
    class_destroy(dev_class);
    cdev_del(&buzzer_device.cdev);
    unregister_chrdev_region(devno, 1);
}

module_init(buzzer_init_module);
module_exit(buzzer_cleanup_module);