//
// FILE: gsmatrix.cc -- Instantiation of common squarematrix types
//
// $Id$
//

#include "gsmatrix.imp"
#include "rational.h"
#include "double.h"


template class gSquareMatrix<double>;
//template class gSquareMatrix<gDouble>;
template class gSquareMatrix<gRational>;

template gOutput & operator<< (gOutput&, const gSquareMatrix<double>&);
template gOutput & operator<< (gOutput&, const gSquareMatrix<gDouble>&);
template gOutput & operator<< (gOutput&, const gSquareMatrix<gRational>&);

