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
void process_cli(int connfd, struct sockaddr_in client);
void savadata_r(char *recvbuf, int len, char *cli_data);
void thread_func(void *arg);
void getthetime(char hyy[12]);
int Authentication(char terminal_id[], char Authentication_code[]);

struct ARG                          /*�̺߳��������������*/
{
    int connfd;                     /*�����׽���*/
    struct sockaddr_in client;      /*�ͻ����׽��ֵ�ַ*/
};

#pragma (push,1)
typedef struct DATA
{
    char start_symbol[16];          /*������*/
    int data_NO;                    /*���ݰ���ˮ��*/
    char time_flag[12];             /*ʱ���ǩ*/
    char terminal_ID[20];           /*ǰ�˻���ʶ��*/
    char schedule_num[10];          /*��κ�*/
    char Authentication_codes[10];  /*��֤��*/
    char data_type;                 /*��������*/
}data;

typedef struct VIDEO_SUMMARY_DATA   /*��ƵժҪ��Ϣ*/
{
    char video_file_name[50];       /*�ļ���*/
    char start_time[12];             /*��ʼʱ��*/
    char end_time[12];               /*����ʱ��*/
}video_summary_data;

typedef struct PERSON_DATA          /*��Ա��Ϣ*/
{
    char bus_work_status;           /*��������״̬*/
    char people_num;                /*��Ա����*/
    char longitude[4];              /*����*/
    char latitude[4];               /*γ��*/
    char time_flag[12];               /*ʱ���ǩ*/
}person_data;
#pragma (pop)

int main()
{
    tpool_t *pool = NULL;                /*�̳߳�ָ��*/
    log = log_open("server.log",0);      /*������¼�ļ�*/
    pool = tpool_init(200,300,1);        /*����һ����200�������̣߳����300��������е��̳߳�*/

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
        tpool_add_work(pool,thread_func,(void *)arg);   //����һ�������߳�,arg��������
	}
	close(listenfd);
}

void process_cli(int connfd, struct sockaddr_in client)
{
    MYSQL  my_connection;                               //���ݿ����Ӿ��
    int i, rtn, offset, res, num = 0;                   //i����ѭ���±꣬num�����ж������Ƿ�������,rtn�ǽ��ܺ����ķ���ֵ��offset���������ݴ���ʱ��ƫ����,res����my_query����ֵ
    int video_data_num, person_data_num;                //��ƵժҪ��Ϣ������������Ϣ����

    char ftpserverIPaddr[4];                            //ftpserverIPaddr�����ݴ�ftp������IP��ַ
    char recvbuf[MAXDATASIZE];                          //�������ݻ�����
    char sql[400] = {0};                                //sql���ָ��
    char receipttime[12];                               //������ݽ���ʱ��

    data *now_data = NULL;                              //���ڴ���յ������ݰ��г����ݳ��Ⱥ����������������������
    video_summary_data *now_video_data = NULL;          //���ڴ�Ž��յ���gps�����е�ÿһ�����ݵ�Ԫ
    person_data *now_person_data = NULL;                //���ڴ�Ž��յ�����Ա��Ϣ�е�ÿһ�����ݵ�Ԫ

    lprintf(log, INFO, "You got a cnnection from %s.\n",inet_ntoa(client.sin_addr));

    do                                                  //��ʼ�������ݣ���ѭ������ֱ���������
    {
        if((rtn = recv(connfd, (recvbuf + num), MAXDATASIZE, 0)) == 0)
        {
            close(connfd);
            break;
        }
        else
        {
            lprintf(log, INFO, "Recevied %d bytes\n",(rtn + num));
                                                        //��¼ʵ�ʽ��յ����ֽ���
            num = (*(int *)(recvbuf + 73));             //��ȡ���ݳ���
            lprintf(log, INFO, "actually should recevied %d bytes\n",num);
                                                        //��¼ԭ��Ӧ�ý��յ����ֽ���
            if(rtn != (num + 77))                       //�ж��Ƿ�������
            {
                num = num + rtn;                        //num��Ϊ���������±���ƶ�
                continue;                               //δ�����������ǰ��������ѭ��,��������
            }
        }
    }while(rtn != (num + 77));                          //ѭ������

    getthetime(receipttime);                            //��ȡ�������ݵ�ʱ��

    now_data = (data *)recvbuf;                         //ȡ��ǰ��һ������Ϣ

    if(strcmp((*now_data).start_symbol,"AAAAAAAAAAAAABB") != 0)
                                                        //����������������������
    {
        lprintf(log, FATAL, "Recvied a invalid data.\n");
        return;                                         //��������
    }
    lprintf(log, INFO, "The start_symbol is %s\n",(*now_data).start_symbol);

    while(1)                            /*�������ݿ�*/
    {
        mysql_init(&my_connection);     /*��ʼ��,my_connection��һ�����Ӿ��������ʹ�����ַ�ʽ�����ǲ���ָ����շ��ؾ��*/
        if(mysql_real_connect(&my_connection, "localhost","caihang", "admincai", "businfo", 0, NULL, 0))
                                        /*����businfo���ݿ⣬���ݿ����ֽ���ʹ��Сд������ƽ̨��ֲ*/
        {
            lprintf(log, INFO, "Connection success\n");
            break;
        }
        else
        {
            lprintf(log, FATAL, "connect database faild,connect again...\n");
                                        /*��¼�������ݿ�ʧ�ܣ�������������*/
        }
    }

    /*�˴���ѯschedule�Ű���ó��Ƿ񷢳�����δ�����򲻴�������Ҫ�޸�*/
    sprintf(sql, "select * from schedule where schedule_num='%s'",(*now_data).schedule_num);
    lprintf(log, INFO, "The s_num is %s\n",(*now_data).schedule_num);

    if(res = mysql_query(&my_connection, sql) != 0)
                                                        //�����ݿ�schedule�����յ��İ�κ��Ƿ񷢳�
    {
        lprintf(log, FATAL, "schedule_num can not found\n");
        mysql_close(&my_connection);
        return;
    }

    sprintf(sql, "select * from bus_base_info where terminal_id='%s'",(*now_data).terminal_ID);
    lprintf(log, INFO, "The TID is %s\n",(*now_data).terminal_ID);

    if(res = mysql_query(&my_connection, sql) != 0)
                                                        //�����ݿ�bus_base_info�����յ���ǰ�˻�ID�Ƿ����
    {
        lprintf(log, FATAL, "TerminalID can not found\n");
        mysql_close(&my_connection);
        return;
    }

    if(rtn = Authentication((*now_data).terminal_ID, (*now_data).Authentication_codes) == 0)
                                                        //�����֤�����
    {
        lprintf(log, FATAL, "Authentication_code wrong!\n");         //����ʾ������
        mysql_close(&my_connection);
        return;
    }

    /*�������󣬿�ʼ����*/
    if((*now_data).data_type == 0x20);                     //���������������ƵժҪ��Ϣ
    {
        memcpy(ftpserverIPaddr, recvbuf+77,4);          //��ftp������ip��ַ����
        video_data_num = (*(int *)recvbuf + 81);        //��ȡ����������gps���ݵ���������

        if(video_data_num <= 0)                         //������ݳ���С��0�������ô������ݣ�����¼��־
        {
            lprintf(log, FATAL, "recvied video summary data's length less than 0.\n");
            mysql_close(&my_connection);
            return;
        }

        offset = 85;
        for(i = 0; i < video_data_num; i++)
        {
            now_video_data = (video_summary_data *)(recvbuf + offset);    //������ת��Ϊvideo�ṹ��

            sprintf(sql, "insert into video_summary_info(serialnumber,terminal_id,FTPserverIPaddr,video_name,starttime,endtime,receipttime)values('%d','%s','%s','%s','%s','%s','%s')",
                    (*now_data).data_NO, (*now_data).terminal_ID, ftpserverIPaddr, (*now_video_data).video_file_name, (*now_video_data).start_time,
                    (*now_video_data).end_time,receipttime);                //���Ӳ�ѯ���

            if(mysql_query(&my_connection,sql) != 0)                      //����һ����ƵժҪ��Ϣ
            {
                lprintf(log, FATAL, "insert video_summary_data failed\n");
                mysql_close(&my_connection);
                return;
            }
            else
            {
                lprintf(log, INFO, "insert video_summary_data sucess\n");
            }
            offset+=64;                                                   //ƫ�����ƶ�64���ֽ��Ի�ȡ��һ����¼
        }

        /***�ش��ͻ���Ӧ�����ݷ�װ�ͷ���***/
        (*now_data).data_type = 0x21;
        (*now_data).data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    if((*now_data).data_type == 0x30)                                 //���������������Ա��Ϣ
    {
        person_data_num = (*(int *)recvbuf + 77);                       //��ȡ���յ�����Ա��Ϣ��������

        if(person_data_num <= 0)                                        //������ݳ���С��0��������
        {
            lprintf(log, FATAL, "recvied person data's length is less than 0.\n");
            mysql_close(&my_connection);
            return ;
        }

        offset = 81;
        for(i = 0; i < person_data_num; i++)
        {
            now_person_data = (person_data *)(recvbuf + offset);        /*������ת��Ϊnow_person_data�ṹ������*/

            sprintf(sql, "insert into data_from_terminal(serialnumber,terminal_id,bus_status,longitude,latitude,num_of_people,collect_time,receipttime)values('%d','%s','%c','%s','%s','%c','%s','%s')",
                    (*now_data).data_NO, (*now_data).terminal_ID, (*now_person_data).bus_work_status, (*now_person_data).longitude, (*now_person_data).latitude,
                    (*now_person_data).people_num, (*now_person_data).time_flag,receipttime);

            if(mysql_query(&my_connection,sql) != 0)                    //����һ����Ա��Ϣ
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

        /***�ش��ͻ���Ӧ�����ݷ�װ�ͷ���***/
        (*now_data).data_type = 0x31;
        (*now_data).data_NO++;
        send(connfd,(char *)now_data,sizeof(data),0);
    }
    else                                                                    //���ǹ涨����������
    {
        lprintf(log, FATAL, "can't identified the data type\n");
        mysql_close(&my_connection);
        return ;
    }
    mysql_close(&my_connection);                                            //�ر����ݿ������
}

void thread_func(void *arg)  /*�̺߳���*/
{
    struct ARG *info = NULL;
    info = (struct ARG *)arg;

    process_cli(info->connfd,info->client);                         /*�����̴߳�����*/
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
