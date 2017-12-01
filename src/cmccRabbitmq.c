#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <amqp.h>
#include <amqp_framing.h>
#include <amqp_tcp_socket.h>
#include <assert.h>

#include <signal.h>
#include "cmccRabbitmq.h"


//#include "utils.h"

#define NEEDCHARNUM 6

int waitAnswerSend(amqp_connection_state_t *);
//typedef char(myType)[64];

//static myType fileinf[NEEDCHARNUM];//0hostname  1port 2exchange 3queuename
//static amqp_connection_state_t conn;
//static int channelid =1;
//static int isGetMethod = 1;//判断模式
//static int port = 0;

//static amqp_queue_declare_ok_t *resQueue;//send part
//static amqp_bytes_t replyToQueue;//send part

int xxh_amqp_error(amqp_rpc_reply_t x,char const *txt)
{
    if(x.reply_type == AMQP_RESPONSE_NORMAL)
    {
        return 0;
    }

    return x.reply_type;
    //printf("%s:%d\n",txt,x.reply_type);

    //exit(1);

}

/*
   int xxhAmqpInit()
   {	
   int ret=0;
   ret =xxhConfFile();
   if(ret != 0)
   {
   return ret;
   }
   amqp_socket_t *socket=NULL;
//int channelid = 1;

//创建连接

conn = amqp_new_connection();
//创建socket
socket = amqp_tcp_socket_new(conn);
if(!socket)
{
printf("amqp_tcp_socket_new");
exit(1);
}
ret = amqp_socket_open(socket, fileinf[0], port);
if(ret)
{
printf("amqp_socket_open");
exit(1);
}


//绑定conn和socket
//	amqp_set_sockfd(conn, socket);
//登录
xxh_amqp_error(amqp_login(conn,"/",0,131072,0,AMQP_SASL_METHOD_PLAIN,fileinf[4],fileinf[5]),"Login in");


//关联 conn 和 交换机
amqp_channel_open(conn, channelid);
//
xxh_amqp_error(amqp_get_rpc_reply(conn),"Opening channel");
//队列说明  
amqp_queue_declare(conn,channelid,amqp_cstring_bytes(fileinf[3]),0,1,0,0,amqp_empty_table);
//
xxh_amqp_error(amqp_get_rpc_reply(conn),"Declare");

amqp_basic_qos(conn,channelid,0,1,0);
//基本消费  
amqp_basic_consume(conn,channelid,amqp_cstring_bytes(fileinf[3]),amqp_empty_bytes,0,0,0,amqp_empty_table);
xxh_amqp_error(amqp_get_rpc_reply(conn),"Consuming");
printf("rabbitmq init is ok");
return 0;
}
*/
int xxhAmqpSendInit(rabbitdata *dat)
{
int ddd=0;
//printf("%d\n",ddd++);//0
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    int ret=0;
    //amqp_connection_state_t conn;
    dat->handle = malloc(sizeof(amqp_connection_state_t));
    if(dat->handle == NULL)
    {
        return TCS_MALLOC_NULL;
    }
//printf("%d\n",ddd++);//1
    memset(dat->handle,0,sizeof(amqp_connection_state_t));
    amqp_connection_state_t *conn = (amqp_connection_state_t *)dat->handle;
//printf("%d\n",ddd++);//2
    amqp_socket_t *socket=NULL;
    //int channelid = 1;

    //创建连接

    *conn = amqp_new_connection();
//printf("%d\n",ddd++);//3
    //创建socket
    socket = amqp_tcp_socket_new(*conn);
    if(!socket)
    {
        return -10;

    }
//printf("%d\n",ddd++);//4
    ret = amqp_socket_open(socket, dat->host.date, dat->port);
    if(ret)
    {

        return ret;
    }
//printf("%d\n",ddd++);//5


    //	amqp_set_sockfd(conn, socket);
    //登录
    ret = xxh_amqp_error(amqp_login(*conn,"/",0,131072,0,AMQP_SASL_METHOD_PLAIN,dat->name.date,dat->passwd.date),"Login in");
    if(ret != 0)
    {
        return ret;
    }

//printf("%d\n",ddd++);//6
    //关联 conn 和 交换机
    amqp_channel_open(*conn, 1);
    //
    ret =  xxh_amqp_error(amqp_get_rpc_reply(*conn),"Opening channel");
    if(ret != 0)
    {
        return ret;
    }
//printf("%d\n",ddd++);//7
//    amqp_queue_declare(*conn,1,amqp_cstring_bytes(dat->queueNa.date),0,0,0,0,amqp_empty_table);
    return 0;
}
int xxhAmqpReSendInit(rabbitdata *rab)
{
    if(rab ==NULL)
    {
        return TCS_ARG_NULL;
    }
    if(rab->handle)
    {
        xxhAmqpStop(rab->handle);
        rab->handle =NULL;
    }
    int ret = xxhAmqpSendInit(rab);
    
    return ret;

}
int xxhAmqpSendDate(char *Date,rabbitdata *rab)
{
    if(Date ==NULL || rab == NULL || rab->handle ==NULL)
    {
        return TCS_ARG_NULL;
    }
    amqp_connection_state_t *conn =(amqp_connection_state_t *)rab->handle;
    amqp_basic_properties_t props;
    props._flags = AMQP_BASIC_CONTENT_TYPE_FLAG |
        AMQP_BASIC_DELIVERY_MODE_FLAG;
    props.content_type = amqp_cstring_bytes("text/plain");

    props.delivery_mode = 1; /* persistent delivery mode */
    amqp_confirm_select(*conn, 1);
    int rest = amqp_basic_publish(*conn,
                                  1,
                                  amqp_cstring_bytes(""),
                                  amqp_cstring_bytes(rab->queueNa.date),
                                  0,
                                  0,
                                  &props,
                                  amqp_cstring_bytes(Date));

    // amqp_bytes_free(props.reply_to);
    if(rest !=0)
    {
        return rest;
    }


    rest =  waitAnswerSend(conn);//判断是否正确传递
    if(rest !=0)
    {
        return rest;
    }

    return 0;
}

int waitAnswerSend(amqp_connection_state_t *conn)
{
    if(conn == NULL)
    {
        return TCS_ARG_NULL;
    }

    amqp_frame_t frame;  
    amqp_rpc_reply_t ret; 

    if (AMQP_STATUS_OK != amqp_simple_wait_frame(*conn, &frame)) {  
        return -3;  
    } 
    if (AMQP_FRAME_METHOD == frame.frame_type) { 

        amqp_method_t method = frame.payload.method;  


        switch (method.id) {  
        case AMQP_BASIC_ACK_METHOD:{  

                                //       amqp_basic_ack_t *s;  
                                //       s = (amqp_basic_ack_t *) method.decoded;  
                                       //                               printf("Ack.delivery_tag=%d", s->delivery_tag);  
                                       //                               printf("Ack.multiple=%d", s->multiple);  

                                   }
                                   break; 
        case AMQP_BASIC_NACK_METHOD:  
                                   {  
                                //       amqp_basic_nack_t *s;  
                                //       s = (amqp_basic_nack_t *) method.decoded;  
                                       //                               printf("NAck.delivery_tag=%d", s->delivery_tag);  
                                       //                               printf("NAck.multiple=%d", s->multiple);  
                                       //                               printf("NAck.requeue=%d", s->requeue);  
                                       return 100;
                                   }  

                                   break;
        case AMQP_BASIC_RETURN_METHOD:  
                                   {  
                                       amqp_message_t message;  
                                //       amqp_basic_return_t *s;  
                                       char str[520];  
                              //         s = (amqp_basic_return_t *) method.decoded;  
                                       //                               printf("Return.reply_code=%d", s->reply_code);  
                                       //                               strncpy(str, s->reply_text.bytes, s->reply_text.len); str[s->reply_text.len] = 0;  
                                       //                               printf("Return.reply_text=%s", str);  

                                       ret = amqp_read_message(*conn, frame.channel, &message, 0);  
                                       if (AMQP_RESPONSE_NORMAL != ret.reply_type) {  
                                           return 104;  
                                       }  
                                       strncpy(str, message.body.bytes, message.body.len); str[message.body.len] = 0;  
                                       //                               printf("Return.message=%s", str);  

                                       amqp_destroy_message(&message);  
                                   }  

                                   break;

        case AMQP_CHANNEL_CLOSE_METHOD:  
                                   return 105;  

        case AMQP_CONNECTION_CLOSE_METHOD:    
                                   return 106;  

        default:  
                                   //                           printf("An unexpected method was received %d", frame.payload.method.id);  
                                   return 107;  


        }
    }
    return 0;
}
/*
   int xxhAmqpGetDate(char *Date)//接受信息
   {
   if(Date == NULL)
   {
   printf("Date is NULL error");
   return -1;
   }
   int result=0;
   struct timeval ti;
   ti.tv_sec=2;
   ti.tv_usec=0;

   while(1)
   {

   amqp_envelope_t envelope;

   amqp_maybe_release_buffers(conn);

   amqp_rpc_reply_t res = amqp_consume_message(conn, &envelope, &ti, 0);

   if (AMQP_RESPONSE_NORMAL != res.reply_type) {
   if(AMQP_RESPONSE_LIBRARY_EXCEPTION == res.reply_type)
   {result = 2;}

   break;
   }

   if(envelope.message.body.len <1)
   {
   result = -4;
   break;
   }
   void *tembuf = memcpy(Date,envelope.message.body.bytes,envelope.message.body.len);
   if(tembuf == NULL)
   {
   result=-3;
   }

   amqp_basic_ack(conn,1,envelope.delivery_tag,0);
   amqp_destroy_envelope(&envelope);
   break;
   }

   return result;
   }
   */
int xxhAmqpStop(void *dat)//清空rabbitmq占用的内存空间
{
    if(dat ==NULL)
    {
        return TCS_ARG_NULL;
    }
    amqp_connection_state_t *conn =(amqp_connection_state_t *)dat;

    int result =0;	
    xxh_amqp_error(amqp_channel_close(*conn, 1, AMQP_REPLY_SUCCESS), "Closing channel");
    xxh_amqp_error(amqp_connection_close(*conn, AMQP_REPLY_SUCCESS), "Closing connection");
    result = amqp_destroy_connection(*conn);
    free(dat);
    if(result<0)
    {
        return result;
    }

    return 0;
}

/*
   int main(int argc, const char **argv) {


   const char *filename[NEEDCHARNUM]={"rabbitHostName","rabbitPort","rabbitExchange","rabbitQueueName","rabbitName","rabbitPassword"};
   myType fileinf[NEEDCHARNUM];//0hostname  1port 2exchange 3queuename
   int ret = readFileDate("./xxh.conf",filename,fileinf,6);
   printf("ret is %d\n",ret);


//	char date[1024]={0};
//	
//	xxhAmqpInit();
//	xxhAmqpGetDate(date);
//	printf("date:%s\n",date);
//	xxhAmqpStop();


return 0;
}
*/
