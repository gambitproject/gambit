//
// FILE: gsmhash.h -- definitions of descendent HashTables used by
//                    GSM and its accompanying modules
//                    companion to GSM
//
// $Id$
//

#ifndef GSMHASH_H
#define GSMHASH_H

#include "ghash.h"

class Portion;
class gclFunction;
class gText;


class RefHashTable : public HashTable<gText, Portion*> {
private:
  int Hash( const gText& ref ) const;
  void DeleteAction(Portion* value );
  
public:
  RefHashTable();
  virtual ~RefHashTable();
};

class FunctionHashTable : public HashTable<gText, gclFunction*> {
private:
  int Hash( const gText& funcname ) const;
  void DeleteAction( gclFunction* func );

public:
  FunctionHashTable();
  virtual ~FunctionHashTable();

};

template <class T> class RefCountHashTable : public HashTable<T, int> {
 private:
  int Hash( const T& ptr ) const;
  void DeleteAction( int value );
  
public:
  RefCountHashTable();
  virtual ~RefCountHashTable();
};

#endif // GSMHASH_H
