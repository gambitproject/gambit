//#
//# FILE: gmisc.cc -- Miscellaneous generally-useful functions
//#
//# $Id$
//#

#include "gmisc.h"
#include "rational.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

//--------------------------------------------------------------------------
//                      Simple mathematical functions
//--------------------------------------------------------------------------

template <class T> T gmin(const T &a, const T &b)
{
  if (a < b)   return a;   else return b;
}

template <class T> T gmax(const T &a, const T &b)
{
  if (a > b)   return a;   else return b;
}

#ifndef hpux
double abs(double a)
{
  if (a >= 0.0)   return a;   else return -a;
}
#endif   //# hpux

//
// Nasty little hack to make Borland C happy
#ifdef __BORLANDC__
inline
#endif   // __BORLANDC__
int sign(const double &a)
{
  if (a > 0.0)   return 1;
  if (a < 0.0)   return -1;
  return 0;
}

//--------------------------------------------------------------------------
//                      Generation of random numbers
//--------------------------------------------------------------------------

#ifdef RAND_MAX
#define GRAND_MAX   RAND_MAX
#elif defined INT_MAX
#define GRAND_MAX   INT_MAX
#endif

void SetSeed(unsigned int seed)
{
  srand(seed);
}

double Uniform(void)
{
  return ((double) rand()) / ((double) GRAND_MAX);
}

//--------------------------------------------------------------------------
//                         Text string conversions
//--------------------------------------------------------------------------

#define GCONVERT_BUFFER_LENGTH     64
char gconvert_buffer[GCONVERT_BUFFER_LENGTH];
int precision = 2;

void ToStringPrecision(int i)
{
  precision = i;
}

gString ToString(int i)
{
  sprintf(gconvert_buffer, "%d", i);
  return gString(gconvert_buffer);
}

gString ToString(long l)
{
  sprintf(gconvert_buffer, "%.*ld", precision, l);
  return gString(gconvert_buffer);
}

gString ToString(double d)
{
  sprintf(gconvert_buffer, "%.*f", precision, d);
  return gString(gconvert_buffer);
}

gString ToString(const gInteger &i)
{
  return gString(Itoa(i));
}

gString ToString(const gRational &r, bool approx)
{
  if (approx)   {
    // This might be quite slow, but it does allow one to have reasonable
    // looking fractions.  I.e. .2500001 =1/4 not xxxxxxxxxxxxxx/yyyyyyyy
    // first check if this is just an integer (denominator==1)
    if (r.denominator() == gInteger(1))
      strncpy(gconvert_buffer, Itoa(r.numerator()), GCONVERT_BUFFER_LENGTH);
    else   {
      int den = (int) pow(10.0, (double) precision);
      int num = (int) ((double) r * den + 0.50 * sign(r));
      gRational R(num, den);
      strncpy(gconvert_buffer, Itoa(R.numerator()),
	      GCONVERT_BUFFER_LENGTH / 2 - 1);
      strcat(gconvert_buffer, "/");
      strncat(gconvert_buffer, Itoa(R.denominator()),
	      GCONVERT_BUFFER_LENGTH / 2 - 1);
    }
  }
  else   {
    strncpy(gconvert_buffer, Itoa(r.numerator()), GCONVERT_BUFFER_LENGTH);
    strcat(gconvert_buffer, "/");
    strncat(gconvert_buffer, Itoa(r.denominator()), GCONVERT_BUFFER_LENGTH);
  }

  return gString(gconvert_buffer);
}


// conversions from strings to numbers

gRational FromString(const gString &f,gRational &y)
{
	char ch = ' ';
	int sign = 1;
	int index=0,length=f.length();
	gInteger num = 0, denom = 1;

	while (isspace(ch) && index<=length)    ch=f[index++];

	if (ch == '-' && index<=length)  {
		sign = -1;
		ch=f[index++];
	}

	while (ch >= '0' && ch <= '9' && index<=length)   {
		num *= 10;
		num += (int) (ch - '0');
		ch=f[index++];
	}

	if (ch == '/')  {
		denom = 0;
		ch=f[index++];
		while (ch >= '0' && ch <= '9' && index<=length)  {
			denom *= 10;
			denom += (int) (ch - '0');
			ch=f[index++];
		}
	}
	else if (ch == '.')  {
		denom = 1;
		ch=f[index++];
		while (ch >= '0' && ch <= '9' && index<=length)  {
			denom *= 10;
			num *= 10;
			num += (int) (ch - '0');
			ch=f[index++];
		}
	}

	y = gRational(sign * num, denom);
	return y;
}

// this two-step process allows us to read in a double using either the
// standard form xxx.xxxx or a/b form.
double FromString(const gString &f,double &d)
{
gRational R;
FromString(f,R);
d=(double)R;
return d;
}

double ToDouble(const gString &s)
{ return strtod(s, NULL); }


//--------------------------------------------------------------------------
//                     Template function instantiations
//--------------------------------------------------------------------------


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE int gmin(const int &a, const int &b);
TEMPLATE float gmin(const float &a, const float &b);
TEMPLATE double gmin(const double &a, const double &b);
TEMPLATE gRational gmin(const gRational &a, const gRational &b);

TEMPLATE int gmax(const int &a, const int &b);
TEMPLATE float gmax(const float &a, const float &b);
TEMPLATE double gmax(const double &a, const double &b);
TEMPLATE gRational gmax(const gRational &a, const gRational &b);











