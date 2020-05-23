//服务器
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#define MAXLINE 1000 //在一条消息中最大的输出字符数
#define LISTENQ 20 //最大监听队列
#define PORT 8081//监听端口
#define MAXFD 20 //最大的在线用户数量
void *get_client(void *);
int i,maxi=-1;//maxi表示当前client数组中最大的用户的i值
int client[MAXFD];
int main()
{
	int connfd,listenfd,sockfd; //connfd存放accept函数的返回值，listenfd表示监听的套接口，
	//sockfd用于遍历client的数组
	socklen_t length;
	struct sockaddr_in server;
	struct sockaddr tcpaddr;
	pthread_t tid;
	listenfd=socket(AF_INET,SOCK_STREAM,0); //建立套接口并监听
	if(listenfd<0){
		printf("failed to creat socket\n");
		exit(1);
	}
	memset(&server,0,sizeof(server));
	server.sin_family=AF_INET;
	server.sin_port=htons(PORT);
	server.sin_addr.s_addr=htonl(INADDR_ANY);
	if( bind(listenfd,(struct sockaddr*)&server,sizeof(server))<0 )
	{
		printf("bind error\n");
		exit(1); //绑定套接口
	}
	length=sizeof(server);
	if(getsockname(listenfd,(struct sockaddr*)&server,&length)<0)
	{
		printf("failed to get server port!\n"); //取得服务器的端口号
		exit(1);
	}
	for(i=0;i<MAXFD;i++)
		client[i]=-1; //initialize the client column
	//监听端口
	listen(listenfd, LISTENQ);
	printf("listening  %d...\n", ntohs(server.sin_port));
	printf("welcome to the chat room!\n");
	//等待用户链接.
	for(;;)
	{
		connfd=accept(listenfd, &tcpaddr, &length);
		for(i=0;i<MAXFD;i++)
		{
			if(client[i]<0)
			{
				client[i]=connfd;
				break;
			} //用户链接成功后，在client数组中保存用户套接口号
			if(i==MAXFD-1)
			{
				printf("reach the maximum number!\n"); //若此时以达到用户最大值，则退出链接
				exit(0);
			}
			if(i>maxi) 
				maxi=i;
		}
		pthread_create(&tid,NULL,&get_client,(void*)connfd); //若链接成功，为此用户创建一个新线程
	} //运行get_client函数，处理用户请求
	 return 0;
}

void *get_client(void *sockfd) //get_client函数
{
	char buf[MAXLINE];
	int rev;
	if (sockfd<0)
	printf("\nfailed enter the chat room\n");
	else
	{
		printf("\nnew user entering...\n");
		do
		{
			memset(buf,0,sizeof(buf)); //初始化buffer
			if ((rev = recv((int)sockfd,buf,1024,0))<0)
				printf("\nfail to get user message\n");
			if (rev==0)
				printf("\nuser disconnect\n");
			else
			{
				printf("%s\n", buf); //若无异常，输出此用户消息
				for(i=0;i<=maxi+1;i++)
				send(client[i],buf,strlen(buf)+1,0);//将刚收到的用户消息分发给其他各用户
			}
		}while (rev != 0);//当不再受到用户信息时，终止循环并且关闭套接口
	}
	close(sockfd);
	return(NULL);
}

