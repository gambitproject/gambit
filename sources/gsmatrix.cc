//#
//# FILE: gsmatrix.cc -- Instantiation of common squarematrix types
//#
//# @(#)gsmatrix.cc	2.5 07/01/97 
//#

#include "gsmatrix.imp"
#include "rational.h"


// template class gSquareMatrix<float>;
template class gSquareMatrix<double>;
template class gSquareMatrix<gRational>;

// template gOutput & operator<< (gOutput&, const gSquareMatrix<float>&);
template gOutput & operator<< (gOutput&, const gSquareMatrix<double>&);
template gOutput & operator<< (gOutput&, const gSquareMatrix<gRational>&);

