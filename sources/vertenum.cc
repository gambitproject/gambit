//#
//# FILE: vertenum.cc -- Instantiation of vertex enumerator
//#
//#  @(#)vertenum.cc	2.4 7/1/97
//#

#include "vertenum.imp"
#include "rational.h"

template class VertEnum<double>;
template class VertEnum<gRational>;

template class NewVertEnum<double>;
template class NewVertEnum<gRational>;

