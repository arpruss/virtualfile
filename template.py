import os
import zlib
import struct
import io
import subprocess
import ast
from datetime import datetime

def filter_neogeo_sfix(data):
    out = bytearray(len(data)//32*32)
    for i in range(0,len(out),32):
        out[i+0x00] = data[i+0x02]
        out[i+0x01] = data[i+0x06]
        out[i+0x02] = data[i+0x0A]
        out[i+0x03] = data[i+0x0E]
        out[i+0x04] = data[i+0x12]
        out[i+0x05] = data[i+0x16]
        out[i+0x06] = data[i+0x1A]
        out[i+0x07] = data[i+0x1E]
        out[i+0x08] = data[i+0x03]
        out[i+0x09] = data[i+0x07]
        out[i+0x0A] = data[i+0x0B]
        out[i+0x0B] = data[i+0x0F]
        out[i+0x0C] = data[i+0x13]
        out[i+0x0D] = data[i+0x17]
        out[i+0x0E] = data[i+0x1B]
        out[i+0x0F] = data[i+0x1F]
        out[i+0x10] = data[i+0x00]
        out[i+0x11] = data[i+0x04]
        out[i+0x12] = data[i+0x08]
        out[i+0x13] = data[i+0x0C]
        out[i+0x14] = data[i+0x10]
        out[i+0x15] = data[i+0x14]
        out[i+0x16] = data[i+0x18]
        out[i+0x17] = data[i+0x1C]
        out[i+0x18] = data[i+0x01]
        out[i+0x19] = data[i+0x05]
        out[i+0x1A] = data[i+0x09]
        out[i+0x1B] = data[i+0x0D]
        out[i+0x1C] = data[i+0x11]
        out[i+0x1D] = data[i+0x15]
        out[i+0x1E] = data[i+0x19]
        out[i+0x1F] = data[i+0x1D]
    return out
    
def filter_niblo(data):
    return bytearray((x & 0xF for x in data))
        
def filter_nibhi(data):
    return bytearray((x >> 4 for x in data))
    
def filter_none(data):
    return data
        
filters = { None: filter_none, "neogeo_sfix": filter_neogeo_sfix, "niblo": filter_niblo, "nibhi": filter_nibhi, }
        
def getItemHelper(read, length, key):
    if isinstance(key, slice):
        start,stop,step = key.indices(length)
        assert step == 1
        return read(start, stop-start)
    else:
        return read(key, 1)[0]

class FileChunk(object):
    def __init__(self, inPath, outPath=None, offset=0, spacing=1, length=None, cache=False, filter=None, chunkFiles=None):
        if inPath == "":
            inPath = FileChunk.prevInPath
        if offset == "":
            offset = FileChunk.prevOffset + FileChunk.prevLength * FileChunk.prevSpacing
        if length == "":
            length = FileChunk.prevLength
        if spacing == "":
            spacing = FileChunk.prevSpacing
        if filter == "":
            filter = FileChunk.prevFilter
        
        if chunkFiles is None:
            chunkFiles = {}
        self.chunkFiles = chunkFiles
        if (inPath,filter) in chunkFiles:
            size = chunkFiles[inPath,filter][0]
            self.fd = chunkFiles[inPath,filter][1]
        else:
            size,self.fd = FileChunk.openInPath(inPath, filter, cache)
            chunkFiles[inPath,filter] = (size, self.fd)
        if offset > size:
            self.offset = 0
            self.length = 0
        elif length is None or offset + spacing * (length - 1) + 1 > size:
            self.length = ( (size - offset - 1) // spacing ) + 1
            self.offset = offset
        else:
            self.length = length
            self.offset = offset
        self.spacing = spacing
        self.outPath = outPath
        
        now = datetime.now() ## TODO: fix timezone
        self.date = (now.date().day) | (now.date().month << 5) | ((now.date().year-1980)<<9)
        self.time = (now.time().second//2) | (now.time().minute<<5) | (now.time().hour<<11)
        self.zipShortHeader = b''
        self.zipChunkLength = self.length
        
        FileChunk.prevInPath = inPath
        FileChunk.prevOffset = offset
        FileChunk.prevLength = length
        FileChunk.prevSpacing = spacing
        FileChunk.prevFilter = filter
        
    @classmethod
    def openInPath(cls, path, filter, cache):
        apply = filters[filter]
        if path.startswith("pipe:"):
            cmd = path[5:]
            p = subprocess.Popen(cmd, shell=True, stdin=subprocess.DEVNULL, stderr=subprocess.DEVNULL, stdout=subprocess.PIPE, universal_newlines=False)
            data = p.stdout.read()
            #return len(data),io.BytesIO(apply(data))
            return len(data),bytearray(apply(data))
        elif path.startswith("zero:"):
            n = ast.literal_eval(path[5:])
            return n,bytearray(n)
        else:
            if path.startswith("file:"):
                path = path[5:]
            size = os.path.getsize(path)
            fd = open(path, "rb")
            if cache or filter is not None:
                cached = bytearray(apply(fd.read(size)))
                #cached = io.BytesIO(apply(fd.read(size)))
                fd.close()
                size = len(cached)
                return size,cached
            return size, fd
        
    def __len__(self):
        return self.length
        
    def __getitem__(self, key):
        return getItemHelper(self.read, len(self), key)
        
    def read(self, pos, count):
        if pos >= self.length:
            return b''
        if pos + count > self.length:
            count = self.length - pos
        if isinstance(self.fd,bytearray):
            return self.fd[self.offset+self.spacing*pos:self.offset+self.spacing*(pos+count):self.spacing]
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
                    fd = self.chunkFiles[path][1]
                    if not isinstance(fd,bytearray):
                        fd.close()
                except:
                    pass
            self.chunkFiles.clear()
            
    def zipLongHeader(self,zipPos):
        return b''

class ZipChunk(FileChunk):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

        self.crc32 = zlib.crc32(self.read(0, self.length))
        p = bytes(self.outPath, 'ascii')
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
            else:
                zipPos += len(chunk.zipShortHeader) + chunk.length
        if count <= 0 or pos + count <= zipPos:
            return data
        if pos < zipPos + len(self.ending):
            toCopy = min(zipPos + len(self.ending) - pos, count)
            start = pos - zipPos
            assert start >= 0
            data += self.ending[start:start+toCopy]
        return data
        
class SegmentedFileTemplate(ZipTemplate):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)        
        self.ending = b''

def FileTemplate(desc,chunkFiles={}):
    outPath = desc["outPath"]
    
    def chunkArgs(data):
        return { "offset":data.get("offset",0), "spacing":data.get("spacing", 1), "length":data.get("length", None), 
            "cache":data.get("cache", False), "filter":data.get("filter", None) }

    if "inPath" in desc:
        return FileChunk(desc["inPath"], outPath,  chunkFiles=chunkFiles, **chunkArgs(desc))
    elif "segmented" in desc:
        return SegmentedFileTemplate(outPath, tuple(FileChunk(c["inPath"], chunkFiles=chunkFiles, **chunkArgs(c)) for c in desc["segmented"] ))
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
    
