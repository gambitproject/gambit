//
// FILE: nfgdbl.cc -- Instantiation of double-precision normal forms
//
// $Id$
//

#include "rational.h"
#include "nfg.imp"
#include "readnfg.imp"
#include "mixedsol.imp"

template class Nfg<double>;
DataType Nfg<double>::Type(void) const { return gDOUBLE; }
double Nfg<double>::NullPayoff(void) const  { return 0.0; }

template class NfgFile<double>;

NfgFile<double>::NfgFile(gInput &f, Nfg<double> *& N)
  : NfgFileReader(f, N, fooR), type(gDOUBLE), fooR(0)
{ }

template class MixedProfile<double>;
template gOutput &operator<<(gOutput &, const MixedProfile<double> &);

template class MixedSolution<double>;
template gOutput &operator<<(gOutput &, const MixedSolution<double> &);

#include "glist.imp"

template class gList<MixedProfile<double> >;
template class gNode<MixedProfile<double> >;

template class gList<MixedSolution<double> >;
template class gNode<MixedSolution<double> >;

#include "grblock.imp"

template class gRectBlock<double>;

