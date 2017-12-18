#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <linux/ioctl.h>
#include "chardevice.h"        

#define BUFFER_LENGTH 256               //buffer to store input string
//send message to device
void sendMessage(int f){
        char stringToSend[BUFFER_LENGTH];
        printf("Type in a  message to send to the device: ");
        scanf("%[^\n]%*c", stringToSend);                // Read in a string (with spaces)
        //printf("Sending message to the device [%s].\n", stringToSend);
        int ret = write(f, stringToSend, strlen(stringToSend)); // Send the string to the LKM
        if (ret < 0){
                perror("Failed to write the message to the device.");
                return;
        } else{
                printf("Send message success.\n");
        }
}

//get the message back
void getMessage(int f){
        char receive[BUFFER_LENGTH];
        int ret = read(f, receive, BUFFER_LENGTH);        // Read the response from the LKM
        if (ret < 0){
                perror("Failed to read the message from the device.");
                return;
        }
        if(strlen(receive) == 0){
                printf("No message received.\n");
        }
        printf("Receive message success, the received message is: [%s]\n", receive);
}

//set device's information
void setDeviceInfo(int f){
        device_info dev;
        int volume, bash, hight;
        printf("Enter volume: ");
        scanf("%d", &volume);
        getchar();
        dev.volume = volume;
        printf("Enter bash: ");
        scanf("%d", &bash);
        getchar();
        dev.bash = bash;
        printf("Enter hight: ");
        scanf("%d", &hight);
        getchar();
        dev.hight = hight;
        if (ioctl(f, SET_INFO, &dev) == -1)
        {
                perror("Set device information failed.\n");
        } else{
                printf("Set device Information success.\n");
        }
}

//get device's information
void getDeviceInfo(int f){
        device_info dev;
        if(ioctl(f, GET_INFO, &dev) == -1){
                 perror("Get device information failed.\n");
        } else{
                printf("Get device information success.\n");
                printf("Volume: %d\n", dev.volume);
                printf("Bash: %d\n", dev.bash);
                printf("Hight:  %d\n", dev.hight);
        }
}

//clear device information
void clearDeviceInfo(int f){
        if(ioctl(f, CLEAR_INFO) == -1){
                 perror("Clear device information failed.\n");
        } else{
                printf("Clear device information success.\n");
        }
}

int main(){
        int ret, fd, opt;
        printf("Starting device ...\n");
        fd = open("/dev/chardevice", O_RDWR);             // Open the device with read/write access
        if (fd < 0){
                perror("Failed to open the device...\n");
                return errno;
        }
        do{
                printf("Choose one of these options:\n1. Send message to device\n2. Get message from device\n3. Set device information\n4. Get device's information\n5. Clear device's information\n6. Exit\n-------------------------------------------\n");
                scanf("%d", &opt);
                getchar();
                switch (opt){
                        case 1:
                                sendMessage(fd);
                                break;
                        case 2:
                                getMessage(fd);
                                break;
                        case 3:
                                setDeviceInfo(fd);
                                break;
                        case 4:
                                getDeviceInfo(fd);
                                break;
                        case 5:
                                clearDeviceInfo(fd);
                                break;
                        default:
                                break;
                }
        } while(opt < 6);
        return 0;
        }