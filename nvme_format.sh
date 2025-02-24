sudo umount /dev/nvme4n1
sudo umount /dev/nvme5n1
sudo umount /dev/nvme8n1
sudo nvme format /dev/nvme4n1 -b 4096 --force
sudo nvme format /dev/nvme5n1 -b 4096 --force
sudo nvme format /dev/nvme8n1 -b 4096 --force