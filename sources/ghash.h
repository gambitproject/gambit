


#include "glist.h"



template <class K, class T> class HashTable
{
 private:
  enum { FAIL = 0, SUCCESS = 1 };
  gList<K> *key_bucket;
  gList<T> *value_bucket;
  int num_of_buckets;
  T illegal_value;
  virtual int NumBuckets() const = 0;
  virtual int Hash( K key ) const = 0;
  int ValidatedHash( K key ) const;

 public:
  HashTable();
  virtual ~HashTable();
  int IsDefined( K key ) const;
  int Define( K key, T value );
  void Remove( K key );
  T operator()( K key ) const;
  T& operator()( K key );
};


