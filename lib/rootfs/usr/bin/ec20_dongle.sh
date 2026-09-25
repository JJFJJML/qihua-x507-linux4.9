#!/bin/sh 

ifconfig eth0 down
pppd call quectel-ppp &
