import os
import zlib
import struct

class ZipChunkInfo(object):
    def __init__(self, inPath, outPath, offset=0, spacing=1, length=-1, chunkFiles=None):
        if chunkFiles is None:
            chunkFiles = {}
        self.zipPos = zipPos
        self.chunkFiles = chunkFiles
        if inPath in chunkFiles:
            size = chunkFiles[inPath][0]
            self.fd = chunkFiles[inPath][1]
        else:
            size = os.path.getsize(inPath)
            self.fd = open(inPath, "rb")
            chunkFiles[inPath] = (size, self.fd)
        if offset > size:
            self.offset = 0
            self.length = 0
        elif length < 0 or offset + spacing * length > size:
            self.length = size - offset
            self.offset = offset
        else:
            self.length = length
            self.offset = offset
        self.spacing = spacing
        self.outPath = outPath
        self.crc32 = zlib.crc32(self.read(0, self.length))
        p = bytes(self.outPath, 'ascii')
        self.zipShortHeader = struct.pack("<IHHHHHIIIHH",#"<I<H<H<H<H<H<I<I<I<H<H",
            0x04034b50,0x14,0,0,0,0,self.crc32,self.length,self.length,len(p),0)+p
        self.zipChunkLength = len(self.zipShortHeader) + self.length
        
    def zipLongHeader(self,zipPos):
        p = bytes(self.outPath, 'ascii')
        return struct.pack("<IHHHHHHIIIHHHHHII",
            0x02014b50,0x14,0x14,0,0,0,0,self.crc32,self.length,self.length,len(p),0,0,0,0,0,zipPos)+p
            
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

    def closeChunks(self):
        if self.chunkFiles:
            for path in self.chunkFiles:
                try:
                    self.chunkFiles[path][1].close()
                except:
                    pass
                del self.chunkFiles[path]
                
class ZipTemplate(object):
    def __init__(self, chunks):
        self.chunks = chunks
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
            
if __name__ == '__main__':
    chunkFiles = {}
    zipPos = 0
    c1 = ZipChunkInfo("LICENSE", "lic.txt", chunkFiles = chunkFiles)
    c2 = ZipChunkInfo("memory.py", "mem.txt", chunkFiles = chunkFiles)
    zt = ZipTemplate((c1,c2))
    with open("out.zip", "wb") as z:
        z.write(zt.read(0,len(zt)))