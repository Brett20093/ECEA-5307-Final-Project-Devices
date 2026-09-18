#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include <linux/delay.h>
#include "device.h"
#include "1602_lcd_ioctl.h"

#define DRIVER_NAME "1602_lcd"

#define LCD_I2C_ADDRESS 0x27

#define PULSE_PERIOD_U 500
#define CMD_PERIOD_U   4100

#define LCD_REGISTER_SELECT 0x01
#define LCD_READ_WRITE      0x02
#define LCD_ENABLE_PIN      0x04
#define LCD_BACK_LIGHT      0x08

int reed_switch_major =   0;
int reed_switch_minor =   0;

static struct class *dev_class;

MODULE_AUTHOR("Brett Lange");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Device driver for a I2C Geeekpi 1602 LCD Display.");
MODULE_VERSION("1.0.0");

static const struct of_device_id lcd_1602_of_match[] = {
    { .compatible = "geeekpi,lcd1602" },
    { /* Sentinel - marks the end of the array */ }
};

MODULE_DEVICE_TABLE(of, lcd_1602_of_match);

static void lcd_send_byte(struct i2c_client *client, uint8_t byte_to_send, bool register_select)
{
    uint8_t data_msn = (byte_to_send & 0xf0) | LCD_BACK_LIGHT;
    uint8_t data_lsn = (byte_to_send << 4) | LCD_BACK_LIGHT;
    if (register_select)
    {
        data_msn |= LCD_REGISTER_SELECT;
        data_lsn |= LCD_REGISTER_SELECT;
    }

    i2c_master_send(client, &data_msn, 1);
    udelay(PULSE_PERIOD_U);
    data_msn |= LCD_ENABLE_PIN;
    i2c_master_send(client, &data_msn, 1);
    udelay(PULSE_PERIOD_U);
    data_msn &= ~LCD_ENABLE_PIN;
    i2c_master_send(client, &data_msn, 1);
    udelay(PULSE_PERIOD_U);

    i2c_master_send(client, &data_lsn, 1);
    udelay(PULSE_PERIOD_U);
    data_lsn |= LCD_ENABLE_PIN;
    i2c_master_send(client, &data_lsn, 1);
    udelay(PULSE_PERIOD_U);
    data_lsn &= ~LCD_ENABLE_PIN;
    i2c_master_send(client, &data_lsn, 1);
    udelay(CMD_PERIOD_U);
}

static void lcd_send_initialize_commands(struct lcd_dev *dev)
{
    struct i2c_client *client = dev->client;
	
    lcd_send_byte(client, 0x02, false); // Set 4-bit mode of the LCD controller
    lcd_send_byte(client, 0x28, false); // 2 lines, 5x8 dot matrix
    lcd_send_byte(client, 0x0c, false); // Set display on, cursor off
    lcd_send_byte(client, 0x06, false); // Increment cursor to right, don't scroll
    lcd_send_byte(client, 0x80, false); // Set cursor to row 1, col 1
}

static void lcd_1602_set_cursor(struct i2c_client *client, uint8_t row, uint8_t col)
{
	uint8_t data = 0;
	
	if (row > 1) 
    {
        row = 1;
    }
    else if (row < 0)
    {
        row = 0;
    }
    
    if (col > 15) 
    {
        col = 15;
    }
    else if (col < 0)
    {
        col = 0;
    }
    
	if (row)
    {
		data = 0xc0;
    }
	else
    {
		data = 0x80;
    }
	
	data |= col;
	lcd_send_byte(client, data, false);
}

static int lcd_1602_open(struct inode *inode, struct file *f)
{	
	struct lcd_dev *dev;
    
    dev = container_of(inode->i_cdev, struct lcd_dev, cdev);
    f->private_data = dev;

    return 0;
}

static ssize_t lcd_1602_write(struct file *filp, const char __user *buf, size_t count, loff_t *off)
{
	struct lcd_dev *dev = filp->private_data;
    char data[16];
	int i;
	
	if (count > sizeof(data))
    {
        count = sizeof(data);
    }

    if (copy_from_user(data, buf, count))
    {
        return -EFAULT;
    }
	
	for (i = 0; i < count; i++)
    {
		lcd_send_byte(dev->client, data[i], true);
	}
	
	return count;
}

static int lcd_1602_release(struct inode *inode, struct file *filp)
{
    return 0;
}

long lcd_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct lcd_dev *dev = filp->private_data;
	int retval = 0;

	if (_IOC_TYPE(cmd) != LCD_IOCTL_MAGIC)
    {
        return -ENOTTY;
    }

	if (_IOC_NR(cmd) > LCD_IOCTL_MAXNR)
    {
        return -ENOTTY;
    }
    
    if (cmd == LCD_IOCTL_SETCURSOR)
    {
        struct lcd_cursor pos;
        if (copy_from_user(&pos, (const void __user *)arg, sizeof(pos)) != 0)
        {
            retval = -EFAULT;
        }
        else
        {
            lcd_1602_set_cursor(dev->client, pos.row, pos.col);
        }
    }
	else
    {
        return -ENOTTY;
    }

    return retval;	
}

struct file_operations lcd_1602_fops = {
    .owner          = THIS_MODULE,
	.open           = lcd_1602_open,
    .write          = lcd_1602_write,
	.release        = lcd_1602_release,
	.unlocked_ioctl = lcd_ioctl,
};

static int lcd_1602_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct lcd_dev *dev;
    int ret = 0;

    dev = devm_kzalloc(&client->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
    {
        return -ENOMEM;
    }

    dev->client = client;
    i2c_set_clientdata(client, dev);
    
    ret = alloc_chrdev_region(&dev->devt, 0, 1, DRIVER_NAME);
    if (ret)
    {
        return ret;
    }

	cdev_init(&dev->cdev, &lcd_1602_fops);
    dev->cdev.owner = THIS_MODULE;

    ret = cdev_add (&dev->cdev, dev->devt, 1);
    if (ret) 
    {
        printk(KERN_ERR "Error %d adding lcd1602 cdev", ret);
        unregister_chrdev_region(dev->devt, 1);
        return ret;
    }
	
	if (!dev_class) 
    {
		if(IS_ERR(dev_class = class_create(THIS_MODULE, "lcd_class")))
        {
			printk(KERN_ERR "Cannot create the struct class\n");
			dev_class = NULL;
			cdev_del(&dev->cdev);
            unregister_chrdev_region(dev->devt, 1);
            return -EINVAL;
		}
	}
 
    if(IS_ERR(device_create(dev_class, NULL, dev->devt, NULL, DRIVER_NAME)))
    {
        printk(KERN_ERR "Cannot create the Device \n");
        device_destroy(dev_class, dev->devt);
        cdev_del(&dev->cdev);
        unregister_chrdev_region(dev->devt, 1);
        return -EINVAL;
    }
    
    lcd_send_initialize_commands(dev);
	
	return ret;
}

static void lcd_1602_remove(struct i2c_client *client)
{
	struct lcd_dev *dev = i2c_get_clientdata(client);
	
	device_destroy(dev_class, dev->devt);
    cdev_del(&dev->cdev);
    unregister_chrdev_region(dev->devt, 1);
	class_destroy(dev_class);
}

static struct i2c_driver lcd_1602_driver = {
    .driver = {
        .name           = DRIVER_NAME,
		.owner          = THIS_MODULE,
        .of_match_table = lcd_1602_of_match,
    },
    .probe     = lcd_1602_probe,
    .remove    = lcd_1602_remove,
};
	
module_i2c_driver(lcd_1602_driver);
