#!/bin/bash
#mkdir mnt
#fusermount -u mnt
echo mounting
python3 memory.py mnt "$1.vzip" &
pid=$!
echo waiting
until [ -f mnt/$1.zip ]
do
    sleep 1
done
echo ready
ln -s mnt/$1.zip .
#cp mnt/$1.zip .
unzip -lv $1.zip
#mame -rompath . $1
d=`pwd`
if [[ $d == *mame-* ]] ; then
/home/pi/scripts/retroarch.sh "$1.zip" mame2003 mame-libretro
else
/home/pi/scripts/retroarch.sh "$1.zip" fbneo neogeo
fi
#/home/pi/scripts/retroarch-mame.sh "$1.zip"
kill $pid
fusermount -u mnt

