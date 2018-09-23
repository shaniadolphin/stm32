#ifdef GLOBALS
#define EXT
#else
#define EXT extern
#endif


#define SOFTWARE_VER	0x10


#define POWERUP 		0
#define SETTCPLINK 		1
#define TCPLINKOK 		2
#define TCPLINKFAIL 	3
#define SMARTWIFI		4
#define SYS_REGISTER	5
#define PROGRAME_SET_AP	6//程序设定链接指定的AP

EXT volatile unsigned int RxCounter1;


