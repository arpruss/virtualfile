#!/bin/bash
#mkdir mnt
#fusermount -u mnt
echo mounting
python3 memory.py mnt "$1.vzip" &
pid=$!
echo waiting
until [ -f mnt/*.zip ]
do
    sleep 1
done
echo mnt/*.zip
fbneo=0
for x in mnt/*.zip ; do
   if [ "$x" == "mnt/neogeo.zip" ] ; then
      echo neogeo detected
      fbneo=1
   else
      zipfile=$(basename -a -s .zip "$x")
   fi
done
echo ready to run $zipfile
unzip -lv mnt/$zipfile.zip
if [ $fbneo -eq 1 ] ; then
    echo /home/pi/scripts/retroarch.sh "mnt/$zipfile.zip" fbneo neogeo
else
    echo /home/pi/scripts/retroarch.sh "mnt/$zipfile.zip" mame2003 mame-libretro
#/home/pi/scripts/retroarch-mame.sh "$1.zip"
fi
echo killing mount
kill $pid
( sleep 1 ; if [ -e mnt ] ; then fusermount -u mnt ; fi ) &

