//
// FILE: lhtab.cc -- Lemke-Howson tableau instantiations
//
// $Id$
//

#include "lhtab.imp"

template class LHTableau<double>;
template class LHTableau<gRational>;
template gMatrix<double> Make_A1(const Nfg &, const NFSupport &, const double &);
template gMatrix<gRational> Make_A1(const Nfg &, const NFSupport &, const gRational &);
template gVector<double> Make_b1(const Nfg &, const NFSupport &, const double &);
template gVector<gRational> Make_b1(const Nfg &, const NFSupport &, const gRational &);
template gMatrix<double> Make_A2(const Nfg &, const NFSupport &, const double &);
template gMatrix<gRational> Make_A2(const Nfg &, const NFSupport &, const gRational &);
template gVector<double> Make_b2(const Nfg &, const NFSupport &, const double &);
template gVector<gRational> Make_b2(const Nfg &, const NFSupport &, const gRational &);



