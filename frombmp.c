#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAX_PLANES 8
#define MAX_SIZE 256

#define FRACTION(num,den)   (0x80000000 | (num) << 27 | (den) << 23)
#define IS_FRACTION(offset) ((offset) & 0x80000000)
#define NUMERATOR(offset)   (((offset) >> 27) & 0x0f)
#define DENOMINATOR(offset) (((offset) >> 23) & 0x0f)
#define OFFSET(offset)      ((offset) & 0x007fffff)

struct Layout
{
	unsigned width,height;
	unsigned total;
	unsigned planes;
	unsigned planeoffset[MAX_PLANES];
	unsigned xoffset[MAX_SIZE];
	unsigned yoffset[MAX_SIZE];
	unsigned charincrement;
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

#define GFX_DEFAULT 0
#define GFX_CENTIPED 1
#define GFX_CCASTLES 2
#define GFX_MILLIPED 3
#define GFX_SPRINT   4
#define GFX_SBRKOUT 5
#define GFX_WARLORDS     6
#define GFX_SKYDIVER     7
#define GFX_MONTECAR 8
#define GFX_FIRETRUCK_CAR 9
#define GFX_SUPERBUG_CAR 10
#define GFX_DESTROYER_MAJOR 11
#define GFX_DESTROYER 12
#define GFX_DESTROYER_WAVES 13
#define GFX_DEFAULT_REV 14
#define GFX_MAZEINV 15

static struct Layout canyon_tile_layout =
{
	8, 8,
    64,
    1,
    { 0 },
    {
		0x4, 0x5, 0x6, 0x7, 0xC, 0xD, 0xE, 0xF
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80
};


static struct Layout canyon_sprite_layout =
{
	32, 16,
	4,
	1,
	{ 0 },
	{
		0x007, 0x006, 0x005, 0x004, 0x003, 0x002, 0x001, 0x000,
		0x00F, 0x00E, 0x00D, 0x00C, 0x00B, 0x00A, 0x009, 0x008,
		0x107, 0x106, 0x105, 0x104, 0x103, 0x102, 0x101, 0x100,
		0x10F, 0x10E, 0x10D, 0x10C, 0x10B, 0x10A, 0x109, 0x108
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
		0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0
	},
	0x200
};
static struct Layout sbrkout_charlayout =
{
    8,8,
    64,
    1,
    { 0 },
    { 4, 5, 6, 7, 0x200*8 + 4, 0x200*8 + 5, 0x200*8 + 6, 0x200*8 + 7 },
    { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
    8*8
};


static struct Layout sbrkout_balllayout =
{
    3,3,
    2,
    1,
    { 0 },
    { 0, 1, 2 },
    { 0*8, 1*8, 2*8 },
    3*8
};

static struct Layout montecar_text_layout =
{
    8, 8,   
    64,     
    1,      
    { 0 },  
    {
        0xC, 0xD, 0xE, 0xF, 0x4, 0x5, 0x6, 0x7
    },
    {
        0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
    },
    0x80
};

static struct Layout montecar_car_layout =
{
    32, 32, 
    8,      
    2,      
    { 1, 0 },
    {
        0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
        0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
        0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
        0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E
    },
    {
        0x000, 0x040, 0x080, 0x0C0, 0x100, 0x140, 0x180, 0x1C0,
        0x200, 0x240, 0x280, 0x2C0, 0x300, 0x340, 0x380, 0x3C0,
        0x400, 0x440, 0x480, 0x4C0, 0x500, 0x540, 0x580, 0x5C0,
        0x600, 0x640, 0x680, 0x6C0, 0x700, 0x740, 0x780, 0x7C0
    },
    0x800
};

static struct Layout firetrk_tile_layout =
{
    16, 16,
    64,   
    1,    
    { 0 },
    {
        0x0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7,
        0x8, 0x9, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF
    },
    {
        0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
        0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0
    },
    0x100
};

static struct Layout firetrk_text_layout =
{
    16, 16, 
    32,     
    1,      
    { 0 },  
    {
        0x1C, 0x1D, 0x1E, 0x1F, 0x04, 0x05, 0x06, 0x07,
        0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17
    },
    {
        0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0,
        0x100, 0x120, 0x140, 0x160, 0x180, 0x1A0, 0x1C0, 0x1E0
    },
    0x200
};

static struct Layout firetrk_car_layout1 =
{
    32, 32,
    4,     
    1,     
    { 0 }, 
    {
        0x000, 0x040, 0x080, 0x0C0, 0x100, 0x140, 0x180, 0x1C0,
        0x200, 0x240, 0x280, 0x2C0, 0x300, 0x340, 0x380, 0x3C0,
        0x400, 0x440, 0x480, 0x4C0, 0x500, 0x540, 0x580, 0x5C0,
        0x600, 0x640, 0x680, 0x6C0, 0x700, 0x740, 0x780, 0x7C0
    },
    {
        0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
        0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F,
        0x24, 0x25, 0x26, 0x27, 0x2C, 0x2D, 0x2E, 0x2F,
        0x34, 0x35, 0x36, 0x37, 0x3C, 0x3D, 0x3E, 0x3B
    },
    0x800
};

static struct Layout firetrk_car_layout2 =
{
    32, 32, 
    4,      
    1,      
    { 0 },  
    {
        0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
        0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F,
        0x24, 0x25, 0x26, 0x27, 0x2C, 0x2D, 0x2E, 0x2F,
        0x34, 0x35, 0x36, 0x37, 0x3C, 0x3D, 0x3E, 0x3B
    },
    {
        0x000, 0x040, 0x080, 0x0C0, 0x100, 0x140, 0x180, 0x1C0,
        0x200, 0x240, 0x280, 0x2C0, 0x300, 0x340, 0x380, 0x3C0,
        0x400, 0x440, 0x480, 0x4C0, 0x500, 0x540, 0x580, 0x5C0,
        0x600, 0x640, 0x680, 0x6C0, 0x700, 0x740, 0x780, 0x7C0
    },
    0x800
};

static struct Layout firetrk_trailer_layout =
{
    64, 64,
    8,     
    1,      
    { 0 },  
    {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F
    },
    {
        0x000, 0x040, 0x080, 0x0C0, 0x100, 0x140, 0x180, 0x1C0,
        0x200, 0x240, 0x280, 0x2C0, 0x300, 0x340, 0x380, 0x3C0,
        0x400, 0x440, 0x480, 0x4C0, 0x500, 0x540, 0x580, 0x5C0,
        0x600, 0x640, 0x680, 0x6C0, 0x700, 0x740, 0x780, 0x7C0,
        0x800, 0x840, 0x880, 0x8C0, 0x900, 0x940, 0x980, 0x9C0,
        0xA00, 0xA40, 0xA80, 0xAC0, 0xB00, 0xB40, 0xB80, 0xBC0,
        0xC00, 0xC40, 0xC80, 0xCC0, 0xD00, 0xD40, 0xD80, 0xDC0,
        0xE00, 0xE40, 0xE80, 0xEC0, 0xF00, 0xF40, 0xF80, 0xFC0
    },
    0x1000
};


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
		0xf, 0xe, 0xd, 0xc, 0xb, 0xa, 0x9, 0x8,
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

static struct Layout mazeinv_charlayout =
{
	8,8,
	256,
	4,
	{ FRACTION(1,2), 0 },
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8
};

static struct Layout mazeinv_spritelayout =
{
	8,16,
	256,
	4,
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

static struct Layout superbug_text_layout =
{
    16, 16,
    32,    
    1,     
    { 0 },
    {
        0x0C, 0x0D, 0x0E, 0x0F, 0x14, 0x15, 0x16, 0x17,
        0x1C, 0x1D, 0x1E, 0x1F, 0x04, 0x05, 0x06, 0x07
    },
    {
        0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0,
        0x100, 0x120, 0x140, 0x160, 0x180, 0x1A0, 0x1C0, 0x1E0
    },
    0x200
};

static struct Layout superbug_tile_layout =
{
    16, 16, 
    64,     
    1,      
    { 0 },  
    {
        0x07, 0x06, 0x05, 0x04, 0x0F, 0x0E, 0x0D, 0x0C,
        0x17, 0x16, 0x15, 0x14, 0x1F, 0x1E, 0x1D, 0x1C
    },
    {
        0x000, 0x020, 0x040, 0x060, 0x080, 0x0A0, 0x0C0, 0x0E0,
        0x100, 0x120, 0x140, 0x160, 0x180, 0x1A0, 0x1C0, 0x1E0
    },
    0x200
};


static struct Layout superbug_car_layout1 =
{
    32, 32, 
    4,      
    1,      
    { 0 },  
    {
        0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
        0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00,
        0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
        0x1800, 0x1900, 0x1A00, 0x1B00, 0x1C00, 0x1D00, 0x1E00, 0x1F00
    },
    {
        0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34, 0x3C,
        0x44, 0x4C, 0x54, 0x5C, 0x64, 0x6C, 0x74, 0x7C,
        0x84, 0x8C, 0x94, 0x9C, 0xA4, 0xAC, 0xB4, 0xBC,
        0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC
    },
    0x001
};

static struct Layout superbug_car_layout2 =
{
    32, 32, 
    4,      
    1,      
    { 0 },  
    {
        0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34, 0x3C,
        0x44, 0x4C, 0x54, 0x5C, 0x64, 0x6C, 0x74, 0x7C,
        0x84, 0x8C, 0x94, 0x9C, 0xA4, 0xAC, 0xB4, 0xBC,
        0xC4, 0xCC, 0xD4, 0xDC, 0xE4, 0xEC, 0xF4, 0xFC
    },
    {
        0x0000, 0x0100, 0x0200, 0x0300, 0x0400, 0x0500, 0x0600, 0x0700,
        0x0800, 0x0900, 0x0A00, 0x0B00, 0x0C00, 0x0D00, 0x0E00, 0x0F00,
        0x1000, 0x1100, 0x1200, 0x1300, 0x1400, 0x1500, 0x1600, 0x1700,
        0x1800, 0x1900, 0x1A00, 0x1B00, 0x1C00, 0x1D00, 0x1E00, 0x1F00
    },
    0x001
};

static struct Layout destroyr_alpha_num_layout =
{
	8, 8,     
	64,       
	1,        
	{ 0 },    
	{
		0x4, 0x5, 0x6, 0x7, 0xC, 0xD, 0xE, 0xF
	},
	{
		0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70
	},
	0x80      
};


static struct Layout destroyr_minor_object_layout =
{
	16, 16,   
	16,       
	1,        
	{ 0 },    
	{
	  0x04, 0x05, 0x06, 0x07, 0x0C, 0x0D, 0x0E, 0x0F,
	  0x14, 0x15, 0x16, 0x17, 0x1C, 0x1D, 0x1E, 0x1F
	},
	{
	  0x000, 0x020, 0x040, 0x060, 0x080, 0x0a0, 0x0c0, 0x0e0,
	  0x100, 0x120, 0x140, 0x160, 0x180, 0x1a0, 0x1c0, 0x1e0
	},
	0x200     
};


static struct Layout destroyr_major_object_layout =
{
	64, 16,   
	4,        
	2,        
	{ 1, 0 }, 
	{
	  0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E,
	  0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
	  0x20, 0x22, 0x24, 0x26, 0x28, 0x2A, 0x2C, 0x2E,
	  0x30, 0x32, 0x34, 0x36, 0x38, 0x3A, 0x3C, 0x3E,
	  0x40, 0x42, 0x44, 0x46, 0x48, 0x4A, 0x4C, 0x4E,
	  0x50, 0x52, 0x54, 0x56, 0x58, 0x5A, 0x5C, 0x5E,
	  0x60, 0x62, 0x64, 0x66, 0x68, 0x6A, 0x6C, 0x6E,
	  0x70, 0x72, 0x74, 0x76, 0x78, 0x7A, 0x7C, 0x7E
	},
	{
	  0x000, 0x080, 0x100, 0x180, 0x200, 0x280, 0x300, 0x380,
	  0x400, 0x480, 0x500, 0x580, 0x600, 0x680, 0x700, 0x780
	},
	0x0800    
};


static struct Layout destroyr_waves_layout =
{
	64, 2,   
	2,       
	1,       
	{ 0 },   
	{
	  0x00, 0x01, 0x02, 0x03, 0x08, 0x09, 0x0A, 0x0B,
	  0x10, 0x11, 0x12, 0x13, 0x18, 0x19, 0x1A, 0x1B,
	  0x20, 0x21, 0x22, 0x23, 0x28, 0x29, 0x2A, 0x2B,
	  0x30, 0x31, 0x32, 0x33, 0x38, 0x39, 0x3A, 0x3B,
	  0x40, 0x41, 0x42, 0x43, 0x48, 0x49, 0x4A, 0x4B,
	  0x50, 0x51, 0x52, 0x53, 0x58, 0x59, 0x5A, 0x5B,
	  0x60, 0x61, 0x62, 0x63, 0x68, 0x69, 0x6A, 0x6B,
	  0x70, 0x71, 0x72, 0x73, 0x78, 0x79, 0x7A, 0x7B
	},
	{
	  0x00, 0x80
	},
	0x04     
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
    { "milliped", 0, 0, 4096, GFX_MILLIPED, &centipede_spritelayout, 8 },
    { "milliped", 0, 0, 4096, GFX_MILLIPED, &centipede_charlayout, 0 },
    { "mazeinv", 0, 0, 4096, GFX_MAZEINV, &centipede_spritelayout, 8 },
    { "mazeinv", 0, 0, 4096, GFX_MAZEINV, &centipede_charlayout, 0 },
    { "ccastles", 0, 0, 16384, GFX_CCASTLES, &ccastles_spritelayout, 0 },
    { "warlords", 0, 0, 0x800, GFX_WARLORDS, &warlords_charlayout, 0 },
    { "warlords", 0, 0x200, 0x600, GFX_WARLORDS, &warlords_charlayout, 0 },
    { "skydiver", 0, 0, 0x400, GFX_SKYDIVER, &skydiver_charlayout, 0 },
    { "skydiver", 1, 0, 0x800, GFX_SKYDIVER, &skydiver_motionlayout, 0 },
    { "sprint2", 0, 0, 0x400, GFX_SPRINT, &sprint_tile_layout, 0 },
    { "sprint2", 1, 0, 0x400, GFX_SPRINT, &sprint_car_layout, 0 },
    
    { "dominos", 0, 0, 0x400, GFX_SPRINT, &sprint_tile_layout, 0 },

    { "montecar", 0, 0, 0x400, GFX_MONTECAR, &montecar_text_layout, 0 },
    { "montecar", 1, 0, 0x800, GFX_MONTECAR, &firetrk_tile_layout, 0 },
    { "montecar", 2, 0, 0x800, GFX_MONTECAR, &montecar_car_layout, 0 },
    { "montecar", 3, 0, 0x800, GFX_MONTECAR, &montecar_car_layout, 0 },
    
    { "firetrk", 0, 0, 0x800, GFX_MONTECAR, &firetrk_text_layout, 0 },
    { "firetrk", 1, 0, 0x800, GFX_MONTECAR, &firetrk_tile_layout, 0 },
    { "firetrk", 2, 0, 0x400, GFX_FIRETRUCK_CAR, &firetrk_car_layout1, 0 },
    { "firetrk", 2, 0, 0x400, GFX_FIRETRUCK_CAR, &firetrk_car_layout2, 0 },
    { "firetrk", 3, 0, 0x1000, GFX_MONTECAR, &firetrk_trailer_layout, 0 },

    { "superbug", 0, 0, 0x800, GFX_MONTECAR, &superbug_text_layout, 0 },
    { "superbug", 1, 0, 0xC00, GFX_MONTECAR, &superbug_tile_layout, 0 },
    { "superbug", 2, 0, 0x400, GFX_SUPERBUG_CAR, &superbug_car_layout1, 0 },
    { "superbug", 2, 0, 0x400, GFX_SUPERBUG_CAR, &superbug_car_layout2, 0 },

    { "sbrkout", 0, 0, 0x400, GFX_SBRKOUT, &sbrkout_charlayout, 0 },
    { "sbrkout", 1, 0, 0x20, GFX_SBRKOUT, &sbrkout_balllayout, 0 },
    
    { "destroyr", 0, 0, 0x400, GFX_DESTROYER, &destroyr_alpha_num_layout, 0 },
    { "destroyr", 1, 0, 0x400, GFX_DESTROYER, &destroyr_minor_object_layout, 0 },
    { "destroyr", 2, 0, 0x800, GFX_DESTROYER_MAJOR, &destroyr_major_object_layout, 0 },
    { "destroyr", 3, 0, 0x020, GFX_DESTROYER_WAVES, &destroyr_waves_layout, 0 },

    { "canyon", 0, 0, 0x400, GFX_DEFAULT_REV, &canyon_tile_layout, 0 },
    { "canyon", 1, 0, 0x100, GFX_DEFAULT_REV, &canyon_sprite_layout, 0 },
    
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
	unsigned offset = width*(  (2*height-1-y)%height )+x;

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

// 0:background
// 1:text
// 2:central dotted wall
// 3:copyright circle
// 4:tile backgrounds 
// 5:lemons/stems, spinner insides ??
// 6:lemons/stems, spinner insides ??
// 7:apples
static unsigned char mazeInvRemap[8] = {
    0, 0, 0, 0, 0, 1, 0, 0
};

static void encode_layout(unsigned char* out, unsigned char* bmp, unsigned regionSize, struct Layout* layout, unsigned start, unsigned bmpX, unsigned bmpY, unsigned mode) {
	unsigned width = layout->width;
	unsigned height = layout->height;
	unsigned region_length = regionSize * 8;
	unsigned total = IS_FRACTION(layout->total) ? region_length / layout->charincrement * NUMERATOR(layout->total) / DENOMINATOR(layout->total) : layout->total;
	for(unsigned i=0;i<layout->planes;i++)
	{
		int value = layout->planeoffset[i];
		if (IS_FRACTION(value))
			layout->planeoffset[i] = OFFSET(value) + region_length * NUMERATOR(value) / DENOMINATOR(value);
        fprintf(stderr, "plane %x\n", layout->planeoffset[i]/8);
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
    int rev = 0;

	if (mode == GFX_CENTIPED) {
		c = width == height ? 64 : 0;
		end = 128;
	}
	else if (mode == GFX_MILLIPED || mode == GFX_MAZEINV) {
		c = width == height ? 0 : 0;
		end = width == height ? 256 : 128;
	}
	else {
		c = 0;
		end = total;
	}
    
    if (mode == GFX_SKYDIVER || mode == GFX_SPRINT || mode == GFX_MONTECAR || mode==GFX_SBRKOUT || mode==GFX_FIRETRUCK_CAR || mode == GFX_SUPERBUG_CAR
        || mode == GFX_DESTROYER || mode == GFX_DESTROYER_MAJOR || mode==GFX_DESTROYER_WAVES || mode==GFX_DEFAULT_REV) 
        rev = 1;
    
    fprintf(stderr,"mode:%d total:%d width:%d height:%d size:%d\n", mode, total, width, height, regionSize);

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
			else if (mode == GFX_MILLIPED || mode == GFX_MAZEINV) {
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
                    if (mode == GFX_MAZEINV) {
                        v &= 3;
                        if ( charNum >= 64) {
//                            v = "\x00\x03\x03\x03"[v];
                        }
                        
                    }
				}
				else {
					unsigned charNum = 2*c;
					if (charNum > 128)
						charNum -= 127;
                    v = get_from_bmp(bmp, bmpX+y, bmpY+width*charNum+x);
                    if (mode == GFX_MAZEINV) {
                        if ( charNum >= 32) {
                        //    v = "\x00\x03\x03\x03"[v];  // 1->5, 2->blank?!, 3->
                        }
                        
                        //if( x==0 && y ==0) fprintf(stderr, "\n %d ", charNum);
                        //fprintf(stderr, "%d ", v);
                        //if (charNum == 127 || charNum == 63)
                        //    v = 3;
                    }
				}
			}
			else if (mode == GFX_CCASTLES) {
		        	v = 0x7^(0x7&get_from_bmp(bmp, bmpX+7-(x+4)%8, height*c+bmpY+y)); //ARP
			}
            else if (mode == GFX_WARLORDS) {
                    int delta = start>0 ? (-1536-512) : 0;
                    v = get_from_bmp(bmp, bmpX+x, (bmpY+height*c+y+delta+4096)%4096);
            }
			else if (mode == GFX_MONTECAR || mode == GFX_SBRKOUT) {
		        	v = get_from_bmp(bmp, bmpX+y, bmpY+width*c+(height-1-x));
			}
			else if (mode == GFX_FIRETRUCK_CAR) {
		        	v = get_from_bmp(bmp, bmpX+x, bmpY+height*c+(height-1-y));
			}
			else if (mode == GFX_SUPERBUG_CAR) {
		        	v = get_from_bmp(bmp, bmpX+x, (bmpY+height*(total-1-c))+(height-1-y));
			}
			else if (mode == GFX_DESTROYER || mode == GFX_DESTROYER_MAJOR) {
		        	v = get_from_bmp(bmp, bmpX+width-1-x, bmpY+height*c+y);
			}
			else if (mode == GFX_DESTROYER_WAVES) {
		        	v = get_from_bmp(bmp, bmpX+width-1-x, bmpY+height*((c+1)%total)+y);
			}
			else {
		        	v = get_from_bmp(bmp, bmpX+x, bmpY+height*c+y);
			}
            
			for (int plane=0;plane<layout->planes;plane++) {
				unsigned pos = layout->planeoffset[plane]+layout->xoffset[x]+layout->yoffset[y]+layout->charincrement*(c%total);
                                
                if(pos/8+start>=regionSize) 
					continue;

				if (v&(1<<plane)) 
					base[pos/8] |=  (1<<(rev ? 7-pos%8 : pos%8));
				else
					base[pos/8] &= ~(1<<(rev ? 7-pos%8 : pos%8));
			}
		}
	}
}




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
    
	if (chunk->mode == GFX_SPRINT || chunk->mode == GFX_DESTROYER_MAJOR) {
		for (unsigned i = 0 ; i < size / 2 ; i++) {
			buf[size/2 + i] = buf[i] & 0xF;
			buf[i] >>= 4;        
        }
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
