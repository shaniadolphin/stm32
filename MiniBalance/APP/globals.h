
#ifdef GLOBALS 
#define EXT
#else
#define EXT extern 
#endif


EXT volatile unsigned char rec_f,tx_flag;
EXT volatile unsigned long Rec_Len;	
EXT volatile unsigned  int  milsec1,milsec2,milsec3;
