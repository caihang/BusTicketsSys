#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "log.h"
#include "tpool.h"
#include "mysql.h"

#define PORT 15110
#define BACKLOG 5
#define MAXDATASIZE 1000000
log_t *log;    /*进程全局日志文件句柄*/
void process_cli(int connfd, struct sockaddr_in client);
void savadata_r(char *recvbuf, int len, char *cli_data);
void thread_func(void *arg);

struct ARG
{
    int connfd;
    struct sockaddr_in client;
};
pthread_key_t key;
pthread_once_t once = PTHREAD_ONCE_INIT;
static void destructor(void *ptr)
{
    free(ptr);
}
static void creatkey_once(void)
{
    pthread_key_create(&key, destructor();)
}
struct ST_DATA
{
    int index;
};
#pragma (push,1)
typedef struct DATA
{
    char start_symbol[16];   /*启动符*/
    char data_NO[4];        /*数据包流水号*/
    char time_flag[7];      /*时间标签*/
    char client_ID[20];     /*前端机标识号*/
    char Authentication_codes[10];  /*认证码*/
    char data_type;  /*数据类型*/
}data;

typedef struct GPS_DATA   //gps信息
{
    char longitude[4];   //经度
    char latitude[4];    //纬度
    char time_flag[7];   //时间
}gps_data;

typedef struct PERSON_DATA //人员信息
{
    char bus_work_status; //汽车运行状态
    char people_num;  //人员数量
    char longitude[4];  //经度
    char latitude[4];  //纬度
    char time_flag[7];  //时间标签
}person_data;
#pragma (pop)

int main()
{
    tpool_t *pool;  /*线程池指针*/
    log = log_open("server.log",0);   /*开启记录文件*/
    pool = tpool_init(200,300,1);  /*创建一个有200个工作线程，最大300个任务队列的线程池*/
	int listenfd, connectfd;
	struct ARG *arg;   //需要传递的参数
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t len;
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

	while (1)
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

void process_cli(int connfd, struct sockaddr_in client)
{
    MYSQL my_connection;
    int i,num = 0, rtn, offset, res;   //i用于循环下标，num用于判断数据是否接收完毕,rtn是接受函数的返回值，offset是用于数据处理时的偏移量,res用于my_query返回值
    int gps_data_num, person_data_num;  //gps信息条数和人数信息条数
    data *now_data;   //用于存放收到的数据包中除数据长度和数据内容以外的其他数据
    gps_data *now_gps_data;   //用于存放接收到的gps数据中的每一个数据单元
    person_data *now_person_data;  //用于存放接收到的人员信息中的每一个数据单元
    char cli_data[1000];
    char recvbuf[MAXDATASIZE],sendbuf[100];
    printf("You got a cnnection from %s.\n",inet_ntoa(client.sin_addr));
    do
    {
        if((rtn = recv(connfd, (recvbuf + num), MAXDATASIZE, 0)) == 0)
        {
            close(connfd);
            break;
        }
        else
        {
            printf("Recevied %d bytes\n",(int)rtn);
            num = (*(int *)(recvbuf + 58));   //获取数据长度
            printf("actually should recevied %d bytes\n",num);
            if(rtn != (num + 58))   //判断是否接受完毕
            {
                num = num + (int)rtn;
                continue;   //提前结束本次循环
            }
        }
    }while(rtn != (num + 58));     //判断接受到的数据是否完整
    now_data = (data *)recvbuf;    //取出前面一部分信息
    if(strcmp(now_data.start_symbol,"AAAAAAAAAAAAABBB") != 0) //若不是启动符，则丢弃数据
    {
        printf("Recvied a invalid data.\n");
        return;  //函数返回
    }
    mysql_init(&my_connection);  /*初始化,my_connection是一个连接句柄，建议使用这种方式而不是采用指针接收返回句柄*/
    if(mysql_real_connect(&my_connection, "localhost","caihang", "admincai", "bus_info", 0, NULL, 0)) /*连接BusInfo数据库，数据库名字建议使用小写，利于平台移植*/
    {
        printf("Connection success！\n");
        res = mysql_query(&my_connection, "INSERT INTO children(fname, age) VALUES('Ann', 3)");
        if (!res)
        {
            printf("Inserted %lu rows\n",
                    (unsigned long)mysql_affected_rows(&my_connection));
        } else
        {
            fprintf(stderr, "Insert error %d: %s\n", mysql_errno(&my_connection), mysql_error(&my_connection));
        }
    }
    if(now_data.Authentication_codes)  /*判断认证码是否正确，查询数据库里已有的认证码进行比对，若没找到则丢弃数据包*/
    {

    }
    if(now_data.data_type == 0x10);  /*如果是GPS信息*/
    {
        gps_data_num = (*(int *)recvbuf + 59);    //获取数据内容里gps数据的数据长度
        if(gps_data_num <= 0)    //如果数据长度小于0，则丢弃该错误数据
        {
            lprintf(log, INFO, "recvied gps data's length less than 0.");
            return;
        }
        offset = 60;
        for(i = 0; i < gps_data_num; i++)
        {
            now_gps_data = (gps_data *)(recvbuf + offset);
            /*将这一条记录存入数据库*/
            /************/


            /************/
            offset+=15;
        }
        /***回传应答数据封装和发送***/
        now_data.data_type = 0x11;
    }
    else if(now_data.data_type == 0x20)  //如果是人员信息
    {
        person_data_num = (*(int *)recvbuf + 59);  //获取接收到的人员信息数据长度
        if(person_data_num <= 0)  //如果数据长度小于0则丢弃数据
        {
            lprintf(log, INFO, "recvied person data's length is less than 0.");
            return ;
        }
        offset = 60;
        for(i = 0; i < person_data_num; i++)
        {
            now_person_data = (person_data *)(recvbuf + offset);
            /*将这条记录存入数据库相应表中*/
            /************/


            /************/
            offset += 17;
        }
        /***回传应答数据封装和发送***/
        now_data.data_type = 0x21;
    }
}

void thread_func(void *arg)
{
    struct ARG *info;
    info = (struct ARG *)arg;
    process_cli(info->connfd,info->client);
    free(arg);
    pthread_exit(NULL);
}
