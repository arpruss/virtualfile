import sys
import math
from PIL import Image

# in: 
# 2 = white
# 3 = blue
# 4 = green

# out:
# 0 = black
# 1 = white
# 2 = blue
# 3 = green
#              x x x
colorMap = [15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14]

if sys.argv[1] == 'ltsmsbc' or sys.argv[1] == 'ltslsbc':
    for y in range(128):
        value = math.floor( 0.5 + 255 * math.sqrt( 1-((y-63.5)/64)**2 ) )
        if value > 255:
            value = 255
        elif value < 0:
            value = 0
        if sys.argv[1] == 'ltsmsbc':
            sys.stdout.buffer.write(bytes((value>>4,)))
        else:
            sys.stdout.buffer.write(bytes((value&0xF,)))
    sys.stdout.buffer.write(bytes(128))        
elif sys.argv[1] == 'lgsmsbc' or sys.argv[1] == 'lgslsbc':
    for x in range(256):
        theta = (x-127.5)/127.5*math.pi/2
        value = math.floor( 0.5 + (math.sin(theta)+1)/2. * 200 )
        if value >= 200:
            value = 255
        elif value < 0:
            value = 0
        #print("%d,%d,%d" % (x, value, lgsmsb[x]<<4 | lgslsb[x]))
        #continue
        if sys.argv[1] == 'lgsmsbc':
            sys.stdout.buffer.write(bytes((value>>4,)))
        else:
            sys.stdout.buffer.write(bytes((value&0xF,)))
    sys.stdout.buffer.write(bytes(128))        
elif sys.argv[1] == 'ltsmsb':
    sys.stdout.buffer.write(bytearray(ltsmsb))
elif sys.argv[1] == 'ltslsb':
    sys.stdout.buffer.write(bytearray(ltslsb))
elif sys.argv[1] == 'lgsmsb':
    sys.stdout.buffer.write(bytearray(lgsmsb))
elif sys.argv[1] == 'lgslsb':
    sys.stdout.buffer.write(bytearray(lgslsb))
elif sys.argv[1] == 'image':
    image = Image.open("FOCAL_Emulator/Liberator.bmp")
    height = 256
    width = 512
    data = bytearray(0x1000*4)
    for planet in range(2):
        start = planet * 0x2000
        for y in range(128):
            segments = []
            x = 0
            while x < width:
                color = image.getpixel((x,y))&0xF
                x0 = x
                while x < width and color == image.getpixel((x,y))&0xF:
                    x += 1
                segments.append((color,x-1))
            if len(segments)>32:
                segments = segments[:31] + [(segments[31][0],0x1FF),]
            while len(segments)<32:
                prev = 0
                for i in range(len(segments)):
                    if segments[i][1]-prev > 15:
                        color = segments[i][0]
                        x = segments[i][1]
                        segments = segments[:i] + [(color,(prev+x)//2),(color,x)] + segments[i+1:]
                        break
                    prev = segments[i][1]

            for i in range(32):
                color,x = segments[i]
                color = colorMap[color]
                value = (color & 0xF) << 8
                value |= (x >> 1) | (x & 1) << 15
                data[start + 0x1000 + (y<<5) + i] = value >> 8
                data[start + 0x0000 + (y<<5) + i] = value & 0xFF
    sys.stdout.buffer.write(data)
    