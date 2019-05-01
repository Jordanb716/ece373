echo 0000:00:03.0 > /sys/module/e1000/drivers/pci\:e1000/unbind
insmod ~/Projects/Ece373/hw3-pci/myCharDev.ko
mknod /dev/myCharDev c 243