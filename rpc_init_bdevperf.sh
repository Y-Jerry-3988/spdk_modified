cd /home/ssd/tmp/spdk/scripts
CLEAR_HUGE=yes HUGEMEM=8192 PCI_ALLOWED="0000:65:00.0 0000:e7:00.0 0000:e9:00.0" ./setup.sh
./setup.sh status
cd /home/ssd/tmp/spdk/build/examples
./bdevperf -s 1024 -t 20 -m [6,7,8,9] -j ./single_bdev.job -l -z