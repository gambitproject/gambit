//
// FILE: gpoly.cc -- Instantiation of gPoly class
//
// $Id$
//

#include "gpoly.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gPoly<int>;
TEMPLATE gPoly<int> operator*(const int val, const gPoly<int> poly);
TEMPLATE gPoly<int> operator*(const gPoly<int> poly, const int val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly<int> &y);

TEMPLATE class gPoly<gRational>;
TEMPLATE gPoly<gRational> operator*(const gRational val, const gPoly<gRational> poly);
TEMPLATE gPoly<gRational> operator*(const gPoly<gRational> poly, const gRational val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly<gRational> &y);

TEMPLATE class gPoly<double>;
TEMPLATE gPoly<double> operator*(const double val, const gPoly<double> poly);
TEMPLATE gPoly<double> operator*(const gPoly<double> poly, const double val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly<double> &y);

/*
TEMPLATE class gPoly<long>;
TEMPLATE gPoly<long> operator*(const long val, const gPoly<long> poly);
TEMPLATE gPoly<long> operator*(const gPoly<long> poly, const long val);
TEMPLATE gOutput &operator<<(gOutput &f, const gPoly<long> &y);
*/

int gPoly<int>::String_Coeff(int nega)
{
  gString Coeff = "";
  while (charc >= '0' && charc <= '9'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * atoi(Coeff.stradr()));  
}

/*
long gPoly<long>::String_Coeff(long nega)
{
  gString Coeff = "";
  while (charc >= '0' && charc <= '9'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * atol(Coeff.stradr()));  
}
*/

double gPoly<double>::String_Coeff(double nega)
{
  double doub;
  gString Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromString(Coeff,doub));  
}

gRational gPoly<gRational>::String_Coeff(gRational nega)
{
  gRational rat;
  gString Coeff = "";
  while (charc >= '0' && charc <= '9' || charc == '/' || charc == '.'){
    Coeff += charc;
    charnum++;
    GetChar();
  }
  if (Coeff == "") return (nega);
  else return (nega * FromString(Coeff,rat));  
}


#include "glist.imp"
#include "garray.imp"
#include "gblock.imp"

TEMPLATE class gArray< gPoly< int > * >;
TEMPLATE class gArray< gPoly< double > * >;
TEMPLATE class gArray< gPoly< gRational > *>;
TEMPLATE class gArray< Variable * >;

TEMPLATE class gBlock<Variable *>;

TEMPLATE class gList< gPoly<int> * >;
TEMPLATE class gNode< gPoly<int> * >;
TEMPLATE class gList< gPoly<gRational> * >;
TEMPLATE class gNode< gPoly<gRational> * >;
TEMPLATE class gList< gPoly<double> * >;
TEMPLATE class gNode< gPoly<double> * >;
//TEMPLATE class gList< gPoly<long> * >;
//TEMPLATE class gNode< gPoly<long> * >;




