//
// FILE: gclinst.cc -- Instantiations for GCL
//
// $Id$
//

#include "gtext.h"

#include "gstack.imp"
#include "garray.imp"
#include "gblock.imp"
#include "glist.imp"
#include "gslist.imp"

class Portion;
class EfgPortion;
class NfgPortion;
class gclFunction;
class CallFuncObj;
class RefHashTable;
class gclExpression;

//----------
// Stacks
//----------

template class gStack<gText>;
template class gStack<int>;
template class gStack<char>;
template class gStack<gInput *>;
template class gStack<unsigned int>;

template class gStack<Portion *>;
template class gStack<gStack<Portion *> *>;
template class gStack<CallFuncObj *>;
template class gStack<RefHashTable *>;

//--------
// Lists
//--------

template class gList<gclExpression *>;

template class gSortList<gclFunction *>;
template class gListSorter<gclFunction *>;
template class gSortList<gText>;
template class gListSorter<gText>;


//--------------------
// Arrays and Blocks
//--------------------

template class gArray<NfgPortion *>;
template class gBlock<NfgPortion *>;

template class gArray<EfgPortion *>;
template class gBlock<EfgPortion *>;


 
