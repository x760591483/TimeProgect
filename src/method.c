#include <stdio.h>                                                                                                             
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include "cjson/cJSON.h"
#include "method.h"

#include "cmccMysql.h"
int sysFreeOnedate(onedate *date);
int charToTime(char *strdat,int nums);
typedef struct SYSDATA
{
    long long seqnum;
    long long seqnumTem;//用于每次运行时记录 最后统一到seqnum上
    rabbitdata rdata;
    mysqldata mdata;
    mysqldata mdatatableA;//this only have mysqltablename is useful
    mysqldata mdatatableB;//this only have mysqltablename is useful
    onedate *odata;

}sysdata;

int sysdeldata(cycdata *dat)
{
    printf("method sysdeldata\n");
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    sysdata *sdata = (sysdata*)dat->para;
    if(sdata ==NULL)
    {
        return TCS_ARG_NULL;
    }
    if(sdata ->mdata.handle !=NULL)
    {
        mysqlClose(sdata ->mdata.handle);
        sdata->mdata.handle =NULL;
        if(sdata->mdata.resData)
        {
            mysqlFreeRes(sdata->mdata.resData);
            sdata->mdata.resData=NULL;
        }
    }
    if(sdata->rdata.handle !=NULL)
    {
        //xxh        xxhAmqpStop(sdata->rdata.handle);
        sdata->rdata.handle =NULL;
    }
    if(sdata->odata !=NULL)
    {
        sysFreeOnedate(sdata->odata);
        sdata->odata = NULL;
    }

    free(sdata);
    dat->para =NULL;

    return 0;
}
int sysFreeOnedate(onedate *date)
{
    if(date ==NULL)
    {
        return TCS_ARG_NULL;
    }

    onedate *otem = date;
    while(otem != NULL)
    {
        date = otem->next;
        if(otem->value.date !=NULL)
        {
            free(otem->value.date);
            otem->value.date=NULL;
            otem->value.len=0;
        }
        if(otem->key.date !=NULL)
        {
            free(otem->key.date);
            otem->key.date=NULL;
            otem->key.len=0;
        }
        free(otem);
        otem = date;

    }
    return 0;

}
int freeDate(sysdata *data)//用于清理每次搜索循环中产生的内存
{
    if(data ==NULL)
    {
        return TCS_ARG_NULL;
    }
    if(data->mdata.resData)
    {
        mysqlFreeRes(data->mdata.resData);
        data->mdata.resData=NULL;
    }
    if(data->odata)
    {
        sysFreeOnedate(data->odata);
        data->odata=NULL;
    }

    return 0;
}

/*
int sysload(filedate *dat,CYCMK cycmk)
{
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    filedate *filetem = dat;
    cycdata *cyctem= NULL;
    for(;filetem != NULL;)
    {
        if(strncmp(filetem->name,"HOUZHENG",strlen("HOUZHENG"))==0)
        {
            cyctem = cycmk(filetem);
            if(cyctem==NULL)
            {
                return TCS_OTHER_ERR;
            }

            cyctem ->init = sysinit;

        }

        filetem = filetem->next;

    }

    return 0;

}
*/
int findArg(onedate *head,char const *src,mystring *strRes)
{
    //printf("method findArg\n");
    if(head == NULL || strRes ==NULL || src ==NULL)
    {
        return TCS_ARG_NULL;
    }
  //  printf("--aaaa\n");
    for(;head != NULL;)
    {
  //      printf("--findArg for %d\n",head->value.len);
        if(strncmp(head->key.date,src,strlen(src))==0)
        {
    //        printf("--%s findArg ok\n",src);
            strRes->date=head->value.date;
            strRes->len=head->value.len;

            return 0;
        }
        head = head->next;
    }
  //  printf("--bbbbbb\n");
    return TCS_DATA_NULL;//没找到

}

int sysinit(cycdata *dat)
{
    printf("method sysinit\n");
  //  int textnum =0;
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    //printf("sysinit start\n");
    cycdata *cyctem =dat;
    cyctem ->para = malloc(sizeof(sysdata));
    if(cyctem->para ==NULL)
    {
        return TCS_MALLOC_NULL;
    }
    memset(cyctem->para,0,sizeof(sysdata));

   // printf("--%d\n",textnum++);
    //结构体赋值
    sysdata *system = (sysdata *)cyctem->para;
    filedate *filetem = cyctem->fdate;
    if(filetem==NULL)
    {
        printf("--filetem is NULL\n");
    }
    onedate *onetem = filetem->date;
   // printf("-- sysinit onedate %p\n",onetem);
    if(onetem ==NULL)
    {
        printf("--onetem is NULL\n");
    }
    int ret =0;
   // printf("--%d\n",textnum++);

    mystring tem={NULL,0};
   // printf("--%d\n",textnum++);

    findArg(onetem,"log",&tem);
   // printf("--%d\n",textnum++);
    logLog(cyctem->ld,LOGWAR,"log addr is %s",tem.date);
    logdata *temlog = cyctem->ld;//临时记录
   // printf("--%d\n",textnum++);
    cyctem->ld = logInit(tem.date,100);
    if(cyctem ->ld !=NULL)
    {
        logLog(cyctem->ld,LOGINF,"%s log addr is %s",cyctem->fdate->name,tem.date);
    }
    else
    {//若没有写log的日志地址 用系统的log地址
        cyctem->ld = temlog;
        logLog(cyctem->ld,LOGWAR,"%s log addr is main log",cyctem->fdate->name);
    }
    memset(&tem,0,sizeof(mystring));

  //  printf("--%d\n",textnum++);

    FILE *seqfg = fopen("./seqnum","r");
    char seqnumstr[64]={0};
    if(seqfg ==NULL)
    {
        logLog(cyctem->ld,LOGWAR,"get seqnum fopen NULL");
    }
    else
    {
        fgets(seqnumstr,sizeof(seqnumstr)-1,seqfg);
        system->seqnum = atol(seqnumstr);//将字符转成数字 自增id
        logLog(cyctem->ld,LOGINF,"seqnum is %lld",system->seqnum);
        fclose(seqfg);
        seqfg=NULL;
    }


  //  printf("--%d\n",textnum++);
    logLog(cyctem->ld,LOGINF,"model name:%s",cyctem->fdate->name);
    ret = findArg(onetem,"cycleTime",&tem);
    if(ret ==0)
    {cyctem->tidata.key.len = atoi(tem.date);}
    else
    {
        cyctem->tidata.key.len =0;
    }
    memset(&tem,0,sizeof(mystring));
    logLog(cyctem->ld,LOGINF,"cycleTime:%d",cyctem->tidata.key.len);


  //  printf("--%d\n",textnum++);
    //rabbitmq
    ret = findArg(onetem,"rabbitHostName",&(system->rdata.host));
    logLog(cyctem->ld,LOGINF,"rabbitHostNmae:%s",system->rdata.host.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret = findArg(onetem,"rabbitPort",&(tem));//port特殊处理
    if(ret != 0)
    {
        logLog(cyctem->ld,LOGERR,"rabbitPort is NULL");
        return TCS_DATA_NULL;
    }
    system->rdata.port= atoi(tem.date);
    logLog(cyctem->ld,LOGINF,"rabbitPort:%d",system->rdata.port);
    memset(&tem,0,sizeof(mystring));

  //  printf("--%d\n",textnum++);

    ret = findArg(onetem,"rabbitExchange",&(system->rdata.exchan));
    logLog(cyctem->ld,LOGINF,"rabbitExchange:%s",system->rdata.exchan.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret = findArg(onetem,"rabbitQueueName",&(system->rdata.queueNa));
    logLog(cyctem->ld,LOGINF,"rabbitQueueName:%s",system->rdata.queueNa.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret = findArg(onetem,"rabbitName",&(system->rdata.name));
    logLog(cyctem->ld,LOGINF,"rabbitName:%s",system->rdata.name.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret = findArg(onetem,"rabbitPassword",&(system->rdata.passwd));
    logLog(cyctem->ld,LOGINF,"rabbitPassword:%s",system->rdata.passwd.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }

  //  printf("--%d\n",textnum++);
    //mysql
    ret = findArg(onetem,"mysqlhost",&(system->mdata.host));
    logLog(cyctem->ld,LOGINF,"mysqlhost:%s",system->mdata.host.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }

    ret = findArg(onetem,"mysqlPort",&(tem));//port特殊处理
    if(ret != 0)
    {
        system->mdata.port =0;
        logLog(cyctem->ld,LOGERR,"mysqlPort is defalut");
    }
    else
    {
        system->mdata.port= atoi(tem.date);
        logLog(cyctem->ld,LOGINF,"mysqlPort:%d",system->mdata.port);

    }

  //  printf("--%d\n",textnum++);

    ret = findArg(onetem,"mysqluser",&(system->mdata.user));
    logLog(cyctem->ld,LOGINF,"mysqluser:%s",system->mdata.user.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret = findArg(onetem,"mysqlpasswd",&(system->mdata.passwd));
    logLog(cyctem->ld,LOGINF,"mysqlpasswd:%s",system->mdata.passwd.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret = findArg(onetem,"mysqldb",&(system->mdata.db));
    logLog(cyctem->ld,LOGINF,"mysqldb:%s",system->mdata.db.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret= findArg(onetem,"mysqltablename",&(system->mdata.tablename));
    logLog(cyctem->ld,LOGINF,"mysqltablename:%s",system->mdata.tablename.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret= findArg(onetem,"mysqltablename_A",&(system->mdatatableA.tablename));
    logLog(cyctem->ld,LOGINF,"mysqltablename_A:%s",system->mdatatableA.tablename.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    ret= findArg(onetem,"mysqltablename_B",&(system->mdatatableB.tablename));
    logLog(cyctem->ld,LOGINF,"mysqltablename_B:%s",system->mdatatableB.tablename.date);
    if(ret !=0)
    {
        return TCS_DATA_NULL;
    }
    printf("method sysinit end\n");
    /*
    //函数指针赋值
    cyctem ->login = sysLogin;
    cyctem->howdo= sysHowdo;
    cyctem->send =sysSend;
    cyctem->seach=sysSeach;
    cyctem->deldate=sysDel;
    */
    return 0;

}


int sysseach(cycdata *dat)
{
    printf("method sysseach\n");
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    sysdata *sdata =(sysdata*)dat->para;
    if(sdata ==NULL)
    {
        return TCS_ARG_NULL;
    }

    int ret =0;
    char seachdata[1024]={0};
    sprintf(seachdata,"%s %s %s %lld %s %s","select seqnum,type,nginxarry,deployidmd5,cmds from",sdata->mdata.tablename.date,"where seqnum >",sdata->seqnum,"order by seqnum","limit 10");

    logLog(dat->ld,LOGINF,"mysql seach:%s",seachdata);
    //    printf("seach  %s\n",seachdata);
    ret = mysqlSearch(&sdata->mdata,seachdata);
    if(ret <0)
    {
        logLog(dat->ld,LOGERR,"mysqlSearch is err:%d\n",ret);
        if((ret = mysqlReLogin(&sdata->mdata)))
        {
            logLog(dat->ld,LOGERR,"mysql relogin err:%d",ret);
            return ret;
        }
        ret = mysqlSearch(&sdata->mdata,seachdata);
        if(ret < 0)
        {
            logLog(dat->ld,LOGERR,"mysqlSearch is err again:%d",ret);
            return ret;
        }
    }
    if(ret ==TCS_DATA_NULL)
    {//表示无数据
        freeDate(sdata);
        return TCS_DATA_NULL;
    }
    sdata->mdatatableA.handle = sdata->mdata.handle;//赋予MySQL句柄
    sdata->mdatatableB.handle = sdata->mdata.handle;

    logLog(dat->ld,LOGINF,"sysSeach is ok");
    return 0;

}
int syssend(cycdata *dat)
{
    printf("method syssend\n");
    if(dat == NULL)
    {
        return TCS_ARG_NULL;
    }

    sysdata *sdata =(sysdata*)dat->para;
    if(sdata ==NULL)
    {
        return TCS_ARG_NULL;
    }

    if(sdata ->odata ==NULL)
    {
        return TCS_DATA_NULL;
    }
    onedate *otem = sdata->odata;

    while(otem != NULL)
    {
        int res = xxhAmqpSendDate(otem->value.date,&sdata->rdata);
        if(res != 0)
        {
            logLog(dat->ld,LOGERR,"xxhAmqpSendDate is err:%d",res);
            res = xxhAmqpReSendInit(&sdata->rdata);
            if(res)
            {
                logLog(dat->ld,LOGERR,"rabbitmq reload err:%d",res);
                return TCS_OTHER_ERR;
            }
            res = xxhAmqpSendDate(otem->value.date,&sdata->rdata);
            if(res != 0)
            {
                logLog(dat->ld,LOGERR,"xxhAmqpSendDate is again  err:%d",res);
                freeDate(sdata);
                return TCS_OTHER_ERR;
            }
        }
        logLog(dat->ld,LOGINF,"%s send %s",dat->fdate->name,otem->value.date);
        otem = otem->next;

    }

    sdata->seqnum = sdata->seqnumTem;//将自增id 更新确定 只有当发出后才更新

    FILE *seqfg = fopen("./seqnum","w");
    if(seqfg ==NULL)
    {
        logLog(dat->ld,LOGWAR,"get seqnum fopen NULL");
    }
    else
    {
        fprintf(seqfg,"%lld",sdata->seqnum);
        logLog(dat->ld,LOGINF,"write file seqnum:%lld",sdata->seqnum);
        fclose(seqfg);
        seqfg=NULL;
    }


    freeDate(sdata);
    logLog(dat->ld,LOGINF,"sysSend is ok");

    return 0;


}

int searchmysql(const char *cmddata,mysqldata *mysdata,logdata *ld)
{
    int ret = mysqlSearch(mysdata,cmddata);
    if(ret <0)
    {
        logLog(ld,LOGERR,"mysqlSearch is err:%d\n",ret);
        return ret;
    }
    if(ret ==TCS_DATA_NULL)
    {//表示无数据
        logLog(ld,LOGERR,"mysqlSearch is no data:%d\n",ret);
        mysqlFreeRes(mysdata->resData);
        return ret;
    }
    return 0;
}

int splitcmdsdata(const char *sourdata,int *whe,char *putout)//传入为源数据char*  本次扫描位置和传出下次位置   传出值
{
    if(sourdata==NULL || putout ==NULL || whe==NULL || *whe<0)
    {
        return TCS_ARG_NULL;
    }
    int num = *whe;
    int numtwo=0;
    while(1)
    {
        if(*(sourdata+num)==0)
        {
            *whe=num;
            if(numtwo==0)
            {
                return 1;//表示本次无数据了
            }
            else
            {
                return 0;
            }
        }
        if(*(sourdata+num)>47 && *(sourdata+num)<58 ||  *(sourdata+num)>64 && *(sourdata+num)<91 || *(sourdata+num)>96 && *(sourdata+num)<123)
        {
            *(putout + numtwo) = *(sourdata + num);
            ++numtwo;
            ++num;
            continue;
        }
        if(numtwo==0)
        {
            ++num;//排除首的非目标字符
            continue;
        }
        *whe=num;
        return 0;

    }

    return 0;
}

int syshowdo(cycdata *dat)
{
    printf("method syshowdo\n");
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    sysdata *sdata =(sysdata*)dat->para;
    if(sdata ==NULL)
    {
        return TCS_ARG_NULL;
    }

    int ret=0;

    char **tem =NULL;
    onedate ohead;
    memset(&ohead,0,sizeof(onedate));
    onedate *otem = &ohead;

//int hownum=0;
    cJSON *json =NULL;
    while((tem=fetchDate(sdata->mdata.resData)))//循环提取信息
    {
        logLog(dat->ld,LOGINF,"%s--%s--%s--%s--%s\n",tem[0],tem[1],tem[2],tem[3],tem[4]);
        //seqnum,type,nginxarry,deployidmd5,cmds 
        sdata->seqnumTem = atol(tem[0]);//将字符转成数字 自增id
        logLog(dat->ld,LOGINF,"seqid is :%lld\n",sdata->seqnumTem);

//printf("howdo while %d\n",hownum++);
        if(strncmp(tem[1],"updateInstance",strlen("updateInstance"))==0)//判断信息类别  updateInstance 和 updateData
        {

//printf("howdo aaaa %d\n",hownum++);
            int splitnum=0;
            char splitchar[32]={0};
            json = cJSON_CreateObject();
//printf("howdo aaaa%d\n",hownum++);
            cJSON *arra =NULL;
            cJSON_AddItemToObject(json,"cmds",arra=cJSON_CreateArray());
//printf("how aaaa%d\n",hownum++);
            while(splitcmdsdata(tem[4],&splitnum,splitchar)==0)
            {
//printf("howdo bbbb %d\n",hownum++);
                logLog(dat->ld,LOGINF,"cmd :%s",splitchar);
                char seachdata[1024]={0};
                sprintf(seachdata,"%s %s %s %s","select seqid,cmd,instance from",sdata->mdatatableA.tablename.date,"where seqid =",splitchar);
                ret = searchmysql(seachdata,&sdata->mdatatableA,dat->ld);//访问A表
                if(ret !=0)
                {
//printf("howdo  ccccc %d\n",hownum++);
                    continue;
                }
                char **cmdtem=fetchDate(sdata->mdatatableA.resData);//提取信息
                //            printf("\t %s--%s--%s\n",cmdtem[0],cmdtem[1],cmdtem[2]);
                //组成json信息格式
                cJSON *jsoncmd = cJSON_CreateObject();
                cJSON_AddItemToObject(jsoncmd,"cmd",cJSON_CreateString(cmdtem[1]));

                cJSON_AddItemToObject(jsoncmd,"seqid",cJSON_CreateString(cmdtem[0]));
                cJSON_AddItemToObject(jsoncmd,"deployidmd5",cJSON_CreateString(tem[3]));
                cJSON *arrains =NULL;
                cJSON_AddItemToObject(jsoncmd,"instance",arrains=cJSON_CreateArray());

                cJSON_AddItemToArray(arrains,cJSON_CreateString(cmdtem[2]));

                cJSON_AddItemToArray(arra, jsoncmd);
                mysqlFreeRes(sdata ->mdatatableA.resData);//清空mysql本次查询占用的内存
                sdata ->mdatatableA.resData = NULL;

//printf("howdo  ddddd %d\n",hownum++);
            }
//printf("howdo dddeeee%d\n",hownum++);
        }
        else if(strncmp(tem[1],"updateData",strlen("updateData"))==0)
        {
//printf("howdo eeeee %d\n",hownum++);
            int splitnum=0;
            char splitchar[32]={0};
            json = cJSON_CreateObject();
            cJSON *arra =NULL;
            cJSON_AddItemToObject(json,"cmds",arra=cJSON_CreateArray());
            while(splitcmdsdata(tem[4],&splitnum,splitchar)==0)
            {
//printf("howdo  ffff %d\n",hownum++);
                logLog(dat->ld,LOGINF,"cmd :%s",splitchar);


                char seachdata[1024]={0};
                sprintf(seachdata,"%s %s %s %s","select dataid,cmd,velocitylimit,blacklist,whitelist from",sdata->mdatatableB.tablename.date,"where dataid =",splitchar);
                //            printf("\t seach  %s\n",seachdata);
                ret = searchmysql(seachdata,&sdata->mdatatableB,dat->ld);
                if(ret !=0)
                {
                    continue;
                }
                cJSON *jsoncmd = cJSON_CreateObject();
                char **cmdtem=fetchDate(sdata->mdatatableB.resData);
                //            printf("\t %s--%s--%s\n",cmdtem[0],cmdtem[1],cmdtem[2]);

                cJSON_AddItemToObject(jsoncmd,"seqid",cJSON_CreateString(cmdtem[0]));
                // cJSON_AddItemToObject(jsoncmd,"deployidmd5",cJSON_CreateString(tem[3]));
                cJSON *arrains =NULL;
                cJSON_AddItemToObject(jsoncmd,"instance",arrains=cJSON_CreateArray());
                //  printf("====%s=%s=%s=%s=%s\n",cmdtem[0],cmdtem[1],cmdtem[2],cmdtem[3],cmdtem[4]);

//printf("howdo gggggg %d\n",hownum++);
                if(cmdtem[2]!=NULL)
                {
                    cJSON_AddItemToArray(arrains,cJSON_CreateString(cmdtem[2]));
                    cJSON_AddItemToObject(jsoncmd,"cmd",cJSON_CreateString("velocity_limit"));
                }
                else if(cmdtem[3]!=NULL)
                {
                    cJSON_AddItemToArray(arrains,cJSON_CreateString(cmdtem[3]));
                    cJSON_AddItemToObject(jsoncmd,"cmd",cJSON_CreateString("black_list"));
                }
                else if(cmdtem[4]!=NULL)
                {
                    cJSON_AddItemToArray(arrains,cJSON_CreateString(cmdtem[4]));
                    cJSON_AddItemToObject(jsoncmd,"cmd",cJSON_CreateString("white_list"));
                }
                if(strncmp("del_rate",cmdtem[1],8)==0)
                {
                    cJSON_DeleteItemFromArray(arrains,0);
                }
                cJSON_AddItemToArray(arra, jsoncmd);
                mysqlFreeRes(sdata ->mdatatableB.resData);//清空mysql本次查询占用的内存
                sdata ->mdatatableB.resData = NULL;

//printf("howdo hhhhh %d\n",hownum++);
            }
        }
        cJSON_AddItemToObject(json,"type",cJSON_CreateString("update"));
        cJSON_AddItemToObject(json,"nginxarry",cJSON_CreateArray());
        cJSON_AddItemToObject(json,"seqnum",cJSON_CreateString(tem[0]));

        otem->next = (onedate*)malloc(sizeof(onedate));
        memset(otem->next,0,sizeof(onedate));
        otem=otem->next;
        otem->key.date = cJSON_Print(json);
        // printf("cjson print: %s\n",otem->key.date);
        otem->value.date = (char*)malloc(sizeof(char)*strlen(otem->key.date)+1);
        otem->value.len= sizeof(char) * strlen(otem->key.date);
        memset(otem->value.date,0,otem->value.len + 1);
        strncpy(otem->value.date,otem->key.date,otem->value.len);

        cJSON_free(otem->key.date);
        otem->key.date=NULL;//指针不用后必须归零  防止最终释放时 发生二次释放
        cJSON_Delete(json); 
        json =NULL;



    }
//printf("howdo iiiii %d\n",hownum++);
    logLog(dat->ld,LOGINF,"howdo  is ing----");
    sdata ->odata = ohead.next;

    mysqlFreeRes(sdata ->mdata.resData);//清空mysql本次查询占用的内存

    sdata ->mdata.resData = NULL;


    logLog(dat->ld,LOGINF,"howdo  is ok");
    return 0;
}
int syslogin(cycdata *dat)
{
    printf("method syslogin\n");
    logLog(dat->ld,LOGINF,"sysLogin start");
//    printf("--syslogin %p\n",dat);
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    sysdata *sdata =(sysdata *)dat->para;
    if(sdata ==NULL)
    {
        return TCS_ARG_NULL;
    }
//    printf("--syslogin sdata %p\n",sdata);
    int ret=0;
//    printf("--syslogin xxhAmqpSendInit new \n");
    ret = xxhAmqpSendInit(&sdata->rdata);
//    printf("--syslogin xxhAmqpSendInit ok\n");
    if(ret != 0)
    {
        logLog(dat->ld,LOGERR,"xxhAmqpSendInit is err:%d",ret);
        return TCS_OTHER_ERR;
    }
    logLog(dat->ld,LOGINF,"xxhAmqpSendInit is ok");

//    printf("--syslogin mysqlLogin\n");
    ret = mysqlLogin(&sdata->mdata);
    if(ret != 0)
    {
        logLog(dat->ld,LOGERR,"mysqlLogin is err:%d",ret);
        return TCS_OTHER_ERR;
    }
    logLog(dat->ld,LOGINF,"mysqlLogin is ok");

    logLog(dat->ld,LOGINF,"sysLogin is ok");
    printf("--syslogin is end\n");
    return 0;

}
/*
   int main(int argc, char *argv[])
   {
   printf("-----test start-------\n");
   struct CYCDATA *cycTem=malloc(sizeof(struct CYCDATA));
   memset(cycTem,0,sizeof(cycTem));


   struct SYSDATA *sysTem = malloc(sizeof(sysdata)); 
   memset(sysTem,0,sizeof(sysdata));

//赋值
sysTem->mdata.port=8066;
sysTem->mdata.host.date="10.1.4.183";
sysTem->mdata.host.len=10;
sysTem->mdata.user.date="cmcc";
sysTem->mdata.user.len=4;
sysTem->mdata.passwd.date="123456";
sysTem->mdata.passwd.len=6;
sysTem->mdata.db.date="ida";
sysTem->mdata.db.len=3;
sysTem->mdata.tablename.date="ida_rule_sync_order";
sysTem->mdata.tablename.len=19;

sysTem->mdatatableA.tablename.date="ida_rule_sync_instance";
sysTem->mdatatableA.tablename.len=22;
sysTem->mdatatableB.tablename.date="ida_rule_sync_real_data";
sysTem->mdatatableB.tablename.len=23;
sysTem->seqnum= 404718686059098656;

sysTem->rdata.port=5672;
sysTem->rdata.host.date="10.1.4.189";
sysTem->rdata.host.len=10;
sysTem->rdata.exchan.date="foo3";
sysTem->rdata.exchan.len=5;
sysTem->rdata.queueNa.date="syncRuleQueue";
sysTem->rdata.queueNa.len=11;
sysTem->rdata.name.date="cmcc";
sysTem->rdata.name.len=4;
sysTem->rdata.passwd.date="123456";
sysTem->rdata.passwd.len=6;



cycTem->para=(void *)sysTem;
//    printf("cycTem host %s\n",sysTem->mdata.host.date);

cycTem->login = sysLogin;
cycTem->howdo= sysHowdo;
cycTem->send =sysSend;
cycTem->seach=sysSeach;
//    cycTem.deldate=sysDel;

logdata *mainTem = logInit("./mainTem.log",100);
logLog(mainTem,LOGINF,"game is start");


cycTem->ld= mainTem;


sysLogin(cycTem);

sysSeach(cycTem);
sysHowdo(cycTem);
sysSend(cycTem);

onedate *playonedate=sysTem->odata;
while(playonedate !=NULL)
{
printf("\t\t %s\n",playonedate->value.date);
playonedate = playonedate->next;
}

printf("-----test end-------\n");
return 0;

}
*/
/*
   int charToTime(char *strdat,int nums)
   {//比较字符时间 和现在时间的差值是否大于 nums秒  是 返回0 否 返回1  出错返回负数
   if(strdat == NULL)
   {
   return TCS_ARG_NULL;
   }
//2017-08-02 12:10:52
struct tm tmtem;
memset(&tmtem,0,sizeof(struct tm));
int ret = sscanf(strdat,"%d-%d-%d %d:%d:%d",&tmtem.tm_yday,&tmtem.tm_mon,&tmtem.tm_mday,&tmtem.tm_hour,&tmtem.tm_min,&tmtem.tm_sec);
if(ret != 6)
{
return TCS_OTHER_ERR;
}
tmtem.tm_isdst =-1;
tmtem.tm_year -=1900;
tmtem.tm_mon -=1;

time_t nowt = time(NULL);
time_t datt = mktime(&tmtem);
if(nowt - datt >= nums)
{
return 0;
}

return 1;

}
*/
