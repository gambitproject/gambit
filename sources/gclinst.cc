//
// FILE: gclinst.cc -- Instantiations for GCL
//
// $Id$
//

#include "base/base.h"

#include "base/gstack.imp"
#include "base/garray.imp"
#include "base/gblock.imp"
#include "base/glist.imp"
#include "base/gslist.imp"

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


 
