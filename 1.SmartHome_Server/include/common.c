/*************************************************************************
	> File Name: common.c
	> Author: 
	> Mail: 
	> Created Time: Fri 01 Mar 2024 11:30:44 AM CST
 ************************************************************************/
#include <head.h>
#include <common.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

char global_server_ip[64] = {0};
char global_server_port[32] = {0};
char global_user_name[64] = {0};
char global_user_passwd[64] = {0};

char *get_config_value(const char* config_file_path, const char * key) {
    FILE *fp = fopen(config_file_path, "r");
    if (!fp) {
        perror("fopen");
        return NULL;
    }
    char buff[512] = {0};
    char *rbuf;
    while (rbuf = fgets(buff, 512, fp)) {
        if (strstr(buff, key) == NULL) continue;
        break;
    } 
    int i = 0;
    while (buff[i] != '=') ++i;
    ++i;
    while (buff[i] == ' ') ++i;

    char config_value[128] = {0};
    for (int j = 0, I = strlen(buff); buff[i] != '\0' && buff[i] != '\n'; ++i, ++j) {
        config_value[j] = buff[i];
    }
    char *r = NULL;
    if (strcmp(key, "SERVER_IP") == 0) 
        strcpy(global_server_ip, config_value), r = global_server_ip;
    else if (strcmp(key, "SERVER_PORT") == 0) 
        strcpy(global_server_port, config_value), r = global_server_port;
    else if (strcmp(key, "USER_NAME") == 0)
        strcpy(global_user_name, config_value), r = global_user_name;
    else if (strcmp(key, "USER_PASSWD") == 0)
        strcpy(global_user_passwd, config_value), r = global_user_passwd;
    return r;
}


int make_block(int fd) {
    int flag;
    if ((flag = fcntl(fd, F_GETFL)) < 0) {
        return -1;
    }
    flag &= ~O_NONBLOCK;
    return fcntl(fd, F_SETFL, flag);
}

int make_nonblock(int fd) {
    int flag;
    if ((flag = fcntl(fd, F_GETFL)) < 0) {
        return -1;
    }
    flag |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flag);
}


int socket_create(int port) {
    int server_listen; //sockfd
    if ((server_listen = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr("0.0.0.0");
    int reuse_val = 1;
    if (setsockopt(server_listen, SOL_SOCKET, SO_REUSEADDR, (void *)&reuse_val, sizeof(int)) < 0) {
        return -1;
    }

    if (bind(server_listen, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return -1;
    }
    if (listen(server_listen, 20) < 0) {
        return -1;
    }
    return server_listen;
}


int socket_connect(const char *ip, const int port) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        return -1;
    }
    return sockfd;
}

int socket_connect_timeout(const char *ip, const int port, const long timeout) {
    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(port); //记得htons ！！
    server.sin_addr.s_addr = inet_addr(ip);

    make_nonblock(sockfd);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = timeout;

    fd_set wfds;
    FD_ZERO(&wfds);
    FD_SET(sockfd, &wfds);
    int ret;
    //非阻塞的话connect一定返回-1 //先进行connect才可能可写
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0) { 
        int err = -1;
        int len = sizeof(int);
        ret = select(sockfd + 1, NULL, &wfds, NULL, &tv);
        DBG(RED"select finish!\n"NONE);
        if (ret <= 0) {
            close(sockfd); //记得close ！
            DBG(RED"ret<=0!\n");
            return -1;
        } else {
            if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &err, (socklen_t *)&len) < 0) {
                close(sockfd);
                DBG(NONE"getsockopt err!\n"NONE);
                return -1;
            }
            if (err) {
                close(sockfd);
                DBG(RED"err\n"NONE);
                return -1;
            }
        }
    }
    make_block(sockfd);//返回阻塞的sockfd
    return sockfd;
}


void log_event(int level, const char* message, const char* filename) {
    time_t now = time(NULL);
    char* level_str;
    FILE* fp;

    switch(level) {
        case LOG_LEVEL_INFO:
            level_str = "INFO";
            break;
        case LOG_LEVEL_WARNING:
            level_str = "WARNING";
            break;
        case LOG_LEVEL_ERROR:
            level_str = "ERROR";
            break;
        default:
            level_str = "UNKNOWN";
            break;
    }
    fp = fopen(filename, "a");
    if (fp == NULL) {
        perror("log_event:fopen");
        exit(1);
    }
    fprintf(fp, "%s [%s]: %s\n", ctime(&now), level_str, message);
    fclose(fp);
    return ;
}
