

#include <stdio.h>

#include "gsm.h"
#include "rational.h"
#include "gstring.h"



#define CRASHTEST


int main( void )
{
  GSM_ReturnCode result;
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
  GSM *machine;


  machine = new GSM( 32 );

  gList< Instruction* > program;


  gout << "Machine setup done\n";
  gout << "*********************** press return to continue ************";
  gin >> cont;

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


  gout << "*********************** press return to continue ************";
  gin >> cont;


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

  machine->PushRef( "z" );
  machine->Push( "hi!!" );
  machine->Assign();
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
  gout << "Testing Add()\n";
  machine->Add();
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
  gout << "Assigning x = (double) 7\n";
  machine->PushRef( x );
  machine->Push( (double) 7 );
  machine->Assign();
  gout << "Assigning y = (double) 11\n";
  machine->PushRef( y );
  machine->Push( (double) 11 );
  machine->Assign();
  gout << "Assigning z = (double) 13\n";
  machine->PushRef( z );
  machine->Push( (double) 13 );
  machine->Assign();

  gout << "Testing CallFunction( \"Sqr\" ) on x\n";
  machine->Flush();
  machine->InitCallFunction( (gString) "Sqr" );
  machine->PushRef( x );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "Testing CallFunction( \"Angle\" )\n";
  machine->Flush();
  machine->InitCallFunction( (gString) "Angle" );
  machine->PushRef( x );
  machine->Bind();
  machine->PushRef( y );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "Testing nested CallFunction(); should give same result as before\n";
  machine->Flush();
  machine->InitCallFunction( (gString) "Angle" );
  machine->InitCallFunction( (gString) "Sqr" );
  machine->PushRef( x );
  machine->PushRef( x );
  machine->Multiply();
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->PushRef( y );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;
  
  gout << "\nCrash testing CallFunction()\n\n";

  machine->InitCallFunction( (gString) "Angle" );
  machine->CallFunction();
  machine->Dump();
  machine->InitCallFunction( (gString) "Angle" );
  machine->CallFunction();
  machine->Dump();
  machine->InitCallFunction( (gString) "Angle" );
  machine->CallFunction();
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


  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "\n\nTesting the instruction memory\n";
  program.Append( new Push<double>( (double) 1 ) );
  program.Append( new Push<double>( (double) 2 ) );
  program.Append( new Push<double>( (double) 3 ) );
  program.Append( new Dump );
  result = machine->Execute( program );
  gout << "Program Status: " << result << "\n";

  program.Append( new Push<double>( (double) 1 ) );
  program.Append( new Push<double>( (double) 2 ) );
  program.Append( new Push<double>( (double) 3 ) );
  program.Append( new Add );
  program.Append( new Dump );
  result = machine->Execute( program );
  gout << "Program Status: " << result << "\n";


  program.Append( new InitCallFunction( (gString) "Sqr" ) );
  program.Append( new Push<double>( (double) 4 ) );
  program.Append( new Bind );
  program.Append( new CallFunction );
  program.Append( new Dump );
  result = machine->Execute( program );
  gout << "Program Status: " << result << "\n";


  program.Append( new Push<double>( (double) 1 ) );
  program.Append( new Push<double>( (double) 2 ) );
  program.Append( new InitCallFunction( (gString) "Angle" ) );
  program.Append( new Bind );
  program.Append( new Bind );
  program.Append( new Push<double>( (double) 3 ) );
  program.Append( new Neg );
  program.Append( new CallFunction );
  program.Append( new Dump );
  result = machine->Execute( program );
  gout << "Program Status: " << result << "\n";


  program.Append( new PushRef( x ) );
  program.Append( new PushRef( y ) );
  program.Append( new InitCallFunction( (gString) "Angle" ) );
  program.Append( new Bind );
  program.Append( new Bind );
  program.Append( new Push<double>( (double) 3 ) );
  program.Append( new CallFunction );
  program.Append( new Dump );
  result = machine->Execute( program );
  gout << "Program Status: " << result << "\n";


  program.Append( new Push<gString>( "hi!" ) );
  program.Append( new Push<gString>( "how are you?" ) );
  program.Append( new Push<gString>( "I'm fine!  you?!" ) );
  program.Append( new Push<gString>( "I'm just great!" ) );
  program.Append( new Push<gString>( "Cool!" ) );
  program.Append( new PushList( 4 ) );
  program.Append( new Dump );
  result = machine->Execute( program );
  gout << "Program Status: " << result << "\n";

  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "\n\nTesting the instruction memory\n";
  gout << "Huge general test\n";

  program.Append( new PushRef( "x" ) );
  program.Append( new Push<gRational>( 17 ) );
  program.Append( new Assign );
  program.Append( new PushRef( "y" ) );
  program.Append( new Push<gRational>( 23 ) );
  program.Append( new Assign );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Dump );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Equ );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Neq );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Gtn );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Ltn );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Geq );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Leq );
  program.Append( new Dump );

  program.Append( new PushRef( "x" ) );
  program.Append( new Push<gString>( "hi!" ) );
  program.Append( new Assign );
  program.Append( new PushRef( "y" ) );
  program.Append( new Push<gString>( "hello!" ) );
  program.Append( new Assign );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Dump );
  program.Append( new Push<gString>( "hi!" ) );
  program.Append( new Push<gString>( "hello!" ) );
  program.Append( new Add );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Equ );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Neq );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Gtn );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Ltn );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Geq );
  program.Append( new PushRef( "x" ) );
  program.Append( new PushRef( "y" ) );
  program.Append( new Leq );
  program.Append( new Dump );

  program.Append( new Push<bool>( false ) );
  program.Append( new Push<bool>( true ) );
  program.Append( new AND );
  program.Append( new Push<bool>( false ) );
  program.Append( new Push<bool>( true ) );
  program.Append( new OR );
  program.Append( new Push<bool>( false ) );
  program.Append( new Push<bool>( true ) );
  program.Append( new NOT );
  program.Append( new Dump );

  result = machine->Execute( program );
  gout << "Program Status: " << result << "\n";

  gout << "*********************** press return to continue ************";
  gin >> cont;



  gout << "Negate\n";
  machine->Push( (double) 10 );
  machine->InitCallFunction( "Negate" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  gout << "EqualTo\n";
  machine->Push( "hi!" );
  machine->Push( "hello!" );
  machine->InitCallFunction( "Equal" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->Push( "hi!" );
  machine->Push( "hello!" );
  machine->InitCallFunction( "NotEqual" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->Push( "hi!" );
  machine->Push( "hello!" );
  machine->InitCallFunction( "Greater" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->Push( "hi!" );
  machine->Push( "hello!" );
  machine->InitCallFunction( "Less" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->Push( "hi!" );
  machine->Push( "hello!" );
  machine->InitCallFunction( "GreaterEqual" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->Push( "hi!" );
  machine->Push( "hello!" );
  machine->InitCallFunction( "LessEqual" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  gout << "AND\n";
  machine->Push( true );
  machine->Push( false );
  machine->InitCallFunction( "And" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->Push( true );
  machine->Push( false );
  machine->InitCallFunction( "Or" );
  machine->Bind();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->Push( true );
  machine->Push( false );
  machine->InitCallFunction( "Not" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;



  machine->Push( (gInteger) 6 );
  machine->Push( (gInteger) 5 );
  machine->Divide();
  machine->Dump();

  machine->InitCallFunction( "Divide" );
  machine->Push( (gInteger) 6 );
  machine->Bind();
  machine->Push( (gInteger) 5 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  machine->Push( (gInteger) 6 );
  machine->Push( (gInteger) 5 );
  machine->Divide();
  machine->Dump();

  machine->InitCallFunction( "Divide" );
  machine->Push( (gInteger) 6 );
  machine->Bind();
  machine->Push( (gInteger) 5 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "Testing Modulus\n";
  machine->Push( (gInteger) 6 );
  machine->Push( (gInteger) 5 );
  machine->Modulus();
  machine->Dump();

  gout << "Testing Modulus\n";
  machine->Push( (gInteger) 5 );
  machine->Push( (gInteger) 5 );
  machine->Modulus();
  machine->Dump();


  gout << "Testing Instructions:\n";

  program.Append( new Push<gInteger>( 6 ) );
  program.Append( new Push<gInteger>( 5 ) );
  program.Append( new Mod );
  program.Append( new Dump );

  program.Append( new Push<gInteger>( 5 ) );
  program.Append( new Push<gInteger>( 5 ) );
  program.Append( new Mod );
  program.Append( new Dump );

  machine->Execute( program );


  gout << "*********************** press return to continue ************";
  gin >> cont;


  gout << "Testing Loops:\n";
  program.Append( new Push<double>( 101 ) );
  program.Append( new Push<double>( 100 ) );
  program.Append( new Gtn );
  program.Append( new IfGoto( 8 ) );
  program.Append( new Push<double>( 3 ) );
  program.Append( new Dump );
  program.Append( new Goto( 10 ) );
  program.Append( new Push<double>( 5 ) );
  program.Append( new Dump );
  program.Append( new NOP );

  program.Append( new Push<bool>( true ) );
  program.Append( new Push<bool>( true ) );
  program.Append( new Equ );
  program.Append( new Dump );

  program.Append( new Push<bool>( true ) );
  program.Append( new Push<bool>( true ) );
  program.Append( new Neq );
  program.Append( new Dump );

  program.Append( new Push<bool>( true ) );
  program.Append( new Push<bool>( false ) );
  program.Append( new Equ );
  program.Append( new Dump );

  program.Append( new Push<bool>( true ) );
  program.Append( new Push<bool>( false ) );
  program.Append( new Neq );
  program.Append( new Dump );


  program.Append( new InitCallFunction( "Modulus" ) );
  program.Append( new Push<gInteger>( 6 ) );
  program.Append( new Bind );
  program.Append( new Push<gInteger>( 5 ) );
  program.Append( new Bind );
  program.Append( new CallFunction );
  program.Append( new Dump );


  program.Append( new InitCallFunction( "Modulus" ) );
  program.Append( new Push<gInteger>( 6 ) );
  program.Append( new Bind );
  program.Append( new Push<gInteger>( 6 ) );
  program.Append( new Bind );
  program.Append( new CallFunction );
  program.Append( new Dump );


  program.Dump( gout );

  machine->Execute( program );

  gout << "*********************** press return to continue ************";
  gin >> cont;

  
  machine->InitCallFunction( "Plus" );
  machine->Push( (double) 12 );
  machine->Bind( "y" );
  machine->Push( (double) 13 );
  machine->Bind( "x" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Plus" );
  machine->Push( (double) 12 );
  machine->Bind( "x" );
  machine->Push( (double) 13 );
  machine->Bind( "y" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Plus" );
  machine->Push( (double) 12 );
  machine->Bind( "x" );
  machine->Push( (double) 13 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Plus" );
  machine->Push( (double) 12 );
  machine->Bind();
  machine->Push( (double) 13 );
  machine->Bind( "y" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Plus" );
  machine->Push( (double) 12 );
  machine->Bind();
  machine->Push( (double) 13 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;






  machine->PushRef( "x" );
  machine->Push( (gInteger) 10 );
  for( i = 1; i <= 9; i++ )
  {
    machine->Push( (gInteger) i );
  }
  machine->PushList( 10 );
  machine->Assign();
  machine->Dump();


  machine->PushRef( "x" );
  machine->Push( (gInteger) 3 );
  machine->Subscript();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Push( (gInteger) 0 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (gInteger) 9 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (gInteger) 10 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (double) 6 );
  machine->Subscript();  
  machine->Dump();





#ifdef CRASHTEST

  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->Push( (double) 10 );
  machine->Push( (double) 11 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Push( (double) 10 );
  machine->Push( (double) 11 );
  machine->Assign();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->PushRef( "x" );
  machine->Push( (double) 10 );
  machine->Add();
  machine->Push( (double) 11 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->PushRef( "y" );
  machine->Add();
  machine->Push( (double) 10 );
  machine->Add();
  machine->Push( (double) 11 );
  machine->Assign();
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;


  machine->PushRef( "x" );
  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->PushRef( "x4" );
  machine->PushRef( "x5" );
  machine->PushList( 5 );
  machine->Assign();
  machine->Dump();

  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->PushRef( "x" );
  machine->Push( (gInteger) 3 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (gInteger) 5 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (gInteger) -2 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (gInteger) 7 );
  machine->Subscript();
  machine->Dump();

#endif




#ifdef CRASHTEST
  gout << "*********************** press return to continue ************";
  gin >> cont;

  machine->PushRef( "x" );
  machine->Push( (double) 10 );
  for( i = 1; i <= 9; i++ )
  {
    machine->Push( (gInteger) i );
  }
  machine->PushList( 9 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Push( (gInteger) 3 );
  machine->Subscript();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Push( (gInteger) 0 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (gInteger) 9 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (gInteger) 10 );
  machine->Subscript();
  machine->PushRef( "x" );
  machine->Push( (double) 6 );
  machine->Subscript();  
  machine->Dump();
#endif


  gout << "*********************** press return to continue ************";
  gin >> cont;



  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Bind();
  machine->Push( (double) 10 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "y" );
  machine->Bind();
  machine->PushRef( "x" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();


  
  program.Append( new InitCallFunction( "Assign" ) );
  program.Append( new PushRef( "z" ) );
  program.Append( new Bind );
  program.Append( new PushRef( "y" ) );
  program.Append( new Bind );
  program.Append( new CallFunction );
  program.Append( new Dump );
  machine->Execute( program );


  machine->Push( (gInteger) 0 );
  machine->Push( (gInteger) 0 );
  machine->Divide();
  machine->Push( (gRational) 0 );
  machine->Push( (gRational) 0 );
  machine->Divide();
  machine->Push( (double) 0 );
  machine->Push( (double) 0 );
  machine->Divide();
  machine->Push( (gInteger) 0 );
  machine->Push( (gInteger) 0 );
  machine->Modulus();
  machine->Dump();




#ifdef CRASHTEST

  machine->InitCallFunction( "Assign" );
  machine->Push( (double) 10 );
  machine->BindVal();
  machine->Push( (double) 15 );
  machine->BindVal();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->Push( (double) 10 );
  machine->BindRef();
  machine->Push( (double) 15 );
  machine->BindVal();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->Push( (double) 10 );
  machine->BindRef();
  machine->Push( (double) 15 );
  machine->BindRef();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->Push( (double) 10 );
  machine->BindVal();
  machine->Push( (double) 15 );
  machine->BindRef();
  machine->CallFunction();
  machine->Dump();

#endif




  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 3 );
  machine->Divide();
  machine->Dump();

  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 3 );
  machine->IntegerDivide();
  machine->Dump();

  machine->Push( (gInteger) 11 );
  machine->Push( (gInteger) 3 );
  machine->Modulus();
  machine->Dump();


#ifdef CRASHTEST
  machine->InitCallFunction( "Divide" );
  machine->Push( (double) 0 );
  machine->Bind();
  machine->Push( (double) 0 );
  machine->Bind();
  machine->CallFunction();

  machine->InitCallFunction( "Divide" );
  machine->Push( (gInteger) 0 );
  machine->Bind();
  machine->Push( (gInteger) 0 );
  machine->Bind();
  machine->CallFunction();

  machine->InitCallFunction( "Divide" );
  machine->Push( (gRational) 0 );
  machine->Bind();
  machine->Push( (gRational) 0 );
  machine->Bind();
  machine->CallFunction();
#endif

  machine->InitCallFunction( "Divide" );
  machine->Push( (gInteger) 5 );
  machine->Bind();
  machine->Push( (gInteger) 3 );
  machine->Bind();
  machine->CallFunction();

  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;





  machine->PushRef( "x" );
#if 0
  machine->InitCallFunction( "ReadNfgFile" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
#endif
  machine->Push( (double) 101 );
  machine->Assign();
  machine->Dump();

  gout << "asssigned sub\n";
  machine->PushRef( "x", "a" );
  machine->Push( (double) 1 );
  machine->Assign();
  machine->Dump();


  machine->PushRef( "x" );
  machine->PushRef( "x", "a" );
  machine->Dump();



  machine->PushRef( "x" );
  machine->Push( (double) 10 );
  machine->Assign();
  machine->Dump();


  machine->PushRef( "x", "a" );
  machine->Push( (double) 1 );
  machine->Assign();
  machine->Dump();



  gout << "*********************** press return to continue ************";
  gin >> cont;




  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Bind();
#if 0
  machine->InitCallFunction( "ReadNfgFile" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
#endif
  machine->Push( (double) 0 );

  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "assigned primary ref\n";


  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x", "a" );
  machine->Bind();
  machine->Push( (double) 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  gout << "Assigned subrefs; now display:\n";

  machine->PushRef( "x", "a" );
  machine->PushRef( "x" );
  machine->Dump();




#ifdef CRASHTEST
  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Bind();
  machine->Push( (gRational) 11 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "assigned primary ref\n";


  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x", "a" );
  machine->Bind();
  machine->Push( (double) 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  gout << "Assigned subrefs; now display:\n";

  machine->PushRef( "x", "a" );
  machine->PushRef( "x" );
  machine->Dump();
#endif



  gout << "*********************** press return to continue ************";
  gin >> cont;



  machine->PushRef( "x" );
  machine->Push( (double) 1 );
  machine->Assign();
  machine->PushRef( "y" );
  machine->Push( (double) 2 );
  machine->Assign();


  machine->InitCallFunction( "Test" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Test" );
  machine->CallFunction();
  machine->Dump();



  machine->InitCallFunction( "Test" );
  machine->PushRef( "x" );
  machine->BindVal();
  machine->PushRef( "y" );
  machine->BindVal();
  machine->CallFunction();
  machine->Dump();


  machine->PushRef( "x" );
  machine->PushRef( "y" );
  machine->Dump();


  machine->InitCallFunction( "Test" );
  machine->PushRef( "x" );
  machine->Bind();
  machine->PushRef( "y" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  machine->PushRef( "x" );
  machine->PushRef( "y" );
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;




#ifdef CRASHTEST
  machine->PushRef( "x" );
  machine->Push( (gRational) 1 );
  machine->Assign();
  machine->PushRef( "y" );
  machine->Push( (gRational) 2 );
  machine->Assign();


  machine->InitCallFunction( "Test" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Test" );
  machine->CallFunction();
  machine->Dump();



  machine->InitCallFunction( "Test" );
  machine->PushRef( "x" );
  machine->BindVal();
  machine->PushRef( "y" );
  machine->BindVal();
  machine->CallFunction();
  machine->Dump();


  machine->PushRef( "x" );
  machine->PushRef( "y" );
  machine->Dump();


  machine->InitCallFunction( "Test" );
  machine->PushRef( "x" );
  machine->Bind();
  machine->PushRef( "y" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  machine->PushRef( "x" );
  machine->PushRef( "y" );
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "x" );
  machine->Push( (double) 1 );
  machine->Push( (double) 2 );
  machine->Push( (double) 3 );
  machine->PushList( 3 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();
  
  machine->PushRef( "x" );
  machine->Push( (gInteger) 2 );
  machine->Subscript();
  machine->Dump();


  machine->PushRef( "x" );
  machine->Push( (gInteger) 2 );
  machine->Subscript();
  machine->Push( (double) 5 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();



  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Bind();
  machine->Push( (double) 1 );
  machine->Push( (double) 2 );
  machine->Push( (double) 3 );
  machine->PushList( 3 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();
  

  machine->PushRef( "x" );
  machine->Push( (gInteger) 2 );
  machine->Subscript();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Push( (gInteger) 2 );
  machine->Subscript();
  machine->Bind();
  machine->Push( (double) 5 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();



  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "\nDeleting machine\n";
  delete machine;

  return 0;
}
