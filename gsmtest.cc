

#include <stdio.h>
#include "gsm.h"



int main( void )
{
  GSM *machine = new GSM( 256 );

  machine->Push( 5 );
  machine->Push( 6 );
  printf( "\nDepth: %d, MaxDepth: %d\n", 
	 machine->Depth(), machine->MaxDepth() );
  machine->Dump();

  printf( "Testing Add()\n");
  machine->Push( 5 );
  machine->Push( 6 );
  machine->Add();
  machine->Dump();

  printf( "Testing Subtract()\n");
  machine->Push( 5 );
  machine->Push( 6 );
  machine->Subtract();
  machine->Dump();

  printf( "Testing Multiply()\n");
  machine->Push( 5 );
  machine->Push( 6 );
  machine->Multiply();
  machine->Dump();

  printf( "Testing Divide()\n");
  machine->Push( 5 );
  machine->Push( 6 );
  machine->Divide();
  machine->Dump();

  printf( "Testing Negate()\n");
  machine->Push( 5 );
  machine->Push( 6 );
  machine->Negate();
  machine->Dump();

  delete machine;

  return 0;
}

