#!/bin/bash

host_num=$1
player_num=$2


now_host=1
comcount=1
com=()

for a in $(seq 1 $player_num) ; do
for b in $(seq $(($a + 1)) $player_num) ; do
for c in $(seq $(($b + 1)) $player_num) ; do
for d in $(seq $(($c + 1)) $player_num) ; do
for e in $(seq $(($d + 1)) $player_num) ; do
for f in $(seq $(($e + 1)) $player_num) ; do
for g in $(seq $(($f + 1)) $player_num) ; do
for h in $(seq $(($g + 1)) $player_num) ; do
	com[${comcount}]="$a $b $c $d $e $f $g $h"
        comcount=$(($comcount+1))
done ; done ; done ; done ; done ; done ; done ; done 

comcount=${#com[@]}
if [[ ${host_num} -gt ${comcount} ]] 
then
	host_num=${comcount}
fi

#echo $comcount
#echo $host_num

for ((i=1;i<=8;i++))
do
    rank_to_point[$i]=$((8-$i))
done

for ((i = 1;i <= player_num;i++))
do
    player[$i]=0
done


for i in $(seq 0 $host_num)
do 
    mkfifo fifo_${i}.tmp
done
exec 4<>fifo_1.tmp
exec 5<>fifo_2.tmp
exec 6<>fifo_3.tmp
exec 7<>fifo_4.tmp
exec 8<>fifo_5.tmp
exec 9<>fifo_6.tmp
exec 10<>fifo_7.tmp
exec 11<>fifo_8.tmp
exec 12<>fifo_9.tmp
exec 13<>fifo_10.tmp
exec 14<>fifo_11.tmp
exec 15<>fifo_12.tmp
now_com=1

for i in $(seq 1 $host_num)
do 
    ./host ${i} ${i} 0 & 
    echo ${com[$((${comcount} - ${i} + 1))]} >fifo_${i}.tmp
    #echo ${com[$((${comcount} - ${i} + 1))]}
done


exec <fifo_0.tmp
for i in $(seq 1 ${comcount})
do
	read key
	id=${key}
	#echo $id
	for j in $(seq 1 8)
	do
		read player_id rank
		#echo $player_id $rank
		player[${player_id}]=$((${player[${player_id}]}+${rank_to_point[${rank}]}))
	done
	if [[ ${i} -le $((${comcount} - ${host_num})) ]]
	then
		echo ${com[${i}]} >fifo_${id}.tmp
	else
		echo "-1 -1 -1 -1 -1 -1 -1 -1" > fifo_${id}.tmp
	fi
done

for i in $(seq 1 $player_num)
do
    echo $i ${player[$i]}
done

for i in $(seq 0 12)
do
    rm fifo_${i}.tmp
done

exit 0
