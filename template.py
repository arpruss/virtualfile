import os
import zlib
import struct
import io
import subprocess
from datetime import datetime

def getItemHelper(read, length, key):
    if isinstance(key, slice):
        start,stop,step = key.indices(length)
        assert step == 1
        return read(start, stop-start)
    else:
        return read(key, 1)[0]

class FileChunk(object):
    def __init__(self, inPath, outPath, offset=0, spacing=1, length=None, cache=False, chunkFiles=None):
        if chunkFiles is None:
            chunkFiles = {}
        self.chunkFiles = chunkFiles
        if inPath in chunkFiles:
            size = chunkFiles[inPath][0]
            self.fd = chunkFiles[inPath][1]
        else:
            size,self.fd = FileChunk.openInPath(inPath, cache)
            chunkFiles[inPath] = (size, self.fd)
        if offset > size:
            self.offset = 0
            self.length = 0
        elif length is None or offset + spacing * length > size:
            self.length = (size - offset) // spacing
            self.offset = offset
        else:
            self.length = length
            self.offset = offset
        self.spacing = spacing
        self.outPath = outPath
        
    @classmethod
    def openInPath(cls, path, cache):
        if not path.startswith("pipe:"):
            if path.startswith("file:"):
                path = path[5:]
            size = os.path.getsize(path)
            fd = open(path, "rb")
            if cache:
                cached = io.BytesIO(fd.read(size))
                fd.close()
                return size, cached
            return size, fd
        else:
            cmd = path[5:]
            print("start pipe")
            p = subprocess.Popen(cmd, shell=True, stdin=subprocess.DEVNULL, stderr=subprocess.DEVNULL, stdout=subprocess.PIPE, universal_newlines=False)
            print("reading")
            data = p.stdout.read()
            print("done")
            return len(data),io.BytesIO(data)
        
    def __len__(self):
        return self.length
        
    def __getitem__(self, key):
        return getItemHelper(self.read, len(self), key)
        
    def read(self, pos, count):
        if pos >= self.length:
            return b''
        if pos + count >= self.length:
            count = self.length - pos
        self.fd.seek(self.offset + self.spacing * pos)
        if self.spacing == 1:
            return bytearray(self.fd.read(count))
        else:
            data = bytearray()
            for i in range(count):
                data.append(self.fd.read(self.spacing)[0])
            return data

    def close(self):
        if self.chunkFiles:
            for path in self.chunkFiles:
                try:
                    self.chunkFiles[path][1].close()
                except:
                    pass
            self.chunkFiles.clear()

class ZipChunk(FileChunk):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.crc32 = zlib.crc32(self.read(0, self.length))
        p = bytes(self.outPath, 'ascii')
        now = datetime.now() ## TODO: fix timezone
        self.date = (now.date().day) | (now.date().month << 5) | ((now.date().year-1980)<<9)
        self.time = (now.time().second//2) | (now.time().minute<<5) | (now.time().hour<<11)
        self.zipShortHeader = struct.pack("<IHHHHHIIIHH",#"<I<H<H<H<H<H<I<I<I<H<H",
            0x04034b50,0x14,0,0,self.time,self.date,self.crc32,self.length,self.length,len(p),0)+p
        self.zipChunkLength = len(self.zipShortHeader) + self.length
        
    def zipLongHeader(self,zipPos):
        p = bytes(self.outPath, 'ascii')
        return struct.pack("<IHHHHHHIIIHHHHHII",
            0x02014b50,0x14,0x14,0,0,self.time,self.date,self.crc32,self.length,self.length,len(p),0,0,0,0,0,zipPos)+p
                            
class ZipTemplate(object):
    def __init__(self, outPath, chunks):
        self.chunks = chunks
        self.outPath = outPath
        cdir = bytearray()
        zipPos = 0
        for chunk in chunks:
            cdir += chunk.zipLongHeader(zipPos)
            zipPos += chunk.zipChunkLength
        self.cdirPos = zipPos
        self.ending = cdir + struct.pack("<IHHHHIIH",
            0x06054b50, 0, 0, len(self.chunks),len(self.chunks),len(cdir),self.cdirPos,0)
        
    def __len__(self):
        return self.cdirPos + len(self.ending) 
        
    def close(self):
        for chunk in self.chunks:
            chunk.close()
    
    def __getitem__(self, key):
        return getItemHelper(self.read, len(self), key)
        
    def read(self, pos, count):
        data = bytearray()
        zipPos = 0
        for chunk in self.chunks:
            if count <= 0 or pos + count <= zipPos:
                return data
            if pos < zipPos + chunk.zipChunkLength:
                if pos < zipPos + len(chunk.zipShortHeader):
                    toCopy = min(zipPos + len(chunk.zipShortHeader) - pos, count)
                    start = pos - zipPos 
                    assert start >= 0
                    data += chunk.zipShortHeader[start:start+toCopy]
                    pos += toCopy
                    count -= toCopy
                    if count <= 0:
                        return data
                zipPos += len(chunk.zipShortHeader)
                if pos < zipPos + chunk.length:
                    toCopy = min(zipPos + chunk.length - pos, count)
                    start = pos - zipPos # must be non-negative
                    assert start >= 0
                    data += chunk.read(start, toCopy)
                    pos += toCopy
                    count -= toCopy
                    if count <= 0:
                        return data
                zipPos += chunk.length
        if count <= 0 or pos + count <= zipPos:
            return data
        if pos < zipPos + len(self.ending):
            toCopy = min(zipPos + len(self.ending) - pos, count)
            start = pos - zipPos
            assert start >= 0
            data += self.ending[start:start+toCopy]
        return data

def FileTemplate(desc,chunkFiles={}):
    outPath = desc["outPath"]
    def chunkArgs(data):
        return { "offset":data.get("offset",0), "spacing":data.get("spacing", 1), "length":data.get("length", None), "cache":data.get("cache", False) }

    if "inPath" in desc:
        return FileChunk(desc["inPath"], outPath,  chunkFiles=chunkFiles, **chunkArgs(desc))
    else:
        return ZipTemplate(outPath, tuple(ZipChunk(c["inPath"], c["outPath"], chunkFiles=chunkFiles, **chunkArgs(c)) for c in desc["zip"] ))
            
if __name__ == '__main__':
    ft = FileTemplate({
        "outPath": "out.zip",
        "zip": [
            { "outPath":"lic.txt", "inPath":"LICENSE", "offset":0, "spacing":2 },
            { "outPath":"mem.txt", "inPath":"memory.py", "offset":5, "length":7 },
        ]
        })
    with open(ft.outPath, "wb") as z:
        z.write(ft[:])
    ft.close()
    
