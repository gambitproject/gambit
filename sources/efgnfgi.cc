// File: efgnfgi.cc -- instantiations of the normal->extensive forms interface
// $Id$
#include "extform.h"
#include "efgnfgi.h"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class EfgNfgInterface<double> ;
TEMPLATE class EfgNfgInterface<gRational> ;
#ifdef __BORLANDC__
#pragma -Jgx
#endif
