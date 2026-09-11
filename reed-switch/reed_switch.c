#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/printk.h>
#include <linux/cdev.h>
#include "device.h"
#define GPIO_18 (18)

int reed_switch_major =   0;
int reed_switch_minor =   0;

MODULE_AUTHOR("Brett Lange");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Device driver for a magnetic reed switch connected to the GPIO 18 pin of a Raspberry Pi 4b.");
MODULE_VERSION("1.0.0");

struct reed_switch_dev reed_switch_device;
static struct class *dev_class;

/***
 * Open function for the GPIO character device
 */
int reed_switch_open(struct inode *inode, struct file *filp)
{
    struct reed_switch_dev *dev;
	
	dev = container_of(inode->i_cdev, struct reed_switch_dev, cdev);
	filp->private_data = dev;
	
    return 0;
}

/**
 * Release function for the GPIO character device.
 * Nothing is needed to be done in this function.
 */
int reed_switch_release(struct inode *inode, struct file *filp)
{
    return 0;
}

/***
 * Read function for the GPIO character device.
 * Sets buf to 0 for open and 1 for closed.
 */
ssize_t reed_switch_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    uint8_t gpio_states = 0;
    struct reed_switch_dev *dev = filp->private_data; 

    PDEBUG("read gpio value");
	
	if (gpio_get_value(GPIO_18) == 0)
    {		
        gpio_states |= 1;
    }
	
	if (mutex_lock_interruptible(&dev->lock))
		return -ERESTARTSYS;
	
	if (count < 1) 
    {
		retval = -EFAULT;
		mutex_unlock(&dev->lock);
        return retval;
	}

	if (copy_to_user(buf, &gpio_states, 1)) 
    {
		retval = -EFAULT;
		mutex_unlock(&dev->lock);
        return retval;
	}
	
	retval = 1;
	*f_pos = 0;

	mutex_unlock(&dev->lock);
    return retval;
}

struct file_operations reed_switch_fops = {
    .owner =    THIS_MODULE,
    .read =     reed_switch_read,
    .open =     reed_switch_open,
    .release =  reed_switch_release,
};

/**
 * Reed switch initialize module function
 */
int reed_switch_init_module(void)
{
    dev_t dev = 0;
    int res = 0;
    int devno = -1;

    res = alloc_chrdev_region(&dev, reed_switch_minor, 1, "reed_switch");
    if (res < 0)
    {
        printk(KERN_WARNING "Unable to dynamically assign major number. Returned error from alloc_chrdev_region: %d\n", res);
        return res;
    }
    reed_switch_major = MAJOR(dev);

    memset(&reed_switch_device, 0, sizeof(struct reed_switch_dev));
    mutex_init(&reed_switch_device.lock);

    devno = MKDEV(reed_switch_major, reed_switch_minor);
    cdev_init(&reed_switch_device.cdev, &reed_switch_fops);
    reed_switch_device.cdev.owner = THIS_MODULE;
    reed_switch_device.cdev.ops = &reed_switch_fops;
    res = cdev_add (&reed_switch_device.cdev, devno, 1);
    if (res)
    {
        printk(KERN_ERR "Error adding switch cdev. Return from cdev_add: %d", res);
        unregister_chrdev_region(dev, 1);
		return res;
    }

    // Create class struct
    if(IS_ERR(dev_class = class_create(THIS_MODULE,"switch_class"))){
        printk(KERN_ERR "Cannot create the struct class\n");
        class_destroy(dev_class);
        return -1;
    }
 
    // Create device
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "switch"))){
        printk(KERN_ERR "Cannot create the Device \n");
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
  
    // Check if the GPIO is valid
    if (gpio_is_valid(GPIO_18) == false){
        printk(KERN_ERR "GPIO %d is not valid\n", GPIO_18);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
  
    // Request the GPIO
	res = gpio_request(GPIO_18, "GPIO_18");
    if (res < 0){
        printk(KERN_ERR "ERROR: GPIO %d request\n", GPIO_18);
        gpio_free(GPIO_18);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
	
    // Make the GPIO visible, direction cannot be changed by the user
    res = gpio_export(GPIO_18, false);
    if (res < 0) 
    {
        printk(KERN_ERR "ERROR: GPIO %d export with failure code: %d\n", GPIO_18, res);
        gpio_free(GPIO_18);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
	
	// Configure the GPIO as an input
	res = gpio_direction_input(GPIO_18);
    if (res < 0) 
    {
        printk(KERN_ERR "ERROR: GPIO %d direction failed to set to input. Failure code: %d\n", GPIO_18, res);
        gpio_free(GPIO_18);
        device_destroy(dev_class, dev);
        class_destroy(dev_class);
        return -1;
    }
	
    return 0;
}

/**
 * Reed siwtch cleanup module function
 */
void reed_switch_cleanup_module(void)
{
    dev_t devno = MKDEV(reed_switch_major, reed_switch_minor);
    
	gpio_unexport(GPIO_18);
    gpio_free(GPIO_18);
    device_destroy(dev_class, devno);
    class_destroy(dev_class);
    cdev_del(&reed_switch_device.cdev);
    unregister_chrdev_region(devno, 1);
}

module_init(reed_switch_init_module);
module_exit(reed_switch_cleanup_module);