/*
 * zx_fft.c
 *
 * Implementation of Fast Fourier Transform(FFT)
 * and reversal Fast Fourier Transform(IFFT)
 * 
 *  Created on: 2013-8-5
 *      Author: monkeyzx
 *
 * TEST OK 2014.01.14
 * == 2014.01.14
 *   Replace @BitReverse(x,x,N,M) by refrence to 
 *   <The Scientist and Engineer's Guide to Digital Signal Processing>
 */
#include "math.h"
#include "stdio.h"

#include "..\Libraries\CMSIS DSP_Library\fft.h"
#include "..\Libraries\CMSIS DSP_Library\iir.h"

float lBUFMAG[SAMPLE_NODES];/* Magnitude vector */
float lBUFIN[SAMPLE_NODES];
float FFT_BUF_REL[SAMPLE_NODES];
float FFT_BUF_IMG[SAMPLE_NODES];

/*-------------------------------------------------------------------------
   This computes an in-place complex-to-complex FFT
   x and y are the real and imaginary arrays of 2^m points.
   dir =  1 gives forward transform
   dir = -1 gives reverse transform

     Formula: forward
                  N-1
                  ---
              1   \          - j k 2 pi n / N
      X(n) = ---   >   x(k) e                    = forward transform
              N   /                                n=0..N-1
                  ---
                  k=0

      Formula: reverse
                  N-1
                  ---
                  \          j k 2 pi n / N
      X(n) =       >   x(k) e                    = forward transform
                  /                                n=0..N-1
                  ---
                  k=0
*/

void fft(int dir, long m, float *x, float *y)
{
   long n,i,i1,j,k,i2,l,l1,l2;
   float c1,c2,tx,ty,t1,t2,u1,u2,z; 
   n = 1;
   for (i=0;i<m;i++)
      n *= 2;
   i2 = n >> 1;
   j = 0;
   for (i=0;i<n-1;i++) 
   {
      if (i < j) 
	  {
         tx = x[i];
         ty = y[i];
         x[i] = x[j];
         y[i] = y[j];
         x[j] = tx;
         y[j] = ty;
      }
      k = i2;
      while (k <= j) 
	  {
         j -= k;
         k >>= 1;
      }
      j += k;
   }
   c1 = -1.0;
   c2 = 0.0;
   l2 = 1;
   for (l=0;l<m;l++) 
   {
      l1 = l2;
      l2 <<= 1;
      u1 = 1.0;
      u2 = 0.0;
      for (j=0;j<l1;j++) 
	  {
         for (i=j;i<n;i+=l2) 
		 {
            i1 = i + l1;
            t1 = u1 * x[i1] - u2 * y[i1];
            t2 = u1 * y[i1] + u2 * x[i1];
            x[i1] = x[i] - t1;
            y[i1] = y[i] - t2;
            x[i] += t1;
            y[i] += t2;
         }
         z =  u1 * c1 - u2 * c2;
         u2 = u1 * c2 + u2 * c1;
         u1 = z;
      }
      c2 = sqrtf((1.0f - c1) / 2.0f);
      if (dir == 1)c2 = -c2;
      c1 = sqrtf((1.0f + c1) / 2.0f);
   } 
   if (dir == -1)
   {
      for (i=0;i<n;i++) 
	  {
         x[i] /= n;
         y[i] /= n;
      }
   }
}

void MakeInput(float fFs)
{
	unsigned int i;
	 
	float HammingWin[SAMPLE_NODES];
	float fY;

	for(i=0;i<SAMPLE_NODES+1;i++)
	{
		HammingWin[i] = (1 - 0.46164) - 0.46164*cos(2*PI*i/(SAMPLE_NODES-1));	
	}
	for(i=0;i<SAMPLE_NODES;i++)
	{
		fY = (float)(5000+3000*cos(PI2*50*i/fFs-PI*30/180)+1200*cos(PI2*75*i/fFs+PI*90/180)+120*cos(PI2*95*i/fFs+PI*90/180)); //(1+sin(PI2 * i * (fFreq1/fFs)));//S=
		//fY = (float)(1000+1000*sin(PI2 * i * (fFreq1/fFs)));
		lBUFIN[i]=fY * 1;//HammingWin[i];
		FFT_BUF_REL[i]=lBUFIN[i];
		FFT_BUF_IMG[i]=0;
		//printf("[%d]=%x ",i,(unsigned int)(FFT_BUF_REL[i]));	
		//if(i%4==3){printf("\r\n");}
	}
}

void powerMag(unsigned int nfill, float *StrPara_REL,float *StrPara_IMG)
{
	int X1,Y1;
	unsigned int  i;
	float X,Y,Mag;
	for (i=0; i < nfill/2; i++)
	{
		Mag = sqrt(StrPara_REL[i] * StrPara_REL[i] + StrPara_IMG[i] * StrPara_IMG[i]) / nfill;
		if(i == 0)
			lBUFMAG[i] = Mag;
		else 
			lBUFMAG[i] = Mag * 2;
		//printf("[%d]=%d ",i,(unsigned int)(lBUFMAG[i]));	
		//if(i%4==3){printf("\r\n");}		
	}
}

void test_fft(void)
{
	unsigned int i;	
	float fFs = 256;	
# if 0	
	float as[50];
	float bs[50];
	float az[50];
	float bz[50];
	struct DESIGN_SPECIFICATION IIR_Filter;
	IIR_Filter.Cothz = 100;
	IIR_Filter.Stophz = 120;
	IIR_Filter.Cotoff	=	(float)(PI2 * IIR_Filter.Cothz / fFs);	
	IIR_Filter.Stopband	=	(float)(PI2 * IIR_Filter.Stophz / fFs);	
	IIR_Filter.Stopband_attenuation	=	30;	
	IIR_Filter.Cotoff = 2 * tanf((IIR_Filter.Cotoff)/2);
	IIR_Filter.Stopband = 2 * tan((IIR_Filter.Stopband)/2);
	IIR_Filter.FilterNum = Buttord(IIR_Filter.Cotoff,IIR_Filter.Stopband,IIR_Filter.Stopband_attenuation);
	Butter(IIR_Filter.FilterNum,IIR_Filter.Cotoff,as,bs);
	Bilinear(IIR_Filter.FilterNum,as,bs,az,bz);
#endif	
	MakeInput(fFs);
	fft(1,8,FFT_BUF_REL,FFT_BUF_IMG);
	//powerMag(256,FFT_BUF_REL,FFT_BUF_IMG);
	//for(i=0;i<100;i++)
	//{
	//	printf("[%d]=%d ",i,(unsigned int)(lBUFMAG[i]));	
	//	if(i%4==3){printf("\r\n");}	
	//}
	fft(-1,8,FFT_BUF_REL,FFT_BUF_IMG);
	//for(i=0;i<SAMPLE_NODES;i++)
	//{
	//	printf("[%d]=%x ",i,(unsigned int)(FFT_BUF_REL[i]));	
	//	if(i%4==3){printf("\r\n");}	
	//}
	//MakeOutput(x);
}

/*
 * @brief   
 *   Init fields of structure @kalman1_state.
 *   I make some defaults in this init function:
 *     A = 1;
 *     H = 1; 
 *   and @q,@r are valued after prior tests.
 *
 *   NOTES: Please change A,H,q,r according to your application.
 *
 * @inputs  
 *   state - Klaman filter structure
 *   init_x - initial x state value   
 *   init_p - initial estimated error convariance
 * @outputs 
 * @retval  
 */
void kalman1_init(kalman1_state *state, float init_x, float init_p)
{
    state->x = init_x;
    state->p = init_p;
    state->A = 1;
    state->H = 1;
    state->q = 2e2;//10e-6;  /* predict noise convariance */
    state->r = 5e2;//10e-5;  /* measure error convariance */
}

/*
 * @brief   
 *   1 Dimension Kalman filter
 * @inputs  
 *   state - Klaman filter structure
 *   z_measure - Measure value
 * @outputs 
 * @retval  
 *   Estimated result
 */
float kalman1_filter(kalman1_state *state, float z_measure)
{
    /* Predict */
    state->x = state->A * state->x;
    state->p = state->A * state->A * state->p + state->q;  /* p(n|n-1)=A^2*p(n-1|n-1)+q */

    /* Measurement */
    state->gain = state->p * state->H / (state->p * state->H * state->H + state->r);
    state->x = state->x + state->gain * (z_measure - state->H * state->x);
    state->p = (1 - state->gain * state->H) * state->p;

    return state->x;
}

/*
 * @brief   
 *   Init fields of structure @kalman1_state.
 *   I make some defaults in this init function:
 *     A = {{1, 0.1}, {0, 1}};
 *     H = {1,0}; 
 *   and @q,@r are valued after prior tests. 
 *
 *   NOTES: Please change A,H,q,r according to your application.
 *
 * @inputs  
 * @outputs 
 * @retval  
 */
void kalman2_init(kalman2_state *state, float *init_x, float (*init_p)[2])
{
    state->x[0]    = init_x[0];
    state->x[1]    = init_x[1];
    state->p[0][0] = init_p[0][0];
    state->p[0][1] = init_p[0][1];
    state->p[1][0] = init_p[1][0];
    state->p[1][1] = init_p[1][1];
    //state->A       = {{1, 0.1}, {0, 1}};
    state->A[0][0] = 1;
    state->A[0][1] = 0.1;
    state->A[1][0] = 0;
    state->A[1][1] = 1;
    //state->H       = {1,0};
    state->H[0]    = 1;
    state->H[1]    = 0;
    //state->q       = {{10e-6,0}, {0,10e-6}};  /* measure noise convariance */
    state->q[0]    = 10e-7;
    state->q[1]    = 10e-7;
    state->r       = 10e-7;  /* estimated error convariance */
}

/*
 * @brief   
 *   2 Dimension kalman filter
 * @inputs  
 *   state - Klaman filter structure
 *   z_measure - Measure value
 * @outputs 
 *   state->x[0] - Updated state value, Such as angle,velocity
 *   state->x[1] - Updated state value, Such as diffrence angle, acceleration
 *   state->p    - Updated estimated error convatiance matrix
 * @retval  
 *   Return value is equals to state->x[0], so maybe angle or velocity.
 */
float kalman2_filter(kalman2_state *state, float z_measure)
{
    float temp0 = 0.0f;
    float temp1 = 0.0f;
    float temp = 0.0f;

    /* Step1: Predict */
    state->x[0] = state->A[0][0] * state->x[0] + state->A[0][1] * state->x[1];
    state->x[1] = state->A[1][0] * state->x[0] + state->A[1][1] * state->x[1];
    /* p(n|n-1)=A^2*p(n-1|n-1)+q */
    state->p[0][0] = state->A[0][0] * state->p[0][0] + state->A[0][1] * state->p[1][0] + state->q[0];
    state->p[0][1] = state->A[0][0] * state->p[0][1] + state->A[1][1] * state->p[1][1];
    state->p[1][0] = state->A[1][0] * state->p[0][0] + state->A[0][1] * state->p[1][0];
    state->p[1][1] = state->A[1][0] * state->p[0][1] + state->A[1][1] * state->p[1][1] + state->q[1];

    /* Step2: Measurement */
    /* gain = p * H^T * [r + H * p * H^T]^(-1), H^T means transpose. */
    temp0 = state->p[0][0] * state->H[0] + state->p[0][1] * state->H[1];
    temp1 = state->p[1][0] * state->H[0] + state->p[1][1] * state->H[1];
    temp  = state->r + state->H[0] * temp0 + state->H[1] * temp1;
    state->gain[0] = temp0 / temp;
    state->gain[1] = temp1 / temp;
    /* x(n|n) = x(n|n-1) + gain(n) * [z_measure - H(n)*x(n|n-1)]*/
    temp = state->H[0] * state->x[0] + state->H[1] * state->x[1];
    state->x[0] = state->x[0] + state->gain[0] * (z_measure - temp); 
    state->x[1] = state->x[1] + state->gain[1] * (z_measure - temp);

    /* Update @p: p(n|n) = [I - gain * H] * p(n|n-1) */
    state->p[0][0] = (1 - state->gain[0] * state->H[0]) * state->p[0][0];
    state->p[0][1] = (1 - state->gain[0] * state->H[1]) * state->p[0][1];
    state->p[1][0] = (1 - state->gain[1] * state->H[0]) * state->p[1][0];
    state->p[1][1] = (1 - state->gain[1] * state->H[1]) * state->p[1][1];

    return state->x[0];
}
/** 
 * @brief  Caluclation of SNR
 * @param  float* 	Pointer to the reference buffer
 * @param  float*	Pointer to the test buffer
 * @param  uint32_t	total number of samples
 * @return float	SNR
 * The function Caluclates signal to noise ratio for the reference output 
 * and test output 
 */

float arm_snr_f32(float *pRef, float *pTest, unsigned int buffSize)
{
  float EnergySignal = 0.0, EnergyError = 0.0;
  unsigned int i;
  float SNR;
  int temp;
  int *test;

  for (i = 0; i < buffSize; i++)
    {
 	  /* Checking for a NAN value in pRef array */
	  test =   (int *)(&pRef[i]);
      temp =  *test;

	  if(temp == 0x7FC00000)
	  {
	  		return(0);
	  }

	  /* Checking for a NAN value in pTest array */
	  test =   (int *)(&pTest[i]);
      temp =  *test;

	  if(temp == 0x7FC00000)
	  {
	  		return(0);
	  }
      EnergySignal += pRef[i] * pRef[i];
      EnergyError += (pRef[i] - pTest[i]) * (pRef[i] - pTest[i]); 
    }

	/* Checking for a NAN value in EnergyError */
	test =   (int *)(&EnergyError);
    temp =  *test;

    if(temp == 0x7FC00000)
    {
  		return(0);
    }
	

  SNR = 10 * log10 (EnergySignal / EnergyError);

  return (SNR);

} 
 
 
