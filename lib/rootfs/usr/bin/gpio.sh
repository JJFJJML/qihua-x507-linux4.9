#!/bin/sh  

echo export pin
echo 130 > /sys/class/gpio/export 
echo 131 > /sys/class/gpio/export 
echo 228 > /sys/class/gpio/export 
echo 67 > /sys/class/gpio/export 
echo 8 > /sys/class/gpio/export 
echo 9 > /sys/class/gpio/export 
  
echo setting direction to output  
echo out > /sys/class/gpio/gpio130/direction
echo out > /sys/class/gpio/gpio131/direction
echo out > /sys/class/gpio/gpio228/direction
echo out > /sys/class/gpio/gpio67/direction
echo out > /sys/class/gpio/gpio8/direction
echo out > /sys/class/gpio/gpio9/direction
  
echo setting pin high or low

ON=1
OFF=0
while true
do
	echo $ON > /sys/class/gpio/gpio130/value
	echo $ON > /sys/class/gpio/gpio131/value
	echo $ON > /sys/class/gpio/gpio228/value
	echo $ON > /sys/class/gpio/gpio67/value
	echo $ON > /sys/class/gpio/gpio8/value
	echo $ON > /sys/class/gpio/gpio9/value
	sleep 1
	echo $OFF > /sys/class/gpio/gpio130/value
	echo $OFF > /sys/class/gpio/gpio131/value
	echo $OFF > /sys/class/gpio/gpio228/value
	echo $OFF > /sys/class/gpio/gpio67/value
	echo $OFF > /sys/class/gpio/gpio8/value
	echo $OFF > /sys/class/gpio/gpio9/value
	sleep 1
done

