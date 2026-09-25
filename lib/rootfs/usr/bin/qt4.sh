#!/bin/sh

export QWS_MOUSE_PROTO="Tslib:/dev/input/event2"
export QWS_DISPLAY="Transformed:Rot90"
(
	cd /usr/share/qt/demos/embedded/fluidlauncher
	./fluidlauncher -qws 
)
