make
python memory.py mem $1 &
pid=$!
until [ -f mem/*.zip ]
do
    sleep 0.5
done
#~/win/Downloads/mame/vmame64 -mouse -rompath mem mem/*
~/win/Downloads/mame2003/mame -mouse -rompath mem mem/*
kill $pid
