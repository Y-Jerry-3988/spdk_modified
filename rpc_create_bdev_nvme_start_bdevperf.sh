cd /home/ssd/tmp/spdk/scripts
./rpc.py bdev_nvme_attach_controller -b "nvme0" -t "pcie" -a 0000:65:00.0
./rpc.py bdev_nvme_attach_controller -b "nvme1" -t "pcie" -a 0000:e9:00.0
./rpc.py bdev_nvme_attach_controller -b "nvme2" -t "pcie" -a 0000:e7:00.0
./rpc.py bdev_get_bdevs
cd /home/ssd/tmp/spdk/examples/bdev/bdevperf
./bdevperf.py perform_tests
