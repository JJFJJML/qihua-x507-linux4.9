#!/bin/sh

BOARD_DIR="$(dirname $0)"
MKIMAGE=$HOST_DIR/usr/bin/mkimage
BOOT_CMD=$BOARD_DIR/boot.cmd
BOOT_CMD_H=$BINARIES_DIR/boot.scr


(

cd 	output/target/
cd dev/
echo -e "\033[30;107mCreate null device . \033[0m"
mknod -m 666 null c 1 3
cd ../

echo '#!/bin/sh
# devtmpfs does not get automounted for initramfs
/bin/mount -t devtmpfs devtmpfs /dev
/bin/mount -t sysfs none /sys
exec 0</dev/console
exec 1>/dev/console
exec 2>/dev/console
exec /sbin/init $*' > init
chmod 777 init

mkdir sdcard/
mkdir udisk/
mv etc/init.d/S40network etc/init.d/S80network

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
rm output/target/etc/resolv.conf
cp ../lib/rootfs/* output/target/ -a
)


