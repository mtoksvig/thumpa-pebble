export PATH=~/pebble-dev/arm-cs-tools/bin:$PATH
./waf configure
./waf build # -vvv
~/pebble-dev/libpebble/p.py load build/thumpa-pebble.pbw
