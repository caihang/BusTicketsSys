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
void process_cli(int connfd, struct sockaddr_in client);
void savadata_r(char *recvbuf, int len, char *cli_data);
void thread_func(void *arg);
void getthetime(char hyy[12]);
int Authentication(char terminal_id[], char Authentication_code[]);

struct ARG                          /*线程函数传递所需参数*/
{
    int connfd;                     /*连接套接字*/
    struct sockaddr_in client;      /*客户端套接字地址*/
};

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

int main()
{
    tpool_t *pool = NULL;                /*线程池指针*/
    log = log_open("server.log",0);      /*开启记录文件*/
    pool = tpool_init(200,300,1);        /*创建一个有200个工作线程，最大300个任务队列的线程池*/

	int listenfd, connectfd;

	struct ARG *arg = NULL;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t len;

	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		lprintf(log, FATAL, "creating socket failed.\n");
		exit(1);
	}

	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(listenfd, (struct sockaddr *) &server, sizeof(server)) == -1) {
		lprintf(log, FATAL, "Bind() error.\n");
		exit(1);
	}

	if (listen(listenfd, BACKLOG) == -1) {
		lprintf(log, INFO, "listen() error.\n");
		exit(1);
	}

	len = sizeof(client);

	while(1)
    {
        lprintf(log, INFO, "waiting connecting...\n");
        if((connectfd = accept(listenfd, (struct sockaddr *)&client,&len)) == -1)
        {
            lprintf(log, FATAL, "accept() error\n");
            exit(1);
        }

        arg = (struct ARG*)malloc(sizeof(struct ARG));
        arg->connfd = connectfd;
        memcpy((void *)&arg->client,&client,sizeof(client));
        lprintf(log, INFO, "Starting add a work\n");
        tpool_add_work(pool,thread_func,(void *)arg);   //增加一个工作线程,arg参数传递
	}
	close(listenfd);
}

void process_cli(int connfd, struct sockaddr_in client)
{
    MYSQL  my_connection;                               //数据库连接句柄
    int i, rtn, offset, res, num = 0;                   //i用于循环下标，num用于判断数据是否接收完毕,rtn是接受函数的返回值，offset是用于数据处理时的偏移量,res用于my_query返回值
    int video_data_num, person_data_num;                //视频摘要信息条数和人数信息条数

    char ftpserverIPaddr[4];                            //ftpserverIPaddr用来暂存ftp服务器IP地址
    char recvbuf[MAXDATASIZE];                          //接收数据缓冲区
    char sql[400] = {0};                                //sql语句指针
    char receipttime[12];                               //存放数据接收时间

    data *now_data = NULL;                              //用于存放收到的数据包中除数据长度和数据内容以外的其他数据
    video_summary_data *now_video_data = NULL;          //用于存放接收到的gps数据中的每一个数据单元
    person_data *now_person_data = NULL;                //用于存放接收到的人员信息中的每一个数据单元

    lprintf(log, INFO, "You got a cnnection from %s.\n",inet_ntoa(client.sin_addr));

    do                                                  //开始接收数据，用循环接收直到接收完毕
    {
        if((rtn = recv(connfd, (recvbuf + num), MAXDATASIZE, 0)) == 0)
        {
            close(connfd);
            break;
        }
        else
        {
            lprintf(log, INFO, "Recevied %d bytes\n",(rtn + num));
                                                        //记录实际接收到的字节数
            num = (*(int *)(recvbuf + 73));             //获取数据长度
            lprintf(log, INFO, "actually should recevied %d bytes\n",num);
                                                        //记录原本应该接收到的字节数
            if(rtn != (num + 77))                       //判断是否接受完毕
            {
                num = num + rtn;                        //num作为接收数组下标而移动
                continue;                               //未接受完毕则提前结束本次循环,继续接收
            }
        }
    }while(rtn != (num + 77));                          //循环条件

    getthetime(receipttime);                            //获取接收数据的时间

    now_data = (data *)recvbuf;                         //取出前面一部分信息

    if(strcmp((*now_data).start_symbol,"AAAAAAAAAAAAABB") != 0)
                                                        //若不是启动符，则丢弃数据
    {
        lprintf(log, FATAL, "Recvied a invalid data.\n");
        return;                                         //函数返回
    }
    lprintf(log, INFO, "The start_symbol is %s\n",(*now_data).start_symbol);

    while(1)                            /*连接数据库*/
    {
        mysql_init(&my_connection);     /*初始化,my_connection是一个连接句柄，建议使用这种方式而不是采用指针接收返回句柄*/
        if(mysql_real_connect(&my_connection, "localhost","caihang", "admincai", "businfo", 0, NULL, 0))
                                        /*连接businfo数据库，数据库名字建议使用小写，利于平台移植*/
        {
            lprintf(log, INFO, "Connection success\n");
            break;
        }
        else
        {
            lprintf(log, FATAL, "connect database faild,connect again...\n");
                                        /*记录连接数据库失败，正在重新连接*/
        }
    }

    /*此处查询schedule排班表看该车是否发车，若未发车则不处理，还需要修改*/
    sprintf(sql, "select * from schedule where schedule_num='%s'",(*now_data).schedule_num);
    lprintf(log, INFO, "The s_num is %s\n",(*now_data).schedule_num);

    if(res = mysql_query(&my_connection, sql) != 0)
                                                        //查数据库schedule表看接收到的班次号是否发车
    {
        lprintf(log, FATAL, "schedule_num can not found\n");
        mysql_close(&my_connection);
        return;
    }

    sprintf(sql, "select * from bus_base_info where terminal_id='%s'",(*now_data).terminal_ID);
    lprintf(log, INFO, "The TID is %s\n",(*now_data).terminal_ID);

    if(res = mysql_query(&my_connection, sql) != 0)
                                                        //查数据库bus_base_info表看接收到的前端机ID是否存在
    {
        lprintf(log, FATAL, "TerminalID can not found\n");
        mysql_close(&my_connection);
        return;
    }

    if(rtn = Authentication((*now_data).terminal_ID, (*now_data).Authentication_codes) == 0)
                                                        //如果验证码错误
    {
        lprintf(log, FATAL, "Authentication_code wrong!\n");         //则显示并返回
        mysql_close(&my_connection);
        return;
    }

    /*数据无误，开始处理*/
    if((*now_data).data_type == 0x20);                     //如果数据类型是视频摘要信息
    {
        memcpy(ftpserverIPaddr, recvbuf+77,4);          //将ftp服务器ip地址拷贝
        video_data_num = (*(int *)recvbuf + 81);        //获取数据内容里gps数据的数据条数

        if(video_data_num <= 0)                         //如果数据长度小于0，则丢弃该错误数据，并记录日志
        {
            lprintf(log, FATAL, "recvied video summary data's length less than 0.\n");
            mysql_close(&my_connection);
            return;
        }

        offset = 85;
        for(i = 0; i < video_data_num; i++)
        {
            now_video_data = (video_summary_data *)(recvbuf + offset);    //将数据转换为video结构体

            sprintf(sql, "insert into video_summary_info(serialnumber,terminal_id,FTPserverIPaddr,video_name,starttime,endtime,receipttime)values('%d','%s','%s','%s','%s','%s','%s')",
                    (*now_data).data_NO, (*now_data).terminal_ID, ftpserverIPaddr, (*now_video_data).video_file_name, (*now_video_data).start_time,
                    (*now_video_data).end_time,receipttime);                //连接查询语句

            if(mysql_query(&my_connection,sql) != 0)                      //插入一条视频摘要信息
            {
                lprintf(log, FATAL, "insert video_summary_data failed\n");
                mysql_close(&my_connection);
                return;
            }
            else
            {
                lprintf(log, INFO, "insert video_summary_data sucess\n");
            }
            offset+=64;                                                   //偏移量移动64个字节以获取下一条记录
        }

        /***回传客户端应答数据封装和发送***/
        (*now_data).data_type = 0x21;
        (*now_data).data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    if((*now_data).data_type == 0x30)                                 //如果数据类型是人员信息
    {
        person_data_num = (*(int *)recvbuf + 77);                       //获取接收到的人员信息数据条数

        if(person_data_num <= 0)                                        //如果数据长度小于0则丢弃数据
        {
            lprintf(log, FATAL, "recvied person data's length is less than 0.\n");
            mysql_close(&my_connection);
            return ;
        }

        offset = 81;
        for(i = 0; i < person_data_num; i++)
        {
            now_person_data = (person_data *)(recvbuf + offset);        /*将数据转换为now_person_data结构体数据*/

            sprintf(sql, "insert into data_from_terminal(serialnumber,terminal_id,bus_status,longitude,latitude,num_of_people,collect_time,receipttime)values('%d','%s','%c','%s','%s','%c','%s','%s')",
                    (*now_data).data_NO, (*now_data).terminal_ID, (*now_person_data).bus_work_status, (*now_person_data).longitude, (*now_person_data).latitude,
                    (*now_person_data).people_num, (*now_person_data).time_flag,receipttime);

            if(mysql_query(&my_connection,sql) != 0)                    //插入一条人员信息
            {
                lprintf(log, FATAL, "insert person_data failed\n");
                mysql_close(&my_connection);
                return;
            }
            else
            {
                lprintf(log, INFO, "insert person_data sucess\n");
            }
            offset += 22;
        }

        /***回传客户端应答数据封装和发送***/
        (*now_data).data_type = 0x31;
        (*now_data).data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    else                                                                    //不是规定的数据类型
    {
        lprintf(log, FATAL, "can't identified the data type\n");
        mysql_close(&my_connection);
        return ;
    }
    mysql_close(&my_connection);                                            //关闭数据库的连接
}

void thread_func(void *arg)  /*线程函数*/
{
    struct ARG *info = NULL;
    info = (struct ARG *)arg;

    process_cli(info->connfd,info->client);                         /*调用线程处理函数*/
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
