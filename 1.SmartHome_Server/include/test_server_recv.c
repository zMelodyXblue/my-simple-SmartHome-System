/*************************************************************************
	> File Name: server_recv.c
	> Author: 
	> Mail: 
	> Created Time: Tue 05 Mar 2024 10:19:38 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <head.h>
#include <datatype.h>
#include <common.h>
#include <server_recv.h>

extern const char *global_server_name;

extern const size_t SmhMsg_size;

extern struct Link_Args ClientLinks[MAX_CLIENT_SUM];

extern struct device *device_list[MAX_DEVICE_SUM];
const char _dev_type_icons[8][8] = {"������", "������", "������"};
const char _dev_state_icons[8][8] = {"✅", "❌", "������"};

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
            //公告信息
            strncpy(com_msg.user.name, client->user.name, sizeof(com_msg.user.name) - 1);
            for (int i = 0; i < MAX_CLIENT_SUM; ++i) {
                if (ClientLinks[i].sockfd != i) continue;
                if (send(ClientLinks[i].sockfd, (char *)&com_msg, SmhMsg_size, 0) < 0) {
                    perror("send_to_all");
                }
                DBG(L_BLUE"send to %d: %s" NONE"\n", i, com_msg.msg);
                /*DBG("send to %d :\n msg.type=%d, msg.size=%d\n user: name: %s, type=%d\n  msg: %s\n",
                    i, com_msg.type, com_msg.size, com_msg.user.name, 
                    com_msg.user.type, com_msg.msg
                );*/
            }
        } else if (com_msg.type & SMH_MSG) {
            //私聊
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
                DBG(L_GREEN"send to %d: %s" NONE"\n", i, com_msg.msg);
            }
            if (flag) {
                sprintf(sys_msg.msg, "client <%s> is not online\n", msg_at_name);
                sys_msg.type = SMH_MSG;
                if (send(fd, (char *)&sys_msg, SmhMsg_size, 0) < 0) {
                    perror("send_sys_msg");
                }
            }
        } else if (com_msg.type & SMH_CTL){
            //控制信息
            sys_msg.type = SMH_CTL; 
            if (HouseCtl(&com_msg, &sys_msg) < 0) {
                fprintf(stderr, L_RED"HouseCtl: illegal operation" NONE"\n");
                strncpy(sys_msg.msg, "illegal operation or target not existing", MAX_MSG);
            }
            if (send(fd, (char *)&sys_msg, SmhMsg_size, 0) < 0) {
                perror("send_ctl_msg");
            }
            DBG(YELLOW"send to %d:(sys) %s" NONE"\n", fd, sys_msg.msg);
        } else if (com_msg.type & SMH_FIN) {
            sys_msg.type = SMH_ACK;
            strcpy(sys_msg.msg, "FIN_ACK");
            if (send(fd, (char *)&sys_msg, SmhMsg_size, 0) < 0) {
                perror("send_FIN_msg");
            }
            DBG(YELLOW"send to %d:(sys) %s" NONE"\n", fd, sys_msg.msg);
            break;
        }
    }

    client->sockfd = -1;
    close(fd);
    return NULL;
}



//解析控制命令，调用操作函数，打印合法返回信息
int HouseCtl(const struct SmhMsg *msg, struct SmhMsg *sys_msg) {
    const char *dev_type;
    const char *dev_state;
    int target_id = 0;  //所要操纵的设备的id
    if (msg->msg[1] == '1') {
        //查看已有设备列表
        DBG("查看已有设备列表:\n");
        strncpy(sys_msg->msg, "当前设备有: ", MAX_MSG); 
        size_t msg_ind = 0;
        for (int i = 1; i < MAX_DEVICE_SUM; ++i) {
            if (device_list[i] == NULL) continue;
            msg_ind = strnlen(sys_msg->msg, MAX_MSG);
            sprintf(&sys_msg->msg[msg_ind], "<%d.%s>[%s: %s] ", 
                    i, _dev_type_icons[device_list[i]->type], device_list[i]->device_name, _dev_state_icons[device_list[i]->state]
            );
        }
        DBG("%s\n", sys_msg->msg);
    } else if (msg->msg[1] == '2') {
        //改变设备状态
        DBG("改变设备状态:\n");
        int act_to_state = msg->msg[2] - '0';
        if (act_to_state < 0 || act_to_state >= _DEVICE_STATE_SUM) {
            return -1;  //非法操作
        }
        int j = 3;
        while (msg->msg[j] == ' ') ++j;
        while (msg->msg[j] <= '9' && msg->msg[j] > '0') {
            target_id *= 10;
            target_id += (msg->msg[j] - '0');
            ++j;
        }
        if (target_id <= 0 || target_id >= MAX_DEVICE_SUM) return -1;  //非法操作
        if (change_device_states[act_to_state](device_list[target_id]) < 0) {
            fprintf(stderr, YELLOW"change_device_state failed!" NONE"\n");
            strncpy(sys_msg->msg, "change device state failed!", MAX_MSG);
        }
    } else if (msg->msg[1] == '3') {
        //添加设备
        DBG("添加设备:\n");
        int dev_type = msg->msg[2] - '0';
        if (dev_type < 0 || dev_type > 9) return -1;
        if (dev_type < 0 || dev_type >= _DEVICE_TYPE_SUM) return -1;  //非法操作
        char dev_name[30] = {0};
        int j = 3;
        while (msg->msg[j] == ' ') ++j;
        DBG(BLUE"j = %d, msg: %s" NONE"\n", j, msg->msg);
        int i;
        for (i = 0; msg->msg[j]; ++j, ++i) dev_name[i] = msg->msg[j];
        DBG(BLUE"j = %d, i = %d\n", j ,i);
        DBG(YELLOW"add: type = %d, name = %s" NONE"\n", dev_type, dev_name);
        if (add_devices[dev_type](device_list, dev_name) < 0) {
            fprintf(stderr, YELLOW"add_device failed!\n" NONE"\n");
            strncpy(sys_msg->msg, "add device failed!\n", MAX_MSG);
        }
    } else if (msg->msg[1] == '4') {
        DBG("移除设备:\n");
        int j = 2, target_id = 0;
        while (msg->msg[j] == ' ') ++j;
        while (msg->msg[j] <= '9' && msg->msg[j] > '0') {
            target_id *= 10;
            target_id += (msg->msg[j] - '0');
            ++j;
        }
        if (target_id <= 0 || target_id >= MAX_DEVICE_SUM) return -1;
        if (device_list[target_id] == NULL) return -1;
        free(device_list[target_id]);
        device_list[target_id] = NULL;
        strncpy(sys_msg->msg, "device has been removed!", MAX_MSG);
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
