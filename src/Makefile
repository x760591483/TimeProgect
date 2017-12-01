CC = gcc
inc =  -I ../inc/

LIBPA = -L ../lib/ 


OTHER = -Wall

#src = $(wildcard *.c)

#obj =$(patsubst %.c, %.o, $(src))

ALL:Time


Time:main.c
	$(CC) $^ -o $@ $(LIBPA)  -llogLog -ldl $(inc) $(OTHER)

#post=TimeCompensation libso
#msgsend=send

#target = $(post)

#ALL:$(target)



#libcmccRabbitmq.so:cmccRabbitmq.o
#	$(CC) $< -o $@ $(LIBPA) -lrabbitmq -shared $(OTHER)
#cmccRabbitmq.o:cmccRabbitmq.c
#	$(CC) -c -fpic $^ $(inc) $(OTHER)


#libcmccMysql.so:cmccMysql.o
#	$(CC) $^ -o $@ $(LIBPA)  -lmysqlclient -lstdc++ -ldl -lpthread -lrt -shared $(OTHER)
#cmccMysql.o:cmccMysql.c
#	$(CC) -c -fpic $^ $(inc) $(OTHER)

#liblogLog.so:logLog.o
#	$(CC) $^ -o $@ -shared $(OTHER)
#logLog.o:logLog.c
#	$(CC) -c -fpic $^ $(inc) $(OTHER)


#libmethod.so:method.o
#	$(CC) $^ -o $@ $(LIBPA) -lcjson -L ./ -lcmccMysql -lcmccRabbitmq -llogLog  -shared $(OTHER)
#method.o:method.c
#	$(CC) -c -fpic $^ $(inc) $(OTHER)

#$(zlog):test_hello.o 
#	$(CC) $^ -o $@  $(LIBPA)  $(LIBA)

#$(post):curl_post.o xxhLog.o xxhRabbitmq.o
#	$(CC) $^ -o $@  $(OTHER)  $(LIBPA) $(LIBA) $(LIBB)

#$(msgsend):msgsend.o xxhLog.o
#	$(CC) $^ -o $@ $(LIBPA) $(LIBA) 

#$(obj):%.o:%.c
#	$(CC) $(OTHER) -c $< -o $@ $(inc)
