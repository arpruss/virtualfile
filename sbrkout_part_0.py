import sys

with open(sys.argv[1], "rb") as f:
    sys.stdout.buffer.write(b'\xFF' * 1024)
    sys.stdout.buffer.write(f.read()[0:1024])