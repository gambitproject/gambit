//#
//# FILE: gsmatrix.cc -- Instantiation of common squarematrix types
//#
//# $Id$ 
//#

#include "gsmatrix.imp"
#include "rational.h"


// template class gSquareMatrix<float>;
template class gSquareMatrix<double>;
template class gSquareMatrix<gRational>;

// template gOutput & operator<< (gOutput&, const gSquareMatrix<float>&);
template gOutput & operator<< (gOutput&, const gSquareMatrix<double>&);
template gOutput & operator<< (gOutput&, const gSquareMatrix<gRational>&);

