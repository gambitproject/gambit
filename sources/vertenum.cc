//#
//# FILE: vertenum.cc -- Instantiation of vertex enumerator
//#
//#  $Id$
//#

#include "vertenum.imp"
#include "rational.h"

template class VertEnum<double>;
template class VertEnum<gRational>;

template class NewVertEnum<double>;
template class NewVertEnum<gRational>;

