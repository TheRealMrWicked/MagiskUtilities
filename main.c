#include <stdio.h>  // Input/Output Stream
#include <stdbool.h> // Adds the boolean data type
#include <stdlib.h> // Running system commands
#include "utilities.h"

// Utility Functions
void help() {
    printf("MagiskUtils 1.0\n");
    printf("Copyright (C) 2023 MrWicked <mrwicked@duck.com>\n");
    printf("\n");
    printf("Options:\n");
    printf("-r, --root                Roots a boot image, file must be given as an argument.\n");
    printf("\n");
    printf("-sd, --samsung-download   Downloads the latest firmware for a connected samsung device.\n");
    printf("\n");
    printf("-pr, --patch-recovery     Patches the recovery image of a Samsung device, to enable fastbootd.\n");
    printf("                          NOTE: Only works with devices launched with Android 10 or higher.\n");
    printf("\n");
    printf("-sr, --samsung-root       Downloads the latest firmware for a connected samsung device,\n");
    printf("                          patches the recovery and roots the boot image.\n");
    printf("\n");
    printf("-h,  --help               Displays this help message.\n");
    printf("\n");
    printf("-v,  --version            Displays the version information.\n");
}

int runcommand(char command[]) {
    char prefix[50];

    strcat(prefix, "..");
    strcat(prefix, os());
    strcat(prefix, "Tools");
    strcat(prefix, os());
    strcat(prefix, command);

    printf("%s", prefix);

    int result = system(prefix);
    return result;
}

char* decompress(char file[]){

    char* extension = getextension(file);

    if (strcmp(extension, "lz4") == 0) {
        char command[40] = "magiskboot decompress ";
        strcat(command, file);
        runcommand(command);
        char* filename = rmvfileext(file);
        return filename;
    } else if (strcmp(extension, "img") == 0){
        return file;
    } else {
        printf("This file type is not supported.");
        exit(1);
    }
}

void existcheck(int argc, char* argv){
    if (argc < 3) {
        printf("Please supply a .img or .lz4 file.");
        exit(1);
    } else if (filexists(argv) == 1) {
        printf("That file does not exist");
        exit(1);
    }
}

// Main Functions
void dependencies() {
    // ADB
    if (strcmp(os(), "\\") == 0 && system("adb >nul 2>&1") == 0) {
        system("winget install Google.PlatformTools");
        printf("Adb has been installed please re launch this program to continue.");
        system("exit");
    } else if (strcmp(os(), "/") == 0) {
        system("sudo apt install adb");
    }

    mkdir("Tools");
    chdir("Tools");

    // Samfirm (Samsung Firmware Downloader)
    if (strcmp(os(), "\\") == 0 && filexists("samfirm.exe") == 0) {
        system("for /f \"tokens=1,* delims=:\" %a in ('curl -s https://api.github.com/repos/ananjaser1211/SamFirm.NET/releases/latest ^| find \"browser_download_url\" ^| find \"win-x64-single.exe\"') do (curl -kL %b -o samfirm.exe)");
    } else if (strcmp(os(), "/") == 0 && filexists("samfirm") == 0) {
        system("curl -s https://api.github.com/repos/ananjaser1211/SamFirm.NET/releases/latest | grep \"browser_download_url\" | grep \"linux\" | cut -d : -f 2,3 | tr -d \\\\\" | wget -qi - -O samfirm");
    }

    // Magiskboot for Windows
    if (strcmp(os(), "\\") == 0 && filexists("magiskboot.exe") == 0) {
        system("for /f \"tokens=1,* delims=:\" %a in ('curl -ks https://api.github.com/repos/svoboda18/magiskboot/releases/latest ^| find \"browser_download_url\" ^| find \"magiskboot.zip\"') do (curl -kL %b -o magiskboot.zip)");
        system("tar -xf magiskboot.zip");
        remove("magiskboot.zip");
    }

     chdir("..");

     mkdir("Magisk");
     chdir("Magisk");

     //Magisk (Files needed for Rooting)
     if (strcmp(os(), "\\") == 0 && filexists("boot_patch.sh") == 0) {
         system("for /f \"tokens=1,* delims=:\" %a in ('curl -s https://api.github.com/repos/topjohnwu/Magisk/releases/latest ^| find \"browser_download_url\"') do (curl -kL %b -o Magisk.zip)");
         mkdir("temp");
         system("tar -xf Magisk.zip -C temp");
     } else if (strcmp(os(), "/") == 0 && filexists("samfirm") == 0) {
         system("curl -s https://api.github.com/repos/topjohnwu/Magisk/releases/latest | grep \"browser_download_url\" | cut -d : -f 2,3 | tr -d \\\\\" | wget -qi - -O Magisk.zip");
         mkdir("temp");
         system("unzip -oq Magisk.zip -d temp");
     }

     if (filexists("boot_patch.sh") == 0) {
         rename("temp/assets/boot_patch.sh", "boot_patch.sh");
         rename("temp/assets/util_functions.sh", "util_functions.sh");
         rename("temp/assets/stub.apk", "stub.apk");
         rename("temp/lib/armeabi-v7a/libmagisk32.so", "magisk32");
         rename("temp/lib/arm64-v8a/libmagisk64.so", "magisk64");
         rename("temp/lib/arm64-v8a/libmagiskinit.so", "magiskinit");
         rename("temp/lib/arm64-v8a/libmagiskboot.so", "magiskboot");
     }

    if (strcmp(os(), "/") == 0 && filexists("samfirm") == 0) {
        rename("temp/lib/x86_64/libmagiskboot.so", "../Tools/magiskboot");
    }

    removefolder("temp");
    remove("Magisk.zip");
    chdir("..");
}

void adbroot(char bootimage[]) {
    rename(bootimage, "Magisk/boot.img");
    system("adb push Magisk /data/local/tmp/Magisk");
    system("adb shell chmod 555 /data/local/tmp/Magisk/magiskboot");
    system("adb shell chmod 555 /data/local/tmp/Magisk/magisk32");
    system("adb shell chmod 555 /data/local/tmp/Magisk/magisk64");
    system("adb shell sh /data/local/tmp/Magisk/boot_patch.sh boot.img");
    system("adb pull /data/local/tmp/Magisk/new-boot.img");
    system("adb shell rm -rf /data/local/tmp/Magisk");
    rename("Magisk/boot.img", "boot.img");
    rename("new-boot.img", "patched-boot.img");
}

void localroot(char bootimage[]) {
    removefolder("temp");
    copyfolder("Magisk", "temp");
    chdir("temp");

    char keepverity[5] = "true";
    char keepforceencrypt[5] = "true";
    char patchvbmetaflag[5] = "false";
    char recoverymode[5] = "false";
    bool legacysar = false;

    //////////
    // Unpack
    //////////

    printf("Unpacking boot image\n");
    int result = runcommand("magiskboot unpack ..\\boot.img");

    if (result != 0) {
        printf("! Unsupported/Unknown image format");
        exit(1);
    }

    ////////////////////
    // Ramdisk Restores
    ////////////////////

    int status;
    bool skipbackup;
    char *init;
    // Test patch status and do restore
    printf("Checking ramdisk status");

    if (filexists("ramdisk.cpio") != 0) {
        status = runcommand( "magiskboot cpio ramdisk.cpio test");
        skipbackup = false;
    } else {
        // Stock A only legacy SAR, or some Android 13 GKIs
        status = 0;
        skipbackup = true;
    }

    if (status == 0) {
        printf("- Stock boot image detected");
    } else if (status == 1) {
        printf("- Magisk patched boot image detected");
        runcommand( "magiskboot cpio ramdisk.cpio");
        runcommand( "magiskboot cpio ramdisk.cpio 'extract .backup/.magisk config.orig'");
        runcommand( "magiskboot cpio ramdisk.cpio 'restore'");
    } else if (status == 2) {
        printf("! Boot image patched by unsupported programs");
        printf("! Please restore back to stock boot image");
        exit(1);
    }

    // Workaround custom legacy Sony /init -> /(s)bin/init_sony : /init.real setup
    init = "init";
    if (status == 4) {
        init = "init.real";
    }

    ///////////////////
    // Ramdisk Patches
    ///////////////////

    bool skip32 = true;
    bool skip64 = true;

    printf("Patching ramdisk");

    // Compress to save precious ramdisk space

    if (filexists("magisk64") == 1) {
        runcommand( "magiskboot compress=xz magisk64 magisk64.xz");
        skip64 = false;
    }

    if (filexists("magisk32") == 1) {
        runcommand( "magiskboot compress=xz magisk32 magisk32.xz");
        skip32 = false;
    }

    runcommand( "magiskboot compress=xz stub.apk stub.xz");


    runcommand( "magiskboot cpio ramdisk.cpio");
    runcommand( "magiskboot cpio ramdisk.cpio");
    runcommand( "magiskboot cpio ramdisk.cpio 'mkdir 0750 overlay.d'");
    runcommand( "magiskboot cpio ramdisk.cpio 'mkdir 0750 overlay.d/sbin'");

    if (skip32 == 0) {
        runcommand( "magiskboot cpio ramdisk.cpio 'add 0644 overlay.d/sbin/magisk32.xz magisk32.xz'");
    }

    if (skip64 == 0) {
        runcommand( "magiskboot cpio ramdisk.cpio 'add 0644 overlay.d/sbin/magisk64.xz magisk64.xz'");
    }

    runcommand( "magiskboot cpio ramdisk.cpio 'add 0644 overlay.d/sbin/stub.xz stub.xz'");
    runcommand( "magiskboot cpio ramdisk.cpio 'patch'");

    if (skipbackup == 0) {
        runcommand( "magiskboot cpio ramdisk.cpio 'backup ramdisk.cpio.orig'");
    }

    runcommand( "magiskboot cpio ramdisk.cpio 'backup ramdisk.cpio.orig'");
    runcommand( "magiskboot cpio ramdisk.cpio 'mkdir 000 .backup'");
    runcommand( "magiskboot cpio ramdisk.cpio 'add 000 .backup/.magisk config'");

    remove("ramdisk.cpio.orig");
    remove("config");
    remove("magisk32.xz");
    remove("magisk64.xz");
    remove("stub.xz");

    //////////////////
    // Binary Patches
    //////////////////

    char *dt = NULL;
    bool patchedkernel;

    if (filexists("dtb") == 1) {
        dt = "dtb";
    } else if (filexists("kernel_dtb") == 1) {
        dt = "kernel_dtb";
    } else if (filexists("extra") == 1) {
        dt = "extra";
    }

    if (filexists(dt) == 1) {
        exit(0);
    }

    if (filexists("kernel") == 1) {
        patchedkernel = false;
        // Remove Samsung RKP
        runcommand( "magiskboot hexpatch kernel 49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054");
        patchedkernel = true;

        // Remove Samsung defex
        // Before: [mov w2, #-221]   (-__NR_execve)
        // After:  [mov w2, #-32768]
        runcommand( "magiskboot hexpatch kernel 821B8012 E2FF8F12");
        patchedkernel = true;

        // Force kernel to load rootfs for legacy SAR devices
        // skip_initramfs -> want_initramfs
        if (legacysar == 1) {
            runcommand( "magiskboot hexpatch kernel 736B69705F696E697472616D667300 77616E745F696E697472616D667300");
            patchedkernel = true;
        }

        // If the kernel doesn't need to be patched at all,
        // keep raw kernel to avoid bootloops on some weird devices
        if (patchedkernel = 0) {
            remove("kernel");
        }

        //////////////////
        // Repack & Flash
        //////////////////

        printf("- Repacking boot image");
        runcommand( "magiskboot repack ../boot.img ../patched-boot.img");

        chdir("..");
        removefolder("temp");
    }

}

void patchrecovery(char recoveryimage[]) {
    rename(recoveryimage, "recovery.img");
    removefolder("temp");
    mkdir("temp");
    chdir("temp");

    runcommand( "magiskboot unpack ../recovery.img");
    runcommand( "magiskboot cpio ramdisk.cpio extract");

    if (strcmp(os(), "\\") == 0) {
        copyfolder("ramdisk", ".");
        removefolder("ramdisk");
    }

    runcommand( "magiskboot hexpatch system/bin/recovery e10313aaf40300aa6ecc009420010034 e10313aaf40300aa6ecc0094");
    runcommand( "magiskboot hexpatch system/bin/recovery eec3009420010034 eec3009420010035");
    runcommand( "magiskboot hexpatch system/bin/recovery 3ad3009420010034 3ad3009420010035");
    runcommand( "magiskboot hexpatch system/bin/recovery 50c0009420010034 50c0009420010035");
    runcommand( "magiskboot hexpatch system/bin/recovery 080109aae80000b4 080109aae80000b5");
    runcommand( "magiskboot hexpatch system/bin/recovery 20f0a6ef38b1681c 20f0a6ef38b9681c");
    runcommand( "magiskboot hexpatch system/bin/recovery 23f03aed38b1681c 23f03aed38b9681c");
    runcommand( "magiskboot hexpatch system/bin/recovery 0f09eef38b1681c 20f09eef38b9681c");
    runcommand( "magiskboot hexpatch system/bin/recovery 26f0ceec30b1681c 26f0ceec30b9681c");
    runcommand( "magiskboot hexpatch system/bin/recovery 24f0fcee30b1681c 24f0fcee30b9681c");
    runcommand( "magiskboot hexpatch system/bin/recovery 27f02eeb30b1681c 27f02eeb30b9681c");
    runcommand( "magiskboot hexpatch system/bin/recovery b4f082ee28b1701c b4f082ee28b970c1");
    runcommand( "magiskboot cpio ramdisk.cpio 'add 0755 system/bin/recovery system/bin/recovery'");
    runcommand( "magiskboot repack ../recovery.img ../patched-recovery.img");

    chdir("..");
    removefolder("temp");
}

int samsungdownload() {
    if (strcmp(os(), "\\") == 0 && system("adb get-state >nul 2>&1") == 1) {
        printf("The phone is not connected over ADB, please ensure the phone is connected and usb debugging is enabled.");
        exit(1);
    } else if (strcmp(os(), "/") == 0 && system("adb get-state >/dev/null 2>&1") == 1) {
        printf("The phone is not connected over ADB, please ensure the phone is connected and usb debugging is enabled.");
        exit(1);
    }

    char *model = commandoutput("adb shell getprop ro.product.model");
    char *region = commandoutput("adb shell getprop ril.matchedcsc");

    if (strcmp(region, "") == 0) {
        printf("This program only works with Samsung devices.");
        exit(1);
    }

    char *serialnum = commandoutput("adb get-serialno");
    char *firstapi = commandoutput("adb shell getprop ro.product.first_api_level");

    char command[100];
    strcat(command, "Tools");
    strcat(command, os());
    strcat(command, "samfirm -m ");
    strcat(command, model);
    strcat(command, " -r ");
    strcat(command, region);
    strcat(command, " -i ");
    strcat(command, serialnum);
    printf("%s", command);

    system(command);
    wildcardrename("SM*", "Firmware");

    free(model); free(region); free(serialnum); free(firstapi);
    return 0;
}

void samsungroot() {
    char* file;

    removefolder("temp");
    samsungdownload();
    wildcardrename("SM*", "Firmware");

    system("tar -xf Firmware/AP.tar boot.img.lz4 init_boot.img.lz4");

    if (filexists("boot.img.lz4") == 0) {
        file = decompress("boot.img.lz4");
        adbroot(file);
        remove("boot.img");
        rename("patched-boot.img", "boot.img");
    }

    if (filexists("init_boot.img.lz4") == 0) {
        file = decompress("init_boot.img.lz4");
        adbroot(file);
        remove("init_boot.img");
        rename("patched-boot.img", "init_boot.img");
    }

    if (filexists("boot.img.lz4") == 0) {
        file = decompress("recovery.img.lz4");
        patchrecovery(file);
        remove("recovery.img");
        rename("patched-recovery.img", "recovery.img");
    }

    system("tar -jcvf Firmware/AP_Magisk.tar boot.img init_boot.img");

    remove("boot.img");
    remove("init_boot.img");
    remove("recovery.img");
}

int main(int argc, char**argv) {
    char* file;
    dependencies();

    if (argc < 2 || strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        help();

    } else if ((strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) {
        printf("MagiskUtils Version 1.0");

    } else if ((strcmp(argv[1], "-sd") == 0 || strcmp(argv[1], "--samsung-download") == 0)) {
        samsungdownload();
        printf("The firmware has been successfully downloaded and is located in the \"Firmware\" folder.");

    } else if ((strcmp(argv[1], "-pr") == 0 || strcmp(argv[1], "--patch-recovery") == 0)) {
        existcheck(argc, argv[2]);
        file = decompress(argv[2]);
        patchrecovery(file);
        printf("The patched recovery file has been created.");

    } else if ((strcmp(argv[1], "-sr") == 0 || strcmp(argv[1], "--samsung-root") == 0)) {
        samsungroot();
        printf("The firmware has been successfully downloaded and is located in the \"Firmware\" folder. An additional file called \"AP_Magisk.tar\" has also been created which contains the patched/rooted files.");

    } else if ((strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--root") == 0)) {
        existcheck(argc, argv[2]);
        file = decompress(argv[2]);

        if (system("adb >nul 2>&1") == 1) {
            printf("Device detected, patching via ADB.");
            adbroot(file);
        } else {
            printf("Device is not connected via ADB, falling back to local method.");
            localroot(file);
        }
    } else {
        help();
    }
    return(0);
}