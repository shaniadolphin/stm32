
#ifndef _FFT_H
#define _FFT_H

#ifndef PI
#define PI      (3.14159265f)
#endif

#define PI2 	(2 * PI)

#define  SAMPLE_NODES              (1024)

typedef struct {
    float real;
	float imag;
}COMPLEX;

/* 
 * NOTES: n Dimension means the state is n dimension, 
 * measurement always 1 dimension 
 */

/* 1 Dimension */
typedef struct {
    float x;  /* state */
    float A;  /* x(n)=A*x(n-1)+u(n),u(n)~N(0,q) */
    float H;  /* z(n)=H*x(n)+w(n),w(n)~N(0,r)   */
    float q;  /* process(predict) noise convariance */
    float r;  /* measure noise convariance */
    float p;  /* estimated error convariance */
    float gain;
} kalman1_state;

/* 2 Dimension */
typedef struct {
    float x[2];     /* state: [0]-angle [1]-diffrence of angle, 2x1 */
    float A[2][2];  /* X(n)=A*X(n-1)+U(n),U(n)~N(0,q), 2x2 */
    float H[2];     /* Z(n)=H*X(n)+W(n),W(n)~N(0,r), 1x2   */
    float q[2];     /* process(predict) noise convariance,2x1 [q0,0; 0,q1] */
    float r;        /* measure noise convariance */
    float p[2][2];  /* estimated error convariance,2x2 [p0 p1; p2 p3] */
    float gain[2];  /* 2x1 */
} kalman2_state;                   

extern void cr4_fft(int dir, long m, float *x, float *y);
extern void test_fft(unsigned short *dataraw, unsigned int datarawlen);
#endif /* ZX_FFT_H_ */

