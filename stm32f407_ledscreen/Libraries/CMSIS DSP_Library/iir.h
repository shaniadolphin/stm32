#ifndef _IIR_H
#define _IIR_H

struct DESIGN_SPECIFICATION 
{ 
	float Cothz;
	float Stophz;
	float Cotoff;
	float Stopband;
	float Stopband_attenuation;
	int FilterNum;
};

extern float IIRFilter(float *a, int Lenth_a,float *b, int Lenth_b,float Input_Data,float *Memory_Buffer); 
extern int Buttord(float Cotoff, float Stopband, float Stopband_attenuation);
extern int Butter(int N, float Cotoff,float *a,float *b);
extern int Bilinear(int N, float *as,float *bs,float *az,float *bz);
#endif