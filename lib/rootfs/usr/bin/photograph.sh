#!/bin/sh

ffmpeg -i /dev/video0 -y -f image2 -t 0.001 -s 1920x1080 test.jpg
