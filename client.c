#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   //数据发送接收函数
#include <sys/socket.h>  //通用套接字地址结构
#include <netinet/in.h>   //主机/网络地址转换
#include <netdb.h>     //netdb is needed for struct hostent
#include <sys/types.h>

#define PORT 15110
#define MAXDATASIZE 100000
void process(FILE *fp, int sockfd);
char *getMessage(char *sendline, int len, FILE *fp);

int main(int argc, char *argv[])
{
    int fd;
    struct hostent *he;       //structure that will get information about remote host
    struct sockaddr_in server;
    if(argc != 2)
    {
        printf("Useage:%s,<IP Address>\n",argv[0]);
        exit(-1);
    }
    if((he = gethostbyname(argv[1])) == NULL)
    {
        perror("gethostbyname error");
        exit(-1);
    }
    if((fd = socket(AF_INET,SOCK_STREAM,0)) == -1)
    {
        perror("Create socket failed");
        exit(-1);
    }
    bzero(&server,sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr = *((struct in_addr*)he->h_addr);
    if(connect(fd,(struct sockaddr *)&server,sizeof(struct sockaddr)) == -1)
    {
        perror("recv error");
        exit(-1);
    }
    process(stdin,fd);
    close(fd);

}

void process(FILE *fp, int sockfd)
{
    char sendline[MAXDATASIZE],recvbuf[MAXDATASIZE];
    int num;
    printf("Connect to server.\n");

    printf("Input th string want to send:");
    scanf("%s",sendline);
    num = strlen(sendline);
    sendline[num] = '\0';
    num = send(sockfd,sendline, strlen(sendline),0);
    printf("Send numbyte is %d\n",num);

    num = recv(sockfd,recvbuf,MAXDATASIZE,0);
    recvbuf[num] = '\0';
    printf("%s\n",recvbuf);
}


