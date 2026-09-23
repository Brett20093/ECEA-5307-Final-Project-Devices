#ifndef BUZZER_DEVICE_H_
#define BUZZER_DEVICE_H_

#include <linux/cdev.h>

#define BUZZER_DEBUG 1

#undef PDEBUG
#ifdef BUZZER_DEBUG
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
struct buzzer_dev
{
   struct mutex lock;
   struct cdev  cdev;
   int gpio_state;
};
#endif // BUZZER_DEVICE_H_
