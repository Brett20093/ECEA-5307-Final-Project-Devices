#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/cdev.h>
#include "device.h"

#define DRIVER_NAME "mcp9808"

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

static int mcp9808_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    return 0;
}

static void mcp9808_remove(struct i2c_client *client)
{
    
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