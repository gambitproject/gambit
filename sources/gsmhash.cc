//
// FILE: gsmhash.h -- definitions of descendent HashTables used by
//                    GSM and its accompanying modules
//                    companion to GSM
//
// $Id$
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
{ return (int)( ref[0] % 26 ); }

void RefHashTable::DeleteAction( Portion* value )
{ delete value; }

RefHashTable::RefHashTable(void)
  : HashTable<gText, Portion *>(26)
{ }

RefHashTable::~RefHashTable()
{ Flush(); }



int FunctionHashTable::Hash( const gText& funcname ) const 
{ return (int)( funcname[0] ) % 26; }

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

