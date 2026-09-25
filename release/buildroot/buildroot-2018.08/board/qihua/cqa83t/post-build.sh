#!/bin/sh

BOARD_DIR="$(dirname $0)"
MKIMAGE=$HOST_DIR/usr/bin/mkimage
BOOT_CMD=$BOARD_DIR/boot.cmd
BOOT_CMD_H=$BINARIES_DIR/boot.scr


(

cd 	output/target/
if [ ! -f init ]; then
	ln -s bin/busybox init
fi
cd dev/
echo -e "\033[30;107mCreate null device . \033[0m"
mknod -m 666 null c 1 3
cd ../

mkdir sdcard/
mkdir udisk/

echo '#!/bin/sh
mount /dev/mmcblk[1-9]p[0-9] /sdcard' > etc/sdcard_insert
chmod 777 etc/sdcard_insert

echo '#!/bin/sh
umount /sdcard' > etc/sdcard_remove
chmod 777 etc/sdcard_remove

echo '#!/bin/sh
mount /dev/sd[a-z][0-9] /udisk' > etc/udisk_insert
chmod 777 etc/udisk_insert

echo '#!/bin/sh
umount /udisk' > etc/udisk_remove
chmod 777 etc/udisk_remove

echo '# sd/mmc
mmcblk[1-9]p[0-9] 0:0 666 @ /etc/sdcard_insert
mmcblk[1-9] 0:0 666 $ /etc/sdcard_remove

# udisk 
sd[a-z][0-9] 0:0 666 @ /etc/udisk_insert
sd[a-z] 0:0 666 $ /etc/udisk_remove' > etc/mdev.conf
)

(
rm -rf ${LICHEE_TOP_DIR}/install.sh
echo "#!/bin/sh" > ${LICHEE_TOP_DIR}/install.sh
echo "PATH=$(pwd)/output/build/qt5base-5.8.0/bin:\$PATH" >> ${LICHEE_TOP_DIR}/install.sh
echo "PATH=$(pwd)/output/host/usr/bin:\$PATH" >> ${LICHEE_TOP_DIR}/install.sh
chmod 777 ${LICHEE_TOP_DIR}/install.sh
if [ -d ${LICHEE_TOP_DIR}/demo ]; then
        cd ${LICHEE_TOP_DIR}/demo/
        ./build.sh
        cd -
fi
)

(
cp ../lib/rootfs/* output/target/ -a
)


