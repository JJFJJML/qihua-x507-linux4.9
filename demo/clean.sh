#!/bin/sh

../install.sh

for n in ./*
do
	echo $n
(
	if [ -d $n ];then
		cd $n
		make clean
		cd -
	fi
)
done
