//
// FILE: nfgrat.cc -- Instantiation of rational-precision normal forms
//
// $Id$
//

#include "rational.h"
#include "nfg.imp"
#include "readnfg.imp"
#include "mixedsol.imp"

template class Nfg<gRational>;
DataType Nfg<gRational>::Type(void) const { return gRATIONAL; }
gRational Nfg<gRational>::NullPayoff(void) const  { return gRational(0); }

template class NfgFile<gRational>;

NfgFile<gRational>::NfgFile(gInput &f, Nfg<gRational> *& N)
  : NfgFileReader(f, fooD, N), type(gRATIONAL), fooD(0)
{ }


template class MixedProfile<gRational>;
template gOutput &operator<<(gOutput &, const MixedProfile<gRational> &);

template class MixedSolution<gRational>;
template gOutput &operator<<(gOutput &, const MixedSolution<gRational> &);

#include "glist.imp"

template class gList<MixedProfile<gRational> >;
template class gNode<MixedProfile<gRational> >;

template class gList<MixedSolution<gRational> >;
template class gNode<MixedSolution<gRational> >;

#include "grblock.imp"

template class gRectBlock<gRational>;
