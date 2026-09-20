#ifndef MCP9808_DEVICE_H_
#define MCP9808_DEVICE_H_

#define MCP9808_DEBUG 1 

#undef PDEBUG
#ifdef MCP9808_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "device: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

struct mcp9808_dev {
     struct i2c_client *client;
     struct cdev cdev;
	dev_t devt;
     int tmp;
     unsigned char *data;
     struct mutex lock;
};

#endif // MCP9808_DEVICE_H_