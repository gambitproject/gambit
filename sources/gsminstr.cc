//#
//# FILE: gsminstr.cc -- implementation of Instruction classes for GSM's
//#                      instruction queue subsystem
//#                      companion to GSM
//#
//# $Id$
//#


#include "gsminstr.h"
#include "gambitio.h"


#include "glist.imp"

template class gList<NewInstr*>;
template class gNode<NewInstr*>;

gOutput& operator << ( gOutput& s, NewInstr* p )
{ s << p->Code; return s; }

