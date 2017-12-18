#include <linux/init.h>          
#include <linux/module.h>       
#include <linux/device.h>         
#include <linux/kernel.h>        
#include <linux/fs.h>             
#include <asm/uaccess.h>
#include "chardevice.h"        
#define  DEVICE_NAME "chardevice"    
#define  CLASS_NAME  "deviceclass"       
 
MODULE_LICENSE("GPL");           
MODULE_AUTHOR("8B Team");   
MODULE_DESCRIPTION("A Linux char driver");  
MODULE_VERSION("1");          
 
static int    majorNumber;                  
static char   message[256] = {0};          
static short  size_of_message;            
static int    numberOpens = 0;              
static struct class*  charClass  = NULL; 
static struct device* charDevice = NULL;

static DEFINE_MUTEX(chardevice_mutex); 
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *, unsigned int cmd, unsigned long arg);
static struct file_operations fileops =
{
   .open = device_open,
   .read = device_read,
   .write = device_write,
   .release = device_release,
   .unlocked_ioctl = device_ioctl,
};
 
static int __init chardevice_init(void){
   printk(KERN_INFO "Character device: Init\n");
   //dynamically allocate a major number for the device 
   majorNumber = register_chrdev(0, DEVICE_NAME, &fileops);
   if (majorNumber<0){
      printk(KERN_ALERT "Character device failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "Character device: registered correctly with major number %d\n", majorNumber);
 
   // Register the device class
   charClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(charClass)){               
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(charClass);         
   }
   printk(KERN_INFO "Character device: device class registered correctly\n");
 
   // Register the device driver
   charDevice = device_create(charClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(charDevice)){               
      class_destroy(charClass);          
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(charDevice);
   }
   printk(KERN_INFO "Character device: device created correctly\n"); 
   mutex_init(&chardevice_mutex);
   return 0;
}

static void __exit chardevice_exit(void){
   mutex_destroy(&chardevice_mutex); 
   device_destroy(charClass, MKDEV(majorNumber, 0));     
   class_unregister(charClass);                        
   class_destroy(charClass);                            
   unregister_chrdev(majorNumber, DEVICE_NAME);           
   printk(KERN_INFO "Character device: Exit!\n");
}

static int device_open(struct inode *inodep, struct file *filep){
   if(!mutex_trylock(&chardevice_mutex)){
      printk(KERN_ALERT "Char device: Device in use by another process");
      return -EBUSY;
   }
   numberOpens++;
   printk(KERN_INFO "Character device: Device has been opened %d time(s)\n", numberOpens);
   return 0;
}

static ssize_t device_read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int error_count = 0;
   // copy_to_user( * to, *from, size), returns 0 on success
   error_count = copy_to_user(buffer, message, size_of_message);
 
   if (error_count==0){            
      printk(KERN_INFO "Character device: Sent %d bytes to the user\n", size_of_message);
      return (size_of_message=0); 
   }
   else {
      printk(KERN_INFO "Character device: Failed to send %d characters to the user\n", error_count);
      return -EFAULT;              
   }
}

// static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
//    sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
//    size_of_message = strlen(message);                 // store the length of the stored message
//    printk(KERN_INFO "Character device: Received %zu characters from the user\n", len);
//    return len;
// }
 static ssize_t device_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
    sprintf(message, "%s(%zu letters)", buffer, len);   // appending received string with its length
    size_of_message = strlen(message);                 // store the length of the stored message
    if(copy_from_user(&message, buffer, size_of_message)){
        printk(KERN_INFO "Character device: Failed to receive from user\n");
    return -EFAULT;
    } else{
        printk(KERN_INFO "Received %zu characters from user, %s\n", len, message);
        return 0;
   }
}

static int device_release(struct inode *inodep, struct file *filep){
   mutex_unlock(&chardevice_mutex);
   printk(KERN_INFO "Character device: Device successfully closed\n");
   return 0;
}

static int volume = 10, bash = 10, hight = 10;
static long device_ioctl(struct file *f, unsigned int cmd, unsigned long arg){
    device_info dev;
    switch(cmd){
        case GET_INFO:
            dev.volume = volume;
            dev.bash = bash;
            dev.hight = hight;
            if(copy_to_user((device_info *)arg, &dev, sizeof(device_info)) != 0){
                printk(KERN_INFO "Character device: Failed to get device's information\n");
                return -EFAULT;              
            }
            break;
        case SET_INFO:
            if(copy_from_user(&dev, (device_info *)arg, sizeof(device_info)) != 0){
                printk(KERN_INFO "Character device: Failed to set device's information\n");
                return -EFAULT;   
            }
            volume = dev.volume;
            bash = dev.bash;
            hight = dev.hight;
            break;
        case CLEAR_INFO:
            volume = 0;
            bash = 0;
            hight = 0;
            break;
   }
   return 0;
}

module_init(chardevice_init);
module_exit(chardevice_exit);