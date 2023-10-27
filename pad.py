import sys
import ast

with open(sys.argv[1],"rb") as fin:
    while len(sys.argv) > 6:
        offset = ast.literal_eval(sys.argv[2])
        length = ast.literal_eval(sys.argv[3])
        padBefore = ast.literal_eval(sys.argv[4])
        padAfter = ast.literal_eval(sys.argv[5])
        padValue = ast.literal_eval(sys.argv[6])
        if padBefore:
            sys.stdout.buffer.write(bytearray((padValue,)*padBefore))
        fin.seek(offset)
        sys.stdout.buffer.write(fin.read(length))
        if padAfter:
            sys.stdout.buffer.write(bytearray((padValue,)*padAfter))
        sys.argv = sys.argv[5:]
