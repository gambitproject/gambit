//#
//# FILE: grblock.cc  -- Instantiation of common gRectBlock types
//#
//# @(#)grblock.cc   1.2 6/1/95
//#

#include "grblock.imp"
#include "rational.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

TEMPLATE class gRectBlock<int>;


