


#include "hash.imp"






class TestHashTable : public HashTable<char, int>
{
 private:
  int NumBuckets() const { return 26; }
  int Hash( char key ) const { return (int)(key / 3 ) % 26; };

 public:
};




int main( void )
{
  TestHashTable hash;
  
  hash.Define( 'a', 1 );
  hash.Define( 'b', 2 );
  hash.Define( 'c', 3 );
  hash.Define( 'd', 4 );
  hash.Define( 'e', 5 );
  hash.Define( 'f', 6 );
  hash.Define( 'g', 7 );
  hash.Define( 'h', 8 );


  printf("a: %d\n", hash('a') );
  printf("b: %d\n", hash('b') );
  printf("c: %d\n", hash('c') );
  printf("d: %d\n", hash('d') );
  printf("e: %d\n", hash('e') );
  printf("f: %d\n", hash('f') );
  printf("g: %d\n", hash('g') );
  printf("h: %d\n", hash('h') );

  printf("\nCalling Remove() on f, h, d, a\n");
  hash.Remove('f');
  hash.Remove('h');
  hash.Remove('d');
  hash.Remove('a');

  printf("a: %d\n", hash('a') );
  printf("b: %d\n", hash('b') );
  printf("c: %d\n", hash('c') );
  printf("d: %d\n", hash('d') );
  printf("e: %d\n", hash('e') );
  printf("f: %d\n", hash('f') );
  printf("g: %d\n", hash('g') );
  printf("h: %d\n", hash('h') );

  printf("\nTesting IsDefined()  // 0 means undefined\n");
  printf("a: %d\n", hash.IsDefined('a') );
  printf("b: %d\n", hash.IsDefined('b') );
  printf("c: %d\n", hash.IsDefined('c') );
  printf("d: %d\n", hash.IsDefined('d') );
  printf("e: %d\n", hash.IsDefined('e') );
  printf("f: %d\n", hash.IsDefined('f') );
  printf("g: %d\n", hash.IsDefined('g') );
  printf("h: %d\n", hash.IsDefined('h') );

  printf("\nAttempt to re-Define() all keys  // 0 means FAIL; previously defined\n");
  printf("a: %d\n", hash.Define( 'a', 11 ) );
  printf("b: %d\n", hash.Define( 'b', 12 ) );
  printf("c: %d\n", hash.Define( 'c', 13 ) );
  printf("d: %d\n", hash.Define( 'd', 14 ) );
  printf("e: %d\n", hash.Define( 'e', 15 ) );
  printf("f: %d\n", hash.Define( 'f', 16 ) );
  printf("g: %d\n", hash.Define( 'g', 17 ) );
  printf("h: %d\n", hash.Define( 'h', 18 ) );
  
  printf("Result:\n");
  printf("a: %d\n", hash('a') );
  printf("b: %d\n", hash('b') );
  printf("c: %d\n", hash('c') );
  printf("d: %d\n", hash('d') );
  printf("e: %d\n", hash('e') );
  printf("f: %d\n", hash('f') );
  printf("g: %d\n", hash('g') );
  printf("h: %d\n", hash('h') );

}


