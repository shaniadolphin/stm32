#ifndef  __PARADEAL_H__
#define  __PARADEAL_H__

#define N 50 //每通道采50次
#define M 3 //为12个通道

extern unsigned short AD_Value[N][M]; //用来存放ADC转换结果，也是DMA的目标地址
 
extern unsigned short After_filter[M]; //用来存放求平均值之后的结果
/*******************************************************************************
定义一个2行3列的系数矩阵
*******************************************************************************/
extern double Para[3][4];
extern int Test(void);
extern void getFitting(void);
extern void filter(void);
extern void displayADValue(void);
extern unsigned short getEndToResult(unsigned char ch, unsigned short *now,unsigned short final, unsigned short step,unsigned short time);


#endif

