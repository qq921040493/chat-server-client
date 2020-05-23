//用户
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<errno.h>
#include<netdb.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<unistd.h>
#include <pthread.h>
#define TRUE 1
#define PORT 8081
#define NUM 100
int quit=0; //quit表示是否用户确定退出
void *get_server(void *);



#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"
#define RESET "\033[0m"


int connfd,snd,slenth;
struct sockaddr_in server;
struct hostent *hp;
struct user
{
	int iNum;
	char szName[9];
};

struct user User[NUM];

char honame[20],msg2[1024],msg1[1024],cln[102],qstr[]={"quit"}; //qstr的值表示用户在输入"Quit"时和服务器断开链接
pthread_t tid;


void openfile();
void savefile();
void load();
void reg();
void address();
void menu();
void chatroom();
void show();



void menu()
{
	printf(KYEL "WELCOME TO CHATROOM!\n"RESET);
	printf(KYEL "1.regest\n"RESET);
	printf(KYEL "2.login\n"RESET);
	printf(KBLU "3.exit\n"RESET);
	printf(KRED "PLEASE MAKE YOUR CHOICE:\n"RESET);
	int select;
	scanf("%d",&select);
	switch(select)
	{
		case 1:reg();break;
		case 2:{load();chatroom();}break;
		case 3:{printf("thank you for using!\n");exit(0);}break;
		default:{printf(KRED"please enter a num between 1~3!\n"RESET);menu;}break;
	}

}

void savefile()
{
	int a=0;
	while(User[a].iNum !=0)a++;
	FILE *fp;
	fp = fopen("a.txt","wb+");
	if (fp == NULL)
	{
			printf("null\n");
			return;
	}
	int f=a-1;
	while ((f+1))
	{
		fwrite(&User[f],sizeof(struct user),1,fp);
		f--;
	}
	printf(KRED"\tsave successful!\n"RESET);
	fclose(fp);
}
void openfile()
{
	FILE *fp;
	int i=0;
	fp = fopen("a.txt","ab+");
	if(fp == NULL)
	{
		printf("null!\n");
	}
	printf(KRED"file opened!"RESET);
	while(fread(&User[i],sizeof(struct user),1,fp)==1)
	{
		i++;
	}
	fclose(fp);
}


void show()
{
	FILE *fp;
	int i = 0;
	fp = fopen("a.txt","ab+");
	if (fp == NULL)
	{
		printf(KRED"null\n"RESET);
	}
	while(fread(&User[i],sizeof(struct user),1,fp)==1)
	{
		printf(KBLU"\t the %d user:Name%s\t Password:%d\n"RESET,i+1,User[i].szName,User[i].iNum);
		printf("\n");
		i++;
	}
	i=0;
	fclose(fp);
}

void reg()
{
	int a=0;
	while(User[a].iNum!=0)
		a++;
	printf(KYEL"Please enter username:\n"RESET);
	scanf("%s",User[a].szName);
	printf(KYEL"enter your password:\n"RESET);
	scanf("%d",&User[a].iNum);
	printf("success!\n");
	printf(KYEL"here's your information:\n");
	printf("\tName:\tpw:\n");
	printf("\t%s\t%d\n",User[a].szName,User[a].iNum);
	savefile();
	show();
	menu();
}

int main()
{
	address();
	menu();

}

void address()
{
	printf("loading.............\n");
	strcpy(honame,"127.0.0.1");
	printf("creating socket...\n");
	if((connfd= socket(AF_INET, SOCK_STREAM, 0))<0) //建立套接口
		printf("creat socket failed\n");
	if ((hp= gethostbyname(honame))== NULL) //获取服务器IP地址
	{
		printf("can not get server IP\n");
		exit(1);
	}
	else printf("connecting server...\n");

	memcpy(&server.sin_addr,hp->h_addr,hp->h_length); //将服务器IP地址放入结构体server中
	server.sin_family = AF_INET;
	server.sin_port=htons(PORT);
	if(connect(connfd,(struct sockaddr*)&server,sizeof(server))<0) //链接套接口
	{
		printf("can not connect with server\n");
		exit(1);
	}
	printf("connected server\n"); //链接成功显示成功的登录信息
	printf("Welcome to chat room\n");
}

void load()
{
	openfile();
	int pw;
	int a=0;
	printf("Enter your username\n");
	scanf("%s",msg1);
	while(User[a].iNum!=0)
		a++;
	for (int n=0;n<a+1;n++)
	{
		if(strcmp(msg1,User[n].szName)==0)
		{
			printf("Please enter your password:\n");
			a:scanf("%d",&pw);
			if (pw==User[n].iNum) break;
			else
			{
				printf(KRED"error!please try again\n"RESET);
				goto a;
			}
		}
		if(n==a)
		{
			printf(KRED"user not exist!\n"RESET);
			menu();
		}
	}
	slenth=strlen(msg1);
	msg1[slenth]=':';
	msg1[slenth+1]='\0';
	strcpy(cln,msg1); //保存用户昵称在名为cln的数组中
	pthread_create(&tid,NULL,&get_server,(void*)connfd);//为客户端创建一个线程用于监听，调用get_server函数
}



	
	void chatroom()//聊天室
{
	printf("\nstart chat!\n(\"quit\"disconnect)\n");
	while(TRUE)
	{
		printf("\n");
		scanf("%s",msg2);
		if(strcmp(msg2,qstr)==0)
		{
			close(connfd);
			quit=1; //若用户输入"Quit"字符则关闭发送套接口,并将quit置为1
		}
		else
		{
			strcat(msg1,msg2);//将消息前加上用户昵称
			snd=send(connfd,msg1,strlen(msg1)+1,0);//否则发送消息给服务器
			strcpy(msg1,cln);
			if(snd<0)
				printf("\nsend error\n");
		}
	}
}
void *get_server(void* sockfd) //get_server函数，用于接受服务器转发的消息
{
	char buf[1024];
	int rev;
	if((sockfd)<0)
		printf("\nfailed to accept server message\n");
	else
	{
		printf("\n");
		for(;;)
		{
			if(!quit)//只要quit不为1，则一直接受服务器消息
			{
				if ((rev = recv(sockfd,buf,1024,0))>0)
					printf("%s\n", buf);
			if (rev==0)
			{
				printf("\nserver stop connecting\n");
				quit=1;
				continue;
			}
			printf("\n");
			}
			else
			{
				close(sockfd);//关闭此套接口
				break;
			}
		}
		return(NULL);
	}
}




/*else if (strncmp (input, "@", 1) == 0)
      {
          message msg;
          msg.type = PRIVATE_MESSAGE;

          char *toUsername, *chatMsg;

          toUsername = strtok (input + 1, " ");

          if (toUsername == NULL)
            {
                puts (KRED "The format for private messages is: @<username> <message>" RESET);
                return;
            }

          if (strlen (toUsername) == 0)
            {
                puts (KRED "You must enter a username for a private message." RESET);
                return;
            }

          if (strlen (toUsername) > 20)
            {
                puts (KRED "The username must be between 1 and 20 characters." RESET);
                return;
            }

          chatMsg = strtok (NULL, "");

          if (chatMsg == NULL)
            {
                puts (KRED "You must enter a message to send to the specified user." RESET);
                return;
            }

          strncpy (msg.username, toUsername, 20);
          strncpy (msg.data, chatMsg, 255);

          if (send (connection->socket, &msg, sizeof (message), 0) < 0)
            {
                perror ("Send failed");
                exit (1);
            }

      }*/

