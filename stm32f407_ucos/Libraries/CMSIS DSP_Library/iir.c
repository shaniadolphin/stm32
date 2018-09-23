#include "math.h"
#include "stdio.h"

#define     pi     (3.1415926f)

typedef struct 
{
    double Real_part;
    double Imag_Part;
} COMPLEX;

/*----------------------------------------------------------------------------
 * @����:	IIRFilter IIR�˲���
 * @����:	ϵ��*a��
			ϵ������Lenth_a��
			ϵ��*b��
			ϵ������Lenth_b��
			��������Input_Data��
			���ݻ���*Memory_Buffer
 * @����:	�˲����Output_Data
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.09.05     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
float IIRFilter(float *a, int Lenth_a,
                float *b, int Lenth_b,
                float Input_Data,
                float *Memory_Buffer) 
{
    int Count;
    float Output_Data = 0; 
    int Memory_Lenth = 0;
    
    if(Lenth_a >= Lenth_b) Memory_Lenth = Lenth_a;
    else Memory_Lenth = Lenth_b;
    
    Output_Data += (*a) * Input_Data;  //a(0)*x(n)             
    
    for(Count = 1; Count < Lenth_a ;Count++)
    {
        Output_Data -= (*(a + Count)) *
                       (*(Memory_Buffer + (Memory_Lenth - 1) - Count));                                       
    } 
    
    //------------------------save data--------------------------// 
    *(Memory_Buffer + Memory_Lenth - 1) = Output_Data;
    Output_Data = 0;
    //----------------------------------------------------------//  
    for(Count = 0; Count < Lenth_b ;Count++)
    {    	
        Output_Data += (*(b + Count)) *
                       (*(Memory_Buffer + (Memory_Lenth - 1) - Count));      
    }  
    //------------------------move data--------------------------// 
    for(Count = 0 ; Count < Memory_Lenth -1 ; Count++)
    {
    	*(Memory_Buffer + Count) = *(Memory_Buffer + Count + 1);
    }
    *(Memory_Buffer + Memory_Lenth - 1) = 0;
    //-----------------------------------------------------------//
    return (float)Output_Data; 
}
/*----------------------------------------------------------------------------
 * @����:	Complex_Multiple
 * @����:	COMPLEX a��
			COMPLEX b��
			*Res_Real��
			*Res_Imag��
 * @����:	N
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.09.05     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
int Complex_Multiple(COMPLEX a,COMPLEX b
	                                 ,double *Res_Real,double *Res_Imag)
	
{
       *(Res_Real) =  (a.Real_part)*(b.Real_part) - (a.Imag_Part)*(b.Imag_Part);
       *(Res_Imag)=  (a.Imag_Part)*(b.Real_part) + (a.Real_part)*(b.Imag_Part);	   
	 return (int)1; 
}
/*----------------------------------------------------------------------------
 * @����:	Ceil
 * @����:	input��
 * @����:	N
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.09.05     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
int Ceil(float input)
{
     if(input != (int)input) return ((int)input) +1;
     else return ((int)input); 
}
/*----------------------------------------------------------------------------
 * @����:	Buttord
 * @����:	Cotoff��
			Stopband,
			Stopband_attenuation
 * @����:	N
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.09.05     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
int Buttord(float Cotoff, float Stopband, float Stopband_attenuation)
{
   int N;

   printf("Wc =  %lf  [rad/sec]\r\n" ,Cotoff);
   printf("Ws =  %lf  [rad/sec]\r\n" ,Stopband);
   printf("As  =  %lf  [dB]\r\n" ,Stopband_attenuation);
	 
   N = Ceil(0.5*( log10f ( powf (10.0f, Stopband_attenuation/10) - 1) / 
	 	            log10f (Stopband/Cotoff) ));
   
   printf("N  =  %d\r\n" ,N);
   return (int)N;
}
/*----------------------------------------------------------------------------
 * @����:	Butter
 * @����:	N
			Cotoff��
			*a,
			*b
 * @����:	1
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.09.05     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
int Butter(int N, float Cotoff,
	           float *a,
	           float *b)
{
    float dk = 0;
    int k = 0;
    int count = 0,count_1 = 0;
	COMPLEX poles[50]={0};
	COMPLEX Res[50]={0};
	COMPLEX Res_Save[50]={0};

    if((N % 2) == 0) dk = 0.5;
    else dk = 0;

    for(k = 0;k <= ((2*N)-1) ; k++)
    {
         if(Cotoff*cos((k+dk)*(pi/N)) < 0)
         {
            poles[count].Real_part = -Cotoff*cos((k+dk)*(pi/N));
			poles[count].Imag_Part = -Cotoff*sin((k+dk)*(pi/N));	   
            count++;
	        if (count == N) break;
         }
    } 

     printf("Pk =   \r\n" );   
     for(count = 0;count < N ;count++)
     {
           printf("(%lf) + (%lf i) \r\n" ,-poles[count].Real_part
		                         	  ,-poles[count].Imag_Part);
     }
     printf("--------------------------------------------------------\n" );
	 
     Res[0].Real_part = poles[0].Real_part; 
     Res[0].Imag_Part= poles[0].Imag_Part;

     Res[1].Real_part = 1; 
     Res[1].Imag_Part= 0;

    for(count_1 = 0;count_1 < N-1;count_1++)
    {
	     for(count = 0;count <= count_1 + 2;count++)
	     {
	        if(0 == count)
			{
				Complex_Multiple(Res[count], poles[count_1+1],
						           &(Res_Save[count].Real_part),
						           &(Res_Save[count].Imag_Part));
			}
			else if((count_1 + 2) == count)
			{
	            Res_Save[count].Real_part  += Res[count - 1].Real_part;
			    Res_Save[count].Imag_Part += Res[count - 1].Imag_Part;	
	         }		  
		    else 
		    {
 	            Complex_Multiple(Res[count], poles[count_1+1],
						       &(Res_Save[count].Real_part),
						       &(Res_Save[count].Imag_Part));
				
			    Res_Save[count].Real_part  += Res[count - 1].Real_part;
			    Res_Save[count].Imag_Part += Res[count - 1].Imag_Part;			
		    }
	     }

	    for(count = 0;count <= N;count++)
		{
			Res[count].Real_part = Res_Save[count].Real_part; 
			Res[count].Imag_Part= Res_Save[count].Imag_Part;		 
			*(a + N - count) = Res[count].Real_part;
		}	 
    }
	*(b+N) = *(a+N);
	//------------------------display---------------------------------//
	printf("bs =  [" );   
	for(count = 0;count <= N ;count++)
	{
		printf("%lf ", *(b+count));
	}
	printf(" ]\r\n" );

	printf("as =  [" );   
	for(count = 0;count <= N ;count++)
	{
		printf("%lf ", *(a+count));
	}
	printf(" ]\r\n" );

	printf("--------------------------------------------------------\n" );

	return (int) 1;
}
/*----------------------------------------------------------------------------
 * @����:	Bilinear
 * @����:	N
			*as��
			*bs,
			*az,
			*bz
 * @����:	1
 * @��ʷ:
 �汾        ����         ����           �Ķ����ݺ�ԭ��
 ------    -----------	  ---------	 ----------------------	
 1.0       2014.09.05     zxjun          ����ģ��	
----------------------------------------------------------------------------*/
int Bilinear(int N, 
	         float *as,float *bs,
	         float *az,float *bz)
{
	int Count = 0,Count_1 = 0,Count_2 = 0,Count_Z = 0;
	float Res[20]={0};
	float Res_Save[20]={0}; 

	for(Count_Z = 0;Count_Z <= N;Count_Z++)//��az bzȫ������
	{
		*(az+Count_Z)  = 0;
		*(bz+Count_Z)  = 0;
	}	
	for(Count = 0;Count <= N;Count ++)
	{    	
	    for(Count_Z = 0;Count_Z <= N;Count_Z++)
		{
			Res[Count_Z] = 0;
		    Res_Save[Count_Z] = 0;	 
	    }
        Res_Save [0] = 1;
		for(Count_1 = 0; Count_1 < N-Count;Count_1++)
	    {
			for(Count_2 = 0; Count_2 <= Count_1+1;Count_2++)
			{
				if(Count_2 == 0)  
				{
					Res[Count_2] += Res_Save[Count_2];
					//printf( "Res[%d] %lf  \n" , Count_2 ,Res[Count_2]);
				} 	
				else if((Count_2 == (Count_1+1))&&(Count_1 != 0))  
				{
					Res[Count_2] += -Res_Save[Count_2 - 1];   
					//printf( "Res[%d] %lf  \n" , Count_2 ,Res[Count_2]);
				} 
				else  
				{
					Res[Count_2] += Res_Save[Count_2] - Res_Save[Count_2 - 1];
					//printf( "Res[%d] %lf  \n" , Count_2 ,Res[Count_2]);
				}				 
			}
			//printf( "Res : ");
			for(Count_Z = 0;Count_Z<= N;Count_Z++)
			{
				Res_Save[Count_Z]  =  Res[Count_Z] ;
				Res[Count_Z]  = 0;
				//printf( "[%d]  %lf  " ,Count_Z, Res_Save[Count_Z]);     
			}
			//printf(" \n" );
		}
		for(Count_1 = (N-Count); Count_1 < N;Count_1++)
		{
			for(Count_2 = 0; Count_2 <= Count_1+1;Count_2++)
			{
				if(Count_2 == 0)  
				{
					Res[Count_2] += Res_Save[Count_2];
					//printf( "Res[%d] %lf  \n" , Count_2 ,Res[Count_2]);
				} 	
				else if((Count_2 == (Count_1+1))&&(Count_1 != 0))  
				{
					Res[Count_2] += Res_Save[Count_2 - 1];
					//printf( "Res[%d] %lf  \n" , Count_2 ,Res[Count_2]);
				} 
				else  
				{
					Res[Count_2] += Res_Save[Count_2] + Res_Save[Count_2 - 1];
					//printf( "Res[%d] %lf  \n" , Count_2 ,Res[Count_2]);
				}				 
		}
		//printf( "Res : ");
		for(Count_Z = 0;Count_Z<= N;Count_Z++)
		{
			Res_Save[Count_Z]  =  Res[Count_Z] ;
			Res[Count_Z]  = 0;
			//printf( "[%d]  %lf  " ,Count_Z, Res_Save[Count_Z]);     
		}
	//printf(" \n" );
	}
	//printf( "Res : ");
	for(Count_Z = 0;Count_Z <= N;Count_Z++)
	{
		*(az+Count_Z) +=  powf(2, N-Count)  *  (*(as+Count)) * Res_Save[Count_Z];
		*(bz+Count_Z) +=  (*(bs+Count)) * Res_Save[Count_Z];		
		//printf( "  %lf  " ,*(bz+Count_Z));         
	}	
	//printf(" \n" );
	} 
	for(Count_Z = N;Count_Z >= 0;Count_Z--)
	{
		*(bz+Count_Z) =  (*(bz+Count_Z))/(*(az+0));
		*(az+Count_Z) =  (*(az+Count_Z))/(*(az+0));
	}
	//------------------------display---------------------------------//
	printf("bz =  [" );   
	for(Count_Z= 0;Count_Z <= N ;Count_Z++)
	{
		printf("%lf ", *(bz+Count_Z));
	}
	printf(" ]\r\n" );
	printf("az =  [" );   
	for(Count_Z= 0;Count_Z <= N ;Count_Z++)
	{
		printf("%lf ", *(az+Count_Z));
	}
	printf(" ]\r\n" );
	printf("--------------------------------------------------------\n" );
	return (int) 1;
}

