#!/bin/sh

BOARD_DIR="$(dirname $0)"
MKIMAGE=$HOST_DIR/usr/bin/mkimage
BOOT_CMD=$BOARD_DIR/boot.cmd
BOOT_CMD_H=$BINARIES_DIR/boot.scr

pwd
(
cd 	output/target/
if [ ! -f init ]; then
	ln -s bin/busybox init
fi
cd dev/
echo -e "\033[30;107mCreate null device . \033[0m"
mknod -m 666 null c 1 3
cd ../
echo '#!/bin/sh
#
# Start mdev....
#

case "$1" in
  start)
        echo "Starting mdev..."
        echo /sbin/mdev >/proc/sys/kernel/hotplug
        /sbin/mdev -s
        ;;
  stop)
        ;;
  restart|reload)
        ;;
  *)
        echo "Usage: $0 {start|stop|restart}"
        exit 1
esac

exit $?' > etc/init.d/S10mdev
chmod 777  etc/init.d/S10mdev
)
