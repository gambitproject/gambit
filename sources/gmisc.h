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

typedef enum { DOUBLE, RATIONAL }  DataType;

//
// A few mathematically-related functions which appear frequently
//
template <class T> T gmin(const T &a, const T &b);
template <class T> T gmax(const T &a, const T &b);

double abs(double a);
int sign(const double &a);

//
// Generation of random numbers
//

// Set the seed of the random number generator
void SetSeed(unsigned int);

// Generate a random variable from the distribution U[0..1]
double Uniform(void);


//
// Converting between strings and other datatypes
//
#include "gstring.h"

class gInteger;
class gRational;

void    ToStringPrecision(int);
gString ToString(int);
gString ToString(long);
gString ToString(double);
gString ToString(const gInteger &);
gString ToString(const gRational &, bool approx = true);

double ToDouble(const gString &);

#endif    // GMISC_H





