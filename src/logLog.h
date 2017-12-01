#ifndef _LOGLOG_H
#define _LOGLOG_H
#include <stdio.h>
#define LOGERR 1
#define LOGINF 2
#define LOGWAR 3
#define logLog(ld,type,args...) logInf(ld,__FILE__,__LINE__,type,##args)
typedef struct LOGDATA
{
    FILE *fg;
    char *filename;
    int maxsize;

}logdata;


int logInf(logdata* ld,char* file,int line,int type,const char *format,...);//写日志函数

logdata* logInit(const char *,int);//int 为Kb

int logDel(logdata*);

#endif
