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

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gList<NewInstr *>;
class gNode<NewInstr *>;
class gList<int>;
class gNode<int>;
class gList<gString>;
class gNode<gString>;
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__

#include "portion.h"
#include "gsmfunc.h"


#include "glist.imp"

//TEMPLATE class gList< gString >;
//TEMPLATE class gNode< gString >;
//TEMPLATE class gList< int >;
//TEMPLATE class gNode< int >;
//TEMPLATE class gList< NewInstr* >;
//TEMPLATE class gNode< NewInstr* >;

TEMPLATE class gList< void* >;
TEMPLATE class gNode< void* >;

TEMPLATE class gList< Portion* >;
TEMPLATE class gNode< Portion* >;

TEMPLATE class gList< FuncDescObj* >;
TEMPLATE class gNode< FuncDescObj* >;

TEMPLATE class gList< gList< NewInstr* >* >;
TEMPLATE class gNode< gList< NewInstr* >* >;



#include "hash.imp"

TEMPLATE class HashTable< gString, Portion* >;
TEMPLATE class HashTable< gString, FuncDescObj* >;
TEMPLATE class HashTable< gList< NewInstr* >*, int >;
TEMPLATE class HashTable< void*, int >;



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




TEMPLATE class RefCountHashTable< gList< NewInstr* >* >;
TEMPLATE class RefCountHashTable< void* >;

