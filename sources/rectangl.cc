//
// FILE: interval.cc -- Instantiation of gInterval class
// 
// $Id$ 
//

#include <assert.h>
#include "glist.imp"
#include "double.h"
#include "rectangl.imp"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gRectangle<gRational>;
TEMPLATE class gList< gRectangle<gRational> >;
TEMPLATE class gNode< gRectangle<gRational> >;
TEMPLATE gOutput& operator << (gOutput& output, const gRectangle<gRational>& x);
TEMPLATE gRectangle<gDouble> TogDouble(gRectangle<gRational>&);

TEMPLATE class gRectangle<int>;
TEMPLATE class gList< gRectangle<int> >;
TEMPLATE class gNode< gRectangle<int> >;
TEMPLATE gOutput& operator << (gOutput& output, const gRectangle<int>& x);
TEMPLATE gRectangle<gDouble> TogDouble(gRectangle<int>&);

TEMPLATE class gRectangle<double>;
TEMPLATE class gList< gRectangle<double> >;
TEMPLATE class gNode< gRectangle<double> >;
TEMPLATE gOutput& operator << (gOutput& output, const gRectangle<double>& x);
TEMPLATE gRectangle<gDouble> TogDouble(gRectangle<double>&);

TEMPLATE class gRectangle<gDouble>;
TEMPLATE class gList< gRectangle<gDouble> >;
TEMPLATE class gNode< gRectangle<gDouble> >;
TEMPLATE gOutput& operator << (gOutput& output, const gRectangle<gDouble>& x);
TEMPLATE gRectangle<gDouble> TogDouble(gRectangle<gDouble>&);
