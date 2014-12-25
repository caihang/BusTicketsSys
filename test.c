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

log_t *log;   /*进程全局日志文件句柄*/
/*任务*/
void thread(void *arg)
{
    char *ptr = (char *)arg;
    sleep(1);
    printf("hello world! %s\n",ptr);
}

int main(int argc, char *argv[])
{
    tpool_t *pool;   /*线程池指针*/
    /*开启记录文件*/
    log = log_open("test.log",0);
    /*创建一个有100个工作线程，最大200任务队列的线程池*/
    pool=tpool_init(100,200,1);
    int i;
    /*开启记录文件*/
    /*添加100个任务*/
    for(i = 0; i < 100; i++)
        tpool_add_work(pool,thread,"test!");
    sleep(10);
    /*终止线程池*/
    tpool_destroy(pool,1);
    /*关闭记录文件*/
    log_close(log);
    pthread_exit(NULL);
}
