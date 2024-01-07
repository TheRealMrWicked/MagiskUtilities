#!/bin/bash

################
# Dependencies #
################

if ! [ -e Magisk ]; then
  echo "Downloading dependencies, this will only happen once."
  rm -rf Magisk
  mkdir Magisk
  cd Magisk || exit

  curl -s https://api.github.com/repos/topjohnwu/Magisk/releases/latest | grep "browser_download_url" | cut -d : -f 2,3 | tr -d \"\ | wget -qi - -O Magisk.zip

  unzip -joq Magisk.zip lib/x86_64/libmagiskboot.so && mv libmagiskboot.so magiskboot_x86
  unzip -joq Magisk.zip assets/boot_patch.sh
  unzip -joq Magisk.zip assets/util_functions.sh
  unzip -joq Magisk.zip assets/stub.apk
  unzip -joq Magisk.zip lib/armeabi-v7a/libmagisk32.so && mv libmagisk32.so magisk32
  unzip -joq Magisk.zip lib/arm64-v8a/libmagisk64.so && mv libmagisk64.so magisk64
  unzip -joq Magisk.zip lib/arm64-v8a/libmagiskinit.so && mv libmagiskinit.so magiskinit
  unzip -joq Magisk.zip lib/arm64-v8a/libmagiskboot.so && mv libmagiskboot.so magiskboot

  rm -rf Magisk.zip
  cd ..
fi

BOOTIMAGE="$1"

if [ -z "$BOOTIMAGE" ]; then
  echo "Boot Patch 1.0"
  echo "Copyright (C) 2024 MrWicked <mrwicked@duck.com>"
  echo ""
  echo "Please supply a boot image to patch."
  echo "Usage: $0 <boot.img> [-sar]"
  echo ""
  echo "Options:"
  echo "-sar     Use if the device is LegacySAR."
  exit 1
fi


if ! [ -e "$BOOTIMAGE" ]; then
	echo "$BOOTIMAGE does not exist!"
	exit 1
fi

chmod -R 755 .

if adb get-state > /dev/null; then
  cp "$BOOTIMAGE" Magisk
  adb push Magisk /data/local/tmp/Magisk
  adb shell chmod 555 /data/local/tmp/Magisk/magiskboot
  adb shell chmod 555 /data/local/tmp/Magisk/magisk32
  adb shell chmod 555 /data/local/tmp/Magisk/magisk64
  adb shell sh /data/local/tmp/Magisk/boot_patch.sh "$BOOTIMAGE"
  adb pull /data/local/tmp/Magisk/new-boot.img
  adb shell rm -rf /data/local/tmp/Magisk
  mv new-boot.img patched-"$BOOTIMAGE"
  rm Magisk/"$BOOTIMAGE"
  exit 0
fi

cp -r Magisk Temp
mv "$BOOTIMAGE" Temp

cd Temp || exit 1
mv magiskboot_x86 magiskboot

echo "Unpacking boot image"

if ! ./magiskboot unpack "$BOOTIMAGE"; then
  echo "Unable to unpack boot image"
  exit 1
fi

echo "Checking ramdisk status"

if [ -e ramdisk.cpio ]; then
  ./magiskboot cpio ramdisk.cpio test
  STATUS=$?
else
  STATUS=0
fi

if [ "$STATUS" == 0 ]; then
  SHA1=$(./magiskboot sha1 "$BOOTIMAGE" 2>/dev/null)
else
  echo "Unsupported boot image detected."
  exit 1
fi

INIT=init
if [ $((STATUS & 4)) -ne 0 ]; then
  INIT=init.real
fi

echo "Patching ramdisk"

./magiskboot compress=xz magisk64 magisk64.xz
./magiskboot compress=xz magisk32 magisk32.xz
./magiskboot compress=xz stub.apk stub.xz

echo "KEEPVERITY=false" > config
echo "KEEPFORCEENCRYPT=false" >> config
echo "RECOVERYMODE=false" >> config
[ -n "$SHA1" ] && echo "SHA1=$SHA1" >> config

./magiskboot cpio ramdisk.cpio \
"add 0750 $INIT magiskinit" \
"mkdir 0750 overlay.d" \
"mkdir 0750 overlay.d/sbin" \
"add 0644 overlay.d/sbin/magisk32.xz magisk32.xz" \
"add 0644 overlay.d/sbin/magisk64.xz magisk64.xz" \
"add 0644 overlay.d/sbin/stub.xz stub.xz" \
"patch" \
"mkdir 000 .backup" \
"add 000 .backup/.magisk config"

rm -f ramdisk.cpio.orig config *.xz

for dt in dtb kernel_dtb extra; do
  if [ -f $dt ]; then
    if ! ./magiskboot dtb $dt test; then
      echo "! Boot image $dt was patched by old (unsupported) Magisk"
      echo "! Please try again with *unpatched* boot image"
      exit 1
    fi
    if ./magiskboot dtb $dt patch; then
      echo "Patch fstab in boot image $dt"
    fi
  fi
done

if [ -e kernel ]; then
  ./magiskboot hexpatch kernel \
  49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054 \
  821B8012 E2FF8F12

  if [ "$LEGACYSAR" = "-sar" ]; then
    ./magiskboot hexpatch kernel 736B69705F696E697472616D667300 77616E745F696E697472616D667300
  fi
fi

echo "Repacking boot image"
./magiskboot repack "$BOOTIMAGE" ../patched-"$BOOTIMAGE" || echo "Unable to repack boot image"

cd ..
rm -rf Temp

exit 0