//#
//# FILE: tableau.cc -- tableau and basis class instantiations
//#
//# $Id$
//#

#include "rational.h"
#include "tableau.imp"

template class BaseTableau<double>;
template class BaseTableau<gRational>;

template class Tableau<double>;
template class Tableau<gRational>;

template class LPTableau<double>;
template class LPTableau<gRational>;

/*
template gOutput & operator<< {gOutput&, const Basis<double>&);
template gOutput & operator<< {gOutput&, const Basis<gRational>&);
template gOutput & operator<< {gOutput&, const Tableau<double>&);
template gOutput & operator<< {gOutput&, const Tableau<gRational>&);
*/

