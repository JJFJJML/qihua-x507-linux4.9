#!/bin/sh  

echo "======== Display system date ========"
date
echo "======== Display rtc date ========"
hwclock 
echo "======== Save system date to rtc date ========"
hwclock -w
echo "======== Display rtc date ========"
hwclock 

