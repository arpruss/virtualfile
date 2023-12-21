#!/bin/bash
rm -rf mnt
mkdir mnt
fusermount -u mnt
echo mounting
python3 memory.py mnt "$1.vbin" &
pid=$!
echo waiting
until [ -f mnt/*.bin ]
do
    sleep 1
done
echo mnt/*.bin
for x in mnt/*.bin ; do
      binfile=$(basename -a "$x")
done
echo ready to run $binfile
rm $binfile
ln -s mnt/$binfile .
shift
echo Running: $*
$*
kill $pid
( sleep 2 ; if [ -e mnt ] ; then fusermount -u mnt ; fi ) &

