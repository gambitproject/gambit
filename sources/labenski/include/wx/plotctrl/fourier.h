/*============================================================================

    fourier.h  -  Don Cross <dcross@intersrv.com>
    
    The file FFT.ZIP contains C source code for performing Discrete Fast Fourier
    Transforms (DFFTs) and inverse DFFTs.  

    Please see fouier.txt which was the readme.txt that Don Cross provided.
    
    This source code is public domain. Use at your own risk.  

    For more information, point your web browser at:  (dead link)
    http://www.intersrv.com/~dcross/fft.html

    Contains definitions for doing Fourier transforms
    and inverse Fourier transforms.

2003 [John Labenski]
    Merged fourierd.c (double), fourierf.c (float), and fftmisc.c into
    fourier.c for simplicity.
    
============================================================================*/

#ifndef _FOURIER_FFT_H_
#define _FOURIER_FFT_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
**   fft() computes the Fourier transform or inverse transform
**   of the complex inputs to produce the complex outputs.
**   The number of samples must be a power of two to do the
**   recursive decomposition of the FFT algorithm.
**   See Chapter 12 of "Numerical Recipes in FORTRAN" by
**   Press, Teukolsky, Vetterling, and Flannery,
**   Cambridge University Press.
**
**   returns 1 on success or 0 on failure
**
**   Notes:  If you pass ImaginaryIn = NULL, this function will "pretend"
**           that it is an array of all zeroes.  This is convenient for
**           transforming digital samples of real number data without
**           wasting memory.
*/

int fft_double (
    unsigned  NumSamples,          /* must be a power of 2 */
    int       InverseTransform,    /* 0=forward FFT, 1=inverse FFT */
    double   *RealIn,              /* array of input's real samples */
    double   *ImaginaryIn,         /* array of input's imag samples */
    double   *RealOut,             /* array of output's reals */
    double   *ImaginaryOut );      /* array of output's imaginaries */


int fft_float (
    unsigned  NumSamples,          /* must be a power of 2 */
    int       InverseTransform,    /* 0=forward FFT, 1=inverse FFT */
    float    *RealIn,              /* array of input's real samples */
    float    *ImaginaryIn,         /* array of input's imag samples */
    float    *RealOut,             /* array of output's reals */
    float    *ImaginaryOut );      /* array of output's imaginaries */

/* IsPowerOfTwo returns 1 if x is a power of two or 0 if not */
int IsPowerOfTwo ( unsigned x );
/* NumberOfBitsNeeded returns the number of bits to make the number, 2^n */
unsigned NumberOfBitsNeeded ( unsigned PowerOfTwo );
unsigned ReverseBits ( unsigned index, unsigned NumBits );

/*
**   The following function returns an "abstract frequency" of a
**   given index into a buffer with a given number of frequency samples.
**   Multiply return value by sampling rate to get frequency expressed in Hz.
*/
double Index_to_frequency ( unsigned NumSamples, unsigned Index );

#ifdef __cplusplus
}
#endif

#endif // _FOURIER_FFT_H_

/*--- end of file fourier.h ---*/
