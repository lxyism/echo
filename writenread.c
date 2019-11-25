#include <sys/types.h>
#include <unistd.h>
#include "writenread.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void err_quit(const char* msg)
{
	perror(msg);
	exit(-1);
}

//void err_quit(const char* msg)
//{
//	printf("%s", msg);
//	exit(-1);
//}


ssize_t readn(int fd, void* vptr, size_t n)
{
	size_t nleft;
	ssize_t nread;
	char* ptr;

	ptr = (char*)vptr;
	nleft = n;

	while (nleft > 0)
	{
		if ((nread = read(fd, ptr, nleft)) < 0)
		{
			if (EINTR == errno)
				nread = 0;  //�ݲ����
			else
				return -1;
		}
		else if (nread == 0)  // EOF
			break;

		nleft -= nread;  //����Ҫ�������ݴ�С
		ptr += nread;   //��ָ���Ƶ��Ѷ�������β ���⸲��
	}
	return (n - nleft);  //�Ѷ���ʵ������
}

ssize_t writen(int fd, const void* vptr, size_t n)
{
	size_t nleft;
	ssize_t nwritten;
	const char* ptr;

	ptr = (char*)vptr;
	nleft = n;
	while (nleft > 0)
	{
		if ((nwritten = write(fd, ptr, nleft)) <= 0)
		{
			if (nwritten < 0 && EINTR == errno) //�ݲ����
				nwritten = 0;
			else
				return -1;
		}

		nleft -= nwritten;  //������д���ݴ�С
		ptr += nwritten;	//��ָ���Ƶ���д����β �����ظ�д
	}

	return n;
}

ssize_t readline(int fd, void* vptr, size_t maxlen)
{
	ssize_t n, rc;
	char c, * ptr;

	ptr = (char*)vptr;
	for (n = 1; n < maxlen; n++)
	{
	again:
		if ((rc = read(fd, &c, 1)) == 1)
		{
			*ptr++ = c;
			if ('\n' == c)
				break;
		}
		else if (0 == rc)
		{
			*ptr = 0;
			return (n - 1);
		}
		else
		{
			if (EINTR == errno)
				goto again;
			return -1;
		}
	}
	*ptr = 0;
	return n;
}

void str_cli(FILE* fp, int sockfd)
{
	char sendline[MAXLINE], recvline[MAXLINE];
	int n;
	while (fgets(sendline, MAXLINE, fp) != NULL)
	{
		while ((n = write(sockfd, sendline, strlen(sendline) + 1)) < 0 && (EINTR == errno));
		if (n < 0)
			err_quit("write error");
		//	printf("client send success\n");
		while ((n = read(sockfd, recvline, MAXLINE)) < 0 && (EINTR == errno));
		if (n < 0)
			err_quit("read error");
		if (fputs(recvline, stdout) == EOF)
			err_quit("fputs error");
	}
}

void str_echo(int sockfd)
{
	char buf[100];
	int n, m;
	while (1)
	{
		while ((n = read(sockfd, buf, MAXLINE)) < 0 && (EINTR == errno));
		if (n < 0)
			err_quit("read error");
		else if (0 == n)
		{
			close(sockfd);
			break;
		}
		while ((m = write(sockfd, buf, n)) < 0 && (EINTR == errno));
		if (m < 0)
			err_quit("write error");
	}
}
