/*************************************************************************
	> File Name: server_recv.h
	> Author: 
	> Mail: 
	> Created Time: Wed 06 Mar 2024 02:02:35 PM CST
 ************************************************************************/

#ifndef _SERVER_RECV_H
#define _SERVER_RECV_H
#include "device_ctl.h"
#include "datatype.h"
#include <netinet/in.h>

#define MAX_CLIENT_SUM 1024


void getLogin(int sockfd, struct User *user);
void send_test_SmhMsg(int new_fd);

struct Link_Args {
    int sockfd;
    pthread_t thread;
    struct User user;
    struct sockaddr_in client;
};


void *server_recv(void *arg);

//解析控制命令，调用操作函数，打印合法返回信息
int HouseCtl(const struct SmhMsg *msg, struct SmhMsg *sys_msg);

const char _dev_type_icons[8][16];
const char _dev_state_icons[8][16];


int change_device_state_to_off(struct device *dev);
int change_device_state_to_on(struct device *dev);
int change_device_state_to_standby(struct device *dev);
int (*change_device_states[_DEVICE_STATE_SUM])(struct device *dev);

int add_device_light(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);
int add_device_switch(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);
int add_device_thermostat(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);
int (*add_devices[_DEVICE_TYPE_SUM])(struct device *devlist[MAX_DEVICE_SUM], const char *dev_name);


#endif  // _SERVER_RECV_H
