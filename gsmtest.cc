

#include <stdio.h>
#include "gsm.h"
#include "rational.h"


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

  gout << "*********************** press return to continue ************";
  gin >> cont;

  gout << "Attempt to add different types()\n";
  machine->Push( d_1 );
  machine->Push( d_2 );
  machine->Push( i_1 );
  machine->Add();
  machine->Dump();

  gout << "Testing Pop() functions\n";
  machine->Push( d_1 );
  machine->Push( i_1 );
  machine->Push( r_1 );
  gout << "Initial Depth(): " << machine->Depth() << "\n";
  machine->Pop( r_temp );
  machine->Pop( i_temp );
  machine->Pop( d_temp );
  gout << r_temp << "\n";
  gout << i_temp << "\n";
  gout << d_temp << "\n";
  gout << "Final Depth(): " << machine->Depth() << "\n\n";
 
  gout << "*********************** press return to continue ************";
  gin >> cont;
 
  d_temp = (double) 0;
  i_temp = (gInteger) 0;
  r_temp = (gRational) 0;
  gout << "Initial d_temp :" << d_temp << "\n";
  gout << "Initial i_temp :" << i_temp << "\n";
  gout << "Initial r_temp :" << r_temp << "\n";
  gout << "\n";
  gout << "Testing: attempt to Pop() with the wrong type\n";
  gout << "Push() double:    " << d_1 << "\n";
  gout << "Push() gInteger:  " << i_1 << "\n";
  gout << "Push() gRational: " << r_1 << "\n";
  machine->Push( d_1 );
  machine->Push( i_1 );
  machine->Push( r_1 );
  gout << "Pop( double d_temp )\n";
  machine->Pop( d_temp );
  gout << "Pop( gInteger i_temp )\n";
  machine->Pop( i_temp );
  gout << "Pop( gRational r_temp )\n";
  machine->Pop( r_temp );
  gout << "resulting d_temp: " << d_temp << "\n";
  gout << "resulting i_temp: " << i_temp << "\n";
  gout << "resulting r_temp: " << r_temp << "\n";
  gout << "\n";
  gout << "Dumping stack:\n";
  machine->Dump();


  gout << "*********************** press return to continue ************";
  gin >> cont;

  d_temp = (double) 0;
  i_temp = (gInteger) 0;
  r_temp = (gRational) 0;
  gout << "Initial d_temp :" << d_temp << "\n";
  gout << "Initial i_temp :" << i_temp << "\n";
  gout << "Initial r_temp :" << r_temp << "\n";
  gout << "\n";
  gout << "Testing Peek()\n";
  gout << "Pushing double:    " << d_1 << "\n";
  machine->Push( d_1 );
  gout << "Pushing gInteger:  " << i_1 << "\n";
  machine->Push( i_1 );
  gout << "Pushing gRational: " << r_1 << "\n";
  machine->Push( r_1 );
  gout << "Attempt to Peek() with double\n";
  machine->Peek( d_temp );
  gout << "Attempt to Peek() with gInteger\n";
  machine->Peek( i_temp );
  gout << "Attempt to Peek() with gRational\n";
  machine->Peek( r_temp );
  gout << "\n";
  gout << "Dumping stack:\n";
  machine->Dump();
  gout << "Resulting d_temp :" << d_temp << "\n";
  gout << "Resulting i_temp :" << i_temp << "\n";
  gout << "Resulting r_temp :" << r_temp << "\n";

  gout << "*********************** press return to continue ************";
  gin >> cont;

  
  delete machine;

  return 0;
}

