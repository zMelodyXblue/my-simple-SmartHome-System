/*************************************************************************
	> File Name: server.c
	> Author: 
	> Mail: 
	> Created Time: Sat 02 Mar 2024 04:59:54 PM CST
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
#include <server_recv.h>
#include <device_ctl.h>

const char *global_conf_file = "./config_server";

char global_server_name[20] = {0};

char global_server_port[20] = {0};

struct Link_Args ClientLinks[MAX_CLIENT_SUM];

struct device *device_list[MAX_DEVICE_SUM];


int main() {
    if (get_config_value(global_conf_file, "SERVER_PORT") == NULL) {
        fprintf(stderr, RED"SERVER_IP not found!" NONE"\n");
        exit(1);
    }
    if (get_config_value(global_conf_file, "SERVER_NAME") == NULL) {
        fprintf(stderr, RED"SERVER_NAME not found!" NONE"\n");
        exit(1);
    }
    DBG(L_BLUE"SERVER_IP = %s" NONE"\n", global_server_port);
    DBG(L_BLUE"SERVER_NAME = %s" NONE"\n", global_server_name);
    for (int i = 0; i < 1024; ++i) ClientLinks[i].sockfd = -1;

    int port = atoi(global_server_port);
    int server_listen = socket_create(port);
    if (server_listen < 0) {
        perror("socket_create");
        exit(1);
    }

    memset(device_list, 0, sizeof(device_list));

    struct sockaddr_in temp_client;
    socklen_t temp_len = sizeof(temp_client);
    while (1) {
        bzero(&temp_client, temp_len);
        int new_fd = accept(server_listen, (struct sockaddr *)&temp_client, &temp_len);
        DBG(L_BLUE"accept {%d}" NONE"\n", new_fd);
        if (new_fd < 0) {
            perror("accept");
            continue;
        }

        ClientLinks[new_fd].sockfd = new_fd;
        ClientLinks[new_fd].client = temp_client;
        pthread_create(&ClientLinks[new_fd].thread, NULL, server_recv, (void *)&ClientLinks[new_fd]);

    }

    close(server_listen);
    return 0;
}

void getLogin(int new_fd, struct User *user) {
    struct LogRequest logReq;
    memset(&logReq, 0, sizeof(logReq));
    struct LogResponse logResp;
    memset(&logResp, 0, sizeof(logResp));
    //char recv_buff[MAX_MSG] = {0};
    if (recv(new_fd, (char *)&logReq, sizeof(logReq), 0) < 0) {
        perror("recv_logRequest");
        logResp.type = 1;
    }
    if (send(new_fd, (char *)&logResp, sizeof(logResp), 0) < 0) {
        perror("send_logResponse");
        ClientLinks[new_fd].sockfd = -1;
        close(new_fd);
        pthread_exit(NULL);
    }
    //memcpy(&logReq, recv_buff, sizeof(logReq));
    if (logResp.type) close(new_fd);
    strncpy(user->name, logReq.name, 19);
    DBG(L_GREEN"%d: user %s  login allowed!" NONE"\n", new_fd, user->name);

    return ;
}

void send_test_SmhMsg(int new_fd) {
    struct SmhMsg msg;
    for (int i = 0; i < 10; ++i) {
        memset(&msg, 0, sizeof(msg));
        msg.user.type = USER_ADMINISTRATOR;
        switch (i % 3) {
            case 0:
                msg.type = SMH_HEART;
                break;
            case 1:
                msg.type = SMH_MSG;
                strncpy(msg.user.name, global_server_name, sizeof(msg.user.name) - 1);
                strncpy(msg.msg, "Hello", MAX_MSG - 1);
                break;
            case 2:
                msg.type = SMH_WALL;
                strncpy(msg.user.name, global_server_name, sizeof(msg.user.name) - 1);
                strncpy(msg.msg, "Hello, everyone!", MAX_MSG - 1);
                break;
        }
        if (send(new_fd, (char *)&msg, sizeof(msg), 0) < 0) {
            perror("send_SmhMsg:send");
        }
        DBG(L_GREEN"send_SmhMsg: type = %d\n user_type = %d\n msg: %s" NONE"\n", msg.type, msg.user.type, msg.msg);
        sleep(1);
    }
    memset(&msg, 0, sizeof(msg));
    msg.type = SMH_MSG;
    strncpy(msg.msg, "Test finish!\n", MAX_MSG);
    if (send(new_fd, (char *)&msg, sizeof(msg), 0) < 0) {
        perror("send_SmhMsg:send");
    }

    return ;
}

/*
#define SMH_HEART 0x01 //心跳
#define SMH_ACK 0x08 //ack
#define SMH_MSG 0x04 //聊天
#define SMH_WALL 0x02 //公告
#define SMH_CTL 0x10 //控制信息
#define SMH_FIN 0x100 //离场

struct SmhMsg{
    int type;  // type & SMH_HEART 验证是否为SMH_HEART信息
    int size;
    struct User user;
    char msg[MAX_MSG];
    struct Ctl ctl;
};*/
