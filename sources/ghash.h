//
  //  FILE:  hash.h :  declaration of HashTable
//
//


#ifndef HASH_H
#define HASH_H

#include "glist.h"



template <class K, class T> class HashTable
{
 private:
  enum { FAIL = 0, SUCCESS = 1 };
  gList<K> *key_bucket;
  gList<T> *value_bucket;
  int num_of_buckets;
  T illegal_value;
  int ValidatedHash( K key ) const;
  virtual int NumBuckets( void ) const = 0;
  virtual int Hash( const K& key ) const = 0;

  //The derived classes need to define this function to do clean ups when
  //a T type member is being removed.
  virtual void DeleteAction( T value ) = 0; //  { return; }

 protected:
  //This function should be called in the constructor of the decendents
  //because the constructor cannot call virtual function NumBuckets()
  void Init( void );

 public:
  HashTable();
  virtual ~HashTable();
  int IsDefined( K key ) const;
  void Define( K key, T value );
  void Remove( K key );
  T operator()( K key ) const;
  T& operator()( K key );

  //This function should be called in the destructor of the decendents
  //because the destructor cannot call virtual function DeleteAction()
  void Flush( void );

};


#endif  // HASH_H
