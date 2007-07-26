/*============================================================================

    fourier.c (originally fourier(f)(d).c and fftmisc.c)
        -  Don Cross <dcross@intersrv.com>

    The file FFT.ZIP contains C source code for performing Discrete Fast Fourier
    Transforms (DFFTs) and inverse DFFTs.

    Please see fouier.txt which was the readme.txt that Don Cross provided.

    This source code is public domain. Use at your own risk.

    For more information, point your web browser at:  (dead link)
    http://www.intersrv.com/~dcross/fft.html

    Helper routines for Fast Fourier Transform implementation.
    Contains common code for fft_float() and fft_double().

    See also:
        ..\include\fourier.h

    Revision history:

1998 September 19 [Don Cross]
    Improved the efficiency of IsPowerOfTwo().
    Updated coding standards.

2003 [John Labenski]
    Merged fourierd.c (double), fourierf.c (float), and fftmisc.c into
    fourier.c for simplicity.

============================================================================*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "wx/plotctrl/fourier.h"

#define BITS_PER_WORD   (sizeof(unsigned) * 8)
#define DDC_PI          (3.14159265358979323846)

/*--- start of file fftmisc.c---*/

// #define IsPowerOfTwo(x) (((x)<2) || ((x)&((x)-1)) ? false : true)

// returns 1 if the numver is a power of two else 0
int IsPowerOfTwo ( unsigned x )
{
    if ( x < 2 )
        return 0;

    if ( x & (x-1) )        // Thanks to 'byang' for this cute trick!
        return 0;

    return 1;
}

// returns the number of bits needed
unsigned NumberOfBitsNeeded ( unsigned PowerOfTwo )
{
    unsigned i;

    for ( i=0; ; i++ )
    {
        if ( PowerOfTwo & (1 << i) )
            return i;
    }

    return 0;
}

unsigned ReverseBits ( unsigned index, unsigned NumBits )
{
    unsigned i, rev;

    for ( i=rev=0; i < NumBits; i++ )
    {
        rev = (rev << 1) | (index & 1);
        index >>= 1;
    }

    return rev;
}

double Index_to_frequency ( unsigned NumSamples, unsigned Index )
{
    if ( Index >= NumSamples )
        return 0.0;
    else if ( Index <= NumSamples/2 )
        return (double)Index / (double)NumSamples;

    return -(double)(NumSamples-Index) / (double)NumSamples;
}


/*--- end of file fftmisc.c---*/


/*============================================================================

    fourierd.c  -  Don Cross <dcross@intersrv.com>

    http://www.intersrv.com/~dcross/fft.html

    Contains definitions for doing Fourier transforms
    and inverse Fourier transforms.

    This module performs operations on arrays of 'double'.

    Revision history:

1998 September 19 [Don Cross]
    Updated coding standards.
    Improved efficiency of trig calculations.

============================================================================*/

#define CHECKPOINTERDOUBLE(p)  if (CheckPointerDouble(p,#p) == 0) return 0;

static int CheckPointerDouble ( void *p, char *name )
{
    if ( p == NULL )
    {
        fprintf ( stderr, "Error in fft_double():  %s == NULL\n", name );
        return 0;
    }
    return 1;
}

int fft_double (
    unsigned  NumSamples,
    int       InverseTransform,
    double   *RealIn,
    double   *ImagIn,
    double   *RealOut,
    double   *ImagOut )
{
    unsigned NumBits;    /* Number of bits needed to store indices */
    unsigned i, j, k, n;
    unsigned BlockSize, BlockEnd;

    double angle_numerator = -2.0 * DDC_PI; // this is - to match matlab
    double tr, ti;     /* temp real, temp imaginary */

    if ( !IsPowerOfTwo(NumSamples) || (NumSamples < 2) )
    {
        fprintf (
            stderr,
            "Error in fft():  NumSamples=%u is not power of two\n",
            NumSamples );

        return 0;
    }

    if ( InverseTransform )
        angle_numerator = -angle_numerator;

    CHECKPOINTERDOUBLE ( RealIn );
    CHECKPOINTERDOUBLE ( RealOut );
    CHECKPOINTERDOUBLE ( ImagOut );

    NumBits = NumberOfBitsNeeded ( NumSamples );

    /*
    **   Do simultaneous data copy and bit-reversal ordering into outputs...
    */

    for ( i=0; i < NumSamples; i++ )
    {
        j = ReverseBits ( i, NumBits );
        RealOut[j] = RealIn[i];
        ImagOut[j] = (ImagIn == NULL) ? 0.0 : ImagIn[i];
    }

    /*
    **   Do the FFT itself...
    */

    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
    {
        double delta_angle = angle_numerator / (double)BlockSize;
        double sm2 = sin ( -2 * delta_angle );
        double sm1 = sin ( -delta_angle );
        double cm2 = cos ( -2 * delta_angle );
        double cm1 = cos ( -delta_angle );
        double w = 2 * cm1;
        /* double ar[3], ai[3]; replaced array with fixed vals below - labenski */
        double ar0, ar1, ar2, ai0, ai1, ai2;

        for ( i=0; i < NumSamples; i += BlockSize )
        {
            ar2 = cm2;
            ar1 = cm1;

            ai2 = sm2;
            ai1 = sm1;

            for ( j=i, n=0; n < BlockEnd; j++, n++ )
            {
                ar0 = w*ar1 - ar2;
                ar2 = ar1;
                ar1 = ar0;

                ai0 = w*ai1 - ai2;
                ai2 = ai1;
                ai1 = ai0;

                k = j + BlockEnd;
                tr = ar0*RealOut[k] - ai0*ImagOut[k];
                ti = ar0*ImagOut[k] + ai0*RealOut[k];

                RealOut[k] = RealOut[j] - tr;
                ImagOut[k] = ImagOut[j] - ti;

                RealOut[j] += tr;
                ImagOut[j] += ti;
            }
        }

        BlockEnd = BlockSize;
    }

    /*
    **   Need to normalize if inverse transform...
    */

    if ( InverseTransform )
    {
        double denom = (double)NumSamples;

        for ( i=0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }

    return 1;
}


/*--- end of file fourierd.c ---*/


/*============================================================================

    fourierf.c  -  Don Cross <dcross@intersrv.com>

    http://www.intersrv.com/~dcross/fft.html

    Contains definitions for doing Fourier transforms
    and inverse Fourier transforms.

    This module performs operations on arrays of 'float'.

    Revision history:

1998 September 19 [Don Cross]
    Updated coding standards.
    Improved efficiency of trig calculations.

============================================================================*/

#define CHECKPOINTERFLOAT(p)  if (CheckPointerFloat(p,#p) == 0) return 0;

static int CheckPointerFloat ( void *p, char *name )
{
    if ( p == NULL )
    {
        fprintf ( stderr, "Error in fft_float():  %s == NULL\n", name );
        return 0;
    }
    return 1;
}

int fft_float (
    unsigned  NumSamples,
    int       InverseTransform,
    float    *RealIn,
    float    *ImagIn,
    float    *RealOut,
    float    *ImagOut )
{
    unsigned NumBits;    /* Number of bits needed to store indices */
    unsigned i, j, k, n;
    unsigned BlockSize, BlockEnd;

    double angle_numerator = -2.0 * DDC_PI; // this is - to match matlab
    double tr, ti;     /* temp real, temp imaginary */

    if ( !IsPowerOfTwo(NumSamples) || (NumSamples < 2) )
    {
        fprintf (
            stderr,
            "Error in fft():  NumSamples=%u is not power of two\n",
            NumSamples );

        return 0;
    }

    if ( InverseTransform )
        angle_numerator = -angle_numerator;

    CHECKPOINTERFLOAT ( RealIn );
    CHECKPOINTERFLOAT ( RealOut );
    CHECKPOINTERFLOAT ( ImagOut );

    NumBits = NumberOfBitsNeeded ( NumSamples );

    /*
    **   Do simultaneous data copy and bit-reversal ordering into outputs...
    */

    for ( i=0; i < NumSamples; i++ )
    {
        j = ReverseBits ( i, NumBits );
        RealOut[j] = RealIn[i];
        ImagOut[j] = (ImagIn == NULL) ? (float)0.0 : ImagIn[i];
    }

    /*
    **   Do the FFT itself...
    */

    BlockEnd = 1;
    for ( BlockSize = 2; BlockSize <= NumSamples; BlockSize <<= 1 )
    {
        double delta_angle = angle_numerator / (double)BlockSize;
        double sm2 = sin ( -2 * delta_angle );
        double sm1 = sin ( -delta_angle );
        double cm2 = cos ( -2 * delta_angle );
        double cm1 = cos ( -delta_angle );
        double w = 2 * cm1;
        /* double ar[3], ai[3]; replaced array with fixed vals below - labenski */
        double ar0, ar1, ar2, ai0, ai1, ai2;
        /* double temp; */

        for ( i=0; i < NumSamples; i += BlockSize )
        {
            ar2 = cm2;
            ar1 = cm1;

            ai2 = sm2;
            ai1 = sm1;

            for ( j=i, n=0; n < BlockEnd; j++, n++ )
            {
                ar0 = w*ar1 - ar2;
                ar2 = ar1;
                ar1 = ar0;

                ai0 = w*ai1 - ai2;
                ai2 = ai1;
                ai1 = ai0;

                k = j + BlockEnd;
                tr = ar0*RealOut[k] - ai0*ImagOut[k];
                ti = ar0*ImagOut[k] + ai0*RealOut[k];

                RealOut[k] = (float)(RealOut[j] - tr);
                ImagOut[k] = (float)(ImagOut[j] - ti);

                RealOut[j] += (float)tr;
                ImagOut[j] += (float)ti;
            }
        }

        BlockEnd = BlockSize;
    }

    /*
    **   Need to normalize if inverse transform...
    */

    if ( InverseTransform )
    {
        float denom = (float)NumSamples;

        for ( i=0; i < NumSamples; i++ )
        {
            RealOut[i] /= denom;
            ImagOut[i] /= denom;
        }
    }
    return 1;
}

/*--- end of file fourierf.c ---*/
