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
#include <thread_pool.h>

extern char global_server_name[20];

extern struct SmhMsg msgBuff[MAX_CLIENT_SUM];

extern struct Link_Args ClientLinks[MAX_CLIENT_SUM];

extern struct device *device_list[MAX_DEVICE_SUM];
const char _dev_type_icons[8][16] = {"������", "������", "������"};
const char _dev_state_icons[8][16] = {"❌->OFF", "✅->ON", "������->Standby"};

int msgAnalyze(struct Link_Args *client, struct SmhMsg *sys_msg);

void *server_recv(void *arg) {
    //struct Link_Args *client = (struct Link_Args *)arg;  
    //改用线程池
    struct task_queue *taskQueue = (struct task_queue *)arg;
    pthread_detach(pthread_self());

    DBG(L_GREEN"pthread-%ld start" NONE"\n", pthread_self());
    //int fd = client->sockfd;
    //send_test_SmhMsg(fd);

    struct SmhMsg sys_msg;
    memset(&sys_msg, 0, sizeof(sys_msg));
    sys_msg.user.type = USER_ADMINISTRATOR;
    strncpy(sys_msg.user.name, global_server_name, sizeof(sys_msg.user.name) - 1);

    while (1) {
        struct Link_Args *clientLink = task_queue_pop(taskQueue);
        msgAnalyze(clientLink, &sys_msg);
        DBG(YELLOW"msgAnalyze finished!" NONE"\n");
    }
    return NULL;
}

int msgAnalyze(struct Link_Args *client, struct SmhMsg *sys_msg) {
    DBG(L_GREEN"msgAnalyze start!" NONE"\n");
    int fd = client->sockfd;
    struct SmhMsg *com_msg = &msgBuff[fd];
    int recv_size;
    char msg_at_name[20] = {0};
    /*if ((recv_size = recv(fd, (char *)&com_msg, sizeof(com_msg), 0)) <= 0) {
        perror("server_recv: recv");
        //sys_msg.type = SMH_MSG;
        //strncpy(sys_msg.msg, "recv failed!", MAX_MSG);
        //send(fd, (char *)&sys_msg, SmhMsg_size, 0);
        break;
    }*/
    DBG(L_BLUE"msg->type=%d\n", com_msg->type);
    DBG("    msg:%s" NONE"\n", com_msg->msg);
    if (com_msg->type & SMH_WALL) {
        //公告信息
        DBG(L_BLUE"公告信息：\n");
        strncpy(com_msg->user.name, client->user.name, sizeof(com_msg->user.name) - 1);
        for (int i = 0; i < MAX_CLIENT_SUM; ++i) {
            //DBG("%d, ", i);
            if (ClientLinks[i].sockfd != i) continue;
            //DBG("\\\n");
            //DBG(L_RED"fd=%d, sizeof=%lu"NONE"\n", i, sizeof(*com_msg));
            //注意不要发送给server_listen !!!!!
            if (send(ClientLinks[i].sockfd, (char *)com_msg, sizeof(*com_msg), 0) < 0) {
                perror("send_to_all");
            }
            DBG(L_BLUE"send to %d: %s" NONE"\n", i, com_msg->msg);
            /*DBG("send to %d :\n msg.type=%d, msg.size=%d\n user: name: %s, type=%d\n  msg: %s\n",
                i, com_msg.type, com_msg.size, com_msg.user.name, 
                com_msg.user.type, com_msg.msg
            );*/
        }
    } else if (com_msg->type & SMH_MSG) {
        //私聊
        memset(msg_at_name, 0, sizeof(msg_at_name));
        strncpy(com_msg->user.name, client->user.name, sizeof(com_msg->user.name) - 1);
        for (int i = 1; com_msg->msg[i] != ' ' && com_msg->msg[i] != '\0'; ++i) {
            msg_at_name[i - 1] = com_msg->msg[i];
        }
        int flag = 1;
        for (int i = 0; i < MAX_CLIENT_SUM; ++i) {
            if (ClientLinks[i].sockfd != i) continue;
            if (strcmp(ClientLinks[i].user.name, msg_at_name) != 0) continue;
            if (send(i, (char *)com_msg, sizeof(*com_msg), 0) < 0) {
                perror("send_msg");
            }
            DBG(L_GREEN"send to %d: %s" NONE"\n", i, com_msg->msg);
            flag = 0;
        }
        if (flag) {
            sprintf(sys_msg->msg, "client <%s> is not online\n", msg_at_name);
            sys_msg->type = SMH_MSG;
            if (send(fd, (char *)sys_msg, sizeof(*sys_msg), 0) < 0) {
                perror("send_sys_msg");
            }
        }
    } else if (com_msg->type & SMH_CTL){
        //控制信息
        sys_msg->type= SMH_CTL; 
        DBG(L_GREEN"recv_ctl: action = %d, device_id = %d" NONE"\n", com_msg->ctl.action, com_msg->ctl.dev.device_id);
        if (com_msg->ctl.action < 0 || HouseCtl(com_msg, sys_msg) < 0) {
            fprintf(stderr, L_RED"HouseCtl: illegal operation" NONE"\n");
            strncpy(sys_msg->msg, "illegal operation or target not existing", MAX_MSG);
        }
        if (send(fd, (char *)sys_msg, sizeof(*sys_msg), 0) < 0) {
            perror("send_ctl_msg");
        }
        DBG(YELLOW"send to %d:(sys) %s" NONE"\n", fd, sys_msg->msg);
    } else if (com_msg->type & SMH_FIN) {
        sys_msg->type = SMH_ACK;
        strcpy(sys_msg->msg, "FIN_ACK");
        if (send(fd, (char *)sys_msg, sizeof(*sys_msg), 0) < 0) {
            perror("send_FIN_msg");
        }
        DBG(YELLOW"send to %d:(sys) %s" NONE"\n", fd, sys_msg->msg);
    }

    return 0;

}


/*
#define ACTION_GET_DEVICES 0x01
#define ACTION_UPDATE_DEVICE 0x02
#define ACTION_ADD_DEVICE 0x04
#define ACTION_DEL_DEVICE 0x08

//使用CTL报文对设备进行控制
struct Ctl {
    int action;
    struct device dev;
};
*/
//解析控制命令，调用操作函数，打印合法返回信息
int HouseCtl(const struct SmhMsg *msg, struct SmhMsg *sys_msg) {
    //const char *dev_type;
    //const char *dev_state;
    int target_dev_id = msg->ctl.dev.device_id;  //所要操纵的设备的id
    int action = msg->ctl.action;
    if (action & ACTION_GET_DEVICES) {
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
    } else if (action & ACTION_UPDATE_DEVICE) {
        //改变设备状态
        DBG("改变设备状态:\n");
        if (target_dev_id <= 0 || target_dev_id >= MAX_DEVICE_SUM) return -1;  //非法操作
        if (device_list[target_dev_id] == NULL) return -1;
        if (change_device_states[msg->ctl.dev.state](device_list[target_dev_id]) < 0) {
            fprintf(stderr, YELLOW"change_device_state failed!" NONE"\n");
            strncpy(sys_msg->msg, "change device state failed!", MAX_MSG);
        } else {
            strncpy(sys_msg->msg, "change device state succeed!", MAX_MSG);
        }
    } else if (action & ACTION_ADD_DEVICE) {
        //添加设备
        DBG("添加设备:\n");
        if (msg->ctl.dev.type < 0 || msg->ctl.dev.type >= _DEVICE_TYPE_SUM) return -1;  //非法操作
        DBG(YELLOW"add: type = %d, name = %s" NONE"\n", msg->ctl.dev.type, msg->ctl.dev.device_name);
        if (add_devices[msg->ctl.dev.type](device_list, msg->ctl.dev.device_name) < 0) {
            fprintf(stderr, YELLOW"add_device failed!\n" NONE"\n");
            strncpy(sys_msg->msg, "add device failed!\n", MAX_MSG);
        } else {
            strncpy(sys_msg->msg, "add device succeed!\n", MAX_MSG);
        }
    } else if (action & ACTION_DEL_DEVICE) {
        DBG("移除设备:\n");
        if (target_dev_id <= 0 || target_dev_id >= MAX_DEVICE_SUM) return -1;
        if (device_list[target_dev_id] == NULL) return -1;
        free(device_list[target_dev_id]);
        device_list[target_dev_id] = NULL;
        strncpy(sys_msg->msg, "device has been removed!", MAX_MSG);
    } else {
        DBG(RED"非法动作" NONE"\n");
        return -1;
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
