//#
//# FILE: hash.h -- declaration of HashTable
//#
//# $Id$
//#



#ifndef HASH_H
#define HASH_H


#include "glist.h"



template <class K, class T> class HashTable
{
 private:
  enum { FAIL = 0, SUCCESS = 1 };

  gList<K>* key_bucket;
  gList<T>* value_bucket;
  int       num_of_buckets;
  T         illegal_value;

  int         ValidatedHash ( K key )        const;
  virtual int NumBuckets    ( void )         const = 0;
  virtual int Hash          ( const K& key ) const = 0;

  //The derived classes need to define this function to do clean ups when
  //a T type member is being removed.
  virtual void DeleteAction( T value ) = 0;


 protected:
  //This function needs to be called in the constructor of decendents classes
  void Init( void );


 public:
  HashTable();
  virtual ~HashTable();

  int  IsDefined  ( K key ) const;
  void Define     ( K key, T value );
  void Remove     ( K key );
  T    operator() ( K key ) const;
  T&   operator() ( K key );

  //This function should be called in the destructor of decendents classes
  void Flush( void );

};


#endif  // HASH_H
