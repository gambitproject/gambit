//
// FILE: gmatrix.cc -- Instantiation of common matrix types
//
// $Id$
//

#include "gmatrix.imp"
#include "rational.h"

template class gMatrix<double>;
template class gMatrix<gRational>;
template class gMatrix<gInteger>;
template class gMatrix<int>;

template gVector<double> operator*(const gVector<double> &,
				   const gMatrix<double> &);
template gVector<gRational> operator*(const gVector<gRational> &,
				      const gMatrix<gRational> &);
template gVector<gInteger> operator*(const gVector<gInteger> &,
				     const gMatrix<gInteger> &);
template gVector<int> operator*(const gVector<int> &,
				const gMatrix<int> &);

template gOutput &operator<<(gOutput &, const gMatrix<double> &);
template gOutput &operator<<(gOutput &, const gMatrix<gRational> &);
template gOutput &operator<<(gOutput &, const gMatrix<gInteger> &);
template gOutput &operator<<(gOutput &, const gMatrix<int> &);
