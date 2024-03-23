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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <server_recv.h>
#include <device_ctl.h>
#include <thread_pool.h>

#define INS 3

const char *global_conf_file = "./config_server";
char global_server_name[20] = {0};
char global_server_port[20] = {0};

struct Link_Args ClientLinks[MAX_CLIENT_SUM];
struct SmhMsg msgBuff[MAX_CLIENT_SUM] = {0};

pthread_t threads[INS] = {0};

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

    //创建线程池和线程
    struct task_queue *taskQueue = (struct task_queue *)malloc(sizeof(struct task_queue));
    task_queue_init(taskQueue, MAX_CLIENT_SUM);
    for (int i = 0; i < INS; ++i) {
        pthread_create(&threads[i], NULL, server_recv, (void *)taskQueue);
    }

    fd_set readfds;
    int maxfd = server_listen;
    ClientLinks[server_listen].sockfd = 0; //为了“server_recv.c”中区分
    DBG("server_listen = %d\n", server_listen);
    int sockfd = 0;
    while (1) {
        //reinit
        FD_ZERO(&readfds);
        for (int i = 3, n = maxfd + 2; i < n; ++i) {
            if (ClientLinks[i].sockfd == -1) continue;
            FD_SET(i, &readfds);
        }
        DBG(BLUE"select start"NONE"\n");
        if (select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0) {
            perror("select");
            exit(1);
        }
        DBG(YELLOW"select finish"NONE"\n");
        if (FD_ISSET(server_listen, &readfds)) {
            //struct sockaddr_in client;
            socklen_t len = sizeof(ClientLinks[server_listen].client);
            bzero(&ClientLinks[server_listen].client, len);
            DBG(GREEN"accept start"NONE"\n");
            if ((sockfd = accept(server_listen, (struct sockaddr *)&ClientLinks[server_listen].client, &len)) < 0) {
                perror("accept");
                exit(1);
            }
            DBG(GREEN"accept finish"NONE"\n");
            memcpy(&ClientLinks[sockfd].client, &ClientLinks[server_listen].client, len);
            printf(L_BLUE"%s:<%d> login!" NONE"\n", inet_ntoa(ClientLinks[sockfd].client.sin_addr), ntohs(ClientLinks[sockfd].client.sin_port));
            if (sockfd > MAX_CLIENT_SUM - 5) {
                fprintf(stderr, "Too Many users!\n");
                close(sockfd);
                printf(YELLOW"client-%d has to leave temporarily!" NONE"\n", sockfd);
            } else {
                ClientLinks[sockfd].sockfd = sockfd;
                if (sockfd > maxfd) maxfd = sockfd;
                ClientLinks[sockfd].tasktype = 1;
                //task_queue_push(taskQueue, &ClientLinks[sockfd]);
            }
        }
        for (int i = 3, n = maxfd + 2; i < n; ++i) {
            if (ClientLinks[i].sockfd != i) continue;
            //if (ClientLinks[i].sockfd == server_listen) continue;
            DBG(L_BLUE"FD_ISSET?"NONE"\n");
            if (FD_ISSET(ClientLinks[i].sockfd, &readfds)) {
                DBG("%d is ready!\n", ClientLinks[i].sockfd);
                DBG(GREEN"recv start"NONE"\n");
                if (ClientLinks[i].tasktype > 0) {
                    getLogin(i, &ClientLinks[i].user);
                    ClientLinks[i].tasktype = 0;
                    DBG(YELLOW"client-%d getLogin finished!" NONE"\n", ClientLinks[i].sockfd);
                    continue;
                }
                int rsize = recv(ClientLinks[i].sockfd, (char *)&msgBuff[i], sizeof(msgBuff[i]), 0);
                DBG(L_GREEN"recv finish"NONE"\n");
                if (rsize <= 0) {
                    perror("recv");
                    //int temp_fd = ClientLinks[i].sockfd;
                    ClientLinks[i].sockfd = -1;
                    close(i);
                    printf(YELLOW"client-%d leave!" NONE"\n", i);
                    continue;
                }
                task_queue_push(taskQueue, &ClientLinks[i]);
            }
        }
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
