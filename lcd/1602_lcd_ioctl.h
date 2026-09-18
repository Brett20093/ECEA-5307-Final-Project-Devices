#ifndef LCD_IOCTL_H_
#define LCD_IOCTL_H_

#ifdef __KERNEL__
#include <asm-generic/ioctl.h>
#include <linux/types.h>
#else
#include <sys/ioctl.h>
#include <stdint.h>
#endif

/**
 * A structure to be passed by IOCTL from user space to kernel space, describing the type
 * of seek performed on the aesdchar driver
 */
struct lcd_cursor 
{
    uint8_t row; // 0 or 1
    uint8_t col; // 0 to 15
};

// Pick an arbitrary unused value from https://github.com/torvalds/linux/blob/master/Documentation/userspace-api/ioctl/ioctl-number.rst
#define LCD_IOCTL_MAGIC 0x16

// Define a write command from the user point of view, use command number 1
#define LCD_IOCTL_SETCURSOR _IOW(LCD_IOCTL_MAGIC, 1, struct lcd_cursor)
/**
 * The maximum number of commands supported, used for bounds checking
 */
#define LCD_IOCTL_MAXNR 1

#endif // LCD_IOCTL_H_
