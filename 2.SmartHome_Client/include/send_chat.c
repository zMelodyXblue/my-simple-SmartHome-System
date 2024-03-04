/*************************************************************************
	> File Name: send_chat.c
	> Author: 
	> Mail: 
	> Created Time: Mon 04 Mar 2024 11:45:39 AM CST
 ************************************************************************/
#include <send_chat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chat_ui.h>
#include <datatype.h>
#include <sys/types.h>
#include <sys/socket.h>
extern int global_sockfd;
extern WINDOW *input_sub, *input_win;
extern struct SmhMsg chat_msg;
extern struct Ctl ctl;

void send_chat() {
    echo();
    nocbreak();
    bzero(chat_msg.msg, sizeof(chat_msg.msg));
    chat_msg.type = SMH_WALL;
    curs_set(1);
    w_gotoxy_puts(input_win, 1, 1, "Input Message : ");
    mvwscanw(input_win, 2, 10, "%[^\n]", chat_msg.msg);
    //判断如果读入的信息非空，则发送到服务端
    if (strlen(chat_msg.msg)) {
        if (chat_msg.msg[0] == '#') {
            chat_msg.type = SMH_CTL;
            chat_msg.ctl = parse_ctl(chat_msg.msg);
        } else if (chat_msg.msg[0] == '@') {
            chat_msg.type = SMH_MSG;
        }
        if (send(global_sockfd, (char *)&chat_msg, sizeof(chat_msg), 0) < 0) {
            perror("send_chat: send");
        }
    }
    wclear(input_win);
    curs_set(0);
    box(input_win, 0, 0);
    wrefresh(input_win);
    noecho();
    cbreak();

}

/*
#1 查看可用的设备列表
#21 {{id}}修改设备状态为开
#22 {{id}}修改设备状态为关
#23 {{id}}修改设备状态为待机
#31 {{name}} 新增设备灯
#32 {{name}} 新增设备开关
#33 {{name}} 新增设备恒温设备
#4 {{id}}删除设备
*/
struct Ctl parse_ctl(char *ctl_str) {
    //struct Ctl ctl = {0};
    memset(&ctl, 0, sizeof(ctl));
    char *instruction = strtok(ctl_str, " ");
    if (instruction == NULL) {
        ctl.action = -1;
        return ctl;
    }
    if (strcmp(instruction, "#1") == 0) {
        ctl.action = ACTION_GET_DEVICES;
    } else if (strcmp(instruction, "#21") == 0) {
        ctl.action = ACTION_UPDATE_DEVICE;
        char *dev_id_str = strtok(NULL, " ");
        if (dev_id_str == NULL) {
            ctl.action = -1;
            return ctl;
        }
        ctl.dev.device_id = atoi(dev_id_str);
        ctl.dev.state = DEVICE_STATE_ON;
    } else if (strcmp(instruction, "#22") == 0) {
        ctl.action = ACTION_UPDATE_DEVICE;
        char *dev_id_str = strtok(NULL, " ");
        if (dev_id_str == NULL) {
            ctl.action = -1;
            return ctl;
        }
        ctl.dev.device_id = atoi(dev_id_str);
        ctl.dev.state = DEVICE_STATE_OFF;
    } else if (strcmp(instruction, "#23") == 0) {
        ctl.action = ACTION_UPDATE_DEVICE;
        char *dev_id_str = strtok(NULL, " ");
        if (dev_id_str == NULL) {
            ctl.action = -1;
            return ctl;
        }
        ctl.dev.device_id = atoi(dev_id_str);
        ctl.dev.state = DEVICE_STATE_STANDBY;
    } else if (strcmp(instruction, "#31") == 0) {
        ctl.action = ACTION_ADD_DEVICE;
        ctl.dev.type = DEVICE_LIGHT;
        char *dev_name = strtok(NULL, " ");
        if (dev_name == NULL) {
            ctl.action = -1;
            return ctl;
        }
        strncpy(ctl.dev.device_name, dev_name, sizeof(ctl.dev.device_name) - 1);
    } else if (strcmp(instruction, "#32") == 0) {
        ctl.action = ACTION_ADD_DEVICE;
        ctl.dev.type = DEVICE_SWITCH;
        char *dev_name = strtok(NULL, " ");
        if (dev_name == NULL) {
            ctl.action = -1;
            return ctl;
        }
        strncpy(ctl.dev.device_name, dev_name, sizeof(ctl.dev.device_name) - 1);
    } else if (strcmp(instruction, "#33") == 0) {
        ctl.action = ACTION_ADD_DEVICE;
        ctl.dev.type = DEVICE_THERMOSTAT;
        char *dev_name = strtok(NULL, " ");
        if (dev_name == NULL) {
            ctl.action = -1;
            return ctl;
        }
        strncpy(ctl.dev.device_name, dev_name, sizeof(ctl.dev.device_name) - 1);
    } else if (strcmp(instruction, "#4") == 0) {
        ctl.action = ACTION_DEL_DEVICE;
        char *dev_id_str = strtok(NULL, " ");
        if (dev_id_str == NULL) {
            ctl.action = -1;
            return ctl;
        }
        ctl.dev.device_id = atoi(dev_id_str);
    } else {
        ctl.action = -1;
    }
    return ctl;
}

