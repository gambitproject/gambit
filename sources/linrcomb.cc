//
// FILE: linrcomb.cc -- Instantiation of common matrix types
//
// $Id$ 
//

#include "linrcomb.imp"
#include "gvector.imp"
#include "gmatrix.imp"
#include "double.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

template gOutput & operator<< (gOutput&, const gMatrix<gDouble>&);
template gVector<gDouble> operator* (const gVector<gDouble>&,
			  	     const gMatrix<gDouble>&);

TEMPLATE class LinearCombination<double>;
TEMPLATE class LinearCombination<gDouble>;
TEMPLATE class LinearCombination<gRational>;

TEMPLATE gOutput & operator<< (gOutput&, const LinearCombination<double>&);
TEMPLATE gOutput & operator<< (gOutput&, const LinearCombination<gDouble>&);
TEMPLATE gOutput & operator<< (gOutput&, const LinearCombination<gRational>&);

