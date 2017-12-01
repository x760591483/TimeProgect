#ifndef _CMCCRABBITMQ_H
#define _CMCCRABBITMQ_H
#include "type.h"
//初始化
//int xxhAmqpInit();

//获取数据
//int xxhAmqpGetDate(char *Date);

//清空回收数据
int xxhAmqpStop(void *);//amqp_connection_state_t *传入

//清除字符串后空格和回车
//void chardelblank(char *ff,int num);

//发送数据
int xxhAmqpSendDate(char *,rabbitdata *);
//发送模式的初始化
int xxhAmqpSendInit(rabbitdata *);
int xxhAmqpReSendInit(rabbitdata *);

#endif
