

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


  gout << "Testing PushRef()\n";
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->Dump();
  

  gout << "Assigning x = (double)7\n";
  machine->PushRef( x );
  machine->Push( (double)7 );
  machine->Assign();
  gout << "Assigning y = (double)11\n";
  machine->PushRef( y );
  machine->Push( (double)11 );
  machine->Assign();
  gout << "Assigning z = (double)13\n";
  machine->PushRef( z );
  machine->Push( (double)13 );
  machine->Assign();
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->Dump();

  gout << "Assigning x = (double)17\n";
  machine->PushRef( x );
  machine->Push( (double)17 );
  machine->Assign();
  gout << "Assigning y = (double)21\n";
  machine->PushRef( y );
  machine->Push( (double)21 );
  machine->Assign();
  gout << "Assigning z = (double)23\n";
  machine->PushRef( z );
  machine->Push( (double)23 );
  machine->Assign();
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  gout << "\nTesting x + y / ( z - x * ( y - ( -z ) ) )\n";
  machine->Negate();
  machine->Subtract();
  machine->Multiply();
  machine->Subtract();
  machine->Divide();
  machine->Add();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "Testing boolean operations:\n";
  gout << "Pushing false\n";
  gout << "Pushing true\n";

  machine->Push( false );
  machine->Push( true );
  gout << "Testing AND()\n";
  machine->AND();
  machine->Dump();

  machine->Push( false );
  machine->Push( true );
  gout << "Testing OR()\n";
  machine->OR();
  machine->Dump();

  machine->Push( false );
  machine->Push( true );
  gout << "Testing NOT()\n";
  machine->NOT();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "Testing string concatenation:\n";
  gout << "Pushing \'Test1 \'\n";
  gout << "Pushing \'Test2 \'\n";
  machine->Push( (gString)"Test1 " );
  machine->Push( (gString)"Test2 " );
  gout << "Testing Concatenate()\n";
  machine->Concatenate();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "Testing relational operators:\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Dump();

  machine->Push( d_1 );
  machine->Push( d_2 );

  gout << "EqualTo()              : " << machine->EqualTo() << "\n";
  gout << "NotEqualTo()           : " << machine->NotEqualTo() << "\n";
  gout << "GreaterThan()          : " << machine->GreaterThan() << "\n";
  gout << "LessThan()             : " << machine->LessThan() << "\n";
  gout << "GreaterThanOrEqualTo() : " << machine->GreaterThanOrEqualTo() << "\n";
  gout << "LessThanOrEqualTo()    : " << machine->LessThanOrEqualTo() << "\n";

  
  gout << "\n";
  machine->Push( r_1 );
  machine->Push( r_1 );
  machine->Dump();

  machine->Push( r_1 );
  machine->Push( r_1 );

  gout << "EqualTo()              : " << machine->EqualTo() << "\n";
  gout << "NotEqualTo()           : " << machine->NotEqualTo() << "\n";
  gout << "GreaterThan()          : " << machine->GreaterThan() << "\n";
  gout << "LessThan()             : " << machine->LessThan() << "\n";
  gout << "GreaterThanOrEqualTo() : " << machine->GreaterThanOrEqualTo() << "\n";
  gout << "LessThanOrEqualTo()    : " << machine->LessThanOrEqualTo() << "\n";


  delete machine;

  return 0;
}

