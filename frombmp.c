#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define MAX_PLANES 8
#define MAX_SIZE 256

#define FRACTION(num,den)   (0x80000000 | (num) << 27 | (den) << 23)
#define IS_FRACTION(offset) ((offset) & 0x80000000)
#define NUMERATOR(offset)   (((offset) >> 27) & 0x0f)
#define DENOMINATOR(offset) (((offset) >> 23) & 0x0f)
#define OFFSET(offset)      ((offset) & 0x007fffff)

struct Layout
{
	unsigned width,height; /* width and height (in pixels) of chars/sprites */
	unsigned total; /* total numer of chars/sprites in the rom */
	unsigned planes; /* number of bitplanes */
	unsigned planeoffset[MAX_PLANES]; /* start of every bitplane (in bits) */
	unsigned xoffset[MAX_SIZE]; /* position of the bit corresponding to the pixel */
	unsigned yoffset[MAX_SIZE]; /* of the given coordinates */
	unsigned charincrement; /* distance between two consecutive characters/sprites (in bits) */
};

struct rom_info 
{
    char* game;
    unsigned region_index;
    unsigned start;
    unsigned length;
    unsigned mode;
    struct Layout* layout;
    unsigned bmpX;
    unsigned bmpY;
};

#define GFX_CENTIPED 1
#define GFX_CCASTLES 2
#define GFX_MILLIPED 3
#define GFX_SPRINT_SPRITES   4
#define GFX_SPRINT_TILES     5
#define GFX_WARLORDS     6
#define GFX_SKYDIVER     7

static struct Layout sprint_tile_layout =
{
	8, 8,
	64,
	1,
	{ 0 },
	{
		0,1,2,3,4,5,6,7
	},
	{
		0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38
	},
	0x40
};


static struct Layout sprint_car_layout =
{
	16, 8,
	32,
	1,
	{ 0 },
	{
		0x7, 0x6, 0x5, 0x4, 0x3, 0x2, 0x1, 0x0,
		0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};

static struct Layout warlords_charlayout =
{
	8,8,
	FRACTION(1,4),
	2,
	{ FRACTION(1,2), 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static struct Layout ccastles_spritelayout =
{
       8,16, // width,height
	        256,
		        4,
		        { 0x2000*8+0, 0x2000*8+4, 0, 4 },
		        { 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
		        { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
                        8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
        32*8
};


static struct Layout centipede_charlayout =
{
	8,8,
	FRACTION(1,2),
	2,
	{ FRACTION(1,2), 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static struct Layout centipede_spritelayout =
{
	8,16,
	FRACTION(1,2),
	2,
	{ FRACTION(1,2), 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8,
			8*8, 9*8, 10*8, 11*8, 12*8, 13*8, 14*8, 15*8 },
	16*8
};

static struct Layout skydiver_charlayout =
{
    8,8,
    64,
    1,
    { 0 },
    { 7, 6, 5, 4, 15, 14, 13, 12 },
    { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16 },
    8*16
};


static struct Layout skydiver_motionlayout =
{
    16,16,
    32,
    1,
    { 0 },
    { 4, 5, 6, 7, 4 + 0x400*8, 5 + 0x400*8, 6 + 0x400*8, 7 + 0x400*8,
      12, 13, 14, 15, 12 + 0x400*8, 13 + 0x400*8, 14 + 0x400*8, 15 + 0x400*8 },
    { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
      8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
    8*32
};


/*
    char* game;
    unsigned region_index;
    unsigned start;
    unsigned length;
    unsigned mode;
    struct graphics_layout* layout;
    unsigned bmpX;
    unsigned bmpY;
    */

struct rom_info chunks[] = {
    { "centiped", 0, 0, 4096, GFX_CENTIPED, &centipede_spritelayout,  8 },
    { "centiped", 0, 0, 4096, GFX_CENTIPED, &centipede_charlayout, 0 },
    { "milliped", 0, 0, 4096, GFX_MILLIPED, &centipede_spritelayout,  },
    { "milliped", 0, 0, 4096, GFX_MILLIPED, &centipede_charlayout,  },
    { "ccastles", 0, 0, 16384, GFX_CCASTLES, &ccastles_spritelayout, 0 },
    { "warlords", 0, 0, 0x800, GFX_WARLORDS, &warlords_charlayout, 0 },
    { "warlords", 0, 0x200, 0x600, GFX_WARLORDS, &warlords_charlayout, 0 },
    { "skydiver", 0, 0, 0x400, GFX_SKYDIVER, &skydiver_charlayout, 0 },
    { "skydiver", 1, 0, 0x800, GFX_SKYDIVER, &skydiver_motionlayout, 0 },
            
            
    { NULL }
};

unsigned get_region_size(char* game, unsigned index) {
    unsigned size = 0;
    struct rom_info* chunk = chunks;
    while (chunk->game != NULL) {
        if (!strcmp(game, chunk->game) && chunk->region_index == index && chunk->start+chunk->length > size) {
            size = chunk->start + chunk->length;
        }
        chunk++;
    }
    return size;
}

static unsigned get16LE(unsigned char* data) {
	return data[0] | ((unsigned)data[1]<<8);
}

static unsigned get32LE(unsigned char* data) {
	return data[0] | ((unsigned)data[1]<<8) | ((unsigned)data[2]<<16) | ((unsigned)data[3]<<24);
}

static unsigned get_from_bmp(unsigned char* bmp, unsigned x, unsigned y) {
	unsigned off_bits = get32LE(bmp+10);
	unsigned width = get32LE(bmp+14+4);
	unsigned height = get32LE(bmp+14+8);
	unsigned bits = get16LE(bmp+0x1c);
	unsigned offset = width*(height-1-y)+x;

	if (bits == 4) {
		if(offset%2) 
			return bmp[off_bits+offset/2] & 0xF;
		else
			return bmp[off_bits+offset/2] >> 4;
	}
	else {
		return bmp[off_bits+offset];
	}
}

static void encode_layout(unsigned char* out, unsigned char* bmp, unsigned regionSize, struct Layout* layout, unsigned start, unsigned bmpX, unsigned bmpY, unsigned mode) {
	unsigned width = layout->width;
	unsigned height = layout->height;
	unsigned region_length = regionSize * 8;
	unsigned total = IS_FRACTION(layout->total) ? region_length / layout->charincrement * NUMERATOR(layout->total) / DENOMINATOR(layout->total) : layout->total;
	for(unsigned i=0;i<MAX_PLANES;i++)
	{
		int value = layout->planeoffset[i];
		if (IS_FRACTION(value))
			layout->planeoffset[i] = OFFSET(value) + region_length * NUMERATOR(value) / DENOMINATOR(value);
	}
	for(unsigned j=0;j<MAX_SIZE;j++) 
	{
		int value = layout->xoffset[j];
		if (IS_FRACTION(value)) 
			layout->xoffset[j] = OFFSET(value) + region_length * NUMERATOR(value) / DENOMINATOR(value);
		value = layout->yoffset[j];
		if (IS_FRACTION(value)) 
			layout->yoffset[j] = OFFSET(value) + region_length * NUMERATOR(value) / DENOMINATOR(value);
	}

	unsigned char* base = out + start;

	unsigned c;
	unsigned end;

	if (mode == GFX_CENTIPED) {
		c = width == height ? 64 : 0;
		end = 128;
	}
	else if (mode == GFX_MILLIPED) {
		c = width == height ? 0 : 0;
		end = width == height ? 256 : 128;
	}
	else {
		c = 0;
		end = total;
	}

	for (;c<end;c++) {
		for(int x=0;x<width;x++) for(int y=0;y<height;y++) {
			unsigned v;
			if (mode == GFX_CENTIPED) {
				if (width==height) {
					unsigned charNum = c;
                    v = get_from_bmp(bmp, bmpX+y, bmpY+width*charNum+width-1-x);
				}
				else {
					unsigned charNum = 2*c;
					if (charNum > 128)
						charNum -= 127;
                    v = get_from_bmp(bmp, bmpX+y, bmpY+width*charNum+x);
				}
			}
			else if (mode == GFX_MILLIPED) {
				if (width==height) {
					unsigned charNum = c;
					//charNum %= 256;
					unsigned k = charNum / 64;
					if (k!=1 && k !=3)
						continue;
					if (k==1) {
						charNum = c + 64;
						//charNum %= 256;
					}
					else {
						charNum = c;
						//charNum %= 256;
					}
                    v = get_from_bmp(bmp, bmpX+y, bmpY+width*charNum+x);
				}
				else {
					unsigned charNum = 2*c;
					if (charNum > 128)
						charNum -= 127;
                    v = get_from_bmp(bmp, bmpX+y, bmpY+width*charNum+x);
				}
			}
			//else if (mode == GFX_SPRINT_TILES) {
		        	//v = get_from_bmp(bmp, bmpX+y, bmpY+height*c+x);
			//}
			else if (mode == GFX_CCASTLES) {
		        	v = 0x7^(0x7&get_from_bmp(bmp, bmpX+7-(x+4)%8, height*c+bmpY+y)); //ARP
                    //if (v == 0xf) v = 7;
			}
            else if (mode == GFX_WARLORDS) {
                    int delta = start>0 ? (-1536-512) : 0;
                    v = get_from_bmp(bmp, bmpX+x, (bmpY+width*c+y+delta+4096)%4096);
            }
			else if (mode == GFX_SKYDIVER) {
		        	v = get_from_bmp(bmp, bmpX+x, bmpY+width*c+y);
                    //v = v ? 1 : 0;
		        	//v = get_from_bmp(bmp, bmpX+y, bmpY+height*c+x);
			}
			else {
		        	v = get_from_bmp(bmp, bmpX+x, bmpY+width*c+y);
		        	//v = get_from_bmp(bmp, bmpX+y, bmpY+height*c+x);
			}


			for (int plane=0;plane<layout->planes;plane++) {
				unsigned pos = layout->planeoffset[plane]+layout->xoffset[x]+layout->yoffset[y]+layout->charincrement*(c%total);
				if(pos/8+start>=regionSize) {
					continue;
				}

				if (v&(1<<plane)) 
					base[pos/8] |= (1<<(7-pos%8));
				else
					base[pos/8] &= ~(1<<(7-pos%8));
				
			}
		}
	}
}




/*************************************
 *
 *	Graphics layouts: Warlords
 *
 *************************************/

/*
static struct Layout ccastles_spritelayout =
{
       8,16,
	        FRACTION(1,2),
		        3,
		        { 4, FRACTION(1,2)+0, FRACTION(1,2)+4 },
		        { 0, 1, 2, 3, 8+0, 8+1, 8+2, 8+3 },
		        { 0*16, 1*16, 2*16, 3*16, 4*16, 5*16, 6*16, 7*16,
                        8*16, 9*16, 10*16, 11*16, 12*16, 13*16, 14*16, 15*16 },
        32*8
};
*/


/*
struct fake_piece {
    const char* name;
    unsigned size;
    const char* originalFile;
    unsigned originalOffset;
    unsigned originalSize;
    struct DecodeInfo* gfx;
    unsigned mode;
};

struct fake_whole {
    const char* zipname;
    struct fake_piece pieces[];
};    

struct fake_whole bwidow = {
    "bwidow.zip", 
    { 
    { "136017.107", 2048, "Black Widow.bin", 0 },
    { "136017.108", 4096 },
    { "136017.109" },
    { "136017.110" },
    { "136017.101" },
    { "136017.102" },
    { "136017.103" },
    { "136017.104" },
    { "136017.105" },
    { "136017.106" },
    { NULL }
    }
};

struct fake_whole sprint2 = {
	"sprint2.zip",
	{
		{"6290-01.b1", 2048, "Sprint2.bin"},
		{"6290-01.c1"},
		{"6404.d1"},
		{"6405.e1"},
		{"6400-01.m2", 256, "zero"},
		{"6401-01.e2", 32, "zero"},
		{"6396-01.p4", 512, "Sprint2Tiles.bmp", 0, 512, sprint_tile_DecodeInfo, GFX_SPRINT_TILES},
		{"6397-01.r4", 512, "Sprint2Tiles.bmp", 512, 512, sprint_tile_DecodeInfo, GFX_SPRINT_TILES},
		{"6399-01.j6", 512, "Sprint2Sprites.bmp", 0, 512, sprint_car_DecodeInfo, GFX_SPRINT_SPRITES},
		{"6398-01.k6", 512, "Sprint2Sprites.bmp", 512, 512, sprint_car_DecodeInfo, GFX_SPRINT_SPRITES},
		{NULL},
	}
};

struct fake_whole centiped3 = {
    "centiped.zip",
    { 
     {"centiped.307", 2048, "Centipede.bin", 0 },
     {"centiped.308" },
     {"centiped.309" },
     {"centiped.310" },
     {"centiped.211", 2048, "Centipede.bmp", 0, 2048, centiped_DecodeInfo, GFX_CENTIPED},
     {"centiped.212", 2048, "Centipede.bmp", 2048, 2048, centiped_DecodeInfo, GFX_CENTIPED},
     {NULL} //TODO:gfx:http://adb.arcadeitalia.net/dettaglio_mame.php?game_name=centiped3&search_id=
    }
};

struct fake_whole milliped = {
    "milliped.zip",
    { 
     {"milliped.104", 4096, "Millipede.bin", 0 },
     {"milliped.103" },
     {"milliped.102" },
     {"milliped.101" },
     {"milliped.107", 2048, "Millipede.bmp", 0, 2048, milliped_DecodeInfo, GFX_MILLIPED },
     {"milliped.106", 2048, "Millipede.bmp", 2048, 2048, milliped_DecodeInfo, GFX_MILLIPED },
     {NULL} //TODO:gfx:http://adb.arcadeitalia.net/dettaglio_mame.php?game_name=centiped3&search_id=
    }
};

struct fake_whole asteroid = {
    "asteroid.zip",
    {
        { "035145.02", 2048, "Asteroids.bin" },
        { "035144.02" },
        { "035143.02" },
        { "035127.02" },
        { NULL }
    }
};

struct fake_whole ccastles = {
	"ccastles.zip",
	{
		{"ccastles.102", 8192, "Crystal Castles.bin", 0 },
		{"ccastles.101", 8192, "Crystal Castles.bin", 0x2000 },
		{"022-403.bin", 8192, "Crystal Castles.bin", 0x4000 },
		{"022-404.bin", 8192, "Crystal Castles.bin", 0x8000 },
		{"022-405.bin", 8192, "Crystal Castles.bin", 0xA000 },
		//{"ccastles.303", 8192, "Crystal Castles.bin", 16384 },
		//{"ccastles.304", 8192, "Crystal Castles.bin", 24576 },
		//{"ccastles.305", 8192, "Crystal Castles.bin", 32768 },
     {"ccastles.106", 8192, "Crystal Castles.bmp", 0, 8192, ccastles_DecodeInfo, GFX_CCASTLES },
     {"ccastles.107", 8192, "Crystal Castles.bmp", 8192, 8192, ccastles_DecodeInfo, GFX_CCASTLES },
		{NULL}
	}
};

struct fake_whole warlords = {
    "warlords.zip",
    {
    { "037154.1m",    0x0800, "Warlords.bin", 0 },
    { "037153.1k",    0x0800, "Warlords.bin", 0x800 },
    { "037158.1j",    0x0800, "Warlords.bin", 0x1000 },
    { "037157.1h",    0x0800, "Warlords.bin", 0x1800 },
    { "037156.1e",    0x0800, "Warlords.bin", 0x2000 },
    { "037155.1d",    0x0800, "Warlords.bin", 0x2800 },
    { "037159.6e",    0x0800, "Warlords.bmp", 0, 0x800, warlords_DecodeInfo, GFX_WARLORDS },
    { NULL },
    }
};

struct fake_whole astdelux = {
    "astdelux.zip",
    {
        {"036430.02", 2048, "Asteroids Deluxe.bin"},
        {"036431.02"},
        {"036432.02"},
        {"036433.03"},
        {"036800.02"},
        {"036799.01"},
        {NULL}
    }
};

struct fake_whole gravitar = {
    "gravitar.zip",
    {
        {"136010.210", 0x800, "Gravitar.bin" },
        {"136010.207", 0x1000 },
        {"136010.208"},
        {"136010.309"},
        {"136010.301"},
        {"136010.302"},
        {"136010.303"}, 
        {"136010.304"},
        {"136010.305"},
        {"136010.306"},
        {NULL}
    }
};

struct fake_whole llander = {
    "llander.zip",
    {
        {"034599.01", 2048, "Lunar Lander.bin"},
        {"034598.01"},{"034597.01"}, {"034572.02"}, {"034571.02"}, {"034570.01"}, {"034569.02"}, 
        {NULL}
    }
};

struct fake_whole mhavoc = {
    "mhavoc.zip",
    {
        {"136025.210", 0x2000, "Major Havoc.bin", 0, 0x1000 },
        {"", 0x2000, "Major Havoc.bin", 0, 0x1000 },
        {"136025.216", 0x4000, "Major Havoc.bin", 0x1000, 0x4000 },
        {"136025.217"},
        {"136025.215", 0x4000, "Major Havoc alpha banks.bin" },
        {"136025.318"},
        {"136025.106", 0x4000, "Major Havoc vector banks.bin" },
        {"136025.107"},
        {"136025.108", 0x4000, "Major Havoc gamma.bin" },        
    }
};

struct fake_whole missile = {
    "missile.zip",
    {
        {"035820.02", 2048, "Missile Command.bin" },
            {"035821.02"}, {"035822.02"}, {"035823.02"}, {"035824.02"}, {"035825.02"}
    }
};

struct fake_whole redbaron = {
    "redbaron.zip",
    {
        {"037587.01", 4096, "Red Baron.bin", 0, 2048 },
        {"", 4096, "Red Baron.bin", 2*2048, 2048 },
        {"037000.01e", 2048, "Red Baron.bin", 2048 },
        {"036998.01e", 2048, "Red Baron.bin", 3*2048 },
        {"036997.01e"}, {"036996.01e"}, {"036995.01e"}, {"037006.01e"}, {"037007.01e"}
    }
};

struct fake_whole spacduel = {
    "spacduel.zip",
    {
        {"136006.106", 0x800, "Space Duel.bin" },
        {"136006.107", 0x1000 }, {"136006.201"}, {"136006.102"}, {"136006.103"}, {"136006.104"}, {"136006.105"}
    }
};

struct fake_whole tempest3 = {
    "tempest3.zip",
    {
        {"237.002", 4096, "Tempest.bin"},
            {"136.002"}, {"235.002"}, {"134.002"}, {"133.002"}, {"138.002"}
    }
};

struct fake_whole centiped = {
    "centiped.zip",
    {
        {"136001.407", 2048, "Centipede.bin" },
            {"136001.408"},{"136001.409"},{"136001.410"},{"136001.211"},{"136001.212"},{"136001.213"}
    }
};

struct fake_whole* substitutions[] = {
    &bwidow,
    &asteroid,
    &astdelux,
    &gravitar,
    &llander,
    &mhavoc,
    &missile,
    &redbaron,
    &spacduel,
    &tempest3,
    &centiped3,
    &milliped,
    &ccastles,
    &sprint2,
    &warlords,
    NULL
};


*/

int getint(char* p) {
    if (!strncasecmp(p,"0x",2))
        return strtol(p, NULL, 16);
    else
        return atoi(p);
}

void encode_chunk(uint8_t* buf, unsigned size, uint8_t* bmp, struct rom_info* chunk) {
	unsigned rlen = size;

/*	if (mode == GFX_CENTIPED || mode == GFX_MILLIPED) {
		bmpX = 8;
		deltaX = -8;
	}
	else {
		bmpX = 0;
		deltaX = 0;
	} 
    */

    encode_layout(buf, bmp, rlen, chunk->layout, chunk->start, chunk->bmpX, chunk->bmpY, chunk->mode);
	
	if (chunk->mode == GFX_SPRINT_TILES || chunk->mode == GFX_SPRINT_TILES) {
		for (unsigned i = 0 ; i < 512 ; i++) 
			buf[512+i] = buf[i] & 0xF;
		for (unsigned i = 0 ; i < 256 ; i++)
			buf[i] >>= 4;
	}
}

int dump(char* name, unsigned index, uint8_t* bmp, FILE* out) {
    unsigned size = get_region_size(name, index);
    if (size == 0)
        return 0;
    uint8_t* buf = malloc(size);
    if (buf == NULL)
        return -1;
    memset(buf, 0, size);
    struct rom_info* chunk = chunks;
    while (chunk->game != NULL) {
        if (!strcmp(chunk->game, name) && index == chunk->region_index) {
            encode_chunk(buf, size, bmp, chunk);
        }
        chunk++;
    }
    int err = fwrite(buf, 1, size, out);
    fclose(out);
    free(buf);
    return err;
}


int main(int argc, char** argv) {
    char* name = argv[1];
    int region = getint(argv[2]);
    fdopen(dup(fileno(stdout)), "wb");
    FILE* bmpFile = fopen(argv[3],"rb");
    if (bmpFile == NULL) {
        fprintf(stderr, "Error opening bmp\n");
        return 1;
    }
    fseek(bmpFile, 0, SEEK_END);
	unsigned bmpSize = ftell(bmpFile);
	uint8_t* bmp = malloc(bmpSize);
	if (bmp == NULL) 
		return 2;
	rewind(bmpFile);
    fread(bmp, 1, bmpSize, bmpFile);
    fclose(bmpFile);
    return dump(name, region, bmp, stdout);
}
