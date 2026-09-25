#!/bin/sh 

ifconfig eth0 down
pppd call cdma2000 &
