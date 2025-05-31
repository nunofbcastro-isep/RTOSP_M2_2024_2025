#!/bin/bash
start=$(date +'%s')
cd linux-5.19.9-moker
make 2>../errors-5.19.9-moker
sudo make modules_install
sudo make install
cd ..
sudo update-grub2
cat errors-5.19.9-moker
echo "Linux kernel compilation and installation took $(($(date +'%s') - $start)) seconds"
