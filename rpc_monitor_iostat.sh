rm -rf ./iostat.log
cd /home/ssd/tmp/spdk/scripts
./iostat.py -d -i 1 -t 10800 > ../iostat.log