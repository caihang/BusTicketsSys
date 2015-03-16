#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>   //数据发送接收函数
#include <sys/socket.h>  //通用套接字地址结构
#include <netinet/in.h>   //主机/网络地址转换
#include <netdb.h>     //netdb is needed for struct hostent
#include <sys/types.h>
#include <time.h>

#define PORT 15110
#define MAXDATASIZE 100000
void process(FILE *fp, int sockfd);
char *getMessage(char *sendline, int len, FILE *fp);
void getthetime(char hyy[12]);

#pragma (push,1)
typedef struct DATA
{
    char start_symbol[16];          /*启动符*/
    int data_NO;                    /*数据包流水号*/
    char time_flag[12];             /*时间标签*/
    char terminal_ID[20];           /*前端机标识号*/
    char schedule_num[10];          /*班次号*/
    char Authentication_codes[10];  /*认证码*/
    char data_type;                 /*数据类型*/
}data;

typedef struct VIDEO_SUMMARY_DATA   /*视频摘要信息*/
{
    char video_file_name[50];       /*文件名*/
    char start_time[12];             /*开始时间*/
    char end_time[12];               /*结束时间*/
}video_summary_data;

typedef struct PERSON_DATA          /*人员信息*/
{
    char bus_work_status;           /*汽车运行状态*/
    char people_num;                /*人员数量*/
    char longitude[4];              /*经度*/
    char latitude[4];               /*纬度*/
    char time_flag[12];               /*时间标签*/
}person_data;
#pragma (pop)

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
    int num = 0, datalength, person_data_num = 1;

    data test_data;
    person_data test_person_data;
    video_summary_data test_video_data;

    printf("Connect to server.\n");

    strcpy(test_data.start_symbol,"AAAAAAAAAAAAABB");
    test_data.start_symbol[15] = '\0';
	strcpy(sendline,test_data.start_symbol);
    test_data.data_NO = 5;
	(*(int *)(sendline + 16)) = test_data.data_NO;
    getthetime(test_data.time_flag);
	strcpy((sendline + 20),test_data.time_flag);
    strcpy(test_data.terminal_ID,"0000000000000000111");
	test_data.terminal_ID[19] = '\0';
	strcpy((sendline + 32),test_data.terminal_ID);
    strcpy(test_data.schedule_num,"0001");
	test_data.schedule_num[9] = '\0';
    strcpy((sendline + 52),test_data.schedule_num);
    test_data.data_type = 48&0x000000ff;
	printf("data_type is %x\n",test_data.data_type);
	strcpy((sendline + 62),"111111111");
	sendline[71] = '\0';
	sendline[72] = test_data.data_type;
    test_person_data.bus_work_status = '1';
    test_person_data.people_num = '9';
    strcpy(test_person_data.latitude,"234");
	test_person_data.latitude[3] = '\0';
    strcpy(test_person_data.longitude,"4578");
    getthetime(test_person_data.time_flag);
    datalength = person_data_num *22 + 4;
    (*(int *)(sendline + 73)) = datalength;
    (*(int *)(sendline + 77)) = person_data_num;
	sendline[81] = test_person_data.bus_work_status;
	sendline[82] = test_person_data.people_num;
	strcpy((sendline + 83),test_person_data.latitude);
	strcpy((sendline + 87),test_person_data.longitude);
	strcpy((sendline + 91),test_person_data.time_flag);
    send(sockfd,sendline,103,0);
    /*num = recv(sockfd,recvbuf,MAXDATASIZE,0);
    recvbuf[num] = '\0';
    printf("recevied:%s\n",recvbuf);*/
	close(sockfd);
}

void getthetime(char hyy[12])
{
    char lyy[3],MM[3],dd[3],hh[3],mm[3],ss[3];
    int year,month,day,hour,min,sec;
    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    year = (1900+timeinfo->tm_year)%100;
    month = 1+timeinfo->tm_mon;
    day = timeinfo->tm_mday;
    hour = timeinfo->tm_hour;
    min = timeinfo->tm_min;
    sec = timeinfo->tm_sec;

    lyy[0] = (year/10) + '0';
    lyy[1] = (year%10) + '0';
    lyy[2] = '\0';
    MM[0] = (month/10) + '0';
    MM[1] = (month%10) + '0';
    MM[2] = '\0';
    dd[0] = (day/10) + '0';
    dd[1] = (day%10) + '0';
    dd[2] = '\0';
    hh[0] = (hour/10) + '0';
    hh[1] = (hour%10) + '0';
    hh[2] = '\0';
    mm[0] = (min/10) + '0';
    mm[1] = (min%10) + '0';
    mm[2] = '\0';
    ss[0] = (sec/10) + '0';
    ss[1] = (sec%10) + '0';
    ss[2] = '\0';
    sprintf(hyy,"%s%s%s%s%s%s",lyy,MM,dd,hh,mm,ss);
}

