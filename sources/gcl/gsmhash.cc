//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of hash tables used by GCL
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

template <class T> class gList;
template <class T> class gNode;


#include "portion.h"
#include "gsmfunc.h"


#include "base/glist.imp"

template class gList< void* >;
template class gList< Portion* >;
template class gList< gclFunction* >;

#include "base/ghash.imp"

template class HashTable< gText, Portion* >;
template class HashTable< gText, gclFunction* >;
template class HashTable< gclExpression*, int >;
template class HashTable< void*, int >;



#include "gsmhash.h"

int RefHashTable::Hash( const gText& ref ) const
{ return (int)( ref[0u] % 26 ); }

void RefHashTable::DeleteAction( Portion* value )
{ delete value; }

RefHashTable::RefHashTable(void)
  : HashTable<gText, Portion *>(26)
{ }

RefHashTable::~RefHashTable()
{ Flush(); }



int FunctionHashTable::Hash( const gText& funcname ) const 
{ return (int)( funcname[0u] ) % 26; }

void FunctionHashTable::DeleteAction( gclFunction* func ) 
{ delete func; }

FunctionHashTable::FunctionHashTable() 
  : HashTable<gText, gclFunction *>(26)
{ }

FunctionHashTable::~FunctionHashTable() 
{ Flush(); }  





template <class T> int RefCountHashTable<T>::Hash( const T& ptr ) const 
{ return ( (unsigned int) ptr / 4 ) % 10; }

template <class T> void RefCountHashTable<T>::DeleteAction( int /*value*/ ) 
{ }

template <class T> RefCountHashTable<T>::RefCountHashTable()
  : HashTable<T, int>(10) 
{ }

template <class T> RefCountHashTable<T>::~RefCountHashTable()
{ Flush(); }




template class RefCountHashTable< gclExpression* >;
template class RefCountHashTable< void* >;

