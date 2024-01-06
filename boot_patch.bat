@echo off

if not exist Magisk (
    echo Downloading dependencies, this will only happen once.
    rmdir /Q /S Magisk >NUL 2>NUL
    mkdir Magisk
    cd Magisk || exit /b

    for /f "tokens=1,* delims=:" %%a in ('curl -ks https://api.github.com/repos/topjohnwu/Magisk/releases/latest ^| find "browser_download_url"') do ( curl -kL %%b -o Magisk.zip >NUL 2>NUL)
    tar -xvf Magisk.zip --strip-components=1 assets/boot_patch.sh assets/util_functions.sh assets/stub.apk >NUL 2>NUL
    tar -xvf Magisk.zip --strip-components=2 lib/armeabi-v7a/libmagisk32.so lib/arm64-v8a/libmagisk64.so lib/arm64-v8a/libmagiskinit.so lib/arm64-v8a/libmagiskboot.so >NUL 2>NUL
    move libmagisk32.so magisk32 >NUL 2>NUL
    move libmagisk64.so magisk64 >NUL 2>NUL
    move libmagiskinit.so magiskinit >NUL 2>NUL
    move libmagiskboot.so magiskboot >NUL 2>NUL

    for /f "tokens=1,* delims=:" %%a in ('curl -ks https://api.github.com/repos/svoboda18/magiskboot/releases/latest ^| find "browser_download_url"') do ( curl -kL %%b -o Magiskboot.zip >NUL 2>NUL)
    tar -xf Magiskboot.zip >NUL 2>NUL

    del Magisk.zip >NUL 2>NUL
    del Magiskboot.zip >NUL 2>NUL
    cd..
)

set BOOTIMAGE=%1
set LEGACYSAR=%2

if [%BOOTIMAGE%] == [] (
  echo Boot Patch 1.0
  echo Copyright ^(C^) 2024 MrWicked ^<mrwicked@duck.com^>
  echo.
  echo Please supply a boot image to patch.
  echo Usage: $0 ^<boot.img^> [-sar]
  echo.
  echo Options:
  echo -sar     Use if the device is LegacySAR.
  exit /b
)

if not exist %BOOTIMAGE% (
    echo %BOOTIMAGE% does not exist!
    exit /b
)

adb get-state >NUL 2>NUL

if "%Errorlevel%" == "0" (
    copy %BOOTIMAGE% Magisk >NUL 2>NUL
    adb push Magisk /data/local/tmp/Magisk
    adb shell chmod 555 /data/local/tmp/Magisk/magiskboot
    adb shell chmod 555 /data/local/tmp/Magisk/magisk32
    adb shell chmod 555 /data/local/tmp/Magisk/magisk64
    adb shell sh /data/local/tmp/Magisk/boot_patch.sh %BOOTIMAGE%
    adb pull /data/local/tmp/Magisk/new-boot.img
    adb shell rm -rf /data/local/tmp/Magisk
    move new-boot.img patched-%BOOTIMAGE% >NUL 2>NUL
    del Magisk\%BOOTIMAGE% >NUL 2>NUL
    exit /b
)

mkdir Temp
copy Magisk Temp >NUL 2>NUL

copy %BOOTIMAGE% Temp >NUL 2>NUL
cd Temp || exit /b

echo Unpacking boot image
magiskboot unpack %BOOTIMAGE%

if not "%Errorlevel%" == "0" (
    echo Unable to unpack boot image
    exit /b
)

echo Checking ramdisk status

if exist ramdisk.cpio (
    magiskboot cpio ramdisk.cpio test
    set status=%Errorlevel%
    set skipbackup=
) else (
    set status=0
    set skipbackup=::
)

if [%status%] == [0] (
    for /f %%i in ('magiskboot sha1 %BOOTIMAGE%') do set sha1=%%i
) else (
    echo Unsupported boot image detected.
    exit /b
)

set init=init
if [%status%] == [4] (
    set init=init.real
)

echo Patching ramdisk

magiskboot compress=xz magisk64 magisk64.xz
magiskboot compress=xz magisk32 magisk32.xz
magiskboot compress=xz stub.apk stub.xz

echo KEEPVERITY=false >> config
echo KEEPFORCEENCRYPT=false >> config
echo RECOVERYMODE=false >> config

if not [%sha1%] == [] (
    echo SHA1=%sha1% >> config
)

magiskboot cpio ramdisk.cpio "add 0750 %init% magiskinit"
magiskboot cpio ramdisk.cpio "mkdir 0750 overlay.d"
magiskboot cpio ramdisk.cpio "mkdir 0750 overlay.d/sbin"
magiskboot cpio ramdisk.cpio "add 0644 overlay.d/sbin/magisk32.xz magisk32.xz"
magiskboot cpio ramdisk.cpio "add 0644 overlay.d/sbin/magisk64.xz magisk64.xz"
magiskboot cpio ramdisk.cpio "add 0644 overlay.d/sbin/stub.xz stub.xz"
magiskboot cpio ramdisk.cpio "patch"
magiskboot cpio ramdisk.cpio "mkdir 000 .backup"
magiskboot cpio ramdisk.cpio "add 000 .backup/.magisk config"

del config Magisk.zip magisk32* magisk64* magiskinit stub*

if exist dtb (
    set dt=dtb
) else if exist kernal_dtb (
    set dt=kernal_dtb
) else if exist extra (
    set dt=extra
) else (
    goto Kernel
)

magiskboot dtb %dt% test

if not "%Errorlevel%" == "0" (
    echo "! Boot image %dt% was patched by old (unsupported) Magisk"
    echo "! Please try again with *unpatched* boot image"
    exit /b
) else (
    magiskboot dtb %dt% patch
    if "%Errorlevel%" == "0" (
        echo - Patch fstab in boot image %dt%
)

:Kernel
if exist kernel (
    magiskboot hexpatch kernel 49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054
    magiskboot hexpatch kernel 821B8012 E2FF8F12
    if [%LEGACYSAR%] == [-sar] (
        magiskboot hexpatch kernel 736B69705F696E697472616D667300 77616E745F696E697472616D667300
    )
)

echo Repacking boot image
magiskboot repack %BOOTIMAGE% ../patched-%BOOTIMAGE% || echo ! Unable to repack boot image

cd ..
rmdir /Q /S Temp

exit /b