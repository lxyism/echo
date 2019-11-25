#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <unistd.h>
#include "writenread.h"
#include <sys/wait.h>
#include <errno.h>
#include "writenread.c"


int main(int argc, char* argv[])
{
	int sockfd, epfd, nready, connfd, rec_number, number, err_number;
	char buf[100] = "Hello Serve";
	double time_use;
	int i,j, * fd_array, per_sec_num;
	struct sockaddr_in servaddr;
	struct epoll_event events[OPEN_MAX], event;
	struct timeval start, end;
	

	if (argc != 3)
	{
		printf("usage: myclient.o <IPadress> <the number of connection>");
		exit(-1);
	}
	memset(&servaddr, 0, sizeof(servaddr)); 
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0)
		err_quit("inet_pton error");

	sscanf(argv[2], "%d", &number);
	if ((epfd = epoll_create(1)) < 0)
		err_quit("epoll_create");
	
	fd_array = (int*)malloc(number * sizeof(int));

	gettimeofday(&start, NULL);
	rec_number = 0;
	err_number = 0;
	j = 0;
	for (i = 0; i < number; ++i)
	{
		if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
			err_quit("socket error");
		if ((connfd = connect(sockfd, (struct sockaddr*) & servaddr, sizeof(servaddr))) < 0)
			err_quit("connect error");
		*(fd_array + j) = sockfd;
		j++;
		event.data.fd = sockfd;
		event.events = EPOLLOUT;	//表示可以向该描述符写入数据
		if (epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &event) < 0)
			err_quit("epoll_ctl error1");
	}
	gettimeofday(&end, NULL);

	time_use = 1000 * (end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000.0;
	printf("time: %fms\n", time_use);

	start.tv_sec = end.tv_sec;
	start.tv_usec = end.tv_usec;

	while (1)
	{
		if ((nready = epoll_wait(epfd, events, number, -1)) < 0)
			err_quit("epoll_wait error");
		for (i = 0; i < nready; ++i)
		{//如果是出现EPOLLERR、EPOLLHUP则计入err_number的数量
			if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP))
			{
				++err_number;
				shutdown(events[i].data.fd, SHUT_WR);
				for (j = 0; j < number; ++j)
				{
					*(fd_array + j) = -1;
					break;
				}
				continue;
			}
			else if (events[i].events & EPOLLOUT) //如果该套接字处于可写状态
			{
				connfd = events[i].data.fd;
				if (write(connfd, buf, strlen(buf) + 1) < 0)
					err_quit("write error");
				event.data.fd = connfd;
				event.events = EPOLLIN;  //将套接字处于可读状态

				if (epoll_ctl(epfd, EPOLL_CTL_MOD, connfd, &event) < 0)
					err_quit("epoll_ctl error2");
			}
			else if (events[i].events & EPOLLIN)  //如果套接字处于可写状态
			{
				++rec_number;
				connfd = events[i].data.fd;
				if (read(connfd, buf, strlen(buf)) < 0)
					err_quit("read error");
				event.data.fd = connfd;
				event.events = EPOLLOUT;  //将套接字置于可读状态
				if (epoll_ctl(epfd, EPOLL_CTL_MOD, connfd, &event) < 0)
					err_quit("epoll_ctl error3");
			}
		}
		gettimeofday(&end, NULL);
		if ((end.tv_sec - start.tv_sec) > 60)
			break;
	}
	time_use = 1000 * (end.tv_sec - start.tv_sec) + ((double)(end.tv_usec - start.tv_usec)) / 1000.0;
	per_sec_num = rec_number / 60;
	printf("每秒负载：%d, 错误量：%d, 时间：%fms %s", per_sec_num, err_number, time_use, buf);

	while (wait(NULL) > 0);
	for (j = 0; j < number; ++j)
	{
		if (*(fd_array + j) != -1)
		{
			shutdown(*(fd_array + j), SHUT_WR);
			while (read(*(fd_array + j), buf, 100) > 0);
		}
	}
	free(fd_array);
	return 0;
}


