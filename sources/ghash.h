


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
  virtual int NumBuckets() const = 0;
  virtual int Hash( K key ) const = 0;

  //The derived classes need to define this function to do clean ups when
  //a T type member is being removed.
  virtual void DeleteAction( T value ) { return; }

 protected:
  //This function should be called in the constructor of the decendents
  //because the constructor cannot call virtual function NumBuckets()
  void Init( void );

 public:
  HashTable();
  ~HashTable();
  int IsDefined( K key ) const;
  void Define( K key, T value );
  void Remove( K key );
  T operator()( K key ) const;
  T& operator()( K key );
};


