#include <stdio.h>            
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include "type.h"
#include <dirent.h>
#include <sys/types.h>
#include <dirent.h>
#include <dlfcn.h>
//#include "method.h"
#include <sys/stat.h>

const char *LIBDIRCHAR = "../libdir";

//const int nameCharNumMax = 32;
#define  nameCharNumMax  32

//logdata *mainld;
cycdata *cychead;//
cycdata *cycheadnext;
char *shead;
logdata *mainld;
filedate *filehead;


int TimeDo(cycdata* cyctem)
{
    if(cyctem ==NULL)
    {
        return TCS_ARG_NULL;

    }

    int ret=0;
    if(cyctem->seach != NULL)
    {

        ret = cyctem->seach(cyctem);
        if(ret < 0)
        {      
            logLog(mainld,LOGWAR,"%s seach %d",cyctem->libdata.name,ret);
            return ret;

        }

    }
    if(cyctem->howdo != NULL)
    {
        ret = cyctem->howdo(cyctem);
        if(ret < 0)
        {
            logLog(mainld,LOGWAR,"%s howdo %d",cyctem->libdata.name,ret);
            return ret;

        }

    }
    if(cyctem->send != NULL)
    {
        ret = cyctem->send(cyctem);
        if(ret < 0)
        {
            logLog(mainld,LOGWAR,"%s send %d",cyctem->libdata.name,ret);
            return ret;

        }                                                                                                                                                                                      

    }

    return 0;


}



void cycleTimeMethod(int num)
{
    //   printf("cycleTimeMethod is start\n");
    cycdata *cyctem = cychead;

    if(cyctem ==NULL)
    {
        return;

    }

    while(cyctem !=NULL)
    {
        if(cyctem->tidata.key.len >= 0)
        {
            ++(cyctem->tidata.value.len);
            if(cyctem->tidata.key.len < cyctem->tidata.value.len)
            {
                cyctem->tidata.value.len =0;
                TimeDo(cyctem);
            }
        }
        cyctem = cyctem->next;
    }
}
int TimeDel(cycdata *cyctem)
{
    if(cyctem ==NULL)
    {
        return -1;

    }

    if(cyctem->deldate != NULL)
    {
        cyctem->deldate(cyctem);//执行每个模块的释放函数
    }
    return 0;
}

int freeFiledate(filedate *dat)
{
   // printf("freeFiledate is start\n");
    if(dat ==NULL)
    {
        return 0;
    }
    filedate *fileAA = dat;
    onedate *onehead = NULL;
    while(fileAA != NULL)
    {
        if(fileAA->date != NULL)
        {
            onehead = fileAA->date;
            onedate *oneAA = onehead;
            while(oneAA != NULL)
            {
                onehead = onehead->next;
                free(oneAA);
                oneAA = onehead;
            }
        }
        dat = dat ->next;
        free(fileAA);
        fileAA = dat;

    }                                                                                   
    return 0;
}

int freeCycdata(cycdata *cdata)
{   //释放单个cycdata
    if(cdata == NULL)
    {
        return -1;
    }
    cycdata *AA=cychead;
    cycdata *BB =NULL;
    while(AA !=NULL)
    {
        if(AA == cdata)
        {
            if(BB == NULL)
            {
                cychead = cychead->next;
            }
            else
            {
                BB->next = AA->next;
            }
            cdata->fdate =NULL;
            if(cdata->ld !=NULL && cdata->ld != mainld)
            {
                logDel(cdata->ld);
            }
            cdata->ld =NULL;
            //TimeDel(cdata);

            if(cdata->libdata.fdlib)
            {
                dlclose(cdata->libdata.fdlib);
                cdata->libdata.fdlib =NULL;
            }

            cdata->next =NULL;
            free(cdata);

            return 0;
        }
        BB =AA;
        AA = AA ->next;


    }



    return -2;
}

void gameover(int num)
{
   // printf("gameover is start\n");
    logLog(mainld,LOGWAR,"start go gameover");
    cycdata *cyctem = cychead;
    if(cyctem == NULL)
    {
        return ;
    }
    cycdata *tem = cyctem;
    while(cyctem != NULL)
    {
        cyctem->fdate =NULL;
        if(cyctem->ld !=NULL && cyctem->ld != mainld)
        {
            logDel(cyctem->ld);
        }
        cyctem->ld =NULL;
        TimeDel(cyctem);

        if(cyctem->libdata.fdlib)
        {
            dlclose(cyctem->libdata.fdlib);
            cyctem->libdata.fdlib =NULL;
        }

        tem = cyctem ->next;
        free(cyctem);
        cyctem = tem;
    }
    cychead =NULL;
    cycheadnext =NULL;

    if(shead !=NULL)
    {
        free(shead);
        shead =NULL;

    }
    if(filehead !=NULL)
    {
        freeFiledate(filehead);
        filehead =NULL;
    }
    logDel(mainld);
    mainld=NULL;
    exit(0);
}





void trimspace(char *dat,int num)
{
    if(dat == NULL || num <1)
    {
        return;
    }
    int len=(int)strlen(dat);
    if(len>num)
    {len=num;}

    int i=len-1;
    for(;i>=0;--i)
    {//清除尾部的 空格 回车 换行
        if(*(dat+i)==13 || *(dat+i)==10 || *(dat+i)==32)
        {
            *(dat+i)=0;

        }
        else
        {
            break;

        }                                                                                           
    }
    if(i<=0 || *(dat) != 32)
    {
        return;
    }
    int j=0;
    for(;;)//表示字符串开始有空格 且字符串长度大于1
    {
        if(*(dat + j) != 32)
        {
            break;
        }
        ++j;
    }
    int k=0;
    for(;j<=i;++k,++j)
    {
        *(dat + k)= *(dat + j);

    }
    for(;k<=i;++k)
    {
        *(dat + k)=0;
    }
}

void* loadFile(char const *path)
{
    if(path == NULL)
    {
        return NULL;
    }
    FILE *fg = fopen(path,"r");
    if(fg==NULL)
    {
        // logLog(mainld,LOGERR,"readFile fopen is err\n");
        return NULL;
    }
    int iswrite = 0;
    char chartem[1024]={0};
    int maxsize = sizeof(char) * 1024;
    int wherenow = 0;
    char *head = (char*)malloc(maxsize);
    if(head == NULL)
    {
        // logLog(mainld,LOGERR,"loadFile malloc err\n");
        return NULL;
    }
    memset(head,0,maxsize);
    //    printf("11111\n");
    while(1)
    {
        memset(chartem,0,sizeof(chartem));
        fgets(chartem,sizeof(chartem)-1,fg);
        //        printf("fget:%s\n",chartem);
        if(feof(fg))
        {   
            break;                                
        }  
        trimspace(chartem,1024);
        //        printf("trim:%s\n",chartem);
        if(chartem[0] == '/' && chartem[1] == '/')
        {//跳过注释行
            continue;
        }
        int temlen = strlen(chartem);
        if(temlen == 0)
        {
            //            printf("-----\n");
            if(iswrite)
            {
                iswrite =0;
                temlen = 1;
                chartem[0] = 32;
            }
            else
            {
                continue;
            }
        }
        else
        {
            iswrite =1;
        }

        if(temlen >= maxsize - wherenow -1 )
        {//表示剩余空间不足
            //            printf("malloc less\n");
            maxsize+=sizeof(char)*1024;
            char *headTem = (char*)malloc(maxsize);
            if(headTem == NULL)
            {
                free(head);
                return NULL;
            }
            memset(headTem,0,maxsize);

            memcpy(headTem,head,wherenow);
            free(head);
            head=headTem;
        }
        strncpy(head + wherenow,chartem,temlen);
        wherenow =wherenow + temlen + 1;//多加一 为字符串后0
    }
    if(wherenow ==0)
    {
        free(head);
        return NULL;
    }

    return head;
}
onedate* fetchInf(char *date)
{//传入的字符串必须是  ***:***** 的格式
    if(date ==NULL)
    {
        return NULL;
    }
    int tt =0;
    for(;;++tt)
    {
        if(*(date + tt)==0)
        {//不符合格式
            return NULL;
        }
        if(*(date + tt)== ':')
        {//找到 ：
            break;
        }
    }
    if(tt == 0 || *(date + tt +1)==0)
    {
        return NULL;
    }
    onedate *res =(onedate*)malloc(sizeof(onedate));
    memset(res,0,sizeof(onedate));


    res->key.date=date;
    res->key.len = tt;

    res->value.date=date + tt +1;
    res->value.len=strlen(date + tt +1);
    return res;
}



int readFile(void *dat,filedate **res)
{
    if(dat ==NULL || res ==NULL)
    {
        return -1;
    }
    char *head = (char*)dat;
    filedate filefirst;
    memset(&filefirst,0,sizeof(filedate));
    filedate *filetem = &filefirst;

    onedate onehead;
    memset(&onehead,0,sizeof(onedate));
    onedate *onetem = &onehead;
    int wherenow =0;
    int isnest =1;//表示是否重新一个结构体
    while(1)
    {
        if(*(head+wherenow)==0)
        {
            break;
        }
        else if(*(head+wherenow)== 32)
        {//一个结构体结束了
            filetem->date = onehead.next;

            isnest =1;
            wherenow = wherenow + 2;//跳过空格 和 0
            continue;
        }
        if(isnest)
        {//创建消息体 并赋值消息体的名字
            isnest =0;
            filetem->next =  (filedate*)malloc(sizeof(filedate));
            memset(filetem->next,0,sizeof(filedate));
            filetem = filetem->next;
            strncpy(filetem->name,head + wherenow,sizeof(filetem->name)-1);
            wherenow = wherenow + strlen(head + wherenow)+1;
            memset(&onehead,0,sizeof(onedate));
            onetem = &onehead;
            continue;
        }
        onetem->next = fetchInf(head + wherenow);
        if(onetem->next==NULL)
        {
            //       logLog(mainld,LOGERR,"fetchInf is err,data:%s\n",head + wherenow);
            freeFiledate(filefirst.next);
            return -2;
        }
        onetem = onetem->next;
        wherenow = wherenow + strlen(head + wherenow) + 1;

    }
    filetem->date = onehead.next;

    *res = filefirst.next;
    return 0;

}
/*
   int palyFiledate(filedate *dat)
   {
   if(dat == NULL)
   {
   printf("palyFiledate dat is NULL\n");
   return -1;
   }

   filedate *filetem = dat;
   while(filetem !=NULL)
   {
   printf("%s\n",filetem->name);
   onedate *onetem = filetem->date;
   while(onetem != NULL)
   {
   printf("  key:");
   int i=0;
   for(;i<onetem->key.len;++i)
   {
   printf("%c",onetem->key.date[i]);
   }
   printf("\n");
   printf("  value:");
   for(i=0;i<onetem->value.len;++i)
   {
   printf("%c",onetem->value.date[i]);
   }
   printf("\n");

   onetem = onetem->next;
   }
   filetem = filetem->next;
   }
   return 0;
   }
   */
cycdata* cycmake(filedate* dat)   
{                               

    //    printf("cycmake-- start\n");
    cycdata *tem = (cycdata*)malloc(sizeof(cycdata));
    if(tem ==NULL)     
    {                       
        //        logLog(mainld,LOGERR,"malloc is get NULL");
        return NULL;                       
    }                                      
    memset(tem,0,sizeof(cycdata));         
    tem->fdate=dat;       

    if(cychead == NULL)
    {
        cychead = tem;
        cycheadnext = tem;
    }
    else
    {
        cycheadnext->next = tem;
        cycheadnext = tem;
    }
    return tem;                                  
}  


int palyFiledate(filedate *dat)
{
    if(dat == NULL)
    {
        printf("palyFiledate dat is NULL\n");
        return -1;

    }

    filedate *filetem = dat;
    while(filetem !=NULL)
    {
        printf("%s\n",filetem->name);
        onedate *onetem = filetem->date;
        printf("--play %p\n",onetem);
        while(onetem != NULL)
        {
            printf("  key:");
            int i=0;
            for(;i<onetem->key.len;++i)
            {
                printf("%c",onetem->key.date[i]);

            }
            printf("\n");
            printf("  value:");
            for(i=0;i<onetem->value.len;++i)
            {
                printf("%c",onetem->value.date[i]);

            }
            printf("\n");

            onetem = onetem->next;

        }
        filetem = filetem->next;                                                                                                                                       

    }
    return 0;

}




int filedatefind()
{
    return 0;
}
int strIsSame(const char *AA,const char *BB,int num,int nAa)//isAa为是否忽视大小写 1 忽视
{
    if(AA==NULL || BB ==NULL || num<1)
    {
        return -1;
    }
    int i=0;
    int Aanum =0;
    if(nAa)
    {//忽略大小写
        Aanum = (int)('a' - 'A');
        //       printf("----Aaunm %d\n",Aanum);
    }
    for(;i<num;++i)
    {
        if(*(AA + i)==0 || *(BB + i)==0)
        {
            //           printf("---- first is 0\n");
            break;
        }
        if((int)*(AA + i) == (int)*(BB + i))
        {
            continue;
        }
        if((int)*(AA+i)+ Aanum == (int)*(BB +i) || (int)*(AA+i) == (int)*(BB +i) + Aanum)
        {
            if((int)*(AA +i)<(int)'A' || (int)*(BB + i)<(int)'A' || (int)*(AA +i)>(int)'z' || (int)*(BB +i)>(int)'z')
            {
                return i+1;
            }
            continue;
        }
        //       printf("----this %d is not same\n",i+1);
        return i+1;
    }
    if(i==num)
    { //  printf("---- i=num %d\n",i);
        return 0;
    }
    if(*(AA +i)==0 && *(BB + i)==0)
    {
        //  printf("----all is 0  %d\n",i);
        return 0;
    }
    // printf("is not same %d\n",i+1);
    return i+1;//相同
}
int toSetCycdata(filedate *fhead, cycdata *chead)
{//将filedate 设置到 cycdata中
    int res=0;
    if(fhead == NULL || chead ==NULL)
    {
        logLog(mainld,LOGERR,"toSetCycdata fhead or chead is NULL");
        return -1;
    }
    filedate *filetem = fhead;
    cycdata *cyctem =NULL;
    while(filetem != NULL)
    {
        cyctem = chead;
        while(cyctem != NULL)
        {
            //      printf("--%s  %s \n",filetem->name,cyctem->libdata.name);
            if(strIsSame(filetem->name,cyctem->libdata.name,nameCharNumMax,1)==0)
            {
                //          printf("--OK\n");
                if(cyctem ->fdate != NULL)
                {//表示有重名
                    //printf("filedate has same name %s",filetem -> name);
                    logLog(mainld,LOGERR,"filedate has same name %s",filetem->name);
                    return TCS_OTHER_ERR;
                }
                cyctem ->fdate = filetem;
                cyctem ->ld = mainld;
                break;
            }
            cyctem = cyctem ->next;
        }
        if(cyctem == NULL)
        {//表示没匹配上 报错
            logLog(mainld,LOGERR,"%s not have libso to fit",filetem->name);
            //printf("%s not have libso to fit\n",filetem->name);
            return TCS_OTHER_ERR -1;
        }

        filetem = filetem->next;
    }
    cyctem = chead;
    while(cyctem != NULL)
    {
        if(cyctem ->fdate ==NULL)
        {
            //printf("%s is not use\n",cyctem->libdata.name);
            logLog(mainld,LOGERR,"%s is not use",cyctem->libdata.name);
            cycdata *cycnext = cyctem ->next;
            res = freeCycdata(cyctem);
            if(res)
            {

                logLog(mainld,LOGERR,"freeCycdata is err %d",res);
                return TCS_OTHER_ERR -2;
            }
            cyctem = cycnext;
            continue;
        }
        cyctem = cyctem->next;
    }

    return 0;
}

int loadLib(const char *dirch,CYCMK cycmk)
{
    if(dirch==NULL)
    {

        logLog(mainld,LOGERR,"loadLib--dirch is NULL");
        return TCS_ARG_NULL;
    }
    DIR *dirdat = opendir(dirch);
    if(dirdat ==NULL)
    {     
        logLog(mainld,LOGERR,"loadLib--opendir get NULL");
        return TCS_OTHER_ERR;
        //printf("opendir is get null\n");
    }
    struct dirent *dir =NULL;
    int libnumhas=0;
    while((dir=readdir(dirdat))!=NULL)
    {
        int dirfilenum = strlen(dir->d_name);
        if(dirfilenum<7 || dirfilenum > nameCharNumMax + 6 -1)
        {//文件长度不够
            //   printf("libname is to less or big %d ",dirfilenum);
            logLog(mainld,LOGWAR,"--libname is to less or big %d",dirfilenum);
            continue;
        }
        if(strncmp(dir->d_name,"lib",3)!=0 || strncmp(dir->d_name + dirfilenum - 3,".so",3)!=0)//文件必须开始是lib   结束为.so
        {
            logLog(mainld,LOGWAR,"--libname is not lib start or .so end %s",dir->d_name);
            continue;
        }
        char chaa[nameCharNumMax]={0};//名字
        char chbb[64]={0};//位置相对
        int chnum = dirfilenum - 6;//库文件中间名字长度
        void *dirhandletem=NULL;
        int res=0;

        cycdata *cyctem = NULL;
        SYSMETHOD inittem=NULL;
        SYSMETHOD logintem=NULL;
        SYSMETHOD seachtem=NULL;
        SYSMETHOD howdotem=NULL;
        SYSMETHOD sendtem=NULL;
        SYSMETHOD deldatetem=NULL;

        strncpy(chaa,dir->d_name+3,chnum);
        sprintf(chbb,"%s/%s",dirch,dir->d_name);            

        dirhandletem = dlopen(chbb,RTLD_LAZY);
        if(!dirhandletem)
        {
            //printf("dlopen is err:%s\n",dlerror());
            logLog(mainld,LOGERR,"--dlopen get NULL %s",dlerror());
            return TCS_OTHER_ERR-1;
        }
        dlerror();
        //   printf("dlerror %s\n",dlerror());

        res=0;
        *(void **)(&inittem) = dlsym(dirhandletem,"sysinit");
        if(!inittem)
        {
            ++res;
            //  printf("init is null\n");

            logLog(mainld,LOGWAR,"--dlsym sysinit null");
           // dlclose(dirhandletem);
           // return TCS_OTHER_ERR -2;
        }
        *(void **)(&logintem) = dlsym(dirhandletem,"syslogin");
        if(!logintem)
        {
            ++res;
            logLog(mainld,LOGWAR,"--dlsym syslogin null");
        //    dlclose(dirhandletem);
        //    return TCS_OTHER_ERR -2;
        }
        *(void **)(&seachtem) = dlsym(dirhandletem,"sysseach");
        if(!seachtem)
        {
            ++res;
            logLog(mainld,LOGWAR,"--dlsym sysseach null");
            //dlclose(dirhandletem);
            //return TCS_OTHER_ERR -2;

        }
        *(void **)(&howdotem) = dlsym(dirhandletem,"syshowdo");
        if(!howdotem)
        {
            ++res;
            logLog(mainld,LOGWAR,"--dlsym syshowdo null");
           // dlclose(dirhandletem);
           // return TCS_OTHER_ERR -2;

        }
        *(void **)(&sendtem) = dlsym(dirhandletem,"syssend");
        if(!sendtem)
        {
            ++res;
            logLog(mainld,LOGWAR,"--dlsym syssend null");
           // dlclose(dirhandletem);
           // return TCS_OTHER_ERR -2;
        }
        *(void **)(&deldatetem) = dlsym(dirhandletem,"sysdeldata");
        if(!deldatetem)
        {
            ++res;
            logLog(mainld,LOGERR,"--dlsym sysdeldata null");
          //  dlclose(dirhandletem);
          //  return TCS_OTHER_ERR -2;
        }
        

        //printf("loadLib--dir:%s\n",chaa);
        logLog(mainld,LOGINF,"--lib name is %s",chaa);
        if(res ==6)
        {
            logLog(mainld,LOGERR,"%s lib all method null",chaa);
            dlclose(dirhandletem);
            return TCS_OTHER_ERR-2;
        }

        cyctem = cycmk(NULL);
        if(!cyctem)
        { 
            logLog(mainld,LOGERR,"--cycmk get null");
            // printf("cycmk is err\n");
            return TCS_OTHER_ERR-3;
        }
        // printf("--loadLib cyctem %p\n\n",cyctem);

        cyctem->libdata.fdlib = dirhandletem;
        strncpy(cyctem->libdata.name,chaa,strlen(chaa));
        strncpy(cyctem->libdata.wherename,chbb,strlen(chbb));
        cyctem->init= inittem;
        cyctem->login= logintem;
        cyctem->seach= seachtem;
        cyctem->howdo= howdotem;
        cyctem->send= sendtem;
        cyctem->deldate= deldatetem;
        ++libnumhas;

    }
    if(libnumhas==0)
    {
        logLog(mainld,LOGERR,"%s not has avail lib",dirch);
        closedir(dirdat);
        return TCS_OTHER_ERR -4;
    }
    logLog(mainld,LOGINF,"%s has %d avail lib",dirch,libnumhas);
    closedir(dirdat);
    return 0;
}

int TimeInit(cycdata* cyctem)
{
    if(cyctem == NULL)
    { 
        logLog(mainld,LOGERR,"TimeInit arc cyctem is NULL");
        return -1;
    }
    int res=0;
    if(cyctem->init != NULL)
    { 
        res = cyctem->init(cyctem);
        if(res <0)
        {
            logLog(mainld,LOGERR,"%s sysinit err %d",cyctem->libdata.name,res);
            return TCS_OTHER_ERR; 
        }
    } 

    if(cyctem->login != NULL)
    { 
        res = cyctem->login(cyctem);
        if(res <0)
        {
            logLog(mainld,LOGERR,"%s syslogin err %d",cyctem->libdata.name,res);
            return TCS_OTHER_ERR -1; 
        }

    } 

    return 0;


}


int main(int argc, char *argv[])
{

    char *confwhere=NULL;
    char *libwhere=NULL;
    int res=1;
    if(argc >2)
    {   
        while(res <argc)
        {
            if(strncmp(argv[res],"-c",2)==0)
            {//为配置文件路径
                confwhere = argv[res + 1];
                res +=2;
                continue;
            }
            if(strncmp(argv[res],"-l",2)==0)
            {
                libwhere = argv[res + 1];
                res +=2;
                continue;
            }
            printf("%s is no avail\n",argv[res]);
            return -1;
        
        }
           
    }


    printf("%d %s %s\n",argc,confwhere,libwhere);

    cychead =NULL;
    cycheadnext = NULL;
    filehead =NULL;
    mainld =NULL;

    pid_t fd;


    if(NULL==opendir("./logs"))
    {
        printf("mkdir logs\n");
        mkdir("./logs",0770);
    }


    mainld = logInit("./logs/main.log",100);     //通过捕获信号的 和 定时器定时周期发送信号
    if(mainld == NULL)
    {
        printf("logInit err\n");
        return 0;
    }
    fd = fork();

    if(fd ==0)
    {
        pid_t pid = setsid();
        if(pid == -1)
        {
            logLog(mainld,LOGERR,"setsid is err");
            return -1;
        }
        int fdnull =open("/dev/null",O_RDWR);
        if(fdnull <3)
        {
            logLog(mainld,LOGERR,"main open() err %d",fdnull);
        }
        dup2(fdnull,0);
        dup2(fdnull,1);
        




        struct sigaction sig;
        sig.sa_handler = cycleTimeMethod;
        sig.sa_flags=0;
        sigemptyset(&sig.sa_mask);//清空相应期间的阻塞信号参数
        sigaction(SIGALRM,&sig,NULL);

        sig.sa_handler = gameover;
        sigaction(SIGUSR1,&sig,NULL);

        //printf("time is set\n");
        //设置定时器
        struct itimerval mytime;
        mytime.it_interval.tv_sec=1;//循环时间
        mytime.it_interval.tv_usec=0;
        mytime.it_value.tv_sec=1;//开始到第一次时间
        mytime.it_value.tv_usec=0;
        setitimer(ITIMER_REAL,&mytime,NULL);
        if(confwhere)
        {
            logLog(mainld,LOGINF,"conf file is %s",confwhere);
            shead =(char*)loadFile(confwhere);

        }
        else
        {
            shead =(char*)loadFile("../conf/xxh.conf");
        }
        if(shead == NULL)
        {
            //printf("shead is NULL\n");
            logLog(mainld,LOGERR,"loadFile is err");

            goto END;
        }


        filehead = NULL;
        int res = readFile(shead,&filehead);
        if(res !=0)
        {
            // printf("readFile is != 0\n");
            logLog(mainld,LOGERR,"readFile is err");
            goto END;
        }
        logLog(mainld,LOGINF,"readFile is ok");

        //  palyFiledate(
        if(libwhere)
        {
            logLog(mainld,LOGINF,"libdir  is %s",libwhere);
            res = loadLib(libwhere,cycmake);

        }
        else
        {
            res = loadLib(LIBDIRCHAR,cycmake);

        }
        if(res <0)
        {
            // printf("loadLib is err %d\n",res);
            logLog(mainld,LOGERR,"loadLib is err %d",res);
            goto END;
        }
        logLog(mainld,LOGINF,"loadLib is ok");

        res = toSetCycdata(filehead,cychead);
        //    palyFiledate(filehead);
        if(res <0)
        {
            //printf("toSetCycdata is err %d\n",res);
            logLog(mainld,LOGERR,"toSetCycdata is err %d",res);
            goto END;
        }
        logLog(mainld,LOGINF,"toSetCycdata is ok");

        cycdata *cyctem = cychead;  
        logLog(mainld,LOGINF,"--start TimeInit");
        while(cyctem != NULL)
        {
            //  printf("----main init cyctem %p\n",cyctem);
            res=TimeInit(cyctem);
            if(res !=0)
            {
                //printf("TimeInit is not 0\n");
                logLog(mainld,LOGERR,"TimeInit is err %d",res);
                goto END;
            }
            cyctem=cyctem->next;

        }
        logLog(mainld,LOGINF,"TimeInit is ok");
        //    printf("\n\n");
        while(1)
        {
            pause();
            //        printf("pause\n");
        }

    }
    else
    {
        return 0;
    }

END:
    gameover(0);



    return 0;
}
