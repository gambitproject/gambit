//#
//# FILE: gsmoper.cc -- implementations for GSM operator functions
//#                     companion to GSM
//#
//# $Id$
//#

#include "gsmoper.h"

Portion* GSM_Assign( Portion** param )
{
  Portion* result = 0;
  if( param[ 0 ] != 0 )
  {
    delete param[ 0 ];
  }
  param[ 0 ] = param[ 1 ]->Copy();
  result = param[ 1 ]->Copy();
  delete param[ 1 ];
  return result;
}


//-------------------------------------------------------------------
//                      mathematical operators
//------------------------------------------------------------------

Portion* GSM_Add( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    ( (numerical_Portion<double>*) param[ 0 ] )->Value() +=
      ( (numerical_Portion<double>*) param[ 1 ] )->Value();
    break;
    
  case porINTEGER:
    ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() +=
      ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value();
    break;

  case porRATIONAL:
    ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() +=
      ( (numerical_Portion<gRational>*) param[ 1 ] )->Value();
    break;

  default:
    delete param[ 0 ];
    param[ 0 ] = 0;
  }

  delete param[ 1 ];
  result = param[ 0 ];
  return result;
}


Portion* GSM_Subtract( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    ( (numerical_Portion<double>*) param[ 0 ] )->Value() -=
      ( (numerical_Portion<double>*) param[ 1 ] )->Value();
    break;
    
  case porINTEGER:
    ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() -=
      ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value();
    break;

  case porRATIONAL:
    ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() -=
      ( (numerical_Portion<gRational>*) param[ 1 ] )->Value();
    break;

  default:
    delete param[ 0 ];
    param[ 0 ] = 0;
  }

  delete param[ 1 ];
  result = param[ 0 ];
  return result;
}


Portion* GSM_Multiply( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    ( (numerical_Portion<double>*) param[ 0 ] )->Value() *=
      ( (numerical_Portion<double>*) param[ 1 ] )->Value();
    break;
    
  case porINTEGER:
    ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() *=
      ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value();
    break;

  case porRATIONAL:
    ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() *=
      ( (numerical_Portion<gRational>*) param[ 1 ] )->Value();
    break;

  default:
    delete param[ 0 ];
    param[ 0 ] = 0;
  }

  delete param[ 1 ];
  result = param[ 0 ];
  return result;
}



Portion* GSM_Divide( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    ( (numerical_Portion<double>*) param[ 0 ] )->Value() /=
      ( (numerical_Portion<double>*) param[ 1 ] )->Value();
    delete param[ 1 ];
    result = param[ 0 ];
    break;
    
  case porINTEGER:
    result = new numerical_Portion<gRational>
      (
       ( (gRational) ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() ) /
       ( (gRational) ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value() )
       );
    delete param[ 0 ];
    delete param[ 1 ];
    break;

  case porRATIONAL:
    ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() /=
      ( (numerical_Portion<gRational>*) param[ 1 ] )->Value();
    delete param[ 1 ];
    result = param[ 0 ];
    break;

  default:
    delete param[ 1 ];
    delete param[ 0 ];
    param[ 0 ] = 0;
  }

  return result;
}


Portion* GSM_Negate( Portion** param )
{
  Portion* result = 0;

  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    ( (numerical_Portion<double>*) param[ 0 ] )->Value() =
      -( (numerical_Portion<double>*) param[ 0 ] )->Value();
    break;
    
  case porINTEGER:
    ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() =
      -( (numerical_Portion<gInteger>*) param[ 0 ] )->Value();
    break;

  case porRATIONAL:
    ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() =
      -( (numerical_Portion<gRational>*) param[ 0 ] )->Value();
    break;

  default:
    delete param[ 0 ];
    param[ 0 ] = 0;
  }

  result = param[ 0 ];
  return result;
}



//--------------------------------------------------------------------
//                    integer math operators
//--------------------------------------------------------------------


Portion* GSM_Modulus( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porINTEGER:
    ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() %=
      ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value();
    break;

  default:
    delete param[ 0 ];
    param[ 0 ] = 0;
  }

  delete param[ 1 ];
  result = param[ 0 ];
  return result;
}



//-------------------------------------------------------------------
//                      relational operators
//------------------------------------------------------------------

Portion* GSM_EqualTo( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    result = new bool_Portion
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() ==
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new bool_Portion
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() ==
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new bool_Portion
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() ==
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;

  case porSTRING:
    result = new bool_Portion
      (
       ( (gString_Portion*) param[ 0 ] )->Value() ==
       ( (gString_Portion*) param[ 1 ] )->Value()
       );
    break;
  }

  delete param[ 1 ];
  delete param[ 0 ];
  return result;
}


Portion* GSM_NotEqualTo( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    result = new bool_Portion
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() !=
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new bool_Portion
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() !=
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new bool_Portion
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() !=
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;

  case porSTRING:
    result = new bool_Portion
      (
       ( (gString_Portion*) param[ 0 ] )->Value() !=
       ( (gString_Portion*) param[ 1 ] )->Value()
       );
    break;
  }

  delete param[ 1 ];
  delete param[ 0 ];
  return result;
}


Portion* GSM_GreaterThan( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    result = new bool_Portion
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() >
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new bool_Portion
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() >
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new bool_Portion
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() >
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;

  case porSTRING:
    result = new bool_Portion
      (
       ( (gString_Portion*) param[ 0 ] )->Value() >
       ( (gString_Portion*) param[ 1 ] )->Value()
       );
    break;
  }

  delete param[ 1 ];
  delete param[ 0 ];
  return result;
}


Portion* GSM_LessThan( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    result = new bool_Portion
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() <
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new bool_Portion
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() <
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new bool_Portion
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() <
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;

  case porSTRING:
    result = new bool_Portion
      (
       ( (gString_Portion*) param[ 0 ] )->Value() <
       ( (gString_Portion*) param[ 1 ] )->Value()
       );
    break;
  }

  delete param[ 1 ];
  delete param[ 0 ];
  return result;
}


Portion* GSM_GreaterThanOrEqualTo( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    result = new bool_Portion
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() >=
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new bool_Portion
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() >=
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new bool_Portion
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() >=
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;

  case porSTRING:
    result = new bool_Portion
      (
       ( (gString_Portion*) param[ 0 ] )->Value() >=
       ( (gString_Portion*) param[ 1 ] )->Value()
       );
    break;
  }

  delete param[ 1 ];
  delete param[ 0 ];
  return result;
}


Portion* GSM_LessThanOrEqualTo( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ]->Type() != param[ 1 ]->Type() )
    return 0;
  
  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    result = new bool_Portion
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() <=
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new bool_Portion
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() <=
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new bool_Portion
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() <=
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;

  case porSTRING:
    result = new bool_Portion
      (
       ( (gString_Portion*) param[ 0 ] )->Value() <=
       ( (gString_Portion*) param[ 1 ] )->Value()
       );
    break;
  }

  delete param[ 1 ];
  delete param[ 0 ];
  return result;
}





//----------------------------------------------------------------
//                       logical operators
//----------------------------------------------------------------



Portion* GSM_AND( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  ( (bool_Portion*) param[ 0 ] )->Value() =
    (
     ( (bool_Portion*) param[ 0 ] )->Value() &&
     ( (bool_Portion*) param[ 1 ] )->Value()
     );

  delete param[ 1 ];
  result = param[ 0 ];
  return result;
}



Portion* GSM_OR( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  ( (bool_Portion*) param[ 0 ] )->Value() =
    (
     ( (bool_Portion*) param[ 0 ] )->Value() ||
     ( (bool_Portion*) param[ 1 ] )->Value()
     );

  delete param[ 1 ];
  result = param[ 0 ];
  return result;
}



Portion* GSM_NOT( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  ( (bool_Portion*) param[ 0 ] )->Value() =
    !( (bool_Portion*) param[ 0 ] )->Value();

  result = param[ 0 ];
  return result;
}



void Init_gsmoper( GSM* gsm )
{
  FuncDescObj* FuncObj;

  FuncObj = new FuncDescObj( (gString) "Assign", GSM_Assign, 2 );
  FuncObj->SetParamInfo( 0, "x", porVALUE, NO_DEFAULT_VALUE, true );
  FuncObj->SetParamInfo( 1, "y", porVALUE, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );



  FuncObj = new FuncDescObj( (gString) "Plus", GSM_Add, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Minus", GSM_Subtract, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Times", GSM_Multiply, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Divide", GSM_Divide, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Negate", GSM_Negate, 1 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );


  FuncObj = new FuncDescObj( (gString) "Modulus", GSM_Modulus, 2 );
  FuncObj->SetParamInfo( 0, "x", porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porINTEGER, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );




  FuncObj = new FuncDescObj( (gString) "Equal", GSM_EqualTo, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "NotEqual", GSM_NotEqualTo, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Greater", GSM_GreaterThan, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Less", GSM_LessThan, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "GreaterEqual", 
			    GSM_GreaterThanOrEqualTo, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "LessEqual", 
			    GSM_LessThanOrEqualTo, 2 );
  FuncObj->SetParamInfo( 0, "x", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );



  FuncObj = new FuncDescObj( (gString) "And", GSM_AND, 2 );
  FuncObj->SetParamInfo( 0, "x", porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Or", GSM_OR, 2 );
  FuncObj->SetParamInfo( 0, "x", porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( 1, "y", porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Not", GSM_NOT, 1 );
  FuncObj->SetParamInfo( 0, "x", porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

}
