echo 0000:00:03.0 > /sys/module/e1000/drivers/pci\:e1000/unbind
cd ledmon
make
./ledmon -s 00:03.0 -L