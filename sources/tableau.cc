//#
//# FILE: tableau.cc -- tableau and basis class instantiations
//#
//# $Id$
//#

#include "rational.h"
#include "tableau.imp"
#include "gmatrix.imp"
#include "grarray.imp"

template class BaseTableau<double>;
template class BaseTableau<gRational>;

template class TableauInterface<double>;
template class TableauInterface<gRational>;

template class Tableau<double>;
template class Tableau<gRational>;

/*
template gOutput & operator<< {gOutput&, const Basis<double>&);
template gOutput & operator<< {gOutput&, const Basis<gRational>&);
template gOutput & operator<< {gOutput&, const Tableau<double>&);
template gOutput & operator<< {gOutput&, const Tableau<gRational>&);
*/

