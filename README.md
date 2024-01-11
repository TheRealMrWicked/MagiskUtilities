# Magisk Utilities
A program that automates various tasks related to rooting/patching Android devices using Magisk.

## Usage
### Arguments
```
-r,  --root               Roots a boot image, a file must be given as an argument.

-sd, --samsung-download   Downloads the latest firmware for a connected samsung device.

-sr, --samsung-root       Downloads the latest firmware for a connected samsung device,
                          patches the recovery and roots the boot image.

-pr, --patch-recovery     Patches the recovery image of a Samsung device, to enable fastbootd.
                          NOTE: Only works with devices launched with Android 10 or higher.

-h,  --help               Displays this help message.

-v,  --version            Displays the version information.
```

### Examples
Root:
```
./magiskutils -r boot.img
```
Samsung Download:
```
./magiskutils -sd
```
Samsung Auto-Root:
```
./magiskutils -sr
```
Patch Recovery:
```
./magiskutils -pr recovery.img
```

### Additional Scripts

There are also shell scripts that can be used to patch boot images, without needing to use the main program,
made for [Windows](https://github.com/TheRealMrWicked/MagiskUtilities/blob/master/scripts/boot_patch.bat) and [Linux](https://github.com/TheRealMrWicked/MagiskUtilities/blob/master/scripts/boot_patch.sh).

Its usage is simply: ./boot_patch <boot.img> [-sar]

These scripts are based on [boot_patch.sh](https://github.com/topjohnwu/Magisk/blob/master/scripts/boot_patch.sh)
made by topjohnwu.

## Documentation
The documentation for this program can be found [here](https://mrwicked.net/docs/Magisk%20Utilities.html).

## Credits
Magisk Utilities written by [MrWicked](https://github.com/TheRealMrWicked).

[Magisk](https://github.com/topjohnwu/Magisk) created by [topjohnwu](https://github.com/topjohnwu).

[Magiskboot For Windows](https://github.com/svoboda18/magiskboot) created by [svoboda18](https://github.com/svoboda18).

[SamFirm.NET](https://github.com/jesec/SamFirm.NET) created by [jesec](https://github.com/jesec).

## Licence
[GNU Public Licence Version 3](https://github.com/TheRealMrWicked/MagiskUtilities/blob/master/LICENSE)