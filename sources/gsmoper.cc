//#
//# FILE: gsmoper.cc -- implementations for GSM operator functions
//#                     companion to GSM
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"


Portion* GSM_Assign( Portion** param )
{
  Portion* result = 0;
  if( param[ 0 ] != 0 )
  {
    delete param[ 0 ];
  }
  param[ 0 ] = param[ 1 ]->Copy( true );

  result = param[ 1 ]->Copy();

  return result;
}


//-------------------------------------------------------------------
//                      mathematical operators
//------------------------------------------------------------------


/*---------------------- GSM_Add ---------------------------*/

Portion* GSM_Add_double( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<double>
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() +
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Add_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gInteger>
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() +
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Add_gRational( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gRational>
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() +
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Add_List( Portion** param )
{
  Portion* result = 0;
  int i;
  int append_result;

  gBlock<Portion*>& p_value = ( (List_Portion*) param[ 1 ] )->Value();
  result = new List_Portion( ( (List_Portion*) param[ 0 ] )->Value() );
  for( i = 1; i <= p_value.Length(); i++ )
  {
    append_result = ( (List_Portion*) result )->Append( p_value[ i ]->Copy() );
    if( append_result == 0 )
    {
      delete result;
      result = new Error_Portion
	( (gString)
	 "GSM_Add Error:\n" +
	 "  Attempted to insert conflicting Portion\n" +
	 "  types into a List_Portion\n"
	 );
      break;
    }
  }
  return result;
}



/*---------------------- GSM_Subtract ------------------------*/

Portion* GSM_Subtract_double( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<double>
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() -
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Subtract_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gInteger>
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() -
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Subtract_gRational( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gRational>
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() -
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}



/*------------------------- GSM_Multiply ---------------------------*/

Portion* GSM_Multiply_double( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<double>
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() *
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Multiply_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gInteger>
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() *
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Multiply_gRational( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gRational>
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() *
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}



/*---------------------------- GSM_Divide -------------------------------*/

Portion* GSM_Divide_double( Portion** param )
{
  Portion* result = 0;
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
    result = new Error_Portion
      ( (gString)
       "GSM_Divide Error:\n" +
       "  Division by zero\n" 
       );
  }
  return result;
}

Portion* GSM_Divide_gInteger( Portion** param )
{
  Portion* result = 0;
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
    result = new Error_Portion
      ( (gString)
       "GSM_Divide Error:\n" +
       "  Division by zero\n" 
       );
  }
  return result;
}

Portion* GSM_Divide_gRational( Portion** param )
{
  Portion* result = 0;
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
    result = new Error_Portion
      ( (gString)
       "GSM_Divide Error:\n" +
       "  Division by zero\n" 
       );
  }
  return result;
}


/*---------------------- GSM_Exp, GSM_Log --------------------*/

Portion *GSM_Exp(Portion **param)
{
  return new numerical_Portion<double>
    (exp(((numerical_Portion<double> *) param[0])->Value()));
}

Portion *GSM_Log(Portion **param)
{
  return new numerical_Portion<double>
    (log(((numerical_Portion<double> *) param[0])->Value()));
}


/*------------------------ GSM_Negate ------------------------*/

Portion* GSM_Negate_double( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<double>
    (
     -( (numerical_Portion<double>*) param[ 0 ] )->Value()
     );
  return result;
}

Portion* GSM_Negate_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gInteger>
    (
     -( (numerical_Portion<gInteger>*) param[ 0 ] )->Value()
     );
  return result;
}

Portion* GSM_Negate_gRational( Portion** param )
{
  Portion* result = 0;
  result = new numerical_Portion<gRational>
    (
     -( (numerical_Portion<gRational>*) param[ 0 ] )->Value()
     );
  return result;
}



//--------------------------------------------------------------------
//                    integer math operators
//--------------------------------------------------------------------

/*---------------------- GSM_Modulus ---------------------------*/

Portion* GSM_Modulus_gInteger( Portion** param )
{
  Portion* result = 0;
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
    result = new Error_Portion
      ( (gString)
       "GSM_Modulus Error:\n" +
       "  Division by zero\n" 
       );
  }
  return result;
}



//-------------------------------------------------------------------
//                      relational operators
//------------------------------------------------------------------

/*---------------------- GSM_EqualTo --------------------------*/

Portion* GSM_EqualTo_double( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() ==
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() ==
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() ==
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (gString_Portion*) param[ 0 ] )->Value() ==
     ( (gString_Portion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_bool( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (bool_Portion*) param[ 0 ] )->Value() ==
     ( (bool_Portion*) param[ 1 ] )->Value()
     );
  return result;
}


/*----------------------- GSM_NotEqualTo --------------------------*/

Portion* GSM_NotEqualTo_double( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() !=
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() !=
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() !=
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (gString_Portion*) param[ 0 ] )->Value() !=
     ( (gString_Portion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_bool( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (bool_Portion*) param[ 0 ] )->Value() !=
     ( (bool_Portion*) param[ 1 ] )->Value()
     );
  return result;
}


/*-------------------------- GSM_GreaterThan -------------------------*/

Portion* GSM_GreaterThan_double( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() >
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThan_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() >
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThan_gRational( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() >
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThan_gString( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (gString_Portion*) param[ 0 ] )->Value() >
     ( (gString_Portion*) param[ 1 ] )->Value()
     );
  return result;
}


/*----------------------- GSM_LessThan -----------------------*/

Portion* GSM_LessThan_double( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() <
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThan_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() <
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThan_gRational( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() <
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThan_gString( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (gString_Portion*) param[ 0 ] )->Value() <
     ( (gString_Portion*) param[ 1 ] )->Value()
     );
  return result;
}


/*--------------------- GSM_GreaterThanOrEqualTo --------------------*/

Portion* GSM_GreaterThanOrEqualTo_double( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() >=
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() >=
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() >=
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (gString_Portion*) param[ 0 ] )->Value() >=
     ( (gString_Portion*) param[ 1 ] )->Value()
     );
  return result;
}


/*--------------------- GSM_LessThanOrEqualTo ---------------------*/

Portion* GSM_LessThanOrEqualTo_double( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<double>*) param[ 0 ] )->Value() <=
     ( (numerical_Portion<double>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThanOrEqualTo_gInteger( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gInteger>*) param[ 0 ] )->Value() <=
     ( (numerical_Portion<gInteger>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThanOrEqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (numerical_Portion<gRational>*) param[ 0 ] )->Value() <=
     ( (numerical_Portion<gRational>*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThanOrEqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new bool_Portion
    (
     ( (gString_Portion*) param[ 0 ] )->Value() <=
     ( (gString_Portion*) param[ 1 ] )->Value()
     );
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


Portion *GSM_Paren(Portion **param)
{
  return param[0]->Copy();
}

//-----------------------------------------------------------------
//    NewStream function - possibly belong somewhere else
//-----------------------------------------------------------------


Portion* GSM_NewStream( Portion** param )
{
  Portion* result = 0;
  gOutput* g;
  
  assert( param[ 0 ]->Type() == porSTRING );

  g = new gFileOutput( ( (gString_Portion*) param[ 0 ] )->Value() );
  result = new Stream_Portion( *g );
  
  return result;
}









void Init_gsmoper( GSM* gsm )
{
  FuncDescObj* FuncObj;


  /*---------------------- Assign ------------------------*/

  FuncObj = new FuncDescObj( (gString) "Assign" );
  FuncObj->SetFuncInfo( GSM_Assign, 2 );
  FuncObj->SetParamInfo( GSM_Assign, 0, "x", 
			porVALUE, NO_DEFAULT_VALUE, 
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_Assign, 1, "y", 
			porVALUE, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );



  /*-------------------- Plus ----------------------------*/

  FuncObj = new FuncDescObj( (gString) "Plus" );

  FuncObj->SetFuncInfo( GSM_Add_double, 2 );
  FuncObj->SetParamInfo( GSM_Add_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Add_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Add_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_Add_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Add_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Add_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Add_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Add_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Add_List, 2 );
  FuncObj->SetParamInfo( GSM_Add_List, 0, "x", 
			porLIST, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Add_List, 1, "y", 
			porLIST, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*----------------------- Minus ------------------------*/

  FuncObj = new FuncDescObj( (gString) "Minus" );
  FuncObj->SetFuncInfo( GSM_Subtract_double, 2 );
  FuncObj->SetParamInfo( GSM_Subtract_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Subtract_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Subtract_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_Subtract_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Subtract_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Subtract_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Subtract_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Subtract_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*----------------------- Times ----------------------------*/

  FuncObj = new FuncDescObj( (gString) "Times" );

  FuncObj->SetFuncInfo( GSM_Multiply_double, 2 );
  FuncObj->SetParamInfo( GSM_Multiply_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Multiply_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Multiply_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_Multiply_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Multiply_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Multiply_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Multiply_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Multiply_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*----------------------- Divide -------------------------*/

  FuncObj = new FuncDescObj( (gString) "Divide" );

  FuncObj->SetFuncInfo( GSM_Divide_double, 2 );
  FuncObj->SetParamInfo( GSM_Divide_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Divide_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Divide_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_Divide_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Divide_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Divide_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Divide_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Divide_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*----------------------- Exp, Log ---------------------*/

  FuncObj = new FuncDescObj("Exp");
  FuncObj->SetFuncInfo(GSM_Exp, 1);
  FuncObj->SetParamInfo(GSM_Exp, 0, "x", porDOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Log");
  FuncObj->SetFuncInfo(GSM_Log, 1);
  FuncObj->SetParamInfo(GSM_Log, 0, "x", porDOUBLE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);


  /*----------------------- Negate -----------------------*/

  FuncObj = new FuncDescObj( (gString) "Negate" );

  FuncObj->SetFuncInfo( GSM_Negate_double, 1 );
  FuncObj->SetParamInfo( GSM_Negate_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Negate_gInteger, 1 );
  FuncObj->SetParamInfo( GSM_Negate_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_Negate_gRational, 1 );
  FuncObj->SetParamInfo( GSM_Negate_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*----------------------- Modulus ------------------------*/

  FuncObj = new FuncDescObj( (gString) "Modulus" );

  FuncObj->SetFuncInfo( GSM_Modulus_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_Modulus_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_Modulus_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );





  /*-------------------------- Equal ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "Equal" );

  FuncObj->SetFuncInfo( GSM_EqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_EqualTo_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_EqualTo_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_EqualTo_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_EqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_EqualTo_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_EqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_gString, 0, "x", 
			porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_EqualTo_gString, 1, "y", 
			porSTRING, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_EqualTo_bool, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_bool, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_EqualTo_bool, 1, "y", 
			porBOOL, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*-------------------------- NotEqual ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "NotEqual" ); 

  FuncObj->SetFuncInfo( GSM_NotEqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_NotEqualTo_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gString, 0, "x", 
			porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gString, 1, "y", 
			porSTRING, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_bool, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_bool, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_NotEqualTo_bool, 1, "y", 
			porBOOL, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*-------------------------- Greater ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "Greater" );

  FuncObj->SetFuncInfo( GSM_GreaterThan_double, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThan_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_GreaterThan_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThan_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_GreaterThan_gRational, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThan_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_GreaterThan_gString, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_gString, 0, "x", 
			porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThan_gString, 1, "y", 
			porSTRING, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*-------------------------- Less ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "Less" );

  FuncObj->SetFuncInfo( GSM_LessThan_double, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThan_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_LessThan_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThan_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_LessThan_gRational, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThan_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_LessThan_gString, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_gString, 0, "x", 
			porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThan_gString, 1, "y", 
			porSTRING, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*---------------------- GreaterThanOrEqualTo ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "GreaterEqual" );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gString, 0, "x", 
			porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gString, 1, "y", 
			porSTRING, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );


  /*----------------------- LessThanOrEqualTo ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "LessEqual" );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_double, 0, "x", 
			porDOUBLE, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_double, 1, "y", 
			porDOUBLE, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_gInteger, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gInteger, 0, "x", 
			porINTEGER, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gInteger, 1, "y", 
			porINTEGER, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gRational, 0, "x", 
			porRATIONAL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gRational, 1, "y", 
			porRATIONAL, NO_DEFAULT_VALUE );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gString, 0, "x", 
			porSTRING, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gString, 1, "y", 
			porSTRING, NO_DEFAULT_VALUE );

  gsm->AddFunction( FuncObj );




  /*--------------------- And -------------------------------*/

  FuncObj = new FuncDescObj( (gString) "And" );
  FuncObj->SetFuncInfo( GSM_AND, 2 );
  FuncObj->SetParamInfo( GSM_AND, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_AND, 1, "y", 
			porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );


  /*--------------------- Or -------------------------------*/

  FuncObj = new FuncDescObj( (gString) "Or" );
  FuncObj->SetFuncInfo( GSM_OR, 2 );
  FuncObj->SetParamInfo( GSM_OR, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  FuncObj->SetParamInfo( GSM_OR, 1, "y", 
			porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );


  /*--------------------- Not -------------------------------*/

  FuncObj = new FuncDescObj( (gString) "Not" );
  FuncObj->SetFuncInfo( GSM_NOT, 1 );
  FuncObj->SetParamInfo( GSM_NOT, 0, "x", 
			porBOOL, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj("Paren");
  FuncObj->SetFuncInfo(GSM_Paren, 1);
  FuncObj->SetParamInfo(GSM_Paren, 0, "x", porVALUE, NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  /*-------------------- NewStream -------------------------*/

  FuncObj = new FuncDescObj( (gString) "NewStream" );
  FuncObj->SetFuncInfo( GSM_NewStream, 1 );
  FuncObj->SetParamInfo( GSM_NewStream, 0, "file",
			porSTRING, NO_DEFAULT_VALUE );
  gsm->AddFunction( FuncObj );

}


