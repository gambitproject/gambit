//
// FILE: btableau.cc -- Base Tableau class instantiations
//
// $Id$
//

#include "math/rational.h"
#include "math/gmatrix.imp"
#include "base/grarray.imp"
#include "btableau.imp"

template class BaseTableau<double>;
template class BaseTableau<gRational>;

template class TableauInterface<double>;
template class TableauInterface<gRational>;


