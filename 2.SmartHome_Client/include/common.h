/*************************************************************************
	> File Name: common.h
	> Author: 
	> Mail: 
	> Created Time: Fri 01 Mar 2024 11:29:38 AM CST
 ************************************************************************/

#ifndef _COMMON_H
#define _COMMON_H
extern char global_server_ip[];
extern char global_server_port[];
extern char global_user_name[];
extern char global_user_passwd[];
char *get_config_value(const char* config_file_path, const char * key);

int make_nonblock(int fd);
int make_block(int fd);

int socket_create(int port);
int socket_connect(const char *ip, int port);
int socket_connect_timeout(const char *ip, const int port, const long timeout);

#define LOG_LEVEL_INFO 0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_ERROR 2
void log_event(int level, const char* message, const char* filename);
/*
#include <ncurses.h>
#include <locale.h>
#define DEFARG(name, default_value) ((#name[0]) ? (name + 0) : default_value)
extern WINDOW *message_sub;
#define Show_Message(arg0, arg1, arg2, arg3) show_message(DEFARG(arg0, message_sub), DEFARG(arg1, NULL), arg2, DEFARG(arg3, 0))
*/
#endif  // _COMMON_H
