//#
//# FILE: btableau.cc -- Base Tableau class instantiations
//#
//# $Id$
//#

#include "rational.h"
#include "gmatrix.imp"
#include "grarray.imp"
#include "btableau.imp"

template class BaseTableau<double>;
template class BaseTableau<gRational>;

template class TableauInterface<double>;
template class TableauInterface<gRational>;


