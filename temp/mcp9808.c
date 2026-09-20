#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include "device.h"

#define DRIVER_NAME "mcp9808"

#define MCP9808_I2C_ADDRESS 0x18

int reed_switch_major =   0;
int reed_switch_minor =   0;

static struct class *dev_class;

MODULE_AUTHOR("Brett Lange");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Device driver for an Adafruit MCP9808 I2C Temperature Sensor.");
MODULE_VERSION("1.0.0");

static const struct of_device_id mcp9808_of_match[] = {
    { .compatible = "adafruit,mcp9808" },
    { /* Sentinel - marks the end of the array */ }
};
MODULE_DEVICE_TABLE(of, mcp9808_of_match);

int mcp9808_open(struct inode *inode, struct file *filp)
{
    
    return 0;
}

ssize_t mcp9808_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
    return 0;
}

int mcp9808_release(struct inode *indoe, struct file *filp)
{
    return 0;
}

struct file_operations mcp9808_fops = {
    .owner   = THIS_MODULE,
	.open    = mcp9808_open,
    .read    = mcp9808_read,
	.release = mcp9808_release,
};

static int mcp9808_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct mcp9808_dev *dev;
    int ret = 0;
    struct i2c_msg msg[2];
    unsigned char i2c_addr = MCP9808_I2C_ADDRESS;
    unsigned char data[2];

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
        PDEBUG("ERROR: MCP9808 probe i2c_check_functionality.");
        return -ENODEV;
    }
    PDEBUG("SUCCESS: MCP9808 probe i2c_check_functionality.");

    msg[0].addr = client->addr;
    msg[0].flags = 0;
    msg[0].len = 1;
    msg[0].buf = &i2c_addr;
    msg[1].addr = client->addr;
    msg[1].flags = I2C_M_RD;
    msg[1].len = 2;
    msg[1].buf = data;

    if (i2c_transfer(client->adapter, msg, 2) < 0)
    {
        PDEBUG("ERROR: MCP9808 probe i2c_transfer");
        return -ENODEV;
    }
    PDEBUG("SUCCESS: MCP9808 probe i2c_transfer, manufacture ID = %x", (data[0] << 8) | data[1]);

    ret = alloc_chrdev_region(&dev->devt, 0, 1, DRIVER_NAME);
    if (ret)
    {
        PDEBUG("ERROR: MCP9808 probe alloc_chrdev_region");
        return ret;
    }
    PDEBUG("SUCCESS: MCP9808 probe alloc_chrdev_region");

    if (!dev_class) 
    {
		if(IS_ERR(dev_class = class_create(THIS_MODULE, DRIVER_NAME)))
        {
			PDEBUG("ERROR: MCP9808 probe class_create");
			dev_class = NULL;
            unregister_chrdev_region(dev->devt, 1);
            return -EINVAL;
		}
        PDEBUG("SUCCESS: MCP9808 probe class_create");
	}
    
    dev = kzalloc(sizeof(struct mcp9808_dev), GFP_KERNEL);
    if (dev == NULL)
    {
        PDEBUG("ERROR: MCP9808 probe kzalloc mcp9808_dev");
        device_destroy(dev_class, dev->devt);
        unregister_chrdev_region(dev->devt, 1);
        return -ENOMEM;
    }
    PDEBUG("SUCCESS: MCP9808 probe kzalloc mcp9808_dev");
    dev->data = NULL;
    dev->client = client;

    cdev_init(&dev->cdev, &mcp9808_fops);
    dev->cdev.owner = THIS_MODULE;

    ret = cdev_add(&dev->cdev, dev->devt, 1);
    if (ret) 
    {
        PDEBUG("ERROR: MCP9808 probe cdev_add");
        device_destroy(dev_class, dev->devt);
        unregister_chrdev_region(dev->devt, 1);
        return ret;
    }
    PDEBUG("SUCCESS: MCP9808 probe cdev_add");

    if(IS_ERR(device_create(dev_class, NULL, dev->devt, NULL, DRIVER_NAME)))
    {
        PDEBUG("ERROR: MCP9808 probe device_create");
        device_destroy(dev_class, dev->devt);
        cdev_del(&dev->cdev);
        unregister_chrdev_region(dev->devt, 1);
        return -EINVAL;
    }
    PDEBUG("SUCCESS: MCP9808 probe device_create");

    i2c_set_clientdata(client, dev);

    return 0;
}

static void mcp9808_remove(struct i2c_client *client)
{
    struct mcp9808_dev *mcp_dev = i2c_get_clientdata(client);
	
	device_destroy(dev_class, mcp_dev->devt);
    cdev_del(&mcp_dev->cdev);
    unregister_chrdev_region(mcp_dev->devt, 1);
	class_destroy(dev_class);
    kfree(mcp_dev);

    PDEBUG("MCP9808 Removed");
}

static struct i2c_driver lcd_1602_driver = {
    .driver = {
        .name           = DRIVER_NAME,
		.owner          = THIS_MODULE,
        .of_match_table = mcp9808_of_match,
    },
    .probe     = mcp9808_probe,
    .remove    = mcp9808_remove,
};
	
module_i2c_driver(lcd_1602_driver);