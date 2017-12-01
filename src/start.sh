#!/bin/bash
export LD_LIBRARY_PATH=../libdir:../lib:$LD_LIBRARY_PATH
echo "welcome user..."

#获取当前用户名 保存到变量MYUSR中
MYUSR=`whoami`

PRO=Time


#显示当前用户名
echo "usrs is $MYUSR"


MYAA=""    


state(){
#判断进程是否存在
#获得对应进程 id号 存于 MYAA
    MYAA=`ps -u $MYUSR | grep -w Time | awk {'print $1'}`
    if [ -z $MYAA ]
    then
        return 1
    else
        echo $MYAA    
        return 2
    fi
}


xxhstop(){
    echo "xxhstop"
    state
    if [ $? -eq 1 ]; then
        echo "$PRO 还没有运行..."
        return
    fi
    kill -10 $MYAA 
    sleep 1s
    state
    if [ $? -eq 2 ]; then
        echo "$PRO 无法关闭... "
        return 
    fi
    echo "已经终止。。"
}
xxhstatus(){
    echo "xxhstatus"
#判断是否已经运行
    state
    if [ $? -eq 1 ]; then
        echo "$PRO 没有运行..."
    else
        echo "$PRO 已经运行中..."
    fi

}

xxhstart(){
#先判断是否已经运行
    state
    if [ $? -eq 2 ]; then
        echo "$PRO 已经运行中..."
        return 
    fi

    echo "开始启动 $PRO---"
#    cd ./bin/    
    if [ $# -eq 0 ]; then 
        echo "can shu 000000"
        ./Time  &
    else
        ./Time $* &
    fi

    sleep 1s
    state
    if [ $? -eq 2 ]; then
        echo "start success!"
        return 
    fi

    echo "sorry! start fault"


}


xxhmain(){
#    echo $1
    case $1 in
        start)
            xxhstart
            ;;
        stop)
            xxhstop
            ;;
        status)
            xxhstatus
            ;;
        *)
            echo "you need use this as: $0 start|stop|status";
            ;;

    esac
    
}

if [ $# -eq 0 ];then
    xxhstart 
elif [ $1 = "-s" ];then
    xxhmain $2
elif [ $# -ge 2 ]; then
    xxhstart $*
else
    echo "you need use this as: $0 start|stop|status"
fi

