

#include <stdio.h>

#include "gsm.h"
#include "rational.h"
#include "gstring.h"


int main( void )
{
  char cont;
  int i;
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
  gString *name;


  GSM *machine = new GSM( 32 );
  

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

/*
  gout << "Attempt to add different types()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Push( i_1 );
  machine->Add();
  machine->Dump();
*/

  gout << "*********************** press return to continue ************";
  gin >> cont;

/*
  gout << "Testing PushRef()\n";
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->PushRef( x );
  machine->PushRef( y );
  machine->PushRef( z );
  machine->Dump();
*/  


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
  gout << "EqualTo()              : ";
  machine->EqualTo();
  machine->Output();
  machine->Flush();
  machine->Push( d_1 );
  machine->Push( d_2 );
  gout << "NotEqualTo()           : ";
  machine->NotEqualTo();
  machine->Output();
  machine->Flush();
  machine->Push( d_1 );
  machine->Push( d_2 );
  gout << "GreaterThan()          : ";
  machine->GreaterThan();
  machine->Output();
  machine->Flush();
  machine->Push( d_1 );
  machine->Push( d_2 );
  gout << "LessThan()             : ";
  machine->LessThan();
  machine->Output();
  machine->Flush();
  machine->Push( d_1 );
  machine->Push( d_2 );
  gout << "GreaterThanOrEqualTo() : ";
  machine->GreaterThanOrEqualTo();
  machine->Output();
  machine->Flush();
  machine->Push( d_1 );
  machine->Push( d_2 );
  gout << "LessThanOrEqualTo()    : ";
  machine->LessThanOrEqualTo();
  machine->Output();
  machine->Flush();

  
  gout << "\n";
  machine->Push( r_1 );
  machine->Push( r_1 );
  machine->Dump();

  machine->Push( r_1 );
  machine->Push( r_1 );
  gout << "EqualTo()              : ";
  machine->EqualTo();
  machine->Output();
  machine->Flush();
  machine->Push( r_1 );
  machine->Push( r_1 );
  gout << "NotEqualTo()           : ";
  machine->NotEqualTo();
  machine->Output();
  machine->Flush();
  machine->Push( r_1 );
  machine->Push( r_1 );
  gout << "GreaterThan()          : ";
  machine->GreaterThan();
  machine->Output();
  machine->Flush();
  machine->Push( r_1 );
  machine->Push( r_1 );
  gout << "LessThan()             : ";
  machine->LessThan();
  machine->Output();
  machine->Flush();
  machine->Push( r_1 );
  machine->Push( r_1 );
  gout << "GreaterThanOrEqualTo() : ";
  machine->GreaterThanOrEqualTo();
  machine->Output();
  machine->Flush();
  machine->Push( r_1 );
  machine->Push( r_1 );
  gout << "LessThanOrEqualTo()    : ";
  machine->LessThanOrEqualTo();
  machine->Output();
  machine->Flush();


  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->Flush();
  machine->PushRef( (gString) "c" );
  machine->Push( (bool) true );
  machine->Assign();
  machine->Flush();

  machine->PushRef( (gString) "c" );
  machine->PushRef( (gString) "c" );
  machine->AND();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->Flush();
/*
  gout << "Testing CallFunction(\"Sign\")\n";
  machine->CallFunction( (gString) "Sign" );
  gout << "Testing CallFunction(\"Abs\")\n";
  machine->CallFunction( (gString) "Abs" );
  gout << "Testing CallFunction(\"Sqr\")\n";
  machine->CallFunction( (gString) "Sqr" );
  machine->Dump();
*/

  gout << "Pushing (double) 8\n";
  machine->Push( (double) 8 );
  gout << "Testing CallFunction(\"Sqr\")\n";
  machine->CallFunction( (gString) "Sqr" );
  machine->Dump();

  machine->Flush();
  gout << "Testing CallFunction(\"Sqr\") 3 times on (double) 300\n";
  machine->Push( (double) 300 );
  machine->CallFunction( (gString) "Sqr" );
  machine->CallFunction( (gString) "Sqr" );
  machine->CallFunction( (gString) "Sqr" );
  machine->Dump();

  gout << "Push( x = (double) 7 )\n";
  machine->PushRef( x );
  machine->Push( (double) 7 );
  machine->Assign();
  gout << "Push( y = (double) 11 )\n";
  machine->PushRef( y );
  machine->Push( (double) 11 );
  machine->Assign();
  gout << "Push( z = (double) 13 )\n";
  machine->PushRef( z );
  machine->Push( (double) 13 );
  machine->Assign();

  gout << "Testing CallFunction( \"Angle\" )\n";
  machine->CallFunction( (gString) "Angle" );
  machine->Dump();

  gout << "Testing CallFunction3()\n";
  machine->Flush();
  machine->PushRef( x );
  machine->PushRef( y );
  name = new gString[2];
  name[0] = "x";
  name[1] = "y";
  machine->CallFunction3( (gString) "Angle", name, 2 );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( y );
  machine->PushRef( x );
  name = new gString[2];
  name[0] = "y";
  name[1] = "x";
  machine->CallFunction3( (gString) "Angle", name, 2 );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( x );
  name = new gString[1];
  name[0] = "x";
  machine->CallFunction3( (gString) "Angle", name, 1 );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( y );
  name = new gString[1];
  name[0] = "y";
  machine->CallFunction3( (gString) "Angle", name, 1 );
  delete[] name;
  machine->Dump();

  machine->Flush();
  name = new gString[0];
  machine->CallFunction3( (gString) "Angle", name, 0 );
  delete[] name;
  machine->Dump();

  gout << "Testing CallFunction4()\n";
  machine->Flush();
  name = new gString[0];
  machine->CallFunction4( (gString) "Angle", 0,  name );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( x );
  name = new gString[1];
  name[0] = "";
  machine->CallFunction4( (gString) "Angle", 1,  name );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( x );
  name = new gString[1];
  name[0] = "x";
  machine->CallFunction4( (gString) "Angle", 1,  name );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( y );
  name = new gString[1];
  name[0] = "y";
  machine->CallFunction4( (gString) "Angle", 1,  name );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( x );
  machine->PushRef( y );
  name = new gString[2];
  name[0] = "";
  name[1] = "";
  machine->CallFunction4( (gString) "Angle", 2,  name );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( x );
  machine->PushRef( y );
  name = new gString[2];
  name[0] = "x";
  name[1] = "";
  machine->CallFunction4( (gString) "Angle", 2,  name );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( x );
  machine->PushRef( y );
  name = new gString[2];
  name[0] = "";
  name[1] = "y";
  machine->CallFunction4( (gString) "Angle", 2,  name );
  delete[] name;
  machine->Dump();

  machine->Flush();
  machine->PushRef( x );
  machine->PushRef( y );
  name = new gString[2];
  name[0] = "x";
  name[1] = "y";
  machine->CallFunction4( (gString) "Angle", 2,  name );
  delete[] name;
  machine->Dump();



  gout << "*********************** press return to continue ************";
  gin >> cont;


  machine->Flush();
  machine->PushRef( x );
  machine->PushRef( x );
  name = new gString[2];
  name[0] = "";
  name[1] = "x";
  machine->CallFunction4( (gString) "Angle", 2,  name );
  delete[] name;
  machine->Dump();



  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->Push( (double) 7 );
  machine->Push( (double) 8 );
  machine->Push( (double) 9 );
  machine->Push( (double) 10 );
  machine->Push( (double) 11 );
  machine->PushList( 4 );
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;
  
  gout << "Testing nested List structures\n";
  machine->Push( (double) 1 );
  machine->Push( (double) 2 );
  machine->Push( (double) 3 );
  machine->Push( (double) 4 );
  machine->Push( (double) 5 );
  machine->PushList( 3 );
  machine->Push( (double) 6 );
  machine->Push( (double) 7 );
  machine->PushList( 4 );
  machine->Push( (double) 8 );
  machine->PushList( 3 );
  machine->Push( (double) 9 );
  machine->Push( (double) 10 );
  machine->Push( (double) 11 );
  machine->PushList( 4 );
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->Flush();
  machine->Push( (gInteger) 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->Push( (gInteger) 2 );
  machine->PushList( 2 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "General testing:\n";
  machine->Flush();
  machine->PushRef( (gString) "x1" );
  machine->Push( (double) 7 );
  machine->Push( (double) 8 );
  machine->EqualTo();
  machine->Assign();
  machine->Dump();
  machine->Flush();
  machine->PushRef( (gString) "y1" );
  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 13 );
  machine->Multiply();
  machine->Push( (gInteger) 130 );
  machine->GreaterThan();
  machine->Assign();
  machine->Dump();
  machine->Flush();
  machine->PushRef( (gString) "x1" );
  machine->PushRef( (gString) "y1" );
  machine->Dump();
  machine->Flush();
  machine->PushRef( (gString) "x1" );
  machine->PushRef( (gString) "y1" );
  machine->AND();
  machine->Dump();
  machine->Flush();
  machine->PushRef( (gString) "x1" );
  machine->PushRef( (gString) "y1" );
  machine->OR();
  machine->Dump();
/*
  machine->Flush();
  machine->PushRef( (gString) "x1" );
  machine->PushRef( (gString) "y1" );
  machine->Push( (bool) true );
  machine->PushList( 3 );
  machine->Dump();
*/
  machine->Flush();
  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 13 );
  machine->GreaterThan();
  machine->Push( (gInteger) 13 );
  machine->Push( (gInteger) 11 );
  machine->GreaterThan();
  machine->Push( (gInteger) 12 );
  machine->Push( (gInteger) 12 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (gInteger) 24 );
  machine->Push( (gInteger) 25 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (double) 11 );
  machine->Push( (double) 11 );
  machine->GreaterThan();
  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 11 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 110 );
  machine->GreaterThan();
  machine->Push( (double) 110 );
  machine->Push( (double) 11 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 11 );
  machine->GreaterThan();
  machine->Push( (gInteger) -10 );
  machine->Push( (gInteger) -11 );
  machine->GreaterThan();
  machine->PushList( 10 );
  machine->Dump();


/*
  gout << "*********************** press return to continue ************";
  gin >> cont;


  gout << "Testing (double) 1 * 10000 using Add()\n";
  machine->Flush();
  machine->Push( (double) 1 );
  for( i = 2; i <= 10000; i++ )
  {
    machine->Push( (double) 1 );
    machine->Add();
  }
  machine->Dump();

  
  gout << "Testing CallFunction( \"Sqr\" ) on (double) 99 ^ 99 for 10 times\n";
  machine->Flush();
  machine->Push( (double) 99 );
  for( i = 2; i <= 99; i++ )
  {
    machine->Push( (double) 99 );
    machine->Multiply();
  }
  for( i = 1; i <= 10; i++ )
  {
    machine->CallFunction( (gString) "Sqr" );
  }
  machine->Dump();

  
  machine->Flush();
  gout << "Testing (gRational) ( 11 / 17 ) ^ 32\n";
  gRational t = ( (gRational) 11 )/( (gRational) 17 );
  for( i = 1; i <= 32; i++ )
  {
    machine->Push( t );  
  }
  for( i = 1; i <= 31; i++ )
  {
    machine->Multiply();  
  }
  machine->Dump();


  gout << "Testing (gRational) 99 ^ 99\n";
  machine->Flush();
  machine->Push( (gRational) 99 );
  for( i = 2; i <= 99; i++ )
  {
    machine->Push( (gRational) 99 );
    machine->Multiply();
  }
  machine->Dump();

  gout << "Testing (gRational) 1 ^ 10000\n";
  machine->Flush();
  machine->Push( (gRational) 1 );
  for( i = 2; i <= 10000; i++ )
  {
    machine->Push( (gRational) 1 );
    machine->Multiply();
  }
  machine->Dump();

*/


  delete machine;

  return 0;
}

