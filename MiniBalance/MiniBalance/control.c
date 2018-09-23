#include "includes.h"

#include "control.h"	
#include "filter.h"
#include "./IIC/ioi2c.h"
#include "./mpu6050/mpu6050.h"
#include "./motor/motor.h"
#include "./usart/usart.h"
#include "./ENCODER/encoder.h"
#include "./adc/adc.h"
#include "math.h"

u8 Way_Angle=2;                             //��ȡ�Ƕȵ��㷨��1����Ԫ��  2��������  3�������˲� 
u8 Flag_Qian,Flag_Hou,Flag_Left,Flag_Right,Flag_sudu=2; //����ң����صı���
u8 Flag_Stop=1,Flag_Show=0;                 //ֹͣ��־λ�� ��ʾ��־λ Ĭ��ֹͣ ��ʾ��
int Encoder_Left,Encoder_Right;             //���ұ��������������
int Moto1,Moto2;                            //���PWM���� Ӧ��Motor�� ��Moto�¾�	
int Temperature;                            //��ʾ�¶�
int Voltage;                                //��ص�ѹ������صı���
float Angle_Balance,Gyro_Balance,Gyro_Turn; //ƽ����� ƽ�������� ת��������
float Show_Data_Mb;                         //ȫ����ʾ������������ʾ��Ҫ�鿴������
u32 Distance;                               //���������
u8 delay_50,delay_flag,Bi_zhang=0;         //Ĭ������£����������Ϲ��ܣ������û�����2s���Ͽ��Խ������ģʽ
float Acceleration_Z;                      //Z����ٶȼ�  
int Balance_Pwm,Velocity_Pwm,Turn_Pwm;
u8 Flag_Target = 1;
/**************************************************************************
�������ܣ����еĿ��ƴ��붼��������
         5ms��ʱ�ж���MPU6050��INT���Ŵ���
         �ϸ�֤���������ݴ����ʱ��ͬ��				 
**************************************************************************/
int mpu6050irqprocess(void) 
{    
	Flag_Target = !Flag_Target;
	if(Flag_Target == 1)                        //5ms��ȡһ�������Ǻͼ��ٶȼƵ�ֵ�����ߵĲ���Ƶ�ʿ��Ը��ƿ������˲��ͻ����˲���Ч��
	{
		Get_Angle(Way_Angle);                   //===������̬	
		return 0;	                                               
	} 
	else
	{
		Get_Angle(Way_Angle);  					//10ms����һ�Σ�Ϊ�˱�֤M�����ٵ�ʱ���׼�����ȶ�ȡ����������
		Encoder_Left = -Read_Encoder(2);       	//===��ȡ��������ֵ����Ϊ�����������ת��180�ȵģ����Զ�����һ��ȡ������֤�������һ��
		Encoder_Right =Read_Encoder(4);       //===��ȡ��������ֵ
												//===������̬	
		//Distance = Read_Distane();            //===��ȡ��������������ֵ
	
		Voltage = Get_battery_volt();           //===��ȡ��ص�ѹ	          
		//printk("Voltage =%dmV\r\n",Voltage);
		//Key();                                //===ɨ�谴��״̬ ����˫�����Ըı�С������״̬
		Balance_Pwm = balance(Angle_Balance,Gyro_Balance);		//===ƽ��PID����	
		//printk("Balance_Pwm =%d\r\n",Balance_Pwm);
		Velocity_Pwm = velocity(Encoder_Left,Encoder_Right);	//===�ٶȻ�PID����	 ��ס���ٶȷ�����������������С�����ʱ��Ҫ����������Ҫ���ܿ�һ��
		//printk("Velocity_Pwm =%d\r\n",Velocity_Pwm);
		Turn_Pwm = turn(Encoder_Left,Encoder_Right,Gyro_Turn);  //===ת��PID����     
		//printk("Turn_Pwm =%d\r\n",Turn_Pwm);
		Moto1 = Balance_Pwm - Velocity_Pwm + Turn_Pwm;          //===�������ֵ������PWM
		Moto2 = Balance_Pwm - Velocity_Pwm - Turn_Pwm;          //===�������ֵ������PWM
		Xianfu_Pwm();                                           //===PWM�޷�
		//printk("Moto1=%d,Moto2=%d\r\n",Moto1,Moto2);
		//if(Pick_Up(Acceleration_Z,Angle_Balance,Encoder_Left,Encoder_Right))//===����Ƿ�С��������
		//	Flag_Stop=1;	                                                      //===���������͹رյ��
		//if(Put_Down(Angle_Balance,Encoder_Left,Encoder_Right))              //===����Ƿ�С��������
			Flag_Stop=0;	                                    //===��������¾��������
		if(Turn_Off(Angle_Balance,Voltage)==0)                  //===����������쳣
			Set_Pwm(Moto1,Moto2);                               //===��ֵ��PWM�Ĵ���       	
		return 0;
	}	
} 

/**************************************************************************
�������ܣ�ֱ��PD����
��ڲ������Ƕȡ����ٶ�
����  ֵ��ֱ������PWM
��    �ߣ�ƽ��С��֮��
**************************************************************************/
int balance(float Angle, float Gyro)
{  
	float Bias;
	float kp = 300;
	float kd = 1;
	int balance;
	Bias = Angle - ZHONGZHI;       //===���ƽ��ĽǶ���ֵ �ͻ�е���
	balance = kp * Bias + Gyro * kd;   //===����ƽ����Ƶĵ��PWM  PD����   kp��Pϵ�� kd��Dϵ�� 
	return balance;
}

/**************************************************************************
�������ܣ��ٶ�PI���� �޸�ǰ�������ٶȣ�����Target_Velocity�����磬�ĳ�60�ͱȽ�����
��ڲ��������ֱ����������ֱ�����
����  ֵ���ٶȿ���PWM
��    �ߣ�ƽ��С��֮��
**************************************************************************/
int velocity(int encoder_left,int encoder_right)
{  
	static float Velocity;
	static float Encoder_Least;
	static float Encoder,Movement;
	static float Encoder_Integral;
	static float Target_Velocity;
	float kp=80;
	float ki=0.4;
	//=============ң��ǰ�����˲���=======================// 
	if(Bi_zhang==1 && Flag_sudu==1)  
		Target_Velocity = 45;                 //����������ģʽ,�Զ��������ģʽ
	else 	                         
		Target_Velocity = 90;                 
	if(1 == Flag_Qian)    	
		Movement = Target_Velocity/Flag_sudu;	         //===ǰ����־λ��1 
	else if(1 == Flag_Hou)	
		Movement = -Target_Velocity/Flag_sudu;         //===���˱�־λ��1
	else  
		Movement=0;	
	if(Bi_zhang==1 && Distance<500 && Flag_Left!=1 && Flag_Right!=1)//���ϱ�־λ��1�ҷ�ң��ת���ʱ�򣬽������ģʽ
		Movement = -Target_Velocity/Flag_sudu;
	//=============�ٶ�PI������=======================//	
	Encoder_Least = (Encoder_Left + Encoder_Right) - 0;             //===��ȡ�����ٶ�ƫ��==�����ٶȣ����ұ�����֮�ͣ�-Ŀ���ٶȣ��˴�Ϊ�㣩 
	Encoder *= 0.8;		                                            //===һ�׵�ͨ�˲���       
	Encoder += Encoder_Least * 0.2;	                                //===һ�׵�ͨ�˲���    
	Encoder_Integral += Encoder;                                    //===���ֳ�λ�� ����ʱ�䣺10ms
	Encoder_Integral = Encoder_Integral - Movement;                 //===����ң�������ݣ�����ǰ������
	if(Encoder_Integral > 10000)  	
		Encoder_Integral = 10000;             						//===�����޷�
	if(Encoder_Integral < -10000)	
		Encoder_Integral = -10000;              					//===�����޷�	
	Velocity = Encoder * kp + Encoder_Integral * ki;                //===�ٶȿ���	
	if(Turn_Off(Angle_Balance,Voltage)==1 || Flag_Stop==1)   
		Encoder_Integral=0;      									//===����رպ��������
	return Velocity;
}

/**************************************************************************
�������ܣ�ת�����  �޸�ת���ٶȣ����޸�Turn_Amplitude����
��ڲ��������ֱ����������ֱ�������Z��������
����  ֵ��ת�����PWM
��    �ߣ�ƽ��С��֮��
**************************************************************************/
int turn(int encoder_left,int encoder_right,float gyro)//ת�����
{
	static float Turn_Target;
	static float Turn;
	static float Encoder_temp;
	static float Turn_Convert = 0.9;
	static float Turn_Count;
	float Turn_Amplitude = 88/Flag_sudu;
	float Kp = 42;
	float Kd = 0;     
	//=============ң��������ת����=======================//
	if(1==Flag_Left||1==Flag_Right)                      //��һ������Ҫ�Ǹ�����תǰ���ٶȵ����ٶȵ���ʼ�ٶȣ�����С������Ӧ��
	{
		if(++Turn_Count==1)
		Encoder_temp = myabs(encoder_left + encoder_right);
		Turn_Convert = 50/Encoder_temp;
		if(Turn_Convert < 0.6)Turn_Convert=0.6;
		if(Turn_Convert > 3)Turn_Convert=3;
	}	
	else
	{
		Turn_Convert = 0.9;
		Turn_Count = 0;
		Encoder_temp = 0;
	}			
	if(1==Flag_Left)	           
		Turn_Target -= Turn_Convert;
	else if(1==Flag_Right)	     
		Turn_Target += Turn_Convert; 
	else 
		Turn_Target = 0;
	if(Turn_Target > Turn_Amplitude)  
		Turn_Target = Turn_Amplitude;    //===ת���ٶ��޷�
	if(Turn_Target < -Turn_Amplitude) 
		Turn_Target=-Turn_Amplitude;
	if(Flag_Qian==1||Flag_Hou==1)  
		Kd=0.5;        
	else 
		Kd=0;   //ת���ʱ��ȡ�������ǵľ��� �е�ģ��PID��˼��
	//=============ת��PD������=======================//
	Turn = -Turn_Target * Kp - gyro * Kd;                 //===���Z�������ǽ���PD����
	return Turn;
}

/**************************************************************************
�������ܣ���ֵ��PWM�Ĵ���
��ڲ���������PWM������PWM
����  ֵ����
**************************************************************************/
void Set_Pwm(int moto1,int moto2)
{
	if(moto1<0){AIN2(1);AIN1(0);}
	else{AIN2(0);AIN1(1);}
	PWMA=myabs(moto1);
	
	if(moto2<0){BIN2(0);BIN1(1);}
	else{BIN2(1);BIN1(0);}
	PWMB=myabs(moto2);	
}

/**************************************************************************
�������ܣ�����PWM��ֵ 
��ڲ�������
����  ֵ����
**************************************************************************/
void Xianfu_Pwm(void)
{	
	int Amplitude=6900;    //===PWM������7200 ������6900
	if(Flag_Qian==1)  Moto1+=DIFFERENCE;  //DIFFERENCE��һ������ƽ��С������ͻ�е��װ�����һ��������ֱ���������������С�����и��õ�һ���ԡ�
	if(Flag_Hou==1)   Moto2-=DIFFERENCE;
	if(Moto1<-Amplitude) Moto1=-Amplitude;	
	if(Moto1>Amplitude)  Moto1=Amplitude;	
	if(Moto2<-Amplitude) Moto2=-Amplitude;	
	if(Moto2>Amplitude)  Moto2=Amplitude;			
}
/**************************************************************************
�������ܣ������޸�С������״̬ 
��ڲ�������
����  ֵ����
**************************************************************************/
void Key(void)
{	
	u8 tmp,tmp2;
	//tmp=click_N_Double(50); 
	if(tmp==1)Flag_Stop=!Flag_Stop;//��������С������ͣ
	if(tmp==2)Flag_Show=!Flag_Show;//˫������С������ʾ״̬
	//tmp2=Long_Press();                   
	if(tmp2==1) Bi_zhang=!Bi_zhang;		//��������С���Ƿ���볬��������ģʽ 
}

/**************************************************************************
�������ܣ��쳣�رյ��
��ڲ�������Ǻ͵�ѹ
����  ֵ��1���쳣  0������
**************************************************************************/
u8 Turn_Off(float angle, int voltage)
{
	u8 temp;
	if(angle<-40||angle>40||1==Flag_Stop||voltage<410)//��ص�ѹ����11.1V�رյ��
	{	                                                 //===��Ǵ���40�ȹرյ��
		temp=1;                                            //===Flag_Stop��1�رյ��
		AIN1(0);                                            
		AIN2(0);
		BIN1(0);
		BIN2(0);
	}
	else
		temp=0;
	return temp;			
}
	
/**************************************************************************
�������ܣ���ȡ�Ƕ� �����㷨�������ǵĵ�У�����ǳ����� 
��ڲ�������ȡ�Ƕȵ��㷨 1��DMP  2�������� 3�������˲�
����  ֵ����
**************************************************************************/
void Get_Angle(u8 way)
{ 
	float Accel_Y,Accel_X,Accel_Z,Gyro_Y,Gyro_Z;
	float tempera;
	unsigned char datah,datal;
	unsigned char databuf[14];
	IICreadBytes(ICM20602, ICM20602_ACCEL_XOUT_H, 14, databuf);
	//Temperature = Read_Temperature();      //===��ȡMPU6050�����¶ȴ��������ݣ����Ʊ�ʾ�����¶ȡ�
	tempera = ((databuf[6] << 8)+ databuf[7]);
	if(tempera > 32768)tempera -= 65536;
	Temperature = (int)((36.53 + tempera / 340) * 10);
	//printk("Temperature = %d\r\n",Temperature);
	if(way==1)                           //===DMP�Ķ�ȡ�����ݲɼ��ж����ѵ�ʱ���ϸ���ѭʱ��Ҫ��
	{	
		//Read_DMP();                      //===��ȡ���ٶȡ����ٶȡ����
		Angle_Balance=Pitch;             //===����ƽ�����
		Gyro_Balance=gyro[1];            //===����ƽ����ٶ�
		Gyro_Turn=gyro[2];               //===����ת����ٶ�
		Acceleration_Z=accel[2];         //===����Z����ٶȼ�
	}			
	else
	{
		//I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_H,&datah);//��ȡY��������
		//I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_YOUT_L,&datal);//��ȡY��������

		Gyro_Y = (databuf[10] << 8) + databuf[11];
		//I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_H,&datah);//��ȡZ��������
		//I2C_ReadOneByte(devAddr,MPU6050_RA_GYRO_ZOUT_L,&datal);//��ȡZ�������� 
		Gyro_Z = (databuf[12] << 8) + databuf[13];

		//I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_H,&datah);//��ȡX����ٶȼ�
		//I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_XOUT_L,&datal);//��ȡX����ٶȼ�		
		Accel_X = (databuf[0] << 8) + databuf[1];
		//I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_H,&datah);//��ȡZ����ٶȼ�
		//I2C_ReadOneByte(devAddr,MPU6050_RA_ACCEL_ZOUT_L,&datal);//��ȡZ����ٶȼ�		
		Accel_Z = (databuf[4] << 8) + databuf[5];
		//IICreadByte(ICM20602, ICM20602_GYRO_YOUT_H, &datah);
		//IICreadByte(ICM20602, ICM20602_GYRO_YOUT_L, &datal);
		//Gyro_Y = (datah << 8) + datal;
		//IICreadByte(ICM20602, ICM20602_GYRO_ZOUT_H, &datah);
		//IICreadByte(ICM20602, ICM20602_GYRO_ZOUT_H, &datal);
		//Gyro_Z = (datah << 8) + datal;
		//IICreadByte(ICM20602, ICM20602_ACCEL_XOUT_H, &datah);
		//IICreadByte(ICM20602, ICM20602_ACCEL_XOUT_H, &datal);
		//Accel_X = (datah << 8) + datal;
		//IICreadByte(ICM20602, ICM20602_ACCEL_ZOUT_H, &datah);
		//IICreadByte(ICM20602, ICM20602_ACCEL_ZOUT_L, &datal);
		//Accel_Z = (datah << 8) + datal;

		
		if(Gyro_Y>32768)  Gyro_Y-=65536;                       	//��������ת��  Ҳ��ͨ��shortǿ������ת��
		if(Gyro_Z>32768)  Gyro_Z-=65536;                       	//��������ת��
		if(Accel_X>32768) Accel_X-=65536;                      	//��������ת��
		if(Accel_Z>32768) Accel_Z-=65536;                      	//��������ת��
		//printk("%d %d %d %d\r\n",(int)Gyro_Y,(int)Gyro_Z,(int)Accel_X,(int)Accel_Z);
		Gyro_Balance=-Gyro_Y;                                  	//����ƽ����ٶ�
		Accel_Y=atan2(Accel_X,Accel_Z)*180/PI;                 	//�������	
		Gyro_Y=Gyro_Y/16.4;                                    	//����������ת��	
		if(Way_Angle==2)		  	
			Kalman_Filter(Accel_Y,-Gyro_Y);						//�������˲�	
		else if(Way_Angle==3)   
			Yijielvbo(Accel_Y,-Gyro_Y);    						//�����˲�
		Angle_Balance=angle;                                   	//����ƽ�����
		Gyro_Turn=Gyro_Z;                                      	//����ת����ٶ�
		Acceleration_Z=Accel_Z;                                	//����Z����ٶȼ�	
	}
}
/**************************************************************************
�������ܣ�����ֵ����
��ڲ�����int
����  ֵ��unsigned int
**************************************************************************/
int myabs(int a)
{ 		   
	  int temp;
		if(a<0)  temp=-a;  
	  else temp=a;
	  return temp;
}
/**************************************************************************
�������ܣ����С���Ƿ�����
��ڲ�����int
����  ֵ��unsigned int
**************************************************************************/
int Pick_Up(float Acceleration,float Angle,int encoder_left,int encoder_right)
{ 		   
	 static u16 flag,count0,count1,count2;
	if(flag==0)                                                                   //��һ��
	 {
	      if(myabs(encoder_left)+myabs(encoder_right)<30)                         //����1��С���ӽ���ֹ
				count0++;
        else 
        count0=0;		
        if(count0>10)				
		    flag=1,count0=0; 
	 } 
	 if(flag==1)                                                                  //����ڶ���
	 {
		    if(++count1>200)       count1=0,flag=0;                                 //��ʱ���ٵȴ�2000ms
	      if(Acceleration>26000&&(Angle>(-20+ZHONGZHI))&&(Angle<(20+ZHONGZHI)))   //����2��С������0�ȸ���������
		    flag=2; 
	 } 
	 if(flag==2)                                                                  //������
	 {
		  if(++count2>100)       count2=0,flag=0;                                   //��ʱ���ٵȴ�1000ms
	    if(myabs(encoder_left+encoder_right)>135)                                 //����3��С������̥��Ϊ�������ﵽ����ת��   
      {
				flag=0;                                                                                     
				return 1;                                                               //��⵽С��������
			}
	 }
	return 0;
}
/**************************************************************************
�������ܣ����С���Ƿ񱻷���
��ڲ�����int
����  ֵ��unsigned int
**************************************************************************/
int Put_Down(float Angle,int encoder_left,int encoder_right)
{ 		   
	 static u16 flag,count;	 
	 if(Flag_Stop==0)                           //��ֹ���      
   return 0;	                 
	 if(flag==0)                                               
	 {
	      if(Angle>(-10+ZHONGZHI)&&Angle<(10+ZHONGZHI)&&encoder_left==0&&encoder_right==0)         //����1��С������0�ȸ�����
		    flag=1; 
	 } 
	 if(flag==1)                                               
	 {
		  if(++count>50)                                          //��ʱ���ٵȴ� 500ms
		  {
				count=0;flag=0;
		  }
	    if(encoder_left>3&&encoder_right>3&&encoder_left<60&&encoder_right<60)                //����2��С������̥��δ�ϵ��ʱ����Ϊת��  
      {
				flag=0;
				flag=0;
				return 1;                                             //��⵽С��������
			}
	 }
	return 0;
}


