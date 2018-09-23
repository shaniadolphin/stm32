#include "includes.h"

//拟合抛物线（包含测试函数）：

extern void printk(char *Data, ...);

extern int valueSensorOne[20];
extern int valueSensorTwo[20];
extern int valueSensorThr[20];
extern unsigned char valueSensorCnt;
extern unsigned char ValueSensorIdx;
extern unsigned char angleResult[5];
extern unsigned int delayActionTime;
extern int valueSensorDiff[3];//差值平均值
/*******************************************************************************
阶乘函数宏定义
*******************************************************************************/
#define     my_pow2(x)      ((x) * (x))
#define     my_pow3(x)      ((x) * my_pow2(x))
#define     my_pow4(x)      (my_pow2(x) * my_pow2(x))

#define SENSORLOWLEVEL  		1400
#define SENSORDIFFLEVEL			500

#define N 30 //每通道采50次
#define M 3 //为12个通道

unsigned short AD_Value[N][M]; //用来存放ADC转换结果，也是DMA的目标地址
 
unsigned short After_filter[M]; //用来存放求平均值之后的结果
/*******************************************************************************
定义一个2行3列的系数矩阵
*******************************************************************************/
double Para[3][4] = {0.0};

/*----------------------------------------------------------------------------
 * @描述: 	系数矩阵的初始化	
 * @输入: 	X
			Y
			Amount
 * @返回: 	0
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2016.09.07     zxjun          创建模块	
----------------------------------------------------------------------------*/
static int ParaInit(const double* X, const double* Y, int Amount)
{
	unsigned char x,y;
	for(x= 0;x<3;x++)
		for(y= 0;y<4;y++)
			Para[x][y] = 0;
	Para[2][2] = Amount;
	for ( ; Amount; Amount--, X++, Y++)
	{
		Para[0][0]  +=  my_pow4(*X);
		Para[0][1]  += my_pow3(*X);
		Para[0][2]  += my_pow2(*X);
		Para[1][2]  += (*X);
		Para[0][3]  +=  my_pow2(*X) * (*Y);
		Para[1][3]  +=  (*X) * (*Y);
		Para[2][3]  +=  (*Y);
	}
	Para[1][0] = Para[0][1];
	Para[1][1] = Para[0][2];
	Para[2][0] = Para[1][1];
	Para[2][1] = Para[1][2];
	return 0;
}
/*----------------------------------------------------------------------------
 * @描述: 	系数矩阵的运算
 * @输入: 	void
 * @返回: 	0
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2016.09.07     zxjun          创建模块	
----------------------------------------------------------------------------*/
static int ParaDeal(void)
{
	//用Para[2][2]把Para[0][2]消成0
	Para[0][0] -= Para[2][0] * (Para[0][2] / Para[2][2]);
	Para[0][1] -= Para[2][1] * (Para[0][2] / Para[2][2]);
	Para[0][3] -= Para[2][3] * (Para[0][2] / Para[2][2]);
	Para[0][2] = 0;
	//用Para[2][2]把Para[1][2] 消成0
	Para[1][0] -= Para[2][0] * (Para[1][2] / Para[2][2]);
	Para[1][1] -= Para[2][1] * (Para[1][2] / Para[2][2]);
	Para[1][3] -= Para[2][3] * (Para[1][2] / Para[2][2]);
	Para[1][2] = 0;
	//用Para[1][1]把Para[0][1]消成0
	Para[0][0] -= Para[1][0] * (Para[0][1] / Para[1][1]);
	Para[0][3] -= Para[1][3] * (Para[0][1] / Para[1][1] );
	Para[0][1] = 0;
	//至此，已经成成为三角矩阵
	//用Para[0][0]把Para[1][0]消成0
	Para[1][3] -= Para[0][3] * (Para[1][0] / Para[0][0]);
	Para[1][0] = 0;
	//用Para[0][0]把Para[2][0]消成0
	Para[2][3] -= Para[0][3] * (Para[2][0] / Para[0][0]);
	Para[2][0] = 0;
	//用Para[1][1]把Para[2][1]消成0
	Para[2][3] -= Para[1][3] * (Para[2][1] / Para[1][1]);
	Para[2][1] = 0;
	//至此，已经成成为对角矩阵
	//把Para[0][0]，Para[1][1]，Para[2][2]消成1
	Para[0][3] /= Para[0][0];//这个就是最后a的值
	//Para[0][0] = 1.0;
	Para[1][3] /= Para[1][1]; //这个就是最后b的值
	//Para[1][1] = 1.0;
	Para[2][3] /= Para[2][2]; //这个就是最后c的值
	//Para[2][2] = 1.0;
	return 0;
}
void getValueMax(unsigned short *values)
{

}

void Bublesort(int a[],int n)
{
     int i,j,k;
     for(j=0;j<n;j++) 
     {
          for(i=0;i<n-j;i++) 
          {
               if(a[i]>a[i+1])
               {
                    k=a[i];
                    a[i]=a[i+1];
                    a[i+1]=k;
               }
          }
     }
}

void getAngleResult(void)
{
	angleResult[2] = angleResult[1];
	switch(angleResult[1])
	{
	case 3:
		if(valueSensorDiff[1] < -SENSORDIFFLEVEL)//远离3
		{
			printk("away 1\r\n");
			if(valueSensorDiff[0] >  SENSORDIFFLEVEL)
			{
				printk("close 0--2\r\n");
				angleResult[1] = 2;
			}
			if(valueSensorDiff[2] >  SENSORDIFFLEVEL)
			{
				printk("close 2--4\r\n");
				angleResult[1] = 4;
			}
		}
		else if(valueSensorDiff[1] >  SENSORDIFFLEVEL)//靠近1
		{
			printk("close 1\r\n");
			if(valueSensorDiff[0] <  -SENSORDIFFLEVEL)//同时离开0
			{
				printk("away 0\r\n");
			}
			else if(valueSensorDiff[0] >   SENSORDIFFLEVEL)//同时靠近0
			{
				printk("close 0--2\r\n");
				angleResult[1] = 2;
			}
			if(valueSensorDiff[2] <  -SENSORDIFFLEVEL)//同时离开2
			{
				printk("away 2\r\n");
			}
			else if(valueSensorDiff[2] >   SENSORDIFFLEVEL)//同时靠近2
			{
				printk("close 2--4\r\n");	
				angleResult[1] = 4;
			}
		}
		else//主体没有移动，但是别的人移动了
		{
			printk("stand 1\r\n");
			if(valueSensorDiff[0] >  SENSORDIFFLEVEL)//靠近0
			{
				printk("close 0--2\r\n");
				angleResult[1] = 2;
			}
			if(valueSensorDiff[2] >  SENSORDIFFLEVEL)//靠近2
			{
				printk("close 2--4\r\n");
				angleResult[1] = 4;
			}
		}
	break;
	case 2:
		if(valueSensorDiff[0] < -SENSORDIFFLEVEL)//远离0
		{
			printk("away 0\r\n");
			if(valueSensorDiff[1] >  SENSORDIFFLEVEL)//往中间移
			{
				printk("close 1--3\r\n");
				angleResult[1] = 3;
			}
			else//往边上移
			{
				printk("slide 0--1\r\n");
				angleResult[1] = 1;
			}	
		}
		if(delayActionTime > 500)//长时间停留在边上,进行归中
		{
			printk("too long\r\n");
			angleResult[1] = 3;			
		}
	break;	
	case 4:
		if(valueSensorDiff[2] < -SENSORDIFFLEVEL)//远离2
		{
			printk("away 2\r\n");
			if(valueSensorDiff[1] >  SENSORDIFFLEVEL)//往中间移
			{
				printk("close 1--3\r\n");
				angleResult[1] = 3;
			}
			else//往边上移
			{
				printk("slide 2--5\r\n");
				angleResult[1] = 5;
			}
		}
		if(delayActionTime > 500)//长时间停留在边上,进行归中
		{
			printk("too long\r\n");
			angleResult[1] = 3;			
		}
	break;
	case 1://仅能往左转
		if(valueSensorDiff[0] < -SENSORDIFFLEVEL)//远离0
		{
			printk("away 0\r\n");
			if(valueSensorDiff[1] >  SENSORDIFFLEVEL)//往中间移
			{
				printk("close 1--2\r\n");
				angleResult[1] = 2;
			}
			else//往边上移
			{
				printk("slide 0--1\r\n");
				angleResult[1] = 1;
			}	
		}
		else if(valueSensorDiff[0] >  SENSORDIFFLEVEL)//靠近0
		{
			printk("close 0\r\n");
		}
		if(valueSensorDiff[2] >  SENSORDIFFLEVEL)//靠近2
		{
			printk("close 2\r\n");
			angleResult[1] = 5;
		}
		if(valueSensorDiff[1] >  SENSORDIFFLEVEL)//靠近1
		{
			printk("close 1\r\n");
			angleResult[1] = 3;
		}
		if(delayActionTime > 1000)//长时间停留在边上,进行归中
		{
			printk("too long\r\n");
			angleResult[1] = 3;			
		}
	break;	
	case 5:
		if(valueSensorDiff[2] < -SENSORDIFFLEVEL)//远离2
		{
			printk("away 2\r\n");
			if(valueSensorDiff[1] >  SENSORDIFFLEVEL)//往中间移
			{
				printk("close 1--4\r\n");
				angleResult[1] = 4;
			}
			else//往边上移
			{
				printk("slide 2--5\r\n");
				angleResult[1] = 5;
			}	
		}
		else if(valueSensorDiff[2] >  SENSORDIFFLEVEL)//远离2			
		{
				printk("close 2\r\n");
		}
		if(valueSensorDiff[0] >  SENSORDIFFLEVEL)//靠近0
		{
			printk("close 2\r\n");
			angleResult[1] = 1;
		}
		if(valueSensorDiff[1] >  SENSORDIFFLEVEL)//靠近1
		{
			printk("close 1\r\n");
			angleResult[1] = 3;
		}
		if(delayActionTime > 1000)//长时间停留在边上,进行归中
		{
			printk("too long\r\n");
			angleResult[1] = 3;			
		}
	break;
	default:
		angleResult[1] = 3;	
		//delayActionTime = 1000;
	break;
	}
	if(angleResult[1] != angleResult[2])delayActionTime = 0;
}
/*----------------------------------------------------------------------------
 * @描述: 	曲线拟合
 * @输入: 	input[3]
			output[3]
 * @返回: 	valueFitting
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2016.09.07     zxjun          创建模块	
----------------------------------------------------------------------------*/
extern float valueFitting;
extern unsigned short anglePosition[20];
void getFitting(void)
{
	double input[3];
	double output[3];
	int i;
	unsigned short valuemax;
	unsigned short valuenum;
	input[0] = 1.0;
	input[1] = 2.0;
	input[2] = 3.0;
	output[0] = (double)After_filter[0];//3000;//
	output[1] = (double)After_filter[1];//2000;//
	output[2] = (double)After_filter[2];//1500;//
	if(After_filter[0] < After_filter[1])
	{
		valuemax = After_filter[1];
		valuenum = 2;
	}
	else
	{
		valuemax = After_filter[0];
		valuenum = 1;
	}
	if(valuemax < After_filter[2])
	{
		valuemax = After_filter[2];
		valuenum = 3;
	}
	ParaInit((double*)input, (double*)output, 3);
	ParaDeal();
	//printk("拟合数据成功，拟合直线为：\r\ny = (%d) * x^2 + (%d) * x + (%d);\r\n", (int)(Para[0][3]), (int)(Para[1][3]), (int)(Para[2][3]));
	//for(i=0;i<3;i++)printk("when x = %d,y= %d\r\n",(int)(input[i]),(int)((Para[0][3]*input[i]*input[i]+Para[1][3]*input[i]+Para[2][3])));
	//valueFitting = (float)(-Para[1][3]*0.5/Para[0][3] - 0);
	if(valuemax > 600)valueFitting = valuenum;//valueFitting = (float)(-Para[1][3]*0.5/Para[0][3] - 0);//
	else valueFitting = 2;
	if(valueFitting <0.5) valueFitting= 0.5;
	if(valueFitting >3.5) valueFitting= 3.5;
	//valueFitting = 1;
}

unsigned short getCloseToResult(unsigned short *now,unsigned short final, unsigned short step)	
{	
	unsigned short result;
	
	if(*now - final > step)//(*now > final)
	{
		result = *now - step;
		*now = result;
		//return result;
	}
	else if(final - *now > step)//(*now < final)
	{
		result = *now + step;
		*now = result;		
	}
	else
	{
		result = final;//*now;
		*now = result;
	}
	return result;
}
//表示在time ms内完成调节动作
//每步的调节为step;
//则分为time / step步进行调节
unsigned short getEndToResult(unsigned char ch, unsigned short *now,unsigned short final, unsigned short step,unsigned short time)	
{	
	unsigned short result;
	unsigned short timecnt = time / step; 
	unsigned short everystep;
	unsigned short i;
	i = 0;
	everystep = (final - *now)/step;
	if(timecnt < 50)timecnt = 50;
	do
	{
		pulseSetting(1, *now + everystep * (i + 1));
		pulseSetting(2, *now + everystep * (i + 1));
		OSTimeDlyHMSM(0, 0, 0, timecnt);
		i ++;
	}while(i < step);
	*now = final;
	pulseSetting(ch, *now);
	return final;
}

/*----------------------------------------------------------------------------
 * @描述: 	模数转换值平均滤波
 * @输入: 	AD_Value[][]
 * @返回: 	After_filter[i]
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2016.09.07     zxjun          创建模块	
----------------------------------------------------------------------------*/
void filter(void)//每4.2ms
{
	int sum = 0;
	unsigned char i;
	unsigned char count;
	int diff[3] = {0};
	for(i = 0;i < M;i++)//20us
	{
		for ( count = 0;count < N;count++)
		{
			sum += AD_Value[count][i];
		}
		After_filter[i] = sum/N;
		After_filter[i] = 3300 * After_filter[i] / 4096;
		if(After_filter[i] < 1300)After_filter[i] = 1300;
		sum = 0;
	}
	valueSensorOne[0] +=  After_filter[0];
	valueSensorTwo[0] +=  After_filter[1];
	valueSensorThr[0] +=  After_filter[2];
	valueSensorCnt ++;
	if(valueSensorCnt == 30)//50对应209ms
	{
		//GPIO_ResetBits(GPIOC, GPIO_Pin_13);
		valueSensorOne[1] = valueSensorOne[0] / 30;
		valueSensorTwo[1] = valueSensorTwo[0] / 30;
		valueSensorThr[1] = valueSensorThr[0] / 30;
		for(i = 5; i > 1; i--)
		{
			valueSensorOne[i]=valueSensorOne[i - 1];
			valueSensorTwo[i]=valueSensorTwo[i - 1];
			valueSensorThr[i]=valueSensorThr[i - 1];
		}
		for(i = 5;i > 1; i--)
		{
			diff[0] += (valueSensorOne[i-1]-valueSensorOne[i]);
			diff[1] += (valueSensorTwo[i-1]-valueSensorTwo[i]);
			diff[2] += (valueSensorThr[i-1]-valueSensorThr[i]);
		}
		valueSensorDiff[0] = diff[0];
		valueSensorDiff[1] = diff[1];
		valueSensorDiff[2] = diff[2];
		//printk("B %d %d %d\r\n",valueSensorDiff[0],valueSensorDiff[1],valueSensorDiff[2]);
		valueSensorOne[0] = 0;
		valueSensorTwo[0] = 0;
		valueSensorThr[0] = 0;
		//GPIO_SetBits(GPIOC, GPIO_Pin_13);
		valueSensorCnt = 0;
		getAngleResult();
		//判断方向
	}
	
}
/*----------------------------------------------------------------------------
 * @描述: 	打印模数转换值平均滤波结果
 * @输入: 	After_filter[i]
 * @返回: 	无
 * @历史:
 版本        日期         作者           改动内容和原因
 ------    -----------	  ---------	 ----------------------	
 1.0       2016.09.07     zxjun          创建模块	
----------------------------------------------------------------------------*/
void displayADValue(void)
{
	printk("ch1=%d, ch2=%d, ch3=%d\r\n",After_filter[0],After_filter[1],After_filter[2]);
	
}
