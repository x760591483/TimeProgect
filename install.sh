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



gcc -c -fpic logLog.c -I ../inc/
gcc logLog.o -o liblogLog.so -shared
mv lib*.so ../lib/


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

