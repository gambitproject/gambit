//
// File: ideal.cc  
//        -- Instantiations of classes gBasis and gIdeal
// 
// $Id$
//

#include "ideal.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

//TEMPLATE class gIdeal<int>;
//TEMPLATE class gBasis<int>;
//TEMPLATE gOutput &operator<<(gOutput &f, const gBasis<int> &y);

TEMPLATE class gIdeal<gRational>;
//TEMPLATE class gBasis<gRational>;
//TEMPLATE gOutput &operator<<(gOutput &f, const gBasis<gRational> &y);

TEMPLATE class gIdeal<double>;
//TEMPLATE class gBasis<double>;
//TEMPLATE gOutput &operator<<(gOutput &f, const gBasis<double> &y);

TEMPLATE class gIdeal<gDouble>;


