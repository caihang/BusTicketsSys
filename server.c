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
log_t *log;    /*����ȫ����־�ļ����*/
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
    char start_symbol[16];   /*������*/
    char data_NO[4];        /*���ݰ���ˮ��*/
    char time_flag[7];      /*ʱ���ǩ*/
    char client_ID[20];     /*ǰ�˻���ʶ��*/
    char Authentication_codes[10];  /*��֤��*/
    char data_type;  /*��������*/
}data;

typedef struct GPS_DATA   //gps��Ϣ
{
    char longitude[4];   //����
    char latitude[4];    //γ��
    char time_flag[7];   //ʱ��
}gps_data;

typedef struct PERSON_DATA //��Ա��Ϣ
{
    char bus_work_status; //��������״̬
    char people_num;  //��Ա����
    char longitude[4];  //����
    char latitude[4];  //γ��
    char time_flag[7];  //ʱ���ǩ
}person_data;
#pragma (pop)

int main()
{
    tpool_t *pool;  /*�̳߳�ָ��*/
    log = log_open("server.log",0);   /*������¼�ļ�*/
    pool = tpool_init(200,300,1);  /*����һ����200�������̣߳����300��������е��̳߳�*/
	int listenfd, connectfd;
	struct ARG *arg;   //��Ҫ���ݵĲ���
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
        tpool_add_work(pool,thread_func,(void *)arg);   //����һ�������߳�,arg��������
	}
	close(listenfd);
}

void process_cli(int connfd, struct sockaddr_in client)
{
    MYSQL my_connection;
    int i,num = 0, rtn, offset, res;   //i����ѭ���±꣬num�����ж������Ƿ�������,rtn�ǽ��ܺ����ķ���ֵ��offset���������ݴ���ʱ��ƫ����,res����my_query����ֵ
    int gps_data_num, person_data_num;  //gps��Ϣ������������Ϣ����
    data *now_data;   //���ڴ���յ������ݰ��г����ݳ��Ⱥ����������������������
    gps_data *now_gps_data;   //���ڴ�Ž��յ���gps�����е�ÿһ�����ݵ�Ԫ
    person_data *now_person_data;  //���ڴ�Ž��յ�����Ա��Ϣ�е�ÿһ�����ݵ�Ԫ
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
            num = (*(int *)(recvbuf + 58));   //��ȡ���ݳ���
            printf("actually should recevied %d bytes\n",num);
            if(rtn != (num + 58))   //�ж��Ƿ�������
            {
                num = num + (int)rtn;
                continue;   //��ǰ��������ѭ��
            }
        }
    }while(rtn != (num + 58));     //�жϽ��ܵ��������Ƿ�����
    now_data = (data *)recvbuf;    //ȡ��ǰ��һ������Ϣ
    if(strcmp(now_data.start_symbol,"AAAAAAAAAAAAABBB") != 0) //����������������������
    {
        printf("Recvied a invalid data.\n");
        return;  //��������
    }
    mysql_init(&my_connection);  /*��ʼ��,my_connection��һ�����Ӿ��������ʹ�����ַ�ʽ�����ǲ���ָ����շ��ؾ��*/
    if(mysql_real_connect(&my_connection, "localhost","caihang", "admincai", "bus_info", 0, NULL, 0)) /*����BusInfo���ݿ⣬���ݿ����ֽ���ʹ��Сд������ƽ̨��ֲ*/
    {
        printf("Connection success��\n");
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
    if(now_data.Authentication_codes)  /*�ж���֤���Ƿ���ȷ����ѯ���ݿ������е���֤����бȶԣ���û�ҵ��������ݰ�*/
    {

    }
    if(now_data.data_type == 0x10);  /*�����GPS��Ϣ*/
    {
        gps_data_num = (*(int *)recvbuf + 59);    //��ȡ����������gps���ݵ����ݳ���
        if(gps_data_num <= 0)    //������ݳ���С��0�������ô�������
        {
            lprintf(log, INFO, "recvied gps data's length less than 0.");
            return;
        }
        offset = 60;
        for(i = 0; i < gps_data_num; i++)
        {
            now_gps_data = (gps_data *)(recvbuf + offset);
            /*����һ����¼�������ݿ�*/
            /************/


            /************/
            offset+=15;
        }
        /***�ش�Ӧ�����ݷ�װ�ͷ���***/
        now_data.data_type = 0x11;
    }
    else if(now_data.data_type == 0x20)  //�������Ա��Ϣ
    {
        person_data_num = (*(int *)recvbuf + 59);  //��ȡ���յ�����Ա��Ϣ���ݳ���
        if(person_data_num <= 0)  //������ݳ���С��0��������
        {
            lprintf(log, INFO, "recvied person data's length is less than 0.");
            return ;
        }
        offset = 60;
        for(i = 0; i < person_data_num; i++)
        {
            now_person_data = (person_data *)(recvbuf + offset);
            /*��������¼�������ݿ���Ӧ����*/
            /************/


            /************/
            offset += 17;
        }
        /***�ش�Ӧ�����ݷ�װ�ͷ���***/
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
