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

extern char global_server_port[20];
extern char global_server_name[20];

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
    fclose(fp);

    int i = 0;
    while (buff[i] != '=') ++i;
    ++i;
    while (buff[i] == ' ') ++i;

    char config_value[64] = {0};
    for (int j = 0, I = strlen(buff); buff[i] != '\0' && buff[i] != '\n'; ++i, ++j) {
        config_value[j] = buff[i];
    }
    char *r = NULL;
    if (strcmp(key, "SERVER_PORT") == 0) 
        strncpy(global_server_port, config_value, sizeof(global_server_port) - 1), r = global_server_port;
    else if (strcmp(key, "SERVER_NAME") == 0) 
        strncpy(global_server_name, config_value, sizeof(global_server_name) - 1), r = global_server_name;
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
