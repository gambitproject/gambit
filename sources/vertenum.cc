//
// FILE: vertenum.cc -- Instantiation of vertex enumerator
//
//  $Id$
//

#include "vertenum.imp"
#include "math/rational.h"

template class VertEnum<double>;
template class VertEnum<gRational>;

template class DoubleVertEnum<double>;
template class DoubleVertEnum<gRational>;
