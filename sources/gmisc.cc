//
// FILE: gmisc.cc -- Miscellaneous generally-useful functions
//
// $Id$
//

#include "gmisc.h"
#include "rational.h"
#include "gnumber.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <ctype.h>

#include "gstream.h"
#include "gtext.h"
#include "garray.h"

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

template <class T> T gmax(const gArray<T> &p_array)
{
  T max = p_array[1];

  for (int i = 2; i <= p_array.Length(); i++) {
    if (p_array[i] > max)
      max = p_array[i];
  }

  return max;
}

//--------------------------------------------------------------------------
//                     Template function instantiations
//--------------------------------------------------------------------------

template int gmin(const int &a, const int &b);
template float gmin(const float &a, const float &b);
template double gmin(const double &a, const double &b);
template gRational gmin(const gRational &a, const gRational &b);

template int gmax(const int &a, const int &b);
template float gmax(const float &a, const float &b);
template double gmax(const double &a, const double &b);
template gRational gmax(const gRational &a, const gRational &b);

template int gmax(const gArray<int> &);

#ifndef hpux
double abs(double a)
{
  if (a >= 0.0)   return a;   else return -a;
}
#endif   //# hpux

gNumber abs(const gNumber &a)
{
  if (a > gNumber(0))  return a;  else return -a;
}

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

// Generates a random number between 0 and IM exclusive of endpoints
// Adapted from _Numerical_Recipes_for_C_

#define IA 16807
#define AM (1.0/IM)
#define IQ 127773
#define IR 2836
#define NTAB 32
#define NDIV (1+(IM-1)/NTAB)

long ran1(long* idum)
{
  int j;
  long k;
  static long iy = 0;
  static long iv[NTAB];

  if(*idum <= 0 || !iy) {
    if(-(*idum) < 1) *idum = 1;
    else *idum = -(*idum);
    for(j = NTAB+7; j >= 0; j--) {
      k = (*idum)/IQ;
      *idum = IA*(*idum-k*IQ)-IR*k;
      if(*idum < 0) *idum += IM;
      if(j < NTAB) iv[j] = *idum;
    }
    iy = iv[0];
  }
  k = (*idum)/IQ;
  *idum = IA*(*idum-k*IQ)-IR*k;
  if(*idum < 0) *idum += IM;
  j = iy/NDIV;
  iy = iv[j];
  iv[j] = *idum;
  return iy;
}


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
static char gconvert_buffer[GCONVERT_BUFFER_LENGTH];
static int precision = 6;
static int width = 0;

void ToTextWidth(int i)
{
  width = i;
}

int ToTextWidth(void)
{
  return width;
}


void ToTextPrecision(int i)
{
  precision = i;
}

int ToTextPrecision(void)
{
  return precision;
}

gText ToText(int i)
{
  sprintf(gconvert_buffer, "%d", i);
  return gText(gconvert_buffer);
}

gText ToText(long l)
{
  // sprintf(gconvert_buffer, "%.*ld", precision, l);
  sprintf(gconvert_buffer, "%*ld", width, l);
  return gText(gconvert_buffer);
}

gText ToText(double d)
{
  // sprintf(gconvert_buffer, "%.*f", precision, d);
  sprintf(gconvert_buffer, "%*.*f", width, precision, d);
  return gText(gconvert_buffer);
}

gText ToText(const gNumber &n)
{
  if (n.Precision() == precDOUBLE)
    return ToText((double) n);
  else
    return ToText(n.operator gRational());
}


gText ToText(const gInteger &i)
{
  return gText(Itoa(i));
}

// Note: when converting a double to a gRational, the num & den often turn out
// to be VERY large numbers due to an inherent imprecision of floating point.
// I.e. 0.50 can become 50001/100000.  This may cause serious display problems.
// However, since the main point of using rationals is to have PRECISE answers,
// no attempt will be made to 'approx reduce' the fractions.  Thus the
// approximation feature is disabled for now.

gText ToText(const gRational &r, bool )
{
/*
  if (approx)   {
    // This might be quite slow, but it does allow one to have reasonable
    // looking fractions.  I.e. .2500001 =1/4 not xxxxxxxxxxxxxx/yyyyyyyy
		// first check if this is just an integer (denominator==1)
		if (r.denominator() == gInteger(1))
      strncpy(gconvert_buffer, Itoa(r.numerator()), GCONVERT_BUFFER_LENGTH);
    else   {
      // first check if the num,den are already reasonable #'s. i.e.
      // less than precision digits long.
      double den=r.denominator().as_double();
      double num=r.numerator().as_double();
      if (log(num)<=precision && log(den)<=precision)
	return ToText(r,false);	// exact will do just fine.
      // these are nasty, huge numbers.  Make num be precision digits long,
			// and hope den will follow
      double order=pow(10.0,ceil(log(gmin(num,den))));
      double prec=pow(10.0,(double)precision);
      den/=order;	// make a double 0-1
      num/=order;
      den*=prec;
      num*=prec;
      gRational R((int)num,(int)den); // reduces automatically
      strncpy(gconvert_buffer, Itoa(R.numerator()),
	      GCONVERT_BUFFER_LENGTH / 2 - 1);
      strcat(gconvert_buffer, "/");
      strncat(gconvert_buffer, Itoa(R.denominator()),
	      GCONVERT_BUFFER_LENGTH / 2 - 1);
    }
  }
  else
    */
  {
		strncpy(gconvert_buffer, Itoa(r.numerator()), GCONVERT_BUFFER_LENGTH);
		if (r.denominator() != gInteger(1))
		{
			strcat(gconvert_buffer, "/");
			strncat(gconvert_buffer, Itoa(r.denominator()), GCONVERT_BUFFER_LENGTH);
		}
  }
  
  return gText(gconvert_buffer);
}


// conversions from strings to numbers

gRational FromText(const gText &f,gRational &y)
{
  char ch = ' ';
  int sign = 1;
  int index=0,length=f.Length();
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

  if (denom != 0)
    y = gRational(sign * num, denom);
  else
    y = gRational(sign * num);
  return y;
}

// this two-step process allows us to read in a double using either the
// standard form xxx.xxxx or a/b form.
double FromText(const gText &f,double &d)
{
  gRational R;
  FromText(f,R);
  d=(double)R;
  return d;
}

double ToDouble(const gText &s)
{ return strtod(s, NULL); }

// Rational if there is no decimal point
gNumber FromText(const gText &s,gNumber &n)
{
  gRational r;
  double d;
  gText tmp=s;
  if (tmp.LastOccur('.'))
    n=FromText(s,d);
  else
    n=FromText(s,r);
  return n;
}

gNumber ToNumber(const gText &p_string)
{
  gNumber tmp;
  return FromText(p_string, tmp);
}

gText EscapeQuotes(const gText &s)
{
  gText ret;
  
  for (int i = 0; i < s.Length(); i++)  {
    if (s[i] == '"')   ret += '\\';
    ret += s[i];
  }

  return ret;
}


//------------------------ TriState functions -----------------//

gText Name(gTriState b)
{
  switch (b) {
  case triTRUE:
    return "Y";
  case triFALSE:
    return "N"; 
  default:
    return "DK";
  }
}

gOutput &operator<<(gOutput &f, gTriState b)
{
  return (f << Name(b));
}

//------------------------ Type dependent epsilon -----------------//

void gEpsilon(double &v, int i)
{ v=pow(10.0,(double)-i); }


void gEpsilon(gRational &v, int /* i */)
{ v = (gRational)0;}

void gEpsilon(gNumber &n, int i)
{
  if (n.Precision() == precRATIONAL)
    n = (gRational)0;
  else
    n = pow(10.0,(double)-i);
}

double pow(int x, long n)
{
  return pow((double)x,(double)n);
}

double pow(double x, long n)
{
  return pow((double)x,(double)n);
}

//--------------------------------------------------------------------------
//               A Simple Class for Compact Reference to Pairs
//--------------------------------------------------------------------------



//---------------------------
// Constructors / Destructors
//---------------------------

index_pair::index_pair(const int& f, const int& s)
: first(f), second(s)
{
}

index_pair::~index_pair()
{
}

bool index_pair::operator == (const index_pair& other) const
{
  if ((first == other.first) && (second == other.second))
    return true;
  else
    return false;
}

bool index_pair::operator != (const index_pair& other) const
{
  if ((first != other.first) || (second != other.second))
    return true;
  else
    return false;
}

int index_pair::operator [] (const int& index) const 
{
  if (index == 1) return first;
  else            return second;
}
 
gOutput &operator<<(gOutput &p_output, const index_pair &p_indexPair)
{
  p_output << "(" << p_indexPair.first << "," << p_indexPair.second << ")";
  return p_output;
}

//--------------------Exceptions Related Stuff---------------------

gText gNewFailed::Description(void) const
{
  return "Memory exhausted in call to new";
}

gException::~gException()
{ }
