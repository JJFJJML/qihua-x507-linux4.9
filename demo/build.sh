#!/bin/bash

LICHEE_TOP_DIR=`cd ../;pwd`
../install.sh
export CC=aarch64-linux-gcc

for n in *
do
	if [ -d $n ]; then
		if [  ${n:0:3} == "Qt_" ];then
			echo $n
			(
	    		    cd $n
			    make clean
      			    qmake -project
        		    echo 'QT += widgets' >> $n.pro
       			    qmake
      			    make
       			    if [ -d ${LICHEE_TOP_DIR}/lib/rootfs/usr/bin ];then
               		   	 cp $n ${LICHEE_TOP_DIR}/lib/rootfs/usr/bin
    			    fi
    			    cd -
			)
		else
			echo $n 
			(
			    cd $n
			    if [ -f Makefile ];then
			        make clean
			    	make
			    else
				./build.sh
			    fi	
               		    cp $n ${LICHEE_TOP_DIR}/lib/rootfs/usr/bin
			    cd -
			)
		fi
	
	fi
done
