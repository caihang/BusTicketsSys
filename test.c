/*
 ============================================================================
 Name        : XCC.c
 Author      : xcc
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <pthread.h>
#include "log.h"
#include "tpool.h"

log_t *log;   /*����ȫ����־�ļ����*/
/*����*/
void thread(void *arg)
{
    char *ptr = (char *)arg;
    sleep(1);
    printf("hello world! %s\n",ptr);
}

int main(int argc, char *argv[])
{
    tpool_t *pool;   /*�̳߳�ָ��*/
    /*������¼�ļ�*/
    log = log_open("test.log",0);
    /*����һ����100�������̣߳����200������е��̳߳�*/
    pool=tpool_init(100,200,1);
    int i;
    /*������¼�ļ�*/
    /*���100������*/
    for(i = 0; i < 100; i++)
        tpool_add_work(pool,thread,"test!");
    sleep(10);
    /*��ֹ�̳߳�*/
    tpool_destroy(pool,1);
    /*�رռ�¼�ļ�*/
    log_close(log);
    pthread_exit(NULL);
}
