import sys
import ast

with open(sys.argv[1],"rb") as fin:
    offset = ast.literal_eval(sys.argv[2])
    length = ast.literal_eval(sys.argv[3])
    padBefore = ast.literal_eval(sys.argv[4])
    padAfter = ast.literal_eval(sys.argv[5])
    if len(sys.argv) > 6:
        padValue = ast.literal_eval(sys.argv[6])
    else:
        padValue = 0
    if padBefore:
        sys.stdout.buffer.write(bytearray((padValue,)*padBefore))
    fin.seek(offset)
    sys.stdout.buffer.write(fin.read(length))
    if padAfter:
        sys.stdout.buffer.write(bytearray((padValue,)*padAfter))
