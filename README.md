# Instructions
1. Run the following steps to setup PRUS with jadonk's setup script. (The config-pin steps may be optional here. I dont remember)
```sh
cd /var/lib/cloud9
git clone https://gist.github.com/jadonk/2ecf864e1b3f250bad82c0eae12b7b64
cd 2ecf864e1b3f250bad82c0eae12b7b64
make
echo none > /sys/class/leds/beaglebone\:green\:usr0/trigger
sudo config-pin overlay cape-universala
sudo config-pin p9.30 pruout
sudo make run
```

You'll see USR0 blinking 5 times a second (toggles every 100ms). Now install our code:

2. Clone pru_beacon into /var/lib/cloud9
3. Clone pru_detector int /var/lib/cloud9
4. Disable HDMI: edit /boot/uEnv.txt and uncomment the line under HDMI Disabled so it looks like this (DO NOT DISABLED THE ONE WITH eMMC!):
```
##Beaglebone Black: HDMI (Audio/Video) disabled:
dtb=am335x-boneblack-emmc-overlay.dtb
```
5. Reboot
6. Clone car-server repo.
7. Run setup_pru.sh in car-server
8. Run beacon.js in this repo and call broadcastCode(x), passing a 5 bit number (0-31) to broadcast.

# Wiring List
beacon GND -> P9_1

beacon power -> P9_7  (5V Line)

beacon signal -> P9_30

Beacon Diagram: (Connectors on opposite side of LEDs)
```
-------------------------
| PWR GND SIG           |
|                       |
|                       |
-------------------------
```
detector PWR -> P9_3  (3v3 Line! NOT 5V!!)

detector GND -> P8_1

detector 0 -> P8_39

detector 1 -> P8_41

detector 2 -> P8_43

detector 3 -> P8_45

Detector Diagram: (Connectors on same side as detectors)
```
      --------------------------------      
      |                              |      
FRONT |                              |
      |               PWR GND 0 1 2 3|      
      --------------------------------
 ```
<!--
1. Run the following steps to setup PRUS
```sh
cd /var/lib/cloud9
git clone https://gist.github.com/jadonk/2ecf864e1b3f250bad82c0eae12b7b64
cd 2ecf864e1b3f250bad82c0eae12b7b64
make
echo none > /sys/class/leds/beaglebone\:green\:usr0/trigger
sudo config-pin overlay cape-universala
sudo config-pin p9.30 pruout
sudo make run
```

You'll see USR0 blinking 5 times a second (toggles every 100ms). Modify hello-pru.c as desired
and enjoy!

# Notes

* sudo perl /opt/scripts/device/bone/show-pins.pl -v
* config-pin --help
* Bug tracker: http://bugs.elinux.org/projects/debian-image-releases
* http://elinux.org/EBC_Exercise_30_PRU_via_remoteproc_and_RPMsg
* http://elinux.org/EBC_Exercise_11b_gpio_via_mmap
* https://docs.google.com/presentation/d/1yMuyQwkYKU48LeMYnQj4sspnsbXf9niojWe_jr4BWjw/edit?usp=sharing
* http://processors.wiki.ti.com/images/3/34/Sitara_boot_camp_pru-module1-hw-overview.pdf
* http://processors.wiki.ti.com/index.php/PRU_Assembly_Instructions
* http://theduchy.ualr.edu/?p=996
* http://processors.wiki.ti.com/index.php/PRU_Projects

# Version

Debian Stretch BeagleBoard.org BeagleBone IoT Image

```sh
git:/opt/scripts/:[09ae22ec483e5483c6ae7f0ca7bbbabf864b06af]
eeprom:[A335BNLTBP00yywwBP000000]
dogtag:[BeagleBoard.org Debian Image 2017-06-11]
bootloader:[microSD-(push-button)]:[/dev/mmcblk0]:[U-Boot 2017.05-00002-ga302d6e48b]
kernel:[4.4.68-ti-r108]
nodejs:[v6.11.0]
uboot_overlay_options:[enable_uboot_overlays=1]
uboot_overlay_options:[uboot_overlay_pru=/lib/firmware/AM335X-PRU-RPROC-4-4-TI-00A0.dtbo]
uboot_overlay_options:[enable_uboot_cape_universal=1]
```
-->
