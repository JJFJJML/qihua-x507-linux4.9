#!/bin/sh

ffmpeg -f video4linux2 -s 1280x720 -r 15 -i /dev/video0 test.avi
