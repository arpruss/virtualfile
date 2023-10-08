#mkdir mnt
#fusermount -u mnt
echo mounting
python3 memory.py mnt ../$1.vzip &
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
#c:/gog/mame/mame -rompath . $1
/home/pi/scripts/retroarch.sh "$1.zip" fbneo neogeo
kill $pid
fusermount -u mnt

