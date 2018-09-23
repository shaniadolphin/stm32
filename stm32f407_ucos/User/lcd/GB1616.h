// ------------------  汉字字模的数据结构定义 ------------------------ //
struct  typFNT_GB16                 // 汉字字模数据结构 
{
       unsigned char  Index[3];               // 汉字内码索引	
       unsigned char   Msk[32];                        // 点阵码数据 
};

/////////////////////////////////////////////////////////////////////////
// 汉字字模表                                                          //
// 汉字库: 宋体16.dot,横向取模左高位,数据排列:从左到右从上到下         //
/////////////////////////////////////////////////////////////////////////
 const struct  typFNT_GB16 codeGB_16[] =          // 数据表 
{
"支", 0x01,0x00,0x01,0x00,0x01,0x04,0xFF,0xFE,
      0x01,0x00,0x01,0x00,0x3F,0xF0,0x08,0x20,
      0x08,0x20,0x04,0x40,0x02,0x80,0x01,0x00,
      0x02,0x80,0x0C,0x60,0x30,0x1E,0xC0,0x04,

"持", 0x10,0x40,0x10,0x40,0x10,0x50,0x13,0xF8,
      0xFC,0x40,0x10,0x48,0x17,0xFC,0x18,0x10,
      0x30,0x14,0xD7,0xFE,0x12,0x10,0x11,0x10,
      0x11,0x10,0x10,0x10,0x50,0x50,0x20,0x20,

"横", 0x11,0x10,0x11,0x10,0x17,0xFC,0x11,0x10,
      0xFD,0x14,0x17,0xFE,0x30,0x48,0x3B,0xFC,
      0x56,0x48,0x53,0xF8,0x92,0x48,0x13,0xF8,
      0x10,0x00,0x11,0x10,0x13,0x0C,0x14,0x04,

"竖", 0x04,0x00,0x25,0xFC,0x25,0x04,0x24,0x88,
      0x24,0x50,0x24,0x20,0x24,0x50,0x24,0x8E,
      0x07,0x04,0x01,0x10,0x3F,0xF8,0x08,0x20,
      0x08,0x20,0x04,0x44,0xFF,0xFE,0x00,0x00,

"屏", 0x00,0x08,0x3F,0xFC,0x20,0x08,0x20,0x08,
      0x3F,0xF8,0x22,0x20,0x21,0x48,0x2F,0xFC,
      0x22,0x20,0x22,0x24,0x3F,0xFE,0x22,0x20,
      0x22,0x20,0x42,0x20,0x84,0x20,0x08,0x20,

"　", 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,       
      0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

"*",  0x00,0x00,0x00,0x00,0x00,0x80,0x03,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,
      0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x03,0xC0,0x00,0x00,0x00,0x00,
			
"１", 0x00,0x00,0x00,0x00,0x01,0x00,0x03,0x00,0x05,0x00,0x01,0x00,0x01,0x00,0x01,0x00,
			0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,/*"１",0*/
			
"２", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x00,0x20,0x00,0x20,0x00,0x40,
      0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x08,0x00,0x1F,0xF0,0x00,0x00,0x00,0x00,/*"２",0*/
			
"３", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x00,0x20,0x00,0x40,0x00,0x80,0x01,0xC0,0x00,0x20,
      0x00,0x10,0x00,0x10,0x00,0x10,0x00,0x10,0x08,0x20,0x07,0xC0,0x00,0x00,0x00,0x00,/*"３",0*/

"４", 0x00,0x00,0x00,0x00,0x02,0x00,0x02,0x00,0x04,0x00,0x04,0x80,0x08,0x80,0x08,0x80,
      0x10,0x80,0x10,0x80,0x3F,0xF0,0x00,0x80,0x00,0x80,0x00,0x80,0x00,0x00,0x00,0x00,/*"４",0*/

"５", 0x00,0x00,0x00,0x00,0x1F,0xC0,0x10,0x00,0x10,0x00,0x10,0x00,0x1F,0x00,0x00,0x80,
      0x00,0x40,0x00,0x40,0x00,0x40,0x00,0x40,0x10,0x80,0x0F,0x00,0x00,0x00,0x00,0x00,/*"５",0*/

"６", 0x00,0x00,0x00,0x00,0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x08,0x00,0x0F,0x80,
      0x10,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,0x00,0x00,0x00,0x00,/*"６",0*/

"７", 0x00,0x00,0x00,0x00,0x1F,0xF0,0x00,0x20,0x00,0x40,0x00,0x80,0x00,0x80,0x01,0x00,
      0x01,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x00,0x00,0x00,0x00,/*"７",0*/

"８", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,
      0x08,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x40,0x07,0x80,0x00,0x00,0x00,0x00,/*"８",0*/

"９", 0x00,0x00,0x00,0x00,0x07,0x80,0x08,0x40,0x10,0x20,0x10,0x20,0x10,0x20,0x08,0x20,
      0x07,0xC0,0x00,0x40,0x00,0x80,0x01,0x00,0x02,0x00,0x04,0x00,0x00,0x00,0x00,0x00,/*"９",0*/

"０", 0x00,0x00,0x00,0x00,0x07,0xC0,0x08,0x20,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,
      0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x08,0x20,0x07,0xC0,0x00,0x00,0x00,0x00,/*"０",0*/			
/*****
"８", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,
      0x06,0x60,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"６", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x00,0x0D,0xC0,0x0E,0x60,
      0x0C,0x30,0x0C,0x30,0x0C,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"７", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x0F,0xF0,0x08,0x30,0x00,0x60,0x00,0xC0,0x00,0xC0,
      0x01,0x80,0x01,0x80,0x03,0x00,0x03,0x00,0x03,0x00,0x03,0x00,0x00,0x00,0x00,0x00,

"０", 0x00,0x00,0x00,0x00,0x03,0xE0,0x06,0x30,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,
      0x0C,0x18,0x0C,0x18,0x0C,0x18,0x0C,0x18,0x06,0x30,0x03,0xE0,0x00,0x00,0x00,0x00,

"５", 0x00,0x00,0x00,0x00,0x0F,0xF0,0x0C,0x00,0x0C,0x00,0x0C,0x00,0x0F,0xC0,0x0E,0x60,
      0x00,0x30,0x00,0x30,0x00,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,
 			
"３", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x00,0x30,0x00,0x60,0x01,0xC0,
      0x00,0x60,0x00,0x30,0x00,0x30,0x0C,0x30,0x06,0x60,0x03,0xC0,0x00,0x00,0x00,0x00,

"２", 0x00,0x00,0x00,0x00,0x03,0xC0,0x06,0x60,0x0C,0x30,0x0C,0x30,0x00,0x30,0x00,0x30,
      0x00,0x60,0x00,0xC0,0x01,0x80,0x03,0x10,0x06,0x10,0x0F,0xF0,0x00,0x00,0x00,0x00,
****/
};

