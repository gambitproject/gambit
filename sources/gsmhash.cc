//
// FILE: gsmhash.h -- definitions of descendent HashTables used by
//                    GSM and its accompanying modules
//                    companion to GSM
//
// $Id$
//



#include "gstring.h"

class NewInstr;

template <class T> class gList;
template <class T> class gNode;


#include "portion.h"
#include "gsmfunc.h"


#include "glist.imp"

//template class gList< gString >;
//template class gNode< gString >;
//template class gList< int >;
//template class gNode< int >;
//template class gList< NewInstr* >;
//template class gNode< NewInstr* >;

template class gList< void* >;
template class gNode< void* >;

template class gList< Portion* >;
template class gNode< Portion* >;

template class gList< FuncDescObj* >;
template class gNode< FuncDescObj* >;

template class gList< gList< NewInstr* >* >;
template class gNode< gList< NewInstr* >* >;



#include "hash.imp"

template class HashTable< gString, Portion* >;
template class HashTable< gString, FuncDescObj* >;
template class HashTable< gList< NewInstr* >*, int >;
template class HashTable< void*, int >;



#include "gsmhash.h"






int RefHashTable::NumBuckets( void ) const
{ return 26; }

int RefHashTable::Hash( const gString& ref ) const
{ return (int)( ref[0] % 26 ); }

void RefHashTable::DeleteAction( Portion* value )
{ delete value; }

RefHashTable::RefHashTable()
{ Init(); }

RefHashTable::~RefHashTable()
{ Flush(); }



int FunctionHashTable::NumBuckets() const
{ return 26; }

int FunctionHashTable::Hash( const gString& funcname ) const 
{ return (int)( funcname[0] ) % 26; }

void FunctionHashTable::DeleteAction( FuncDescObj* func ) 
{ delete func; }

FunctionHashTable::FunctionHashTable() 
{ Init(); }

FunctionHashTable::~FunctionHashTable() 
{ Flush(); }  





template <class T> int RefCountHashTable<T>::NumBuckets( void ) const 
{ return 10; }

template <class T> int RefCountHashTable<T>::Hash( const T& ptr ) const 
{ return ( (unsigned int) ptr / 4 ) % 10; }

template <class T> void RefCountHashTable<T>::DeleteAction( int /*value*/ ) 
{ }

template <class T> RefCountHashTable<T>::RefCountHashTable() 
{ Init(); }

template <class T> RefCountHashTable<T>::~RefCountHashTable()
{ Flush(); }




template class RefCountHashTable< gList< NewInstr* >* >;
template class RefCountHashTable< void* >;

