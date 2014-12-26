#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "log.h"
#include "tpool.h"
#include "mysql.h"

#define PORT 15110
#define BACKLOG 5
#define MAXDATASIZE 1000000

log_t *log;                         /*进程全局日志文件句柄*/
void process_cli(int connfd, struct sockaddr_in client,MYSQL my_connection);
void savadata_r(char *recvbuf, int len, char *cli_data);
void thread_func(void *arg);
int Authentication(char terminal_id[], char Authentication_code[]);

struct ARG                          /*线程函数传递所需参数*/
{
    int connfd;                     /*连接套接字*/
    struct sockaddr_in client;      /*客户端套接字地址*/
    MYSQL my_connection;            /*MySQL连接句柄*/
};

#pragma (push,1)
typedef struct DATA
{
    char start_symbol[16];          /*启动符*/
    int data_NO;                    /*数据包流水号*/
    char time_flag[7];              /*时间标签*/
    char terminal_ID[20];           /*前端机标识号*/
    char Authentication_codes[10];  /*认证码*/
    char data_type;                 /*数据类型*/
}data;

typedef struct VIDEO_SUMMARY_DATA   /*视频摘要信息*/
{
    char video_file_name[50];       /*文件名*/
    char start_time[7];             /*开始时间*/
    char end_time[7];               /*结束时间*/
}video_summary_data;

typedef struct PERSON_DATA          /*人员信息*/
{
    char bus_work_status;           /*汽车运行状态*/
    char people_num;                /*人员数量*/
    char longitude[4];              /*经度*/
    char latitude[4];               /*纬度*/
    char time_flag[7];               /*时间标签*/
}person_data;
#pragma (pop)

int main()
{
    MYSQL my_connection;                /*数据库连接句柄*/
    tpool_t *pool = NULL;               /*线程池指针*/
    log = log_open("server.log",0);     /*开启记录文件*/
    pool = tpool_init(200,300,1);       /*创建一个有200个工作线程，最大300个任务队列的线程池*/

	int listenfd, connectfd;

	struct ARG *arg = NULL;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t len;

    while(1)                            /*连接数据库*/
    {
        mysql_init(&my_connection);     /*初始化,my_connection是一个连接句柄，建议使用这种方式而不是采用指针接收返回句柄*/
        if(mysql_real_connect(&my_connection, "localhost","caihang", "admincai", "businfo", 0, NULL, 0))
                                        /*连接businfo数据库，数据库名字建议使用小写，利于平台移植*/
        {
            printf("Connection success！\n");
            break;
        }
        else
        {
            printf("connect database faild,connect again...\n");
                                        /*提示连接数据库失败，正在重新连接*/
        }
    }

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("creating socket failed.");
		exit(1);
	}

	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_PORT = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
		perror("Bind() error.");
		exit(1);
	}

	if (listen(listenfd, BACKLOG) == -1) {
		perror("listen() error.");
		exit(1);
	}

	len = sizeof(client);
    arg->my_connection = my_connection;

	while(1)
    {
        if((connectfd = accept(listenfd, (struct sockaddr *)&client,&len)) == -1)
        {
            perror("accept() error\n");
            exit(1);
        }

        arg = (struct ARG*)malloc(sizeof(struct ARG));
        arg->connfd = connectfd;
        memcpy((void *)&arg->client,&client,sizeof(client));

        tpool_add_work(pool,thread_func,(void *)arg);   //增加一个工作线程,arg参数传递
	}
	close(listenfd);
}

void process_cli(int connfd, struct sockaddr_in client,MYSQL my_connection)
{
    int i, rtn, offset, res, num = 0;                   //i用于循环下标，num用于判断数据是否接收完毕,rtn是接受函数的返回值，offset是用于数据处理时的偏移量,res用于my_query返回值
    int video_data_num, person_data_num;                //视频摘要信息条数和人数信息条数

    data *now_data = NULL;                              //用于存放收到的数据包中除数据长度和数据内容以外的其他数据
    video_summary_data *now_video_data = NULL;          //用于存放接收到的gps数据中的每一个数据单元
    person_data *now_person_data = NULL;                //用于存放接收到的人员信息中的每一个数据单元

    char ftpserverIPaddr[4];                            //cli_data用于存放客户信息，ftp用来暂存ftp服务器IP地址
    char recvbuf[MAXDATASIZE];                          //接收数据缓冲区
    char *sql, *dest;                                   //分别是sql语句和用来连接sql语句的指针
    sql = dest = NULL;

    printf("You got a cnnection from %s.\n",inet_ntoa(client.sin_addr));

    do                                                  //开始接收数据，用循环接收直到接收完毕
    {
        if((rtn = recv(connfd, (recvbuf + num), MAXDATASIZE, 0)) == 0)
        {
            close(connfd);
            break;
        }
        else
        {
            printf("Recevied %d bytes\n",rtn);          //显示实际接收到的字节数
            num = (*(int *)(recvbuf + 58));             //获取数据长度
            printf("actually should recevied %d bytes\n",num);
                                                        //显示原本应该接收到的字节数
            if(rtn != (num + 58))                       //判断是否接受完毕
            {
                num = num + rtn;                        //num作为接收数组下标而移动
                continue;                               //未接受完毕则提前结束本次循环,继续接收
            }
        }
    }while(rtn != (num + 58));                          //循环条件

    now_data = (data *)recvbuf;                         //取出前面一部分信息

    if(strcmp(now_data.start_symbol,"AAAAAAAAAAAAABBB") != 0)
                                                        //若不是启动符，则丢弃数据
    {
        printf("Recvied a invalid data.\n");
        return;                                         //函数返回
    }

    dest = "select * from bus_base_info where terminal_id=";
                                                        //连接sql查询语句字符串
    strcat(sql,dest);
    strcat(sql,"'");
    strcat(sql,now_data.terminal_ID);
    strcat(sql,"'");

    if(res = mysql_query(&my_connection, sql) != 0)
                                                        //查数据库bus_base_info表看接收到的前端机ID是否存在
    {
        printf("TerminalID can not found\n");
        return;
    }

    if(rtn = Authentication(now_data.terminal_ID, now_data.Authentication_codes) == 0)
                                                        //如果验证码错误
    {
        printf("Authentication_code wrong!\n");         //则显示并返回
        return;
    }

    /*数据无误，开始处理*/
    if(now_data.data_type == 0x20);                     //如果数据类型是视频摘要信息
    {
        memcpy(ftpserverIPaddr, recvbuf+63,4);          //将ftp服务器ip地址拷贝
        video_data_num = (*(int *)recvbuf + 67);        //获取数据内容里gps数据的数据长度

        if(video_data_num <= 0)                         //如果数据长度小于0，则丢弃该错误数据，并记录日志
        {
            lprintf(log, INFO, "recvied video summary data's length less than 0.");
            return;
        }

        offset = 71;
        for(i = 0; i < video_data_num; i++)
        {
            now_video_data = (video_summary_data *)(recvbuf + offset);    //将数据转换为video结构体

            sprintf(sql, "insert into video_summary_info(serialnumber,terminal_id,FTPserverIPaddr,video_name,starttime,endtime,receipttime)values('%s','%s','%s','%s','%s','%s','%s')",
                    now_data.data_NO, now_data.terminal_ID, ftpserverIPaddr, now_video_data.video_file_name, now_video_data.start_time,
                    now_video_data.end_time,/*接收时间*/);                //连接查询语句

            if(mysql_query(&my_connection,sql) != 0)                      //插入一条视频摘要信息
            {
                printf("insert video_summary_data failed\n");
                return;
            }
            else
            {
                printf("insert video_summary_data sucess\n");
                sql = dest = NULL;
            }
            offset+=64;                                                   //偏移量移动64个字节以获取下一条记录
        }

        /***回传客户端应答数据封装和发送***/
        now_data.data_type = 0x21;
        now_data.data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    else if(now_data.data_type == 0x30)                                 //如果数据类型是人员信息
    {
        person_data_num = (*(int *)recvbuf + 59);                       //获取接收到的人员信息数据长度

        if(person_data_num <= 0)                                        //如果数据长度小于0则丢弃数据
        {
            lprintf(log, INFO, "recvied person data's length is less than 0.");
            return ;
        }

        offset = 60;
        for(i = 0; i < person_data_num; i++)
        {
            now_person_data = (person_data *)(recvbuf + offset);        /*将数据转换为now_person_data结构体数据*/

            sprintf(sql, "insert into data_from_terminal(serialnumber,terminal_id,bus_status,longitude,latitude,num_of_people,collect_time,receipttime)values('%s','%s','%c','%s','%s','%c','%s','%s')",
                    now_data.data_NO, now_data.terminal_ID, now_person_data.bus_work_status, now_person_data.longitude, now_person_data.latitude,
                    now_person_data.people_num, now_person_data.time_flag,/*接收时间*/);

            if(mysql_query(&my_connection,sql) != 0)                    //插入一条人员信息
            {
                printf("insert person_data failed\n");
                return;
            }
            else
            {
                printf("insert person_data sucess\n");
                sql = dest = NULL;
            }
            offset += 17;
        }

        /***回传客户端应答数据封装和发送***/
        now_data.data_type = 0x31;
        now_data.data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    else                                                                  //不是规定的数据类型
    {
        printf("can't identified the data type\n");
        return ;
    }
}

void thread_func(void *arg)  /*线程函数*/
{
    struct ARG *info = NULL;
    info = (struct ARG *)arg;

    process_cli(info->connfd,info->client,info->my_connection);   /*调用线程处理函数*/
    free(arg);
    pthread_exit(NULL);
}

int Authentication(char terminal_id[], char Authentication_code[])   /*验证码处理函数*/
{
    int i, j;/*i,j用于循环*/
    char result[10];

    for(i =0, j = 6; i < 10; i++, j++)
    {
        result[i] = ~terminal_id[j];
    }

    if(strcmp(result,Authentication_code) != 0)return 0;  /*验证码不正确*/
    else return 1;
}

void gettime(char time[7])
{
    char testtime[25];
    time_t now;
	time(&now);
	testtime = ctime(&t);
}
