#include <stdio.h>  // Input/Output Stream
#include <stdlib.h> // System Commands, Malloc, etc
#include <dirent.h> // Reading Directories
#include "utilities.h" // MrWicked's Utility Library

// Utility Functions

void help(char *programName, char *version)
{
    printf("MagiskUtils Version %s\n", version);
    printf("Copyright (C) 2024 MrWicked <mrwicked@duck.com>\n");
    printf("\n");
    printf("Usage: %s [OPTION]\n", programName);
    printf("-r, --root                Roots a boot image, a file must be given as an argument.\n");
    printf("\n");
    printf("-sd, --samsung-download   Downloads the latest firmware for a connected samsung device.\n");
    printf("\n");
    printf("-sr, --samsung-root       Downloads the latest firmware for a connected samsung device,\n");
    printf("                          patches the recovery and roots the boot image.\n");
    printf("\n");
    printf("-pr, --patch-recovery     Patches the recovery image of a Samsung device, to enable fastbootd.\n");
    printf("                          NOTE: Only works with devices launched with Android 10 or higher.\n");
    printf("\n");
    printf("-h,  --help               Displays this help message.\n");
    printf("\n");
    printf("-v,  --version            Displays the version information.\n");
}

int runCommand(char *command)
{
    char *fullCommand = malloc(strlen(command) + 10);
    sprintf(fullCommand, "..%sTools%s%s", os(), os(), command);
    return system(fullCommand);
}

char *fileChecker(int argc, char *imageFile)
{
    if (argc < 3) {
        printf("Please supply a .img or .lz4 file.");
        exit(1);

    } else if (fileExists(imageFile) == 0) {
        printf("The supplied file does not exist.");
        exit(1);

    } else if (strcmp(getExtension(imageFile), "lz4") == 0) {
        char *command = malloc(strlen(imageFile) + 23);
        char *filename = removeFileExtension(imageFile);

        sprintf(command, "magiskboot fileChecker %s", imageFile);
        runCommand(command);
        free(command);
        return filename;

    } else if (strcmp(getExtension(imageFile), "img") == 0) {
        return imageFile;

    } else {
        printf("This file type is not supported.");
        exit(1);
    }
}

// Main Functions

void dependencies()
{
    mkdir("Tools");
    chdir("Tools");

    if (strcmp(os(), "\\") == 0 && supressOutput("adb >nul 2>&1") == 0) {   // ADB
        supressOutput("winget install Google.PlatformTools");
        printf("Adb has been installed please re launch this program to continue.");
        supressOutput("exit");
    } else if (strcmp(os(), "/") == 0) {
        supressOutput("sudo apt install adb");
    }

    if (strcmp(os(), "\\") == 0 && fileExists("samfirm.exe") == 0) {   // Samfirm (Samsung Firmware Downloader)
        supressOutput("for /f \"tokens=1,* delims=:\" %a in ('curl -s https://api.github.com/repos/ananjaser1211/SamFirm.NET/releases/latest ^| find \"browser_download_url\" ^| find \"win-x64-single.exe\"') do (curl -kL %b -o samfirm.exe)");
    } else if (strcmp(os(), "/") == 0 && fileExists("samfirm") == 0) {
        supressOutput("curl -s https://api.github.com/repos/ananjaser1211/SamFirm.NET/releases/latest | grep \"browser_download_url\" | grep \"linux\" | cut -d : -f 2,3 | tr -d \\\\\" | wget -qi - -O samfirm");
    }

    if (strcmp(os(), "\\") == 0 && fileExists("magiskboot.exe") == 0) {   // Magiskboot for Windows
        supressOutput("for /f \"tokens=1,* delims=:\" %a in ('curl -ks https://api.github.com/repos/svoboda18/magiskboot/releases/latest ^| find \"browser_download_url\" ^| find \"magiskboot.zip\"') do (curl -kL %b -o magiskboot.zip)");
        supressOutput("tar -xf magiskboot.zip");
        remove("magiskboot.zip");
    }

    chdir("..");

    mkdir("Magisk");
    chdir("Magisk");
     
    if (strcmp(os(), "\\") == 0 && fileExists("boot_patch.sh") == 0) {
        supressOutput("for /f \"tokens=1,* delims=:\" %a in ('curl -s https://api.github.com/repos/topjohnwu/Magisk/releases/latest ^| find \"browser_download_url\"') do (curl -kL %b -o Magisk.zip)");
         mkdir("Temp");
        supressOutput("tar -xf Magisk.zip -C Temp");
    } else if (strcmp(os(), "/") == 0 && fileExists("boot_patch.sh") == 0) {
        supressOutput("curl -s https://api.github.com/repos/topjohnwu/Magisk/releases/latest | grep \"browser_download_url\" | cut -d : -f 2,3 | tr -d \\\\\" | wget -qi - -O Magisk.zip");
         mkdir("Temp");
        supressOutput("unzip -oq Magisk.zip -d Temp");
     }

    if (fileExists("boot_patch.sh") == 0) {
         rename("Temp/assets/boot_patch.sh", "boot_patch.sh");
         rename("Temp/assets/util_functions.sh", "util_functions.sh");
         rename("Temp/assets/stub.apk", "stub.apk");
         rename("Temp/lib/armeabi-v7a/libmagisk32.so", "magisk32");
         rename("Temp/lib/arm64-v8a/libmagisk64.so", "magisk64");
         rename("Temp/lib/arm64-v8a/libmagiskinit.so", "magiskinit");
         rename("Temp/lib/arm64-v8a/libmagiskboot.so", "magiskboot");
    }

    if (strcmp(os(), "/") == 0 && fileExists("magiskboot") == 0) {
        rename("Temp/lib/x86_64/libmagiskboot.so", "../Tools/magiskboot");
    }

    removeFolder("Temp");
    remove("Magisk.zip");
    chdir("..");
}

void adbRoot(char *bootImage)
{
    char *newName = malloc(strlen(bootImage) + 9);
    rename(bootImage, "Magisk/boot.img");
    system("adb push Magisk /data/local/tmp/Magisk");
    system("adb shell chmod 555 /data/local/tmp/Magisk/magiskboot");
    system("adb shell chmod 555 /data/local/tmp/Magisk/magisk32");
    system("adb shell chmod 555 /data/local/tmp/Magisk/magisk64");
    system("adb shell sh /data/local/tmp/Magisk/boot_patch.sh boot.img");
    system("adb pull /data/local/tmp/Magisk/new-boot.img");
    system("adb shell rm -rf /data/local/tmp/Magisk");
    rename("Magisk/boot.img", bootImage);
    sprintf(newName, "patched-%s",bootImage);
    rename("new-boot.img", newName);
    free(newName);
}

void localRoot(char *bootImage)
{
    removeFolder("Temp");
    copyFolder("Magisk", "Temp");
    rename(bootImage, "boot.img");
    chdir("Temp");

    printf("Unpacking boot image\n");
    int result = runCommand("magiskboot unpack ..\\boot.img");

    if (result != 0) {
        printf("Unsupported/Unknown image format\n");
        exit(1);
    }

    printf("Checking ramdisk status\n");

    int status;
    if (fileExists("ramdisk.cpio") == 1) status = runCommand("magiskboot cpio ramdisk.cpio test");
    else status = 0;

    char *sha1  = malloc(37);
    if (status == 0 || status >= 3) {
        sprintf(sha1, "..%sTools%smagiskboot sha1 ..\\boot.img", os(), os());
        sha1 = commandOutput(sha1);
    } else {
        printf("Unsupported boot image detected.\n");
        exit(1);
    }

    printf("Patching ramdisk\n");

    runCommand("magiskboot compress=xz magisk64 magisk64.xz");
    runCommand("magiskboot compress=xz magisk32 magisk32.xz");
    runCommand("magiskboot compress=xz stub.apk stub.xz");

    FILE *config = fopen("config", "w");

    if (config == NULL) {
        printf("Unable to create Magisk config config.");
        exit(1);
    }

    fprintf(config, "KEEPVERITY=false\n");
    fprintf(config, "KEEPFORCEENCRYPT=false\n");
    fprintf(config, "RECOVERYMODE=false\n");

    if (sha1 != NULL) {
        fprintf(config, "SHA1=%s", sha1);
        free(sha1);
    }


    fclose(config);

    runCommand("magiskboot cpio ramdisk.cpio");
    runCommand("magiskboot cpio ramdisk.cpio \"add 0750 init magiskinit\"");
    runCommand("magiskboot cpio ramdisk.cpio \"mkdir 0750 overlay.d\"");
    runCommand("magiskboot cpio ramdisk.cpio \"mkdir 0750 overlay.d/sbin\"");
    runCommand("magiskboot cpio ramdisk.cpio \"add 0644 overlay.d/sbin/magisk32.xz magisk32.xz\"");
    runCommand("magiskboot cpio ramdisk.cpio \"add 0644 overlay.d/sbin/magisk64.xz magisk64.xz\"");
    runCommand("magiskboot cpio ramdisk.cpio \"add 0644 overlay.d/sbin/stub.xz stub.xz\"");
    runCommand("magiskboot cpio ramdisk.cpio \"patch\"");
    runCommand("magiskboot cpio ramdisk.cpio \"mkdir 000 .backup\"");
    runCommand("magiskboot cpio ramdisk.cpio \"add 000 .backup/.magisk config\"");

    remove("ramdisk.cpio.orig");
    remove("config");
    remove("magisk32.xz");
    remove("magisk64.xz");
    remove("stub.xz");

    char *dt = NULL;

    if (fileExists("dtb") == 1) dt = "dtb";
    else if (fileExists("kernel_dtb") == 1) dt = "kernel_dtb";
    else if (fileExists("extra") == 1) dt = "extra";

    if (fileExists(dt) == 1) {
        char* dtcommands = malloc(strlen(dt) + 22);
        sprintf(dtcommands, "magiskboot dtb $%s test", dt);

        if (runCommand(dtcommands) == 1) {
            printf("The boot image was patched by an old (unsupported) version of Magisk.\n");
            printf("Please try again with an unpatched boot image.\n");
            exit(1);
        }

        sprintf(dtcommands, "magiskboot dtb $%s patch", dt);
        runCommand(dtcommands);

        printf("Patched fstab in boot image.\n");
        free(dtcommands);
    }

    if (fileExists("kernel") == 1) {
        runCommand("magiskboot hexpatch kernel 49010054011440B93FA00F71E9000054010840B93FA00F7189000054001840B91FA00F7188010054 A1020054011440B93FA00F7140020054010840B93FA00F71E0010054001840B91FA00F7181010054");
        runCommand("magiskboot hexpatch kernel 821B8012 E2FF8F12");

        //if (legacySAR == 1) runCommand("magiskboot hexpatch kernel 736B69705F696E697472616D667300 77616E745F696E697472616D667300");

        printf("Repacking boot image");

        char *repackCommand = malloc(strlen(bootImage) + 42);
        sprintf(repackCommand, "magiskboot repack ../boot.img ../patched-%s",bootImage);
        runCommand(repackCommand);
        free(repackCommand);

        chdir("..");
        rename("boot.img", bootImage);
        removeFolder("Temp");
    }
}

void patchRecovery(char *recoveryImage)
{
    rename(recoveryImage, "recovery.img");
    removeFolder("Temp");
    mkdir("Temp");
    chdir("Temp");

    runCommand("magiskboot unpack ../recovery.img");
    runCommand("magiskboot cpio ramdisk.cpio extract");

    if (strcmp(os(), "\\") == 0) {
        copyFolder("ramdisk", ".");
        removeFolder("ramdisk");
    }

    runCommand("magiskboot hexpatch system/bin/recovery e10313aaf40300aa6ecc009420010034 e10313aaf40300aa6ecc0094");
    runCommand("magiskboot hexpatch system/bin/recovery eec3009420010034 eec3009420010035");
    runCommand("magiskboot hexpatch system/bin/recovery 3ad3009420010034 3ad3009420010035");
    runCommand("magiskboot hexpatch system/bin/recovery 50c0009420010034 50c0009420010035");
    runCommand("magiskboot hexpatch system/bin/recovery 080109aae80000b4 080109aae80000b5");
    runCommand("magiskboot hexpatch system/bin/recovery 20f0a6ef38b1681c 20f0a6ef38b9681c");
    runCommand("magiskboot hexpatch system/bin/recovery 23f03aed38b1681c 23f03aed38b9681c");
    runCommand("magiskboot hexpatch system/bin/recovery 0f09eef38b1681c 20f09eef38b9681c");
    runCommand("magiskboot hexpatch system/bin/recovery 26f0ceec30b1681c 26f0ceec30b9681c");
    runCommand("magiskboot hexpatch system/bin/recovery 24f0fcee30b1681c 24f0fcee30b9681c");
    runCommand("magiskboot hexpatch system/bin/recovery 27f02eeb30b1681c 27f02eeb30b9681c");
    runCommand("magiskboot hexpatch system/bin/recovery b4f082ee28b1701c b4f082ee28b970c1");
    runCommand("magiskboot cpio ramdisk.cpio 'add 0755 system/bin/recovery system/bin/recovery'");

    char *repackCommand = malloc(strlen(recoveryImage) + 42);
    sprintf(repackCommand, "magiskboot repack ../recovery.img ../patched-%s", recoveryImage);
    runCommand(repackCommand);
    free(repackCommand);

    chdir("..");
    rename("recovery.img", recoveryImage);
    removeFolder("Temp");
}

int samsungDownload()
{
    if (strcmp(os(), "\\") == 0 && system("adb get-state >nul 2>&1") == 1) {
        printf("The phone is not connected over ADB, please ensure the phone is connected and usb debugging is enabled.");
        exit(1);
    } else if (strcmp(os(), "/") == 0 && system("adb get-state >/dev/null 2>&1") == 1) {
        printf("The phone is not connected over ADB, please ensure the phone is connected and usb debugging is enabled.");
        exit(1);
    }

    char *model = commandOutput("adb shell getprop ro.product.model");
    char *region = commandOutput("adb shell getprop ril.matchedcsc");

    if (strcmp(region, "") == 0) {
        printf("This program only works with Samsung devices.");
        exit(1);
    }

    char *serialnum = commandOutput("adb get-serialno");
    int firstapi = strtol(commandOutput("adb shell getprop ro.product.first_api_level"), NULL, 10);

    char *command = malloc(strlen(model) + strlen(region) + strlen(serialnum) + 26);
    sprintf(command, "Tools%ssamfirm -m %s -r %s -i %s", os(), model, region, serialnum);
    system(command);
    free(command);

    wildcardRename("SM*", "Firmware");

    free (model); free (region); free (serialnum);
    return firstapi;
}

void samsungRoot()
{
    removeFolder("Temp");
    int firstapi = samsungDownload();
    wildcardRename("SM*", "Firmware");

    supressOutput("tar -xf Firmware/AP.tar boot.img.lz4 init_boot.img.lz4");

    if (fileExists("boot.img.lz4") == 1) {
        runCommand("magiskboot fileChecker boot.img.lz4");
        adbRoot("boot.img");
        remove("boot.img");
        rename("patched-boot.img", "boot.img");
    }

    if (fileExists("init_boot.img.lz4") == 1) {
        runCommand("magiskboot fileChecker init_boot.img.lz4");
        adbRoot("init_boot.img");
        remove("init_boot.img");
        rename("patched-init_boot.img", "init_boot.img");
    }

    if (fileExists("recovery.img.lz4") == 1 && firstapi >= 29) {
        runCommand("magiskboot fileChecker recovery.img.lz4");
        adbRoot("recovery.img");
        remove("recovery.img");
        rename("patched-recovery.img", "recovery.img");
    }

    supressOutput("tar -jcvf Firmware/AP_Magisk.tar boot.img init_boot.img recovery.img");

    remove("boot.img");
    remove("init_boot.img");
    remove("recovery.img");
}

int main(int argc, char **argv)
{
    char *imgName;
    char *version = "1.1";

    dependencies();

    if (argc < 2) help(argv[0], version);

    else if ((strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)) printf("MagiskUtils Version %s\n", version);

    else if ((strcmp(argv[1], "-sd") == 0 || strcmp(argv[1], "--samsung-download") == 0)) {
        samsungDownload();
        printf("The firmware has been successfully downloaded and is located in the \"Firmware\" folder.\n");
    }
    else if ((strcmp(argv[1], "-sr") == 0 || strcmp(argv[1], "--samsung-root") == 0)) {
        samsungRoot();
        printf("The firmware has been successfully downloaded and is located in the \"Firmware\" folder. An"
               "additional imgName called \"AP_Magisk.tar\" has also been created which contains the patched/rooted files.\n");
    }
    else if ((strcmp(argv[1], "-pr") == 0 || strcmp(argv[1], "--patch-recovery") == 0)) {
        imgName = fileChecker(argc, argv[2]);
        patchRecovery(imgName);
        printf("The patched recovery imgName has been created.\n");
    }
    else if ((strcmp(argv[1], "-r") == 0 || strcmp(argv[1], "--root") == 0)) {
        imgName = fileChecker(argc, argv[2]);
        if (supressOutput("adb get-state") == 0) {
            printf("Device detected, patching via ADB.\n");
            adbRoot(imgName);
        } else {
            printf("Device is not connected via ADB, falling back to local method.\n");
            localRoot(imgName);
        }
    }
    else help(argv[0], version);
    return(0);
}