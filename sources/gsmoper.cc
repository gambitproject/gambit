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
  param[ 0 ]->MakeCopyOfData( param[ 1 ] );

  result = param[ 1 ]->Copy();

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
    result = new numerical_Portion<double>
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() +
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new numerical_Portion<gInteger>
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() +
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new numerical_Portion<gRational>
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() +
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;
  }
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
    result = new numerical_Portion<double>
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() -
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new numerical_Portion<gInteger>
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() -
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new numerical_Portion<gRational>
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() -
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;
  }
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
    result = new numerical_Portion<double>
      (
       ( (numerical_Portion<double>*) param[ 0 ] )->Value() *
       ( (numerical_Portion<double>*) param[ 1 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new numerical_Portion<gInteger>
      (
       ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() *
       ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new numerical_Portion<gRational>
      (
       ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() *
       ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
       );
    break;
  }
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
    if( ( (numerical_Portion<double>*) param[ 1 ] )->Value() != 0 )
    {
      result = new numerical_Portion<double>
	(
	 ( (numerical_Portion<double>*) param[ 0 ] )->Value() /
	 ( (numerical_Portion<double>*) param[ 1 ] )->Value()
	 );
    }
    else
    {
      gerr << "GSM_Divide Error: division by zero\n";
    }
    break;
    
  case porINTEGER:
    if( ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value() != 0 )
    {
      result = new numerical_Portion<gInteger>
	(
	 ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() /
	 ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
	 );
    }
    else
    {
      gerr << "GSM_Divide Error: division by zero\n";
    }
    break;

  case porRATIONAL:
    if( ( (numerical_Portion<gRational>*) param[ 1 ] )->Value() != 0 )
    {
      result = new numerical_Portion<gRational>
	(
	 ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() /
	 ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
	 );
    }
    else
    {
      gerr << "GSM_Divide Error: division by zero\n";
    }
    break;
  }
  return result;
}



Portion* GSM_Negate( Portion** param )
{
  Portion* result = 0;

  switch( param[ 0 ]->Type() )
  {
  case porDOUBLE:
    result = new numerical_Portion<double>
      (
       -( (numerical_Portion<double>*) param[ 0 ] )->Value()
       );
    break;
    
  case porINTEGER:
    result = new numerical_Portion<gInteger>
      (
       -( (numerical_Portion<gInteger>*) param[ 0 ] )->Value()
       );
    break;

  case porRATIONAL:
    result = new numerical_Portion<gRational>
      (
       -( (numerical_Portion<gRational>*) param[ 0 ] )->Value()
       );
    break;
  }
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
    if( ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value() != 0 )
    {
      result = new numerical_Portion<gInteger>
	(
	 ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() %
	 ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
	 );
    }
    else
    {
      gerr << "GSM_Divide Error: division by zero\n";
    }
    break;
  }
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

  return result;
}





//----------------------------------------------------------------
//                       logical operators
//----------------------------------------------------------------



Portion* GSM_AND( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  result = new bool_Portion
    (
     ( (bool_Portion*) param[ 0 ] )->Value() &&
     ( (bool_Portion*) param[ 1 ] )->Value()
     );
  return result;
}



Portion* GSM_OR( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  result = new bool_Portion
    (
     ( (bool_Portion*) param[ 0 ] )->Value() ||
     ( (bool_Portion*) param[ 1 ] )->Value()
     );

  return result;
}



Portion* GSM_NOT( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  result = new bool_Portion( !( (bool_Portion*) param[ 0 ] )->Value() );

  return result;
}



void Init_gsmoper( GSM* gsm )
{
  FuncDescObj* FuncObj;

  FuncObj = new FuncDescObj( (gString) "Assign" );
  FuncObj->SetFuncInfo( GSM_Assign, 2 );
  FuncObj->SetParamInfo( GSM_Assign, 0, "x", 
			porVALUE, NO_DEFAULT_VALUE, 
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_Assign, 1, "y", 
			porVALUE, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );



  FuncObj = new FuncDescObj( (gString) "Plus" );
  FuncObj->SetFuncInfo( GSM_Add, 2 );
  FuncObj->SetParamInfo( GSM_Add, 0, "x", porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Add, 1, "y", porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Minus" );
  FuncObj->SetFuncInfo( GSM_Subtract, 2 );
  FuncObj->SetParamInfo( GSM_Subtract, 0, "x", 
			porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Subtract, 1, "y", 
			porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Times" );
  FuncObj->SetFuncInfo( GSM_Multiply, 2 );
  FuncObj->SetParamInfo( GSM_Multiply, 0, "x", 
			porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Multiply, 1, "y", 
			porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Divide" );
  FuncObj->SetFuncInfo( GSM_Divide, 2 );
  FuncObj->SetParamInfo( GSM_Divide, 0, "x", 
			porNUMERICAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Divide, 1, "y", 
			porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Negate" );
  FuncObj->SetFuncInfo( GSM_Negate, 1 );
  FuncObj->SetParamInfo( GSM_Negate, 0, "x", 
			porNUMERICAL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );


  FuncObj = new FuncDescObj( (gString) "Modulus" );
  FuncObj->SetFuncInfo( GSM_Modulus, 2 );
  FuncObj->SetParamInfo( GSM_Modulus, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Modulus, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );




  FuncObj = new FuncDescObj( (gString) "Equal" );
  FuncObj->SetFuncInfo( GSM_EqualTo, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo, 0, "x", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_EqualTo, 1, "y", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "NotEqual" ); 
  FuncObj->SetFuncInfo( GSM_NotEqualTo, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo, 0, "x", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_NotEqualTo, 1, "y", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Greater" );
  FuncObj->SetFuncInfo( GSM_GreaterThan, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan, 0, "x", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThan, 1, "y", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Less" );
  FuncObj->SetFuncInfo( GSM_LessThan, 2 );
  FuncObj->SetParamInfo( GSM_LessThan, 0, "x", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThan, 1, "y", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "GreaterEqual" );
  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo, 0, "x", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo, 1, "y", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "LessEqual" );
  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo, 0, "x", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo, 1, "y", 
			porNUMERICAL | porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );



  FuncObj = new FuncDescObj( (gString) "And" );
  FuncObj->SetFuncInfo( GSM_AND, 2 );
  FuncObj->SetParamInfo( GSM_AND, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_AND, 1, "y", 
			porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Or" );
  FuncObj->SetFuncInfo( GSM_OR, 2 );
  FuncObj->SetParamInfo( GSM_OR, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_OR, 1, "y", 
			porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Not" );
  FuncObj->SetFuncInfo( GSM_NOT, 1 );
  FuncObj->SetParamInfo( GSM_NOT, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

}
