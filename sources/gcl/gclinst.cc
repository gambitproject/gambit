//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Instantiations of templates used in GCL
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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


 
