//#
//# FILE: gsmhash.h -- definitions of descendent HashTables used by
//#                    GSM and its accompanying modules
//#                    companion to GSM
//#
//# $Id$
//#




#include "gsmhash.h"

#include "gstring.h"
#include "portion.h"
#include "gsmfunc.h"







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
{ return ( (int) ptr / 4 ) % 10; }

template <class T> void RefCountHashTable<T>::DeleteAction( int value ) 
{ }

template <class T> RefCountHashTable<T>::RefCountHashTable() 
{ Init(); }

template <class T> RefCountHashTable<T>::~RefCountHashTable()
{ Flush(); }









#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__






#include "glist.imp"

TEMPLATE class gNode< int >;
TEMPLATE class gList< int >;

TEMPLATE class gNode< gOutput* >;
TEMPLATE class gList< gOutput* >;



TEMPLATE class gNode< BaseNormalForm* >;
TEMPLATE class gList< BaseNormalForm* >;

TEMPLATE class gNode< BaseExtForm* >;
TEMPLATE class gList< BaseExtForm* >;

TEMPLATE class gList< Portion* >;
TEMPLATE class gNode< Portion* >;

TEMPLATE class gList< FuncDescObj* >;
TEMPLATE class gNode< FuncDescObj* >;

TEMPLATE class gList< gList< Instruction* >* >;
TEMPLATE class gNode< gList< Instruction* >* >;

/* 
   already declared in readefg.y
   TEMPLATE class gList< gString >;
   TEMPLATE class gNode< gString >;
   */





#include "hash.imp"

TEMPLATE class HashTable< gOutput*, int >;
TEMPLATE class RefCountHashTable< gOutput* >;



TEMPLATE class HashTable< BaseNormalForm*, int >;
TEMPLATE class RefCountHashTable< BaseNormalForm* >;

TEMPLATE class HashTable< BaseExtForm*, int >;
TEMPLATE class RefCountHashTable< BaseExtForm* >;

TEMPLATE class HashTable< gList< Instruction* >*, int >;
TEMPLATE class RefCountHashTable< gList< Instruction* >* >;


TEMPLATE class HashTable< gString, Portion* >;
TEMPLATE class HashTable< gString, FuncDescObj* >;




