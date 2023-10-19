# 1: blue outside, red inside
# 0: 0x82 inner field ok
def rgb(r,g,b):
   return b|g<<1|r<<2

# 0: outer field and face
# 1: border
# 2: inner field
# 3: text
# 12: ball

data = [0,]*64
data[0] = rgb(0,0,0)
data[1] = rgb(0,1,0)
data[2] = rgb(0,1,1)
data[16+1] = rgb(0,0,1)
data[16+2] = rgb(1,0,1)
data[32+1] = rgb(1,0,1)
data[32+2] = rgb(0,1,0)
data[48+1] = rgb(0,1,1)
data[48+2] = rgb(0,0,1)

for i in range(0,64,16):
   data[i+3] = rgb(1,1,1)
   data[i+4] = data[i+2] #paddle
   data[i+5] = rgb(1,0,0)
   data[i+6] = rgb(1,0,0)
   data[i+7] = rgb(1,0,0)
   data[i+8] = rgb(1,0,0)
   data[i+9] = rgb(1,0,0)
   data[i+10] = rgb(1,0,0)
   data[i+11] = rgb(1,1,0) # C and P and date
   data[i+12] = rgb(1,1,0) #data[i+2] # ball
   data[i+13] = rgb(1,0,0)
   data[i+14] = rgb(1,1,1)
   data[i+15] = rgb(1,1,1)




#data[32+0] = rgb(0,0,0)
#data[32+1] = rgb(0,1,0)

#data[48+0] = rgb(0,1,0)
#data[48+1] = rgb(0,1,1)

print(data)
with open("warlord.clr","wb") as f:
 f.write(bytearray(data))
 f.write(bytearray(data))
 f.write(bytearray(data))
 f.write(bytearray(data))
