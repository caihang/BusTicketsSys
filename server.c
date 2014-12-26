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

log_t *log;                         /*����ȫ����־�ļ����*/
void process_cli(int connfd, struct sockaddr_in client,MYSQL my_connection);
void savadata_r(char *recvbuf, int len, char *cli_data);
void thread_func(void *arg);
int Authentication(char terminal_id[], char Authentication_code[]);

struct ARG                          /*�̺߳��������������*/
{
    int connfd;                     /*�����׽���*/
    struct sockaddr_in client;      /*�ͻ����׽��ֵ�ַ*/
    MYSQL my_connection;            /*MySQL���Ӿ��*/
};

#pragma (push,1)
typedef struct DATA
{
    char start_symbol[16];          /*������*/
    int data_NO;                    /*���ݰ���ˮ��*/
    char time_flag[7];              /*ʱ���ǩ*/
    char terminal_ID[20];           /*ǰ�˻���ʶ��*/
    char Authentication_codes[10];  /*��֤��*/
    char data_type;                 /*��������*/
}data;

typedef struct VIDEO_SUMMARY_DATA   /*��ƵժҪ��Ϣ*/
{
    char video_file_name[50];       /*�ļ���*/
    char start_time[7];             /*��ʼʱ��*/
    char end_time[7];               /*����ʱ��*/
}video_summary_data;

typedef struct PERSON_DATA          /*��Ա��Ϣ*/
{
    char bus_work_status;           /*��������״̬*/
    char people_num;                /*��Ա����*/
    char longitude[4];              /*����*/
    char latitude[4];               /*γ��*/
    char time_flag[7];               /*ʱ���ǩ*/
}person_data;
#pragma (pop)

int main()
{
    MYSQL my_connection;                /*���ݿ����Ӿ��*/
    tpool_t *pool = NULL;               /*�̳߳�ָ��*/
    log = log_open("server.log",0);     /*������¼�ļ�*/
    pool = tpool_init(200,300,1);       /*����һ����200�������̣߳����300��������е��̳߳�*/

	int listenfd, connectfd;

	struct ARG *arg = NULL;
	struct sockaddr_in server;
	struct sockaddr_in client;
	socklen_t len;

    while(1)                            /*�������ݿ�*/
    {
        mysql_init(&my_connection);     /*��ʼ��,my_connection��һ�����Ӿ��������ʹ�����ַ�ʽ�����ǲ���ָ����շ��ؾ��*/
        if(mysql_real_connect(&my_connection, "localhost","caihang", "admincai", "businfo", 0, NULL, 0))
                                        /*����businfo���ݿ⣬���ݿ����ֽ���ʹ��Сд������ƽ̨��ֲ*/
        {
            printf("Connection success��\n");
            break;
        }
        else
        {
            printf("connect database faild,connect again...\n");
                                        /*��ʾ�������ݿ�ʧ�ܣ�������������*/
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

        tpool_add_work(pool,thread_func,(void *)arg);   //����һ�������߳�,arg��������
	}
	close(listenfd);
}

void process_cli(int connfd, struct sockaddr_in client,MYSQL my_connection)
{
    int i, rtn, offset, res, num = 0;                   //i����ѭ���±꣬num�����ж������Ƿ�������,rtn�ǽ��ܺ����ķ���ֵ��offset���������ݴ���ʱ��ƫ����,res����my_query����ֵ
    int video_data_num, person_data_num;                //��ƵժҪ��Ϣ������������Ϣ����

    data *now_data = NULL;                              //���ڴ���յ������ݰ��г����ݳ��Ⱥ����������������������
    video_summary_data *now_video_data = NULL;          //���ڴ�Ž��յ���gps�����е�ÿһ�����ݵ�Ԫ
    person_data *now_person_data = NULL;                //���ڴ�Ž��յ�����Ա��Ϣ�е�ÿһ�����ݵ�Ԫ

    char ftpserverIPaddr[4];                            //cli_data���ڴ�ſͻ���Ϣ��ftp�����ݴ�ftp������IP��ַ
    char recvbuf[MAXDATASIZE];                          //�������ݻ�����
    char *sql, *dest;                                   //�ֱ���sql������������sql����ָ��
    sql = dest = NULL;

    printf("You got a cnnection from %s.\n",inet_ntoa(client.sin_addr));

    do                                                  //��ʼ�������ݣ���ѭ������ֱ���������
    {
        if((rtn = recv(connfd, (recvbuf + num), MAXDATASIZE, 0)) == 0)
        {
            close(connfd);
            break;
        }
        else
        {
            printf("Recevied %d bytes\n",rtn);          //��ʾʵ�ʽ��յ����ֽ���
            num = (*(int *)(recvbuf + 58));             //��ȡ���ݳ���
            printf("actually should recevied %d bytes\n",num);
                                                        //��ʾԭ��Ӧ�ý��յ����ֽ���
            if(rtn != (num + 58))                       //�ж��Ƿ�������
            {
                num = num + rtn;                        //num��Ϊ���������±���ƶ�
                continue;                               //δ�����������ǰ��������ѭ��,��������
            }
        }
    }while(rtn != (num + 58));                          //ѭ������

    now_data = (data *)recvbuf;                         //ȡ��ǰ��һ������Ϣ

    if(strcmp(now_data.start_symbol,"AAAAAAAAAAAAABBB") != 0)
                                                        //����������������������
    {
        printf("Recvied a invalid data.\n");
        return;                                         //��������
    }

    dest = "select * from bus_base_info where terminal_id=";
                                                        //����sql��ѯ����ַ���
    strcat(sql,dest);
    strcat(sql,"'");
    strcat(sql,now_data.terminal_ID);
    strcat(sql,"'");

    if(res = mysql_query(&my_connection, sql) != 0)
                                                        //�����ݿ�bus_base_info�����յ���ǰ�˻�ID�Ƿ����
    {
        printf("TerminalID can not found\n");
        return;
    }

    if(rtn = Authentication(now_data.terminal_ID, now_data.Authentication_codes) == 0)
                                                        //�����֤�����
    {
        printf("Authentication_code wrong!\n");         //����ʾ������
        return;
    }

    /*�������󣬿�ʼ����*/
    if(now_data.data_type == 0x20);                     //���������������ƵժҪ��Ϣ
    {
        memcpy(ftpserverIPaddr, recvbuf+63,4);          //��ftp������ip��ַ����
        video_data_num = (*(int *)recvbuf + 67);        //��ȡ����������gps���ݵ����ݳ���

        if(video_data_num <= 0)                         //������ݳ���С��0�������ô������ݣ�����¼��־
        {
            lprintf(log, INFO, "recvied video summary data's length less than 0.");
            return;
        }

        offset = 71;
        for(i = 0; i < video_data_num; i++)
        {
            now_video_data = (video_summary_data *)(recvbuf + offset);    //������ת��Ϊvideo�ṹ��

            sprintf(sql, "insert into video_summary_info(serialnumber,terminal_id,FTPserverIPaddr,video_name,starttime,endtime,receipttime)values('%s','%s','%s','%s','%s','%s','%s')",
                    now_data.data_NO, now_data.terminal_ID, ftpserverIPaddr, now_video_data.video_file_name, now_video_data.start_time,
                    now_video_data.end_time,/*����ʱ��*/);                //���Ӳ�ѯ���

            if(mysql_query(&my_connection,sql) != 0)                      //����һ����ƵժҪ��Ϣ
            {
                printf("insert video_summary_data failed\n");
                return;
            }
            else
            {
                printf("insert video_summary_data sucess\n");
                sql = dest = NULL;
            }
            offset+=64;                                                   //ƫ�����ƶ�64���ֽ��Ի�ȡ��һ����¼
        }

        /***�ش��ͻ���Ӧ�����ݷ�װ�ͷ���***/
        now_data.data_type = 0x21;
        now_data.data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    else if(now_data.data_type == 0x30)                                 //���������������Ա��Ϣ
    {
        person_data_num = (*(int *)recvbuf + 59);                       //��ȡ���յ�����Ա��Ϣ���ݳ���

        if(person_data_num <= 0)                                        //������ݳ���С��0��������
        {
            lprintf(log, INFO, "recvied person data's length is less than 0.");
            return ;
        }

        offset = 60;
        for(i = 0; i < person_data_num; i++)
        {
            now_person_data = (person_data *)(recvbuf + offset);        /*������ת��Ϊnow_person_data�ṹ������*/

            sprintf(sql, "insert into data_from_terminal(serialnumber,terminal_id,bus_status,longitude,latitude,num_of_people,collect_time,receipttime)values('%s','%s','%c','%s','%s','%c','%s','%s')",
                    now_data.data_NO, now_data.terminal_ID, now_person_data.bus_work_status, now_person_data.longitude, now_person_data.latitude,
                    now_person_data.people_num, now_person_data.time_flag,/*����ʱ��*/);

            if(mysql_query(&my_connection,sql) != 0)                    //����һ����Ա��Ϣ
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

        /***�ش��ͻ���Ӧ�����ݷ�װ�ͷ���***/
        now_data.data_type = 0x31;
        now_data.data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    else                                                                  //���ǹ涨����������
    {
        printf("can't identified the data type\n");
        return ;
    }
}

void thread_func(void *arg)  /*�̺߳���*/
{
    struct ARG *info = NULL;
    info = (struct ARG *)arg;

    process_cli(info->connfd,info->client,info->my_connection);   /*�����̴߳�����*/
    free(arg);
    pthread_exit(NULL);
}

int Authentication(char terminal_id[], char Authentication_code[])   /*��֤�봦����*/
{
    int i, j;/*i,j����ѭ��*/
    char result[10];

    for(i =0, j = 6; i < 10; i++, j++)
    {
        result[i] = ~terminal_id[j];
    }

    if(strcmp(result,Authentication_code) != 0)return 0;  /*��֤�벻��ȷ*/
    else return 1;
}

void gettime(char time[7])
{
    char testtime[25];
    time_t now;
	time(&now);
	testtime = ctime(&t);
}
