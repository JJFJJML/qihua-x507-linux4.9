#!/bin/bash

OUT_PATH=""
export PATH=${LICHEE_BUILD_DIR}/bin:$PATH

echo "generating rootfs..."

cd ${LICHEE_BUILDROOT_DIR}/output/
NR_SIZE=`du -sm target | awk '{print $1}'`
NEW_NR_SIZE=$((((6262)/16)*16))
#NEW_NR_SIZE=360
TARGET_IMAGE=rootfs.ext4

echo "blocks: $NR_SIZE"M" -> $NEW_NR_SIZE"M""
make_ext4fs -s -l $NEW_NR_SIZE"M" $TARGET_IMAGE target/

echo "success in generating rootfs"
echo "Build at: `date`"
