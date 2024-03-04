/*************************************************************************
	> File Name: main.c
	> Author: 
	> Mail: 
	> Created Time: Fri 01 Mar 2024 05:04:43 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <common.h>
#include <head.h>
#include <chat_ui.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <ncurses.h>
#include <locale.h>
#include <send_chat.h>

char global_server_ip[20] = {0};
char global_server_port[20] = {0};
char global_user_name[20] = {0};
char global_user_passwd[20] = {0};

int global_sockfd = -1;
const char *global_conf = "./config";
struct SmhMsg chat_msg;
//struct SmhMsg ctl_msg;
struct Ctl ctl; //在 send_chat.c 中
//int show_name = 0;
//char data_stream[20] = {0};
WINDOW *message_win, *message_sub,  *info_win, *input_win, *info_sub;
int message_num = 0;


void logout(int signum) {
    struct SmhMsg msg;
    //发送type为SMH_FIN的数据包
    //关闭连接
    msg.type = SMH_FIN;
    if (send(global_sockfd, (char *)&msg, sizeof(msg), 0) < 0) {
        perror("logout:send");
        exit(1);
    }
    sleep(2);
    make_nonblock(global_sockfd);
    if (recv(global_sockfd, (char *)&msg, sizeof(msg), 0) <= 0) {
        perror("logout:recv");
        close(global_sockfd);
        #ifndef _D
        endwin();
        #endif
        exit(1);
    }
    printf("final msg: %s\n", msg.msg);
    close(global_sockfd);
    #ifndef _D
    endwin();
    #endif
    DBG(RED"Bye!\n"NONE);
    exit(0);
}

void mainLogin();

int main() {
    signal(SIGINT, logout);
    mainLogin();

    setlocale(LC_ALL,"");
    #ifndef _D
    init_ui();
    #endif
    pthread_t recv_tid;
    pthread_create(&recv_tid, NULL, client_recv, NULL);
    while (1) {
	    int c = getchar();
        if (c == EOF) break;
	    switch (c) {
		    case 13:{
			    send_chat();
			    break;
		    }
		    default:
			break;
	    }
	    box(input_win, 0, 0);
	    wrefresh(input_win);
    }
    //sleep(100);
    close(global_sockfd);
    #ifndef _D
    endwin();
    #endif
    return 0;
}

void Login(int sockfd, struct LogRequest *logReq);

void mainLogin() {
    int log_msg_error_flag = 0;
    char *server_ip = get_config_value(global_conf, "SERVER_IP");
    if (server_ip == NULL) {
        fprintf(stderr, RED"SERVER_IP not found!"NONE"\n");
        log_msg_error_flag = 1;
    }
    char *server_port = get_config_value(global_conf, "SERVER_PORT");
    if (server_port == NULL) {
        fprintf(stderr, RED"SERVER_PORT not found!"NONE"\n");
        log_msg_error_flag = 1;
    }
    char *user_name = get_config_value(global_conf, "USER_NAME");
    if (user_name == NULL) {
        fprintf(stderr, RED"USER_NAME not found!"NONE"\n");
        log_msg_error_flag = 1;
    }
    char *user_passwd = get_config_value(global_conf, "USER_PASSWD");
    if (user_passwd == NULL) {
        fprintf(stderr, RED"USER_PASSWD not found!"NONE"\n");
        log_msg_error_flag = 1;
    }
    DBG("server_ip = %s, strlen = %lu\n", server_ip, strlen(server_ip));
    DBG("server_port = %s, strlen = %lu\n", server_port, strlen(server_port));
    DBG("user_name = %s, strlen = %lu\n", user_name, strlen(user_name));
    DBG("user_passwd = %s, strlen = %lu\n", user_passwd, strlen(user_passwd));
    if (log_msg_error_flag) exit(1);

    int port = atoi(server_port);
    global_sockfd = socket_connect(server_ip, port);

    struct LogRequest logReq;
    memset(&logReq, 0, sizeof(logReq));
    strncpy(logReq.name, user_name, 19);
    strncpy(logReq.passwd, user_passwd, 19);
    Login(global_sockfd, &logReq);

    return ;
}

void Login(int sockfd, struct LogRequest *logReq) {
    struct LogResponse logResp;
    memset(&logResp, 0, sizeof(logResp));
    //char snd_buff[MAX_MSG] = {0};
    //memcpy(snd_buff, logReq, sizeof(*logReq));
    if (send(sockfd, (char *)logReq, sizeof(*logReq), 0) < 0) { //此处应是sizeof(* ) !
        perror("send_LogRequest");
        close(sockfd);
        exit(1);
    }
    if (recv(sockfd, (char *)&logResp, sizeof(logResp), 0) < 0) {
        perror("recv_LogResponse");
        close(sockfd);
        exit(1);
    }
    if (logResp.type != 0) {
        fprintf(stderr, L_RED"Server refuse!\n" NONE"\n");
        close(sockfd);
        exit(1);
    }
    DBG(L_GREEN"login succeed!" NONE"\n");
    return ;
}

