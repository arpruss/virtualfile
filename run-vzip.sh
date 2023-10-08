#mkdir mnt
fusermount -u mnt
python3 memory.py mnt $1.vzip &
pid=$!
until [ -f mnt/$1.zip ]
do
    ls mnt
    sleep 1
done
ln -s mnt/$1.zip .
c:/gog/mame/mame -rompath . $1
kill $pid
fusermount -u mnt