#ifndef  __PARADEAL_H__
#define  __PARADEAL_H__

#define N 50 //ÿͨ����50��
#define M 3 //Ϊ12��ͨ��

extern unsigned short AD_Value[N][M]; //�������ADCת�������Ҳ��DMA��Ŀ���ַ
 
extern unsigned short After_filter[M]; //���������ƽ��ֵ֮��Ľ��
/*******************************************************************************
����һ��2��3�е�ϵ������
*******************************************************************************/
extern double Para[3][4];
extern int Test(void);
extern void getFitting(void);
extern void filter(void);
extern void displayADValue(void);
extern unsigned short getEndToResult(unsigned char ch, unsigned short *now,unsigned short final, unsigned short step,unsigned short time);


#endif

