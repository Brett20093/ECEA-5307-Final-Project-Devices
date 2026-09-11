#ifndef REED_SWITCH_DEVICE_H_
#define REED_SWITCH_DEVICE_H_

#include <linux/cdev.h>

#define REED_SWITCH_DEBUG 1

#undef PDEBUG
#ifdef REED_SWITCH_DEBUG
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

// Reed switch device structure
struct reed_switch_dev
{
   struct mutex lock;
   struct cdev  cdev;
};
#endif // REED_SWITCH_DEVICE_H_
