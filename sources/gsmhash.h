//#
//# FILE: gsmhash.h -- definitions of descendent HashTables used by
//#                    GSM and its accompanying modules
//#                    companion to GSM
//#
//# $Id$
//#



#ifndef GSMHASH_H
#define GSMHASH_H



#include "hash.h"
#include "portion.h"
#include "gsmfunc.h"



class RefHashTable : public HashTable<gString, Portion*>
{
 private:
  int NumBuckets( void ) const 
  { return 26; }
  
  int Hash( const gString& ref ) const 
  { return (int)( ref[0] % 26 ); }
  
  void DeleteAction( Portion* value ) 
  { delete value; }
  
 public:
  RefHashTable() 
  { Init(); }
  
  ~RefHashTable()
  { Flush(); }
};



class FunctionHashTable : public HashTable<gString, FuncDescObj*>
{
 private:
  int NumBuckets() const 
  { return 26; }
  
  int Hash( const gString& funcname ) const 
  { return (int)( funcname[0] % 26 ); }

  void DeleteAction( FuncDescObj* func ) 
  { delete func; }

 public:
  FunctionHashTable() 
  { Init(); }

  ~FunctionHashTable() 
  { Flush(); }  
};


#endif // GSMHASH_H
