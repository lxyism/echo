#pragma once
#ifndef WRITENREAD_H
#define WRITENREAD_H

#include <sys/types.h>
#include <stdio.h>

#define SERV_PORT 9877
#define LISTENQ 3000
#define MAXLINE 1024
#define OPEN_MAX 30000
#define INFTIM -1

ssize_t writen(int fd, const void* vptr, size_t n);
ssize_t readn(int fd, void* vptr, size_t n);
ssize_t readline(int fd, void* vptr, size_t maxlen);
void str_cli(FILE* fp, int sockfd);
void str_echo(int sockfd);
void err_quit(arg);
#endif

