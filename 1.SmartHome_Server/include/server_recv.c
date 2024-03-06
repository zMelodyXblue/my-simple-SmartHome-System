/*************************************************************************
	> File Name: server_recv.c
	> Author: 
	> Mail: 
	> Created Time: Tue 05 Mar 2024 10:19:38 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <head.h>
#include <datatype.h>
#include <common.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAX_CLIENT_SUM 1024
#define MAX_DEVICE_SUM 20
extern const char *global_server_name;

const size_t SmhMsg_size = sizeof(struct SmhMsg);

void getLogin(int sockfd, struct User *user);
void send_test_SmhMsg(int new_fd);

struct Link_Args {
    int sockfd;
    pthread_t thread;
    struct User user;
    struct sockaddr_in client;
};
extern struct Link_Args ClientLinks[MAX_CLIENT_SUM];

extern struct device *Devices[MAX_DEVICE_SUM];

int HouseCtl(const struct SmhMsg *msg, struct SmhMsg *sys_msg);

void *server_recv(void *arg) {
    struct Link_Args *client = (struct Link_Args *)arg;
    pthread_detach(client->thread);

    int fd = client->sockfd;
    getLogin(fd, &client->user);
    //send_test_SmhMsg(fd);

    struct SmhMsg com_msg, sys_msg;
    memset(&com_msg, 0, SmhMsg_size);
    memset(&sys_msg, 0, SmhMsg_size);
    sys_msg.user.type = USER_ADMINISTRATOR;
    strncpy(sys_msg.user.name, global_server_name, sizeof(sys_msg.user.name) - 1);

    char msg_at_name[32] = {0};
    int recv_size;
    while (1) {
        if ((recv_size = recv(fd, (char *)&com_msg, SmhMsg_size, 0)) <= 0) {
            perror("server_recv: recv");
            /*sys_msg.type = SMH_MSG;
            strncpy(sys_msg.msg, "recv failed!", MAX_MSG);
            send(fd, (char *)&sys_msg, SmhMsg_size, 0);*/
            break;
        }
        if (com_msg.type & SMH_WALL) {
            strncpy(com_msg.user.name, client->user.name, sizeof(com_msg.user.name) - 1);
            for (int i = 0; i < MAX_CLIENT_SUM; ++i) {
                if (ClientLinks[i].sockfd != i) continue;
                if (send(ClientLinks[i].sockfd, (char *)&com_msg, SmhMsg_size, 0) < 0) {
                    perror("send_to_all");
                }
                DBG("send to %d: %s\n", i, com_msg.msg);
                /*DBG("send to %d :\n msg.type=%d, msg.size=%d\n user: name: %s, type=%d\n  msg: %s\n",
                    i, com_msg.type, com_msg.size, com_msg.user.name, 
                    com_msg.user.type, com_msg.msg
                );*/
            }
        } else if (com_msg.type & SMH_MSG) {
            memset(msg_at_name, 0, sizeof(msg_at_name));
            strncpy(com_msg.user.name, client->user.name, sizeof(com_msg.user.name) - 1);
            for (int i = 1; com_msg.msg[i] != ' ' && com_msg.msg[i] != '\0'; ++i) {
                msg_at_name[i - 1] = com_msg.msg[i];
            }
            int flag = 1;
            for (int i = 0; i < MAX_CLIENT_SUM; ++i) {
                if (ClientLinks[i].sockfd != i) continue;
                if (strcmp(ClientLinks[i].user.name, msg_at_name) != 0) continue;
                if (send(i, (char *)&com_msg, SmhMsg_size, 0) < 0) {
                    perror("send_msg");
                }
                DBG("send to %d: %s\n", i, com_msg.msg);
            }
            if (flag) {
                sprintf(sys_msg.msg, "client %s is not online\n", msg_at_name);
                sys_msg.type = SMH_MSG;
                if (send(fd, (char *)&sys_msg, SmhMsg_size, 0) < 0) {
                    perror("send_sys_msg");
                }
            }
        } else if (com_msg.type & SMH_CTL){
            sys_msg.type = SMH_CTL; 
            HouseCtl(&com_msg, &sys_msg);
            if (send(fd, (char *)&sys_msg, SmhMsg_size, 0) < 0) {
                perror("send_ctl_msg");
            }
        } else if (com_msg.type & SMH_FIN) {
            sys_msg.type = SMH_ACK;
            strcpy(sys_msg.msg, "FIN_ACK");
            if (send(fd, (char *)&sys_msg, SmhMsg_size, 0) < 0) {
                perror("send_FIN_msg");
            }
            break;
        }
    }

    client->sockfd = -1;
    close(fd);
    return NULL;
}

const char _dev_type_icons[8][8] = {"������", "������", "������"};
const char _dev_state_icons[8][8] = {"✅", "❌", "������"};
//extern struct device *Devices[MAX_DEVICE_SUM];

int HouseCtl(const struct SmhMsg *msg, struct SmhMsg *sys_msg) {
    const char *dev_type;
    const char *dev_state;
    int target_id = 0;  //所要操纵的设备的id
    if (msg->msg[1] == '1') { //查看已有设备列表
        for (int i = 1; i < MAX_DEVICE_SUM; ++i) {
            if (Devices[i] == NULL) continue;
            sprintf(sys_msg->msg, "当前设备有: <%d.%s>[%s: %s] ", 
                    i, _dev_type_icons[Devices[i]->type], Devices[i]->device_name, _dev_state_icons[Devices[i]->state]
            );
        }
    } else if (msg->msg[1] == '2') {  //改变设备状态
        int j = 3, i = 0;
        while (msg->msg[j] == ' ') ++j;
        while (msg->msg[j] <= '9' && msg->msg[j] > '0') {
            target_id *= 10;
            target_id += (msg->msg[j] - '0');
        }
        if (target_id <= 0 || target_id >= MAX_DEVICE_SUM) return -1;  //非法操作
        if (msg->msg[2] == '1') {  //turn on
            
        }
    }
    return 0;
}  
/*
enum device_type {
    DEVICE_LIGHT,
    DEVICE_SWITCH,
    DEVICE_THERMOSTAT,
    // 其他设备类型
};

// 设备状态枚举
enum device_state {
    DEVICE_STATE_OFF,
    DEVICE_STATE_ON,
    DEVICE_STATE_STANDBY,
    // 其他设备状态
};

// 设备结构体
struct device {
    int device_id;              // 设备ID
    char device_name[32];       // 设备名称
    enum device_type type;      // 设备类型
    enum device_state state;    // 设备状态
    // 其他设备属性
};
*/

