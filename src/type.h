#ifndef _TYPE_H
#define _TYPE_H
#include "logLog.h"
typedef struct CYCDATA cycdata;
typedef struct LIBSODATA libsodata;
typedef enum TCS_TYPE_ENUM {
    TCS_OTHER_ERR = -10,
    TCS_MALLOC_NULL =-2,
    TCS_ARG_NULL = -1,
    TCS_OK =0,
    TCS_DATA_NULL =1

}tcs_type_enum;

//typedef struct CYCDATA cycdata;

//typedef struct FILEDATE filedate;


typedef struct LIBSODATA
{
    void *fdlib;
    char name[32];
    char wherename[64];

}libsodata;


typedef struct MYSTRING
{
    char *date;
    int len;

}mystring;

typedef struct ONEDATE
{
    mystring key;
    mystring value;
    struct ONEDATE *next;

}onedate;
typedef struct FILEDATE
{
    char name[32];
    onedate *date;
    struct FILEDATE *next;

}filedate;
struct CYCDATA
{
    onedate tidata;//循环时间
    void *para;//自定义数据结构
    //void *libdata;//库的句柄
    libsodata libdata;
    filedate *fdate;//文件读取信息
    int (*init)(cycdata*);//初始话
    int (*login)(cycdata*); //登录
    int (*seach)(cycdata*);//查询
    int (*howdo)(cycdata*);//数组处理
    int (*send)(cycdata*);//发送数据
    int (*deldate)(cycdata*);//清空
    logdata *ld;//日志系统
    cycdata *next;
};



typedef struct RABBITDATA
{
    int port;
    mystring host;
    mystring exchan;
    mystring queueNa;
    mystring name;
    mystring passwd;
    void *handle;
}rabbitdata;
typedef struct MYSQLDATA
{
    int port;
    mystring host;//位置
    mystring user;//数据库
    mystring passwd;
    mystring db;//数据库名
    mystring tablename;
    void *handle;//链接句柄
    void *resData;//放结果的
}mysqldata;

typedef cycdata* (*CYCMK)(filedate*);
typedef int (*SYSMETHOD)(cycdata*);

#endif
