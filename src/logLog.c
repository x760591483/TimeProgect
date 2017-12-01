#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "logLog.h"

int isneedcp(const char* path,int size);
void charChange(char *dat,char oldC,char newC);

logdata* logInit(const char *path,int size)
{
    if(path ==NULL)
    {
        return NULL;
    }
    FILE *logfg = fopen(path,"a");
    if(logfg ==NULL)
    {
        return NULL;
    }
    logdata *ld = (logdata*)malloc(sizeof(logdata));
    if(ld ==NULL)
    {
        fclose(logfg);
        return NULL;
    }
    memset(ld,0,sizeof(logdata));
    ld->filename =(char*)malloc(sizeof(char)*strlen(path)+1);
    if(ld->filename ==NULL)
    {
        fclose(logfg);
        free(ld);
        return NULL;
    }
    memset(ld->filename,0,strlen(path)+1);
    ld->fg = logfg;
    strncpy(ld->filename,path,strlen(path));
    if(size >1024)//限制每个文件的最大值
    {
        size =1024;
    }
    ld->maxsize = size*1024;

    return ld;
}

int logDel(logdata *ld)
{
    if(ld ==NULL)
    {
        return -1;
    }
    if(ld->fg != NULL)
    {
        fclose(ld->fg);
        ld->fg =NULL;
    }
    if(ld->filename != NULL)
    {
        free(ld->filename);
        ld->filename =NULL;
    }
    free(ld);

    return 0;
}

int logInf(logdata *ld,char* file,int line,int type,const char *format,...)
{
    if(ld ==NULL || ld->fg ==NULL)
    {
        return -1;
    }

    char buff[1024] ={0};
    char temchar[1024]={0};
    int tennum =0;
    va_list ap;
    va_start(ap,format);
    time_t ti = time(NULL);
    char *tim = asctime(localtime(&ti));//将时间转成字符串
    tennum= strlen(tim);
    buff[0] = '[';   
    strcat(buff,tim);//时间
    buff[tennum] =']';//去除换行

    if(type == LOGERR)
    {
        strcat(buff," [ERROR]");
    }
    else if(type == LOGINF)
    {
        strcat(buff," [INF]  ");
    }
    else if(type == LOGWAR)
    {
        strcat(buff," [WARN] ");

    }
    sprintf(temchar," [%s:%d] ",file,line);
    strcat(buff,temchar);

    memset(temchar,0,sizeof(temchar));


    vsprintf(temchar,format,ap);
    strcat(buff,temchar);
    strcat(buff,"\n");

    va_end(ap);
    if( isneedcp(ld->filename,ld->maxsize)==1)
    {
        memset(temchar,0,sizeof(temchar));
        sprintf(temchar,"%s_%s.log",ld->filename,tim);

        tennum = strlen(temchar)-5;
        temchar[tennum]='_';
        charChange(temchar,' ','_');
        fclose(ld->fg);
        ld->fg =NULL;
        rename(ld->filename,temchar);

        ld->fg = fopen(ld->filename,"a");
        if(ld->fg == NULL)
        {
            logDel(ld);
            return -2;
        }

    }
    fwrite(buff,strlen(buff),1,ld->fg);
    fflush(ld->fg);
    return 0;
}


//--------------------内部函数-------------
void charChange(char *dat,char oldC,char newC)
{
    if(dat ==NULL)
    {
        return;
    }
    int tem=0;
    while(*(dat+tem)!=0)
    {
        if(*(dat+tem)==oldC)
        {
            *(dat+tem)=newC;
        }
        ++tem;
    }

}
int isneedcp(const char* path,int size)
{
    if(path ==NULL || size <1024)
    {
        return -1;
    }

    struct stat myst;
    int ret = stat(path,&myst);
    if(ret !=0)
    {
        return -1;
    }

    int filesize = (int)myst.st_size;
    if(filesize >= size-1024)
    {//超出范围
        return 1;
    }

    return 0;
}

//int main(int argc, char *argv[])
//{
//    logdata *ld = logInit("./log.log",2);
//    int i=0;
//    for(;i<20;++i)
//    {
//        logLog(ld,LOGERR,"%d:%s",i,"xxxxxxxxxxaa");
//        logLog(ld,LOGINF,"%d %s",199,"fadfafaf");
//        logLog(ld,LOGWAR,"%d %s",199,"fadfaffadfadfaaf");
//    }
//        logDel(ld);
//    return 0;
//}
