//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiation of algorithm to compute mixed strategy equilibria
// of constant sum normal form games via linear programming
//

#include "nfgcsum.imp"
#include "math/rational.h"

template class nfgLp<double>;
template class nfgLp<gRational>;

