make
python memory.py mem $1 &
pid=$!
until [ -f mem/*.zip ]
do
    sleep 0.5
done
~/win/Downloads/mame/vmame64 -rompath mem mem/*
kill $pid
