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



class Portion;
class FuncDescObj;
class gString;


class RefHashTable : public HashTable<gString, Portion*>
{
 private:
  int Hash( const gString& ref ) const;
  void DeleteAction( Portion* value );
  
 public:
  RefHashTable();
  ~RefHashTable();

  int NumBuckets( void ) const;
};



class FunctionHashTable : public HashTable<gString, FuncDescObj*>
{
 private:
  int Hash( const gString& funcname ) const;
  void DeleteAction( FuncDescObj* func );

 public:
  FunctionHashTable();
  ~FunctionHashTable();

  int NumBuckets() const;
};




template <class T> class RefCountHashTable : public HashTable<T, int>
{
 private:
  int Hash( const T& ptr ) const;
  void DeleteAction( int value );
  
 public:
  RefCountHashTable();
  ~RefCountHashTable();

  int NumBuckets( void ) const;
};






#endif // GSMHASH_H
