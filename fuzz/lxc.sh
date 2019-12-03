#!/bin/bash

#
# install and run everything in an LXC container
#

set -ex

lxc delete -f tinydtb-fuzz || echo "No previous container"

lxc launch images:ubuntu/18.04 tinydtb-fuzz
lxc config device add tinydtb-fuzz shareddir disk source=`pwd`/.. path=/home/user/tinydtb

lxc exec tinydtb-fuzz -- useradd -m -s /bin/bash -G sudo -p "" user
sleep 5

lxc exec tinydtb-fuzz -- su -l user << EOF
set -ex
sudo apt update
sudo apt install -y --no-install-recommends git mg make wget unzip

# folder is read-only, we need to store our stuff somewhere else
WORKDIR=/tmp/build make -C tinydtb/fuzz setup
WORKDIR=/tmp/build make -C tinydtb/fuzz fuzz
EOF


