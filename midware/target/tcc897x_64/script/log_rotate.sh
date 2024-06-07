#!/bin/sh

limit_size_log(){  
    local logfile=$1  
    local maxsize=$2 

    if [ ! -f "$logfile" ]; then
    touch $logfile
    fi

    filesize=`ls -l $logfile | awk '{ print $5 }'`
    if [ $filesize -gt $maxsize ]
    then
        echo "Log file dump backup."
        rm $logfile*.logbak
        cp $logfile $logfile"`date +%Y-%m-%d_%H-%M-%S`".logbak
        cp /dev/null $logfile
    fi
}

# running
while true
do
    limit_size_log /var/log/zh_user.log 5242880 # max size 5M
    sleep 10
done