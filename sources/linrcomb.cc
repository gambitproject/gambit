//
// FILE: linrcomb.cc -- Instantiation of common matrix types
//
// $Id$
//

#include "linrcomb.imp"
#include "math/gvector.imp"
#include "math/gmatrix.imp"
#include "math/double.h"

template gOutput & operator<< (gOutput&, const gMatrix<gDouble>&);
template gVector<gDouble> operator* (const gVector<gDouble>&,
			  	     const gMatrix<gDouble>&);

template class LinearCombination<double>;
template class LinearCombination<gDouble>;
template class LinearCombination<gRational>;

template gOutput & operator<< (gOutput&, const LinearCombination<double>&);
template gOutput & operator<< (gOutput&, const LinearCombination<gDouble>&);
template gOutput & operator<< (gOutput&, const LinearCombination<gRational>&);

