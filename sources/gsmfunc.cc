//
//  FILE:  gsmfunc.cc -- handles initialization of defined functions for GSM
//                       companion to GSM  
//




// Usage:
//
// Create an 'void Init_<name>( void )' function that calls GSM::AddFunction() to add
// the desired functions into GSM, where <name> is the name of the particular module.
// Then create a header file for that module, and include it in this file.
// Put the Init_<name>() function call in the GSM::InitFunctions() in this file.
//
// The definition for GSM::AddFunction is:
//   GSM::AddFunction( gString reference, Portion *(*actual_function_name)( void ) );
//
// The calling syntax should be:
//   GSM::AddFunction( (gString) reference, actual_function_name );
//
// Example:
//   void Init_myfunc( void )
//   {
//     GSM::AddFunction( (gString) "cos", my_own_cosine_function );
//   }
//
// All the header files should #include "gsm.h", and all defined functions should return
// type 'Portion *'.  The definitions for Portion types are already included in "gsm.h".
//
// Take a look at gclmath.h and gclmath.cc to see how it works.




// The header files for each module should be placed here:

#include "gclmath.h"



// This function is called once at the first instance of GSM.
// The Init function of each module should be placed in this function:

void GSM::InitFunctions( void )
{
  Init_gclmath();
}

