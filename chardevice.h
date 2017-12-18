#include <linux/ioctl.h>
 
typedef struct
{
    int volume, bash, hight;
} device_info;
 
#define GET_INFO _IOR('q', 1, device_info *)
#define CLEAR_INFO _IO('q', 2)
#define SET_INFO _IOW('q', 3, device_info *)

void sendMessage(int f);	//send message to device
void getMessage(int f);		//get the message back
void setDeviceInfo(int f);	//set device's information
void getDeviceInfo(int f);	//get device's information
void clearDeviceInfo(int f);	//clear device information 
