//#
//# FILE: gmisc.h -- Miscellaneous generally-useful functions
//#
//# $Id$ 
//#

#ifndef GMISC_H
#define GMISC_H

//
// Defining shorthand names for some types
//
typedef unsigned int uint;
typedef unsigned long ulong;
typedef unsigned short ushort;

#ifdef __BORLANDC__
typedef unsigned char bool;
#define true     1
#define false    0
#endif   // __BORLANDC__

typedef enum { DT_ERROR, DT_DOUBLE, DT_RATIONAL, DT_MIXED }  DataType;
const DataType DOUBLE = DT_DOUBLE;
const DataType RATIONAL = DT_RATIONAL;

typedef enum { T_NO=0, T_YES=1, T_DONTKNOW=2 } TriState;

//#include "gambitio.h"
//#include "gstring.h"
class gString;
class gOutput;
gString NameTriState(TriState i);
void DisplayTriState(gOutput& o, TriState i);

//
// A few mathematically-related functions which appear frequently
//
template <class T> T gmin(const T &a, const T &b);
template <class T> T gmax(const T &a, const T &b);

//#ifndef hpux
double abs(double a);
//#endif   // hpux

int sign(const double &a);

//
// Generation of random numbers
//

// Set the seed of the random number generator
void SetSeed(unsigned int);

// Generate a random variable from the distribution U[0..1]
double Uniform(void);

// Generates a random number between 0 and IM exclusive of endpoints
// Adapted from _Numerical_Recipes_for_C_
#define IM 2147483647
long ran1(long* idum);

//
// Converting between strings and other datatypes
//

class gInteger;
class gRational;

void ToStringWidth(int); // Set # of decimal places for floating point
int  ToStringWidth(void); // Get the current value of the above
void ToStringPrecision(int); // Set # of decimal places for floating point
int  ToStringPrecision(void); // Get the current value of the above
gString ToString(int);
gString ToString(long);
gString ToString(double);
gString ToString(const gInteger &);
gString ToString(const gRational &, bool approx = true);

double		FromString(const gString &s,double &d);
gRational	FromString(const gString &s,gRational &r);

double ToDouble(const gString &);

//
// Type dependent epsilon
//

void gEpsilon(double &v, int i = 8);
void gEpsilon(gRational &v, int i = 8);

#endif    // GMISC_H





