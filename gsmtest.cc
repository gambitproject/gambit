

#include <stdio.h>
#include "gsm.h"
#include "rational.h"
#include "gstring.h"


int main( void )
{
  char cont;
  double d_1 = (double) 7.5;
  double d_2 = (double) 11.5;
  double d_temp;
  gInteger i_1 = (gInteger) 13;
  gInteger i_2 = (gInteger) 17;
  gInteger i_temp;
  gRational r_1 = (gRational) 19/3;
  gRational r_2 = (gRational) 23/7;
  gRational r_temp;
  gString x = "x";
  gString y = "y";
  gString z = "z";


  GSM *machine = new GSM( 256 );
  

  gout << "\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  gout << "Depth: " << machine->Depth() << "\n";
  gout << "MaxDepth: " << machine->MaxDepth() << "\n";
  machine->Dump();

  gout << "Testing Add()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Add();
  machine->Dump();

  gout << "Testing Subtract()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Subtract();
  machine->Dump();

  gout << "Testing Multiply()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Multiply();
  machine->Dump();

  gout << "Testing Divide()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Divide();
  machine->Dump();

  gout << "Testing Negate()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Negate();
  machine->Dump();

  gout << "Attempt to add different types()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Push( i_1 );
  machine->Add();
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;


  gout << "Testing PushRef()\n";   // Push() and PushRef() are interchangeable
  machine->Push( x );              // for gStrings
  machine->Push( y );
  machine->Push( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->Dump();
  
  gout << "Assigning x = (double)7\n";
  machine->Assign( x, (double)7 );
  gout << "Assigning y = (double)11\n";
  machine->Assign( y, (double)11 );
  gout << "Assigning z = (double)13\n";
  machine->Assign( z, (double)13 );
  gout << "\nTesting x + y / ( z - x * ( y - ( -z ) ) )\n";
  machine->Push( x );
  machine->Push( y );
  machine->Push( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->Negate();
  machine->Subtract();
  machine->Multiply();
  machine->Subtract();
  machine->Divide();
  machine->Add();
  machine->Dump();

  gout << "Assigning x = (gInteger)5\n";
  machine->Assign( x, (gInteger)5 );
  gout << "Assigning y = (gInteger)7\n";
  machine->Assign( y, (gInteger)7 );
  gout << "Assigning z = (gInteger)11\n";
  machine->Assign( z, (gInteger)11 );
  gout << "\nTesting x + y / ( z - x * ( y - ( -z ) ) )\n";
  machine->Push( x );
  machine->Push( y );
  machine->Push( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->Negate();
  machine->Subtract();
  machine->Multiply();
  machine->Subtract();
  machine->Divide();
  machine->Add();
  machine->Dump();

  gout << "Assigning y = (gRational)7\n";
  machine->Assign( y, (gRational)7 );
  gout << "\nTesting x + y / ( z - x * ( y - ( -z ) ) )\n";
  machine->Push( x );
  machine->Push( y );
  machine->Push( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->Negate();
  machine->Subtract();
  machine->Multiply();
  machine->Subtract();
  machine->Divide();
  machine->Add();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;


  gout << "Testing PushVal()\n";
  machine->UnAssign( x );
  machine->UnAssign( y );
  machine->UnAssign( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  gout << "PushVal (gRational)11 into z\n";
  machine->PushVal( (gRational)11 );
  gout << "PushVal (gRational)7 into y\n";
  machine->PushVal( (gRational)7 );
  gout << "PushVal (gRational)5 into x\n";
  machine->PushVal( (gRational)5 );
  gout << "\nTesting x + y / ( z - x * ( y - ( -z ) ) )\n";
  machine->Negate();
  machine->Subtract();
  machine->Multiply();
  machine->Subtract();
  machine->Divide();
  machine->Add();
  machine->Dump();

  gout << "Attempt to do additional (illegal) PushVals\n";
  machine->PushVal( (gRational)13 );



  delete machine;

  return 0;
}

