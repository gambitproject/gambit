//#
//# FILE: gsmtest.cc  test driver for GSM
//#
//# $Id$
//#


#include "gsm.h"
#include "integer.h"
#include "rational.h"
#include "gstring.h"
#include "gambitio.h"
#include "glist.h"
#include "gsminstr.h"
#include "gsmfunc.h"



#define CRASHTEST
#define INTERACTIVE


int main( void )
{
  GSM_ReturnCode result;
  char cont;
  int i;
  double d_1 = (double) 7.5;
  double d_2 = (double) 11.5;
  double d_temp;
  long i_1 = (long) 13;
  long i_2 = (long) 17;
  long i_temp;
  gRational r_1 = (gRational) 19/3;
  gRational r_2 = (gRational) 23/7;
  gRational r_temp;
  gString x = "x";
  gString y = "y";
  gString z = "z";
  GSM* machine;
  GSM* gsm;
  gList< Instruction* >* prog;
  FuncDescObj* func;


/*
  gFileOutput _sout( "sout" );
  gOutput& sout = _sout;
  gFileOutput _serr( "serr" );
  gOutput& serr = _serr;
*/

  machine = new GSM( 32, gin, gout, gerr );
//  gsm = new GSM( 10, gin, gout, gerr );

  gList< Instruction* > program;


  gout << "Machine setup done\n";
  // sout << "Machine setup done\n";


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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


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
  machine->PushRef( "z" );
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


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

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  gout << "Testing string concatenation:\n";
  gout << "Pushing \'Test1 \'\n";
  gout << "Pushing \'Test2 \'\n";
  machine->Push( (gString)"Test1 " );
  machine->Push( (gString)"Test2 " );
  gout << "Testing Add()\n";
  machine->Add();
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  machine->Flush();
  machine->PushRef( (gString) "c" );
  machine->Push( (bool) true );
  machine->Assign();
  machine->Flush();

  machine->PushRef( (gString) "c" );
  machine->PushRef( (gString) "c" );
  machine->AND();
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif
  
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

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  machine->Push( (double) 7 );
  machine->Push( (double) 8 );
  machine->Push( (double) 9 );
  machine->Push( (double) 10 );
  machine->Push( (double) 11 );
  machine->PushList( 4 );
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif
  
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

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  machine->Flush();
  machine->Push( (long) 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->Push( (long) 2 );
  machine->PushList( 2 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->PushList( 1 );
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

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
  machine->Push( (long) 11 );
  machine->Push( (long) 13 );
  machine->Multiply();
  machine->Push( (long) 130 );
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
  machine->Push( (long) 11 );
  machine->Push( (long) 13 );
  machine->GreaterThan();
  machine->Push( (long) 13 );
  machine->Push( (long) 11 );
  machine->GreaterThan();
  machine->Push( (long) 12 );
  machine->Push( (long) 12 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (long) 24 );
  machine->Push( (long) 25 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (double) 11 );
  machine->Push( (double) 11 );
  machine->GreaterThan();
  machine->Push( (long) 11 );
  machine->Push( (long) 11 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (long) 11 );
  machine->Push( (long) 110 );
  machine->GreaterThan();
  machine->Push( (double) 110 );
  machine->Push( (double) 11 );
  machine->GreaterThanOrEqualTo();
  machine->Push( (long) 11 );
  machine->Push( (long) 11 );
  machine->GreaterThan();
  machine->Push( (long) -10 );
  machine->Push( (long) -11 );
  machine->GreaterThan();
  machine->PushList( 10 );
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  gout << "\n\nTesting the instruction memory\n";
  gout << "Huge general test\n";

  program.Append( new PushRef( "ix" ) );
  program.Append( new Push<gRational>( 17 ) );
  program.Append( new Assign );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Push<gRational>( 23 ) );
  program.Append( new Assign );
  program.Append( new PushRef( "ix" ) );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Dump );
  program.Append( new PushRef( "ix" ) );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Equ );
  program.Append( new PushRef( "ix" ) );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Neq );
  program.Append( new PushRef( "ix" ) );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Gtn );
  program.Append( new PushRef( "ix" ) );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Ltn );
  program.Append( new PushRef( "ix" ) );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Geq );
  program.Append( new PushRef( "ix" ) );
  program.Append( new PushRef( "iy" ) );
  program.Append( new Leq );
  program.Append( new Dump );

  program.Append( new PushRef( "jx" ) );
  program.Append( new Push<gString>( "hi!" ) );
  program.Append( new Assign );
  program.Append( new PushRef( "jy" ) );
  program.Append( new Push<gString>( "hello!" ) );
  program.Append( new Assign );
  program.Append( new PushRef( "jx" ) );
  program.Append( new PushRef( "jy" ) );
  program.Append( new Dump );
  program.Append( new Push<gString>( "hi!" ) );
  program.Append( new Push<gString>( "hello!" ) );
  program.Append( new Add );
  program.Append( new PushRef( "jx" ) );
  program.Append( new PushRef( "jy" ) );
  program.Append( new Equ );
  program.Append( new PushRef( "jx" ) );
  program.Append( new PushRef( "jy" ) );
  program.Append( new Neq );
  program.Append( new PushRef( "jx" ) );
  program.Append( new PushRef( "jy" ) );
  program.Append( new Gtn );
  program.Append( new PushRef( "jx" ) );
  program.Append( new PushRef( "jy" ) );
  program.Append( new Ltn );
  program.Append( new PushRef( "jx" ) );
  program.Append( new PushRef( "jy" ) );
  program.Append( new Geq );
  program.Append( new PushRef( "jx" ) );
  program.Append( new PushRef( "jy" ) );
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

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  machine->Push( (long) 6 );
  machine->Push( (long) 5 );
  machine->Divide();
  machine->Dump();

  machine->InitCallFunction( "Divide" );
  machine->Push( (long) 6 );
  machine->Bind();
  machine->Push( (long) 5 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  machine->Push( (long) 6 );
  machine->Push( (long) 5 );
  machine->Divide();
  machine->Dump();

  machine->InitCallFunction( "Divide" );
  machine->Push( (long) 6 );
  machine->Bind();
  machine->Push( (long) 5 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "Testing Modulus\n";
  machine->Push( (long) 6 );
  machine->Push( (long) 5 );
  machine->Modulus();
  machine->Dump();

  gout << "Testing Modulus\n";
  machine->Push( (long) 5 );
  machine->Push( (long) 5 );
  machine->Modulus();
  machine->Dump();


  gout << "Testing Instructions:\n";

  program.Append( new Push<long>( 6 ) );
  program.Append( new Push<long>( 5 ) );
  program.Append( new Mod );
  program.Append( new Dump );

  program.Append( new Push<long>( 5 ) );
  program.Append( new Push<long>( 5 ) );
  program.Append( new Mod );
  program.Append( new Dump );

  machine->Execute( program );


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


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
  program.Append( new Push<long>( 6 ) );
  program.Append( new Bind );
  program.Append( new Push<long>( 5 ) );
  program.Append( new Bind );
  program.Append( new CallFunction );
  program.Append( new Dump );


  program.Append( new InitCallFunction( "Modulus" ) );
  program.Append( new Push<long>( 6 ) );
  program.Append( new Bind );
  program.Append( new Push<long>( 6 ) );
  program.Append( new Bind );
  program.Append( new CallFunction );
  program.Append( new Dump );


  program.Dump( gout );

  machine->Execute( program );

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  
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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif






  machine->PushRef( "lx" );
  machine->Push( (long) 10 );
  for( i = 1; i <= 9; i++ )
  {
    machine->Push( (long) i );
  }
  machine->PushList( 10 );
  machine->Assign();
  machine->Dump();


  machine->PushRef( "lx" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Dump();


#ifdef CRASHTEST
  machine->PushRef( "lx" );
  machine->Push( (long) 0 );
  machine->Subscript();
  machine->PushRef( "lx" );
  machine->Push( (long) 9 );
  machine->Subscript();
  machine->PushRef( "lx" );
  machine->Push( (long) 10 );
  machine->Subscript();

  machine->PushRef( "lx" );
  machine->Push( (double) 6 );
  machine->Subscript();  
  machine->Dump();
#endif // CRASHTEST



#ifdef CRASHTEST

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  machine->Push( (double) 10 );
  machine->Push( (double) 11 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Push( (double) 10 );
  machine->Push( (double) 11 );
  machine->Assign();
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "lx" );
  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->PushRef( "x4" );
  machine->PushRef( "x5" );
  machine->PushList( 5 );
  machine->Assign();
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  machine->PushRef( "lx" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->PushRef( "lx" );
  machine->Push( (long) 5 );
  machine->Subscript();
  machine->PushRef( "lx" );
  machine->Push( (long) -2 );
  machine->Subscript();
  gout << "here1!\n";
  machine->PushRef( "lx" );
  gout << "here2!\n";
  machine->Push( (long) 7 );
  gout << "here3!\n";
  machine->Subscript();
  gout << "here4!\n";
  machine->Dump();

#endif




#ifdef CRASHTEST
#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  machine->PushRef( "lx" );
  machine->Push( (double) 10 );
  for( i = 1; i <= 9; i++ )
  {
    machine->Push( (long) i );
  }
  machine->PushList( 9 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "lx" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Dump();

  machine->PushRef( "lx" );
  machine->Push( (long) 0 );
  machine->Subscript();
  machine->PushRef( "lx" );
  machine->Push( (long) 9 );
  machine->Subscript();
  machine->PushRef( "lx" );
  machine->Push( (long) 10 );
  machine->Subscript();
  machine->PushRef( "lx" );
  machine->Push( (double) 6 );
  machine->Subscript();  
  machine->Dump();
#endif



#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


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


#ifdef CRASHTEST
  machine->Push( (long) 0 );
  machine->Push( (long) 0 );
  machine->Divide();
  machine->Push( (gRational) 0 );
  machine->Push( (gRational) 0 );
  machine->Divide();
  machine->Push( (double) 0 );
  machine->Push( (double) 0 );
  machine->Divide();
  machine->Push( (long) 0 );
  machine->Push( (long) 0 );
  machine->Modulus();
  machine->Dump();
#endif // CRASHTEST




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

  machine->InitCallFunction( "Assign" );
  machine->Push( (double) 10 );
  machine->BindRef();
  machine->Push( (double) 15 );
  machine->BindRef();
  machine->CallFunction();
  machine->Dump();

#endif




  machine->Push( (long) 11 );
  machine->Push( (long) 3 );
  machine->Divide();
  machine->Dump();

  machine->Push( (long) 11 );
  machine->Push( (long) 3 );
  machine->IntegerDivide();
  machine->Dump();

  machine->Push( (long) 11 );
  machine->Push( (long) 3 );
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
  machine->Push( (long) 0 );
  machine->Bind();
  machine->Push( (long) 0 );
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
  machine->Push( (long) 5 );
  machine->Bind();
  machine->Push( (long) 3 );
  machine->Bind();
  machine->CallFunction();

  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "nx" );
  machine->InitCallFunction( "ReadNfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();


  machine->PushRef( "nx" );
  machine->Dump();



  machine->PushRef( "nx" );
  machine->Push( (double) 10 );
  machine->Assign();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif





  
  machine->InitCallFunction( "Assign" );
  machine->PushRef( "nx" );
  machine->Bind();
  machine->InitCallFunction( "ReadNfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();

  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  gout << "assigned primary ref\n";


  machine->PushRef( "x" );
  machine->Dump();




#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  machine->PushRef( "x" );
  machine->Push( (double) 5 );
  machine->Assign();
  machine->PushRef( "y" );
  machine->Push( (double) 6 );
  machine->Assign();
  machine->Dump();

#ifdef CRASHTEST
  machine->InitCallFunction( "Test" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Test" );
  machine->CallFunction();
  machine->Dump();
#endif // CRASHTEST


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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



#ifdef CRASHTEST
  machine->PushRef( "x" );
  machine->Push( (gRational) 1 );
  machine->Assign();
  machine->PushRef( "y" );
  machine->Push( (gRational) 2 );
  machine->Assign();


  gout << "testing Test\n";
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


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif
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
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Dump();


  machine->PushRef( "x" );
  machine->Push( (long) 2 );
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
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Bind();
  machine->Push( (double) 5 );
  machine->Bind(); 
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  machine->PushRef( "x" );
  machine->Push( (double) 7 );
  machine->Assign();
  machine->PushRef( "y" );
  machine->Push( (double) 8 );
  machine->Assign();

#ifdef CRASHTEST
  machine->InitCallFunction( "Test" );
  machine->PushRef( "x" );
  machine->Bind(  );
  machine->PushRef( "y" );
  machine->Bind( "x2" );
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->PushRef( "y" );
  machine->Dump();
#endif // CRASHTEST

  machine->InitCallFunction( "Test" );
  machine->Push( (double) 5 );
  machine->Bind();
  machine->Push( (double) 6 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "Test" );
  machine->Push( (gRational) 5 );
  machine->Bind();
  machine->Push( (gRational) 6 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

#ifdef CRASHTEST
  machine->InitCallFunction( "Test" );
  machine->Push( (long) 5 );
  machine->Bind();
  machine->Push( (long) 6 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
#endif // CRASHTEST

#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  machine->PushRef( "x" );
  machine->Push( (long) 1 );
  machine->Push( (long) 2 );

  machine->Push( (long) 11 );
  machine->Push( (long) 12 );

  machine->Push( (long) 21 );
  machine->Push( (long) 22 );
  machine->Push( (long) 23 );
  machine->Push( (long) 24 );
  machine->Push( (long) 25 );
  machine->PushList( 5 );

  machine->Push( (long) 14 );
  machine->Push( (long) 15 );
  machine->PushList( 5 );

  machine->Push( (long) 4 );
  machine->Push( (long) 5 );
  machine->PushList( 5 );
  machine->Assign();
  machine->Dump();



  machine->PushRef( "x" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 33 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Bind();
  machine->Push( (long) 43 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Bind();
  machine->Push( (long) 53 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Bind();
  machine->Push( (long) 52 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();

#ifdef CRASHTEST
  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 1 );
  machine->Subscript();
  machine->Bind();
  machine->Push( (double) 62 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();
#endif // CRASHTEST





#ifdef CRASHTEST
#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  machine->InitCallFunction( "Times" );
  machine->Push( (gRational) 3 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
#endif // CRASHTEST


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  machine->PushRef( "i" );
  machine->Push( (long) 3 );
  machine->Assign();

  machine->PushRef( "x" );
  machine->Push( (gRational) 1 );
  machine->Push( (gRational) 2 );
  machine->Push( (gRational) 3 );
  machine->Push( (gRational) 4 );
  machine->Push( (gRational) 5 );
  machine->PushList( 5 );
  machine->Assign();

  machine->PushRef( "x" );
  machine->PushRef( "i" );
  machine->Subscript();
  machine->Output();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "N" );
  machine->InitCallFunction( "ReadNfg" );
  machine->Push( "e02.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Output();


  machine->InitCallFunction( "Lemke" );
  machine->PushRef( "N" );
  machine->Bind( "N" );
  machine->PushRef( "time" );
  machine->Bind( "time" );
  machine->CallFunction();
  machine->Output();

  machine->PushRef( "time" );
  machine->Dump();


#ifdef CRASHTEST


  machine->InitCallFunction( "GobitNfg" );
  machine->PushRef( "N" );
  machine->Bind();
  machine->Push( "test" );
  machine->Bind();
  machine->PushRef( "time" );
  machine->Bind( "time" );
  machine->CallFunction();
  machine->Output();

  machine->InitCallFunction( "GobitNfg" );
  machine->PushRef( "N" );
  machine->Bind( "N" );
  machine->Push( "test" );
  machine->Bind();
  machine->PushRef( "time" );
  machine->Bind( "time" );
  machine->CallFunction();
  machine->Output();

  machine->InitCallFunction( "GobitNfg" );
  machine->PushRef( "N" );
  machine->Bind();
  machine->Push( "test" );
  machine->Bind( "pxifile" );
  machine->PushRef( "time" );
  machine->Bind( "time" );
  machine->CallFunction();
  machine->Output();

  machine->InitCallFunction( "GobitNfg" );
  machine->PushRef( "N" );
  machine->Bind( "N" );
  machine->Push( "test" );
  machine->Bind( "pxifile" );
  machine->PushRef( "time" );
  machine->Bind( "time" );
  machine->CallFunction();
  machine->Output();

#endif // CRASHTEST


#if 0
  machine->InitCallFunction( "GobitNfg" );
  machine->PushRef( "N" );
  machine->Bind( "N" );
  machine->PushStream( "test" );
  machine->Bind( "pxifile" );
  machine->PushRef( "time" );
  machine->Bind( "time" );
  machine->CallFunction();
  machine->Output();
#endif



#ifdef CRASHTEST
#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


  machine->InitCallFunction( "GobitEfg" );
  machine->PushRef( "E" );
  machine->Bind( "E" );
  machine->Push( "/dev/null" );
  machine->Bind( "pxifile" );
  machine->PushRef( "time" );
  machine->Bind( "time" );
  machine->CallFunction();
  machine->Output();

  machine->PushRef( "time" );
  machine->Dump();


#endif // CRASHTEST


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  machine->InitCallFunction( "Output" );
  machine->Push( "stdout1" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  
  machine->PushRef( "x1" );
  machine->InitCallFunction( "Output" );
  machine->Push( "stdout1" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x2" );
  machine->Bind();
  machine->InitCallFunction( "Output" );
  machine->Push( "stdout2" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();



  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->Assign();
  machine->Dump();




#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "F" );
  machine->InitCallFunction( "ReadEfg" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();


  machine->InitCallFunction( "Assign" );
  machine->PushRef( "E" );
  machine->Bind();
  machine->InitCallFunction( "ReadEfg" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();



  machine->PushRef( "G" );
  machine->InitCallFunction( "ReadEfg" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->PushRef( "H" );
  machine->InitCallFunction( "ReadEfg" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif


  machine->InitCallFunction( "ListTest" );
  machine->Push( (double) 1 );
  machine->PushList( 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "ListTest" );
  machine->Push( (gRational) 1 );
  machine->PushList( 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

#ifdef CRASHTEST
  machine->InitCallFunction( "ListTest" );
  machine->Push( (long) 1 );
  machine->PushList( 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
#endif // CRASHTEST


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "x1" );
  machine->InitCallFunction( "ReadNfg" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x2" );
  machine->Bind();
  machine->InitCallFunction( "ReadNfg" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x3" );
  machine->InitCallFunction( "ReadNfg" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "y1" );
  machine->Push( (double) 1 );
  machine->Assign();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "y2" );
  machine->Bind();
  machine->Push( (double) 2 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif


#ifdef CRASHTEST
  machine->PushRef( "xyz" );
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Dump();
#endif // CRASHTEST


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif



  prog = new gList< Instruction* >;
  prog->Append( new Push<double>( 1 ) );
  prog->Append( new Push<double>( 2 ) );
  prog->Append( new Push<double>( 3 ) );
  prog->Append( new Dump );

  func = new FuncDescObj( "TestUsr" );
  func->SetFuncInfo( prog, 0 );
  machine->AddFunction( func );

  prog = new gList< Instruction* >;
  prog->Append( new Push<double>( 4 ) );
  prog->Append( new InitCallFunction( "TestUsr" ) );
  prog->Append( new CallFunction );
  prog->Append( new Dump );

  func = new FuncDescObj( "TestUsr2" );
  func->SetFuncInfo( prog, 0 );
  machine->AddFunction( func );

  prog = new gList< Instruction* >;
  prog->Append( new Push<gString>( "x" ) );
  prog->Append( new PushRef( "x" ) );
  prog->Append( new Push<double>( 10 ) );
  prog->Append( new Assign );
  prog->Append( new Dump );
  prog->Append( new PushRef( "x" ) );
  prog->Append( new Dump );
  prog->Append( new Push<gString>( "::x" ) );
  prog->Append( new PushRef( "::x" ) );
  prog->Append( new Dump );
  prog->Append( new InitCallFunction( "Assign" ) );
  prog->Append( new PushRef( "::x" ) );
  prog->Append( new Bind );
  prog->Append( new Push<double>( 11 ) );
  prog->Append( new Bind );
  prog->Append( new CallFunction );
  prog->Append( new Dump );
  prog->Append( new PushRef( "::x" ) );
  prog->Append( new Dump );
  prog->Append( new Push<gString>( "::::x" ) );
  prog->Append( new PushRef( "::::x" ) );
  prog->Append( new Dump );


  func = new FuncDescObj( "TestUsr3" );
  func->SetFuncInfo( prog, 0 );
  machine->AddFunction( func );

  prog = new gList< Instruction* >;
  prog->Append( new Push<double>( 4 ) );
  prog->Append( new Dump );

  func = new FuncDescObj( "TestUsr4" );
  func->SetFuncInfo( prog, 0 );
  machine->AddFunction( func );




  machine->InitCallFunction( "TestUsr" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestUsr2" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestUsr" );
  machine->CallFunction();
  machine->Dump();


  gout << "Global x:\n";
  machine->PushRef( "x" );
  machine->Push( (double) 1 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x" );
  machine->Dump();
  
  gout << "Function x:\n";
  machine->InitCallFunction( "TestUsr3" );
  machine->CallFunction();
  machine->Dump();

  gout << "Global x:\n";
  machine->PushRef( "x" );
  machine->Dump();
  gout << "Done\n";

  machine->Push( (double) 1 );
  machine->Push( (double) 2 );
  machine->Push( (double) 3 );
  machine->InitCallFunction( "TestUsr4" );
  machine->CallFunction();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif



  machine->InitCallFunction( "VarTest" );
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "VarTestChange" );
  machine->Push( (double) 123 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "VarTest" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "VarTestChange" );
  machine->Push( (double) 456 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "VarTest" );
  machine->CallFunction();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "x1" );
  machine->Push( "test" );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->Push( (long) 0 );
  machine->Subscript();
  machine->Output();
  machine->PushRef( "x1" );
  machine->Push( (long) 1 );
  machine->Subscript();
  machine->Output();
  machine->PushRef( "x1" );
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Output();
  machine->PushRef( "x1" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Output();
  machine->PushRef( "x1" );
  machine->Push( (long) 4 );
  machine->Subscript();
  machine->Output();
  machine->PushRef( "x1" );
  machine->Push( (long) 5 );
  machine->Subscript();
  machine->Output();
  machine->PushRef( "x1" );
  machine->Push( (double) 1 );
  machine->Subscript();
  machine->Output();


  machine->InitCallFunction( "Plus" );
  machine->Push( "hi!" );
  machine->Bind();
  machine->Push( "hi!" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif

  machine->PushRef( "x1" );
  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->Push( (long) 3 );
  machine->PushList( 3 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x2" );
  machine->Push( (long) 4 );
  machine->Push( (long) 5 );
  machine->Push( (long) 6 );
  machine->PushList( 3 );
  machine->Assign();
  machine->Dump();


  gout << "\n4 combinations of Add & Assign\n";


  machine->PushRef( "x3" );
  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->Add();
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();



  machine->PushRef( "x3" );
  machine->InitCallFunction( "Plus" );
  machine->PushRef( "x1" );
  machine->Bind();
  machine->PushRef( "x2" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();




  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x3" );
  machine->Bind();
  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->Add();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();



  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x3" );
  machine->Bind();
  machine->InitCallFunction( "Plus" );
  machine->PushRef( "x1" );
  machine->Bind();
  machine->PushRef( "x2" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();



#ifdef CRASHTEST

#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif

  machine->PushRef( "x1" );
  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->Push( (long) 3 );
  machine->PushList( 3 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x2" );
  machine->Push( (double) 4 );
  machine->Push( (double) 5 );
  machine->Push( (double) 6 );
  machine->PushList( 3 );
  machine->Assign();
  machine->Dump();


  gout << "\n4 combinations of Add & Assign with wrong types\n";


  machine->PushRef( "x3" );
  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->Add();
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();



  machine->PushRef( "x3" );
  machine->InitCallFunction( "Plus" );
  machine->PushRef( "x1" );
  machine->Bind();
  machine->PushRef( "x2" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();



  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x3" );
  machine->Bind();
  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->Add();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();



  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x3" );
  machine->Bind();
  machine->InitCallFunction( "Plus" );
  machine->PushRef( "x1" );
  machine->Bind();
  machine->PushRef( "x2" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "x1" );
  machine->PushRef( "x2" );
  machine->PushRef( "x3" );
  machine->Dump();
#endif // CRASHTEST


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "L" );
  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->PushList( 2 );
  machine->Assign();
  machine->Dump();
  machine->PushRef( "L" );
  machine->Dump();

  machine->PushRef( "M" );
  machine->InitCallFunction( "Remove" );
  machine->PushRef( "L" );
  machine->Bind();
  machine->Push( (long) 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();
  machine->PushRef( "M" );
  machine->Dump();

  machine->PushRef( "N" );
  machine->InitCallFunction( "Remove" );
  machine->PushRef( "M" );
  machine->Bind();
  machine->Push( (long) 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();
  machine->PushRef( "N" );
  machine->Dump();


  machine->PushRef( "N" );
  machine->PushRef( "M" );
  machine->PushRef( "L" );
  machine->Dump();


  machine->PushRef( "L" );
  machine->PushList( 0 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "L" );
  machine->Dump();



#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "a" );
  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->PushList( 2 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "a" );
  machine->PushList( 0 );
  machine->Assign();
  machine->Dump();


  machine->PushRef( "a" );
  machine->Push( "a" );
  machine->PushList( 1 );
  machine->Assign();
  machine->Dump();

  machine->InitCallFunction( "Assign" );
  machine->PushRef( "a" );
  machine->Bind();
  machine->Push( "a" );
  machine->PushList( 1 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "a" );
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif



  prog = new gList< Instruction* >;
  prog->Append( new PushRef( "a" ) );
  prog->Append( new PushRef( "a" ) );
  prog->Append( new PushRef( "a" ) );
  prog->Append( new Add );
  prog->Append( new Assign );

  func = new FuncDescObj( "TestDump" );
  func->SetFuncInfo( prog, 1 );
  func->SetParamInfo( prog, 0, "a", porTEXT,
		     NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  machine->AddFunction( func );

  machine->PushRef( "yow" );
  machine->Push( "yowser! " );
  machine->Assign();
  machine->Dump();
  machine->InitCallFunction( "TestDump" );
  machine->PushRef( "yow" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  machine->PushRef( "yow" );
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif





  prog = new gList< Instruction* >;
  prog->Append( new PushRef( "temp" ) );
  prog->Append( new PushRef( "a" ) );
  prog->Append( new Assign );
  prog->Append( new PushRef( "a" ) );
  prog->Append( new PushRef( "b" ) );
  prog->Append( new Assign );
  prog->Append( new PushRef( "b" ) );
  prog->Append( new PushRef( "temp" ) );
  prog->Append( new Assign );
  prog->Append( new Flush );
  prog->Append( new PushRef( "temp" ) );
  
  func = new FuncDescObj( "TestSwap" );
  func->SetFuncInfo( prog, 2 );
  func->SetParamInfo( prog, 0, "a", porVALUE,
		     NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  func->SetParamInfo( prog, 1, "b", porVALUE,
		     NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  machine->AddFunction( func );


  machine->PushRef( "t1" );
  machine->Push( "string1" );
  machine->Assign();
  machine->PushRef( "t2" );
  machine->Push( "string2" );
  machine->Assign();
  machine->Flush();

  gout << "Before TestSwap\n";
  machine->PushRef( "t1" );
  machine->PushRef( "t2" );
  machine->Dump();

  machine->InitCallFunction( "TestSwap" );
  machine->PushRef( "t1" );
  machine->Bind();
  machine->PushRef( "t2" );
  machine->Bind();
  machine->CallFunction();
  machine->Flush();

  gout << "After TestSwap\n";
  machine->PushRef( "t1" );
  machine->PushRef( "t2" );
  machine->Dump();



  machine->PushRef( "t1" );
  machine->Push( "t1, before:" );
  machine->Dump();

  machine->InitCallFunction( "TestSwap" );
  machine->PushRef( "t1" );
  machine->Bind();
  machine->Push( "test string" );
  machine->Bind();
  machine->CallFunction();
  machine->Flush();


  machine->PushRef( "t1" );
  machine->Push( "t1, after:" );
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif





  machine->InitCallFunction( "DefaultEfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestDefEfg" );
  machine->CallFunction();
  machine->Dump();


  machine->InitCallFunction( "ReadDefaultEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02rat.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "DefaultEfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestDefEfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "ReadDefaultEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "DefaultEfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestDefEfg" );
  machine->CallFunction();
  machine->Dump();


  machine->PushRef( "test_efg" );
  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02rat.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->InitCallFunction( "CopyDefaultEfg" );
  machine->PushRef( "test_efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "DefaultEfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestDefEfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestDefEfg" );
  machine->InitCallFunction( "NewEfg" );
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "TestDefEfg" );
  machine->CallFunction();
  machine->Dump();




#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "OUTPUT" );
  machine->Dump();


  machine->PushRef( "OUTPUT" );
  machine->InitCallFunction( "Output" );
  machine->Push( "hello;" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();



#ifdef INTERACTIVE
  gout << "*********************** Press Return to continue ************";
  gin >> cont;
#endif



  machine->InitCallFunction( "DefaultNfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "ReadDefaultNfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "DefaultNfg" );
  machine->CallFunction();
  machine->Dump();

  machine->InitCallFunction( "ReadDefaultNfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "DefaultNfg" );
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "test_nfg" );
  machine->InitCallFunction( "ReadNfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "2x2.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->InitCallFunction( "CopyDefaultNfg" );
  machine->PushRef( "test_nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  
  machine->InitCallFunction( "DefaultNfg" );
  machine->CallFunction();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  machine->PushRef( "E" );
  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Output();

  machine->InitCallFunction( "WriteEfg" );
  machine->PushRef( "OUTPUT" );
  machine->Bind();
  machine->PushRef( "E" );
  machine->Bind();
  machine->CallFunction();
  machine->Output();
  
  machine->Dump();




#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif





  machine->PushRef( "x" );
  machine->InitCallFunction( "Output" );
  machine->Push( "stdout0" );
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();




  machine->InitCallFunction( "Assign" );
  machine->PushRef( "x" );
  machine->Bind();
  machine->InitCallFunction( "Output" );
  machine->Push( "stdout1" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Dump();


  
  machine->PushRef( "x" );
  machine->Dump();



#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  gout << "Testing Contains\n";
  machine->InitCallFunction( "Contains" );
  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->Push( (long) 3 );
  machine->Push( (long) 4 );
  machine->Push( (long) 5 );
  machine->PushList( 5 );
  machine->Bind();
  machine->Push( (double) 6 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();



#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif

  gout << "Testing UnAssign\n";
  machine->PushRef( "lx" );
  machine->UnAssign();
  machine->Dump();
  machine->PushRef( "lx" );
  machine->UnAssign();
  machine->Dump();
  machine->PushRef( "lx2" );
  machine->UnAssign();
  machine->Dump();
  machine->PushRef( "OUTPUT" );
  machine->UnAssign();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif
  

  prog = new gList< Instruction* >;
  prog->Append( new PushRef( "a" ) );
  prog->Append( new Push<double>( 1 ) );
  prog->Append( new Assign );

  func = new FuncDescObj( "TestRef" );
  func->SetFuncInfo( prog, 1 );
  func->SetParamInfo( prog, 0, "a", porFLOAT, 
		     NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  machine->AddFunction( func );


  machine->PushRef( "dx1" );
  machine->Push( (double) 2 );
  machine->Assign();
  machine->Dump();


  machine->InitCallFunction( "TestRef" );
  machine->PushRef( "dx1" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  machine->PushRef( "dx1" );
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif





  machine->PushRef( "E" );
  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Output();

  machine->PushRef( "E" );
  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e03.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Output();






  machine->InitCallFunction( "WriteEfg" );

  machine->InitCallFunction( "WriteEfg" );
  machine->InitCallFunction( "Output" );
  machine->Push( "e02out.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->PushRef( "E" );
  machine->Bind();
  machine->CallFunction();

  machine->Bind();
  machine->PushRef( "E" );
  machine->Bind();
  machine->CallFunction();

  machine->Output();
  
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


  

  machine->PushRef( "la" );
  machine->PushList( 0 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "la" );
  machine->PushRef( "la" );
  machine->Push( (long) 1 );
  machine->PushList( 1 );
  machine->Add();
  machine->Assign();
  machine->Dump();
  
  machine->PushRef( "la" );
  machine->Dump();



#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "intx" );
  machine->Push( (long) 1 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "inty" );
  machine->Push( (long) 2 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "intx" );
  machine->PushRef( "inty" );
  machine->EqualTo();
  machine->Dump();



#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif



  prog = new gList< Instruction* >;
  prog->Append( new PushRef( "a" ) );
  prog->Append( new Push<long>( 1 ) );
  prog->Append( new Leq );
  prog->Append( new IfGoto( 14 ) );

  prog->Append( new InitCallFunction( "Factorial" ) );
  prog->Append( new PushRef( "a" ) );
  prog->Append( new Push<long>( 1 ) );
  prog->Append( new Sub );
  prog->Append( new Bind );
  prog->Append( new CallFunction );
  prog->Append( new PushRef( "a" ) );
  prog->Append( new Mul );
  prog->Append( new Goto( 15 ) );

  prog->Append( new PushRef( "a" ) );
  prog->Append( new NOP );


  func = new FuncDescObj( "Factorial" );
  func->SetFuncInfo( prog, 1 );
  func->SetParamInfo( prog, 0, "a", porINTEGER );
  machine->AddFunction( func );


  gout << "Testing Factorial\n";
  machine->InitCallFunction( "Factorial" );
  machine->Push( (long) 6 );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();
  




#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "sx" );
  machine->Push( "Testing Testing 123" );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "sx" );
  machine->Push( (long) 5 );
  machine->Subscript();
  machine->Dump();

  machine->PushRef( "sx" );
  machine->Push( (long) 5 );
  machine->Subscript();
  machine->Push( "H" );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "sx" );
  machine->Dump();



#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


  machine->PushRef( "lix" );
  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->Push( (long) 3 );
  machine->Push( (long) 4 );
  machine->Push( (long) 5 );
  machine->PushList( 5 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "lix" );
  machine->Push( (long) 3 );
  machine->Subscript();
  machine->Push( (long) 13 );
  machine->Assign();
  machine->Dump();

  machine->PushRef( "lix" );
  machine->Dump();





#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif


  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->Assign();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif







  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->Push( (long) 3 );
  machine->Push( (long) 4 );
  machine->PushList( 4 );
  machine->Dump();

  machine->Push( (long) 1 );
  machine->Push( (long) 2 );
  machine->Push( (long) 3 );
  machine->Push( (long) 4 );
  machine->Push( (double) 5 );
  machine->PushList( 5 );
  machine->Dump();






#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "lE" );

  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02rat.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();

  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02rat.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();

  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();

  machine->PushList( 3 );
  machine->Assign();
  machine->Dump();

  
  machine->PushRef( "lE" );
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Dump();

#if 0  
  machine->PushRef( "lE" );
  machine->Push( (long) 2 );
  machine->Subscript();

  machine->InitCallFunction( "ReadEfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02rat.efg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();

  machine->Assign();
  machine->Dump();
#endif // 0

  machine->PushRef( "lE" );
  machine->Push( (long) 2 );
  machine->Subscript();
  machine->Dump();


#ifdef INTERACTIVE
  gout << "*********************** press return to continue ************";
  gin >> cont;
#endif




  machine->PushRef( "nx" );
  machine->InitCallFunction( "ReadNfg" );
  machine->InitCallFunction( "Input" );
  machine->Push( "e02.nfg" );
  machine->Bind();
  machine->CallFunction();
  machine->Bind();
  machine->CallFunction();
  machine->Assign();
  machine->Dump();

  machine->InitCallFunction( "PureNash" );
  machine->PushRef( "nx" );
  machine->Bind();
  machine->CallFunction();
  machine->Dump();

  

  gout << "*********************** Press Return to continue ************";
  gin >> cont;



  gout << "\nDeleting machine\n";
//  sout << "\nDeleting machine\n";
//  delete gsm;
  delete machine;

  return 0;
}



