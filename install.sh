#!/bin/bash
echo "TimeCompensation"

path=`pwd`
#环境变量
LDpath="$path/lib/"

#datepath="export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$LDpath"

#echo $datepath >> ~/.bashrc

#source ~/.bashrc
#if [ 0 -ne $?  ]
#then 
#        echo "source file  error"
#        exit
#fi



mkdir ./logs
mkdir ./bin
mkdir ./bin/logs
mkdir ./libdir

#-------------------------------
cd ./src

unzip ./cJSON-master.zip
cd cJSON-master
mkdir build
cd ./build
cmake ..
if [ 0 -ne $?  ]
then 
        echo "cjson master cmake  error"
        exit
fi
make
if [ 0 -ne $?  ]
then 
        echo "cjson master make  error"
        exit
fi
cp libcjson.so* $path/lib/
cd $path/src/
rm cJSON-master -rf


unzip ./rabbitmq-c-master.zip
cd rabbitmq-c-master
mkdir build
cd ./build
cmake ..
if [ 0 -ne $?  ]
then 
        echo "rabbitmq master cmake error"
        exit
fi
make
if [ 0 -ne $?  ]
then 
        echo "rabbitmq master make  error"
        exit
fi
cd ./librabbitmq
cp librabbitmq.so* $path/lib 
cd $path/src
rm rabbitmq-c-master -rf


gcc -c -fpic cmccRabbitmq.c -I ../inc/
gcc cmccRabbitmq.o -o libcmccRabbitmq.so -L ../lib/ -lrabbitmq -shared
gcc -c -fpic cmccMysql.c -I ../inc/
gcc cmccMysql.o -o libcmccMysql.so -L ../lib/ -lmysqlclient -lstdc++ -ldl -lpthread -lrt -shared
gcc -c -fpic logLog.c -I ../inc/
gcc logLog.o -o liblogLog.so -shared
mv lib*.so ../lib/

# gcc -c -fpic method.c -I ../inc/
#gcc method.o -o libmethod.so -L ../lib/ -llogLog -lcmccRabbitmq -lcmccMysql -lcjson -shared
#mv lib*.so ../lib/

rm ./*.o -f 


make
if [ 0 -ne $?  ]
then 
        echo "make file  error happen"
        exit
fi


cp start.sh ../bin/
cp Time ../bin/

echo ">>>>>>>>>>>>>ok<<<<<<<<<<<<<<<<<<<"

