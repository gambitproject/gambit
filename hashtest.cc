

#include "gambitio.h"
#include "hash.imp"




class Test
{
 private:
  //static int num = 0;
  int val;

 public:
  Test( int v = 0 )
  {
    //num++;
    //gout << num << "\n";
    gout << "Created\n";
    val = v;
  }
  ~Test()
  {
    //num++;
    //gout << num << "\n";
    gout << "Deleted\n";
  }
  int Value()
  {
    return val;
  }
};



class TestHashTable : public HashTable<char, Test *>
{
 private:
  int NumBuckets() const { return 26; }
  int Hash( char key ) const { return ((int)(key) / 3) % 26; }

 protected:
  void DeleteAction( Test *value )
  { delete value; }

 public:
  TestHashTable() { Init(); }
};




int main( void )
{
  TestHashTable hash;

  Test *v1, *v2, *v3, *v4, *v5;
  v1 = new Test( 1 );
  v2 = new Test( 2 );
  v3 = new Test( 3 );
  v4 = new Test( 4 );
  v5 = new Test( 5 );
  
  hash.Define( 'a', v1 );
  hash.Define( 'b', v2 );
  hash.Define( 'c', v3 );
  hash.Define( 'd', v4 );
  hash.Define( 'e', v5 );


  printf("a: %d\n", (hash('a'))->Value() );
  printf("a: %d\n", (hash('b'))->Value() );
  printf("a: %d\n", (hash('c'))->Value() );
  printf("a: %d\n", (hash('d'))->Value() );
  printf("a: %d\n", (hash('e'))->Value() );


  printf("\nCalling Remove() on d, a\n");
  hash.Remove('d');
  hash.Remove('a');

  printf("a: %d\n", (hash('a'))->Value() );
  printf("a: %d\n", (hash('b'))->Value() );
  printf("a: %d\n", (hash('c'))->Value() );
  printf("a: %d\n", (hash('d'))->Value() );
  printf("a: %d\n", (hash('e'))->Value() );

  printf("\nTesting IsDefined()  // 0 means undefined\n");
  
  printf("a: %d\n", hash.IsDefined('a') );
  printf("b: %d\n", hash.IsDefined('b') );
  printf("c: %d\n", hash.IsDefined('c') );
  printf("d: %d\n", hash.IsDefined('d') );
  printf("e: %d\n", hash.IsDefined('e') );

  printf("\nAttempt to re-Define() all keys\n");
  // Purposely did not delete the old pointers; the hashtable should clean up
  v1 = new Test( 11 );
  v2 = new Test( 12 );
  v3 = new Test( 13 );
  v4 = new Test( 14 );
  v5 = new Test( 15 );
  hash.Define( 'a', v1 );
  hash.Define( 'b', v2 );
  hash.Define( 'c', v3 );
  hash.Define( 'd', v4 );
  hash.Define( 'e', v5 );

  
  printf("Result:\n");
  printf("a: %d\n", (hash('a'))->Value() );
  printf("a: %d\n", (hash('b'))->Value() );
  printf("a: %d\n", (hash('c'))->Value() );
  printf("a: %d\n", (hash('d'))->Value() );
  printf("a: %d\n", (hash('e'))->Value() );

  printf("Cleaning up\n");

  return 0;
}


