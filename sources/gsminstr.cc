//#
//# FILE: gsminstr.cc -- implementation of Instruction classes for GSM's
//#                      instruction queue subsystem
//#                      companion to GSM
//#
//# @(#)gsminstr.cc	1.32 7/24/96
//#


#include "gsminstr.h"
#include "gambitio.h"

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "glist.imp"

TEMPLATE class gList<NewInstr*>;
TEMPLATE class gNode<NewInstr*>;

gOutput& operator << ( gOutput& s, NewInstr* p )
{ s << p->Code; return s; }

