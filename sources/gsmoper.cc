//#
//# FILE: gsmoper.cc -- implementations for GSM operator functions
//#                     companion to GSM
//#
//# $Id$
//#


#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "rational.h"
#include "gblock.h"



Portion* GSM_Assign( Portion** param )
{
  Portion* result = 0;

  if( param[ 0 ] != 0 && param[ 1 ] != 0 &&
     param[ 0 ]->Type() == param[ 1 ]->Type() )
  {
    if( param[ 0 ]->Type() != porLIST )
    {
      param[ 0 ]->AssignFrom( param[ 1 ] );
      result = param[ 0 ]->RefCopy();
    }
    else if( ( (ListPortion*) param[ 0 ] )->DataType() ==
	    ( (ListPortion*) param[ 1 ] )->DataType() )
    {
      param[ 0 ]->AssignFrom( param[ 1 ] );
      result = param[ 0 ]->RefCopy();
    }
    else
    {
      result = new ErrorPortion( "Attempted to change the type of a list\n" );
    }
  }
  else
  {
    delete param[ 0 ];
    param[ 0 ] = param[ 1 ]->ValCopy();
    result = param[ 0 ]->RefCopy();
  }

  return result;
}



//-------------------------------------------------------------------
//                      mathematical operators
//------------------------------------------------------------------


/*---------------------- GSM_Add ---------------------------*/

Portion* GSM_Add_double( Portion** param )
{
  Portion* result = 0;
  result = new FloatValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() +
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Add_int( Portion** param )
{
  Portion* result = 0;
  result = new IntValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() +
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Add_gRational( Portion** param )
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() +
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Add_gString( Portion** param )
{
  Portion* result = 0;
  result = new TextValPortion
    (
     ( (TextPortion*) param[ 0 ] )->Value() +
     ( (TextPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Add_List( Portion** param )
{
  Portion* result = 0;
  int i;
  int append_result;

  gBlock<Portion*>& p_value = ( (ListPortion*) param[ 1 ] )->Value();
  result = new ListValPortion( ( (ListPortion*) param[ 0 ] )->Value() );
  for( i = 1; i <= p_value.Length(); i++ )
  {
    append_result = ( (ListPortion*) result )->Append( p_value[ i ]->ValCopy() );
    if( append_result == 0 )
    {
      delete result;
      result = new ErrorPortion
	( (gString)
	 "GSM_Add Error:\n" +
	 "  Attempted to insert conflicting Portion\n" +
	 "  types into a ListPortion\n"
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
  result = new FloatValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() -
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Subtract_int( Portion** param )
{
  Portion* result = 0;
  result = new IntValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() -
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Subtract_gRational( Portion** param )
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() -
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}



/*------------------------- GSM_Multiply ---------------------------*/

Portion* GSM_Multiply_double( Portion** param )
{
  Portion* result = 0;
  result = new FloatValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() *
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Multiply_int( Portion** param )
{
  Portion* result = 0;
  result = new IntValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() *
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_Multiply_gRational( Portion** param )
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() *
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}



/*---------------------------- GSM_Divide -------------------------------*/

Portion* GSM_Divide_double( Portion** param )
{
  Portion* result = 0;
  if( ( (FloatPortion*) param[ 1 ] )->Value() != 0 )
  {
    result = new FloatValPortion
      (
       ( (FloatPortion*) param[ 0 ] )->Value() /
       ( (FloatPortion*) param[ 1 ] )->Value()
       );
  }
  else
  {
    result = new ErrorPortion
      ( (gString)
       "GSM_Divide Error:\n" +
       "  Division by zero\n" 
       );
  }
  return result;
}

Portion* GSM_Divide_int( Portion** param )
{
  Portion* result = 0;
  if( ( (IntPortion*) param[ 1 ] )->Value() != 0 )
  {
    result = new IntValPortion
      (
       ( (IntPortion*) param[ 0 ] )->Value() /
       ( (IntPortion*) param[ 1 ] )->Value()
       );
  }
  else
  {
    result = new ErrorPortion
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
  if( ( (RationalPortion*) param[ 1 ] )->Value() != 0 )
  {
    result = new RationalValPortion
      (
       ( (RationalPortion*) param[ 0 ] )->Value() /
       ( (RationalPortion*) param[ 1 ] )->Value()
       );
  }
  else
  {
    result = new ErrorPortion
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
  return new FloatValPortion
    (exp(((FloatPortion *) param[0])->Value()));
}

Portion *GSM_Log(Portion **param)
{
  return new FloatValPortion
    (log(((FloatPortion *) param[0])->Value()));
}


/*------------------------ GSM_Negate ------------------------*/

Portion* GSM_Negate_double( Portion** param )
{
  Portion* result = 0;
  result = new FloatValPortion
    (
     -( (FloatPortion*) param[ 0 ] )->Value()
     );
  return result;
}

Portion* GSM_Negate_int( Portion** param )
{
  Portion* result = 0;
  result = new IntValPortion
    (
     -( (IntPortion*) param[ 0 ] )->Value()
     );
  return result;
}

Portion* GSM_Negate_gRational( Portion** param )
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     -( (RationalPortion*) param[ 0 ] )->Value()
     );
  return result;
}



//--------------------------------------------------------------------
//                    integer math operators
//--------------------------------------------------------------------

/*---------------------- GSM_Modulus ---------------------------*/

Portion* GSM_Modulus_int( Portion** param )
{
  Portion* result = 0;
  if( ( (IntPortion*) param[ 1 ] )->Value() != 0 )
  {
    result = new IntValPortion
      (
       ( (IntPortion*) param[ 0 ] )->Value() %
       ( (IntPortion*) param[ 1 ] )->Value()
       );
  }
  else
  {
    result = new ErrorPortion
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
  result = new BoolValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() ==
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_int( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() ==
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() ==
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (TextPortion*) param[ 0 ] )->Value() ==
     ( (TextPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_EqualTo_bool( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (BoolPortion*) param[ 0 ] )->Value() ==
     ( (BoolPortion*) param[ 1 ] )->Value()
     );
  return result;
}


/*----------------------- GSM_NotEqualTo --------------------------*/

Portion* GSM_NotEqualTo_double( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() !=
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_int( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() !=
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() !=
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (TextPortion*) param[ 0 ] )->Value() !=
     ( (TextPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_NotEqualTo_bool( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (BoolPortion*) param[ 0 ] )->Value() !=
     ( (BoolPortion*) param[ 1 ] )->Value()
     );
  return result;
}


/*-------------------------- GSM_GreaterThan -------------------------*/

Portion* GSM_GreaterThan_double( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() >
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThan_int( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() >
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThan_gRational( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() >
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThan_gString( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (TextPortion*) param[ 0 ] )->Value() >
     ( (TextPortion*) param[ 1 ] )->Value()
     );
  return result;
}


/*----------------------- GSM_LessThan -----------------------*/

Portion* GSM_LessThan_double( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() <
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThan_int( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() <
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThan_gRational( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() <
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThan_gString( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (TextPortion*) param[ 0 ] )->Value() <
     ( (TextPortion*) param[ 1 ] )->Value()
     );
  return result;
}


/*--------------------- GSM_GreaterThanOrEqualTo --------------------*/

Portion* GSM_GreaterThanOrEqualTo_double( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() >=
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_int( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() >=
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() >=
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (TextPortion*) param[ 0 ] )->Value() >=
     ( (TextPortion*) param[ 1 ] )->Value()
     );
  return result;
}


/*--------------------- GSM_LessThanOrEqualTo ---------------------*/

Portion* GSM_LessThanOrEqualTo_double( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (FloatPortion*) param[ 0 ] )->Value() <=
     ( (FloatPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThanOrEqualTo_int( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (IntPortion*) param[ 0 ] )->Value() <=
     ( (IntPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThanOrEqualTo_gRational( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (RationalPortion*) param[ 0 ] )->Value() <=
     ( (RationalPortion*) param[ 1 ] )->Value()
     );
  return result;
}

Portion* GSM_LessThanOrEqualTo_gString( Portion** param )
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ( (TextPortion*) param[ 0 ] )->Value() <=
     ( (TextPortion*) param[ 1 ] )->Value()
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
  
  result = new BoolValPortion
    (
     ( (BoolPortion*) param[ 0 ] )->Value() &&
     ( (BoolPortion*) param[ 1 ] )->Value()
     );
  return result;
}



Portion* GSM_OR( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  result = new BoolValPortion
    (
     ( (BoolPortion*) param[ 0 ] )->Value() ||
     ( (BoolPortion*) param[ 1 ] )->Value()
     );

  return result;
}



Portion* GSM_NOT( Portion** param )
{
  Portion* result = 0;

  assert( param[ 0 ]->Type() == porBOOL );
  
  result = new BoolValPortion( !( (BoolPortion*) param[ 0 ] )->Value() );

  return result;
}


Portion *GSM_Paren(Portion **param)
{
  return param[0]->ValCopy();
}

//-----------------------------------------------------------------
//    NewStream function - possibly belong somewhere else
//-----------------------------------------------------------------


Portion* GSM_NewOutputStream( Portion** param )
{
  Portion* result = 0;
  gOutput* g;
  
  assert( param[ 0 ]->Type() == porTEXT );

  g = new gFileOutput( ( (TextPortion*) param[ 0 ] )->Value() );
  result = new OutputValPortion( *g );
  
  return result;
}


Portion* GSM_NewInputStream( Portion** param )
{
  Portion* result = 0;
  gInput* g;
  
  assert( param[ 0 ]->Type() == porTEXT );

  g = new gFileInput( ( (TextPortion*) param[ 0 ] )->Value() );
  result = new InputValPortion( *g );
  
  return result;
}







void Init_gsmoper( GSM* gsm )
{
  FuncDescObj* FuncObj;


  /*---------------------- Assign ------------------------*/
  /*
  FuncObj = new FuncDescObj( (gString) "Assign" );
  FuncObj->SetFuncInfo( GSM_Assign, 2 );
  FuncObj->SetParamInfo( GSM_Assign, 0, "x", 
			porVALUE, NO_DEFAULT_VALUE,
			PASS_BY_REFERENCE );
  FuncObj->SetParamInfo( GSM_Assign, 1, "y", 
			porVALUE );
  gsm->AddFunction( FuncObj );
  */


  /*-------------------- Plus ----------------------------*/

  FuncObj = new FuncDescObj( (gString) "Plus" );

  FuncObj->SetFuncInfo( GSM_Add_double, 2 );
  FuncObj->SetParamInfo( GSM_Add_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_Add_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_Add_int, 2 );
  FuncObj->SetParamInfo( GSM_Add_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_Add_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_Add_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Add_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_Add_gRational, 1, "y", 
			porRATIONAL );

  FuncObj->SetFuncInfo( GSM_Add_gString, 2 );
  FuncObj->SetParamInfo( GSM_Add_gString, 0, "x", 
			porTEXT );
  FuncObj->SetParamInfo( GSM_Add_gString, 1, "y", 
			porTEXT );

  FuncObj->SetFuncInfo( GSM_Add_List, 2 );
  FuncObj->SetParamInfo( GSM_Add_List, 0, "x", 
			porLIST );
  FuncObj->SetParamInfo( GSM_Add_List, 1, "y", 
			porLIST );

  gsm->AddFunction( FuncObj );


  /*----------------------- Minus ------------------------*/

  FuncObj = new FuncDescObj( (gString) "Minus" );
  FuncObj->SetFuncInfo( GSM_Subtract_double, 2 );
  FuncObj->SetParamInfo( GSM_Subtract_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_Subtract_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_Subtract_int, 2 );
  FuncObj->SetParamInfo( GSM_Subtract_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_Subtract_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_Subtract_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Subtract_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_Subtract_gRational, 1, "y", 
			porRATIONAL );

  gsm->AddFunction( FuncObj );


  /*----------------------- Times ----------------------------*/

  FuncObj = new FuncDescObj( (gString) "Times" );

  FuncObj->SetFuncInfo( GSM_Multiply_double, 2 );
  FuncObj->SetParamInfo( GSM_Multiply_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_Multiply_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_Multiply_int, 2 );
  FuncObj->SetParamInfo( GSM_Multiply_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_Multiply_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_Multiply_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Multiply_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_Multiply_gRational, 1, "y", 
			porRATIONAL );

  gsm->AddFunction( FuncObj );


  /*----------------------- Divide -------------------------*/

  FuncObj = new FuncDescObj( (gString) "Divide" );

  FuncObj->SetFuncInfo( GSM_Divide_double, 2 );
  FuncObj->SetParamInfo( GSM_Divide_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_Divide_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_Divide_int, 2 );
  FuncObj->SetParamInfo( GSM_Divide_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_Divide_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_Divide_gRational, 2 );
  FuncObj->SetParamInfo( GSM_Divide_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_Divide_gRational, 1, "y", 
			porRATIONAL );

  gsm->AddFunction( FuncObj );


  /*----------------------- Exp, Log ---------------------*/

  FuncObj = new FuncDescObj("Exp");
  FuncObj->SetFuncInfo(GSM_Exp, 1);
  FuncObj->SetParamInfo(GSM_Exp, 0, "x", porFLOAT);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Log");
  FuncObj->SetFuncInfo(GSM_Log, 1);
  FuncObj->SetParamInfo(GSM_Log, 0, "x", porFLOAT);
  gsm->AddFunction(FuncObj);


  /*----------------------- Negate -----------------------*/

  FuncObj = new FuncDescObj( (gString) "Negate" );

  FuncObj->SetFuncInfo( GSM_Negate_double, 1 );
  FuncObj->SetParamInfo( GSM_Negate_double, 0, "x", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_Negate_int, 1 );
  FuncObj->SetParamInfo( GSM_Negate_int, 0, "x", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_Negate_gRational, 1 );
  FuncObj->SetParamInfo( GSM_Negate_gRational, 0, "x", 
			porRATIONAL );

  gsm->AddFunction( FuncObj );



  /*----------------------- IntegerDivide ------------------------*/

  FuncObj = new FuncDescObj( (gString) "IntegerDivide" );

  FuncObj->SetFuncInfo( GSM_Divide_int, 2 );
  FuncObj->SetParamInfo( GSM_Divide_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_Divide_int, 1, "y", 
			porINTEGER );

  gsm->AddFunction( FuncObj );



  /*----------------------- Modulus ------------------------*/

  FuncObj = new FuncDescObj( (gString) "Modulus" );

  FuncObj->SetFuncInfo( GSM_Modulus_int, 2 );
  FuncObj->SetParamInfo( GSM_Modulus_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_Modulus_int, 1, "y", 
			porINTEGER );

  gsm->AddFunction( FuncObj );





  /*-------------------------- Equal ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "Equal" );

  FuncObj->SetFuncInfo( GSM_EqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_EqualTo_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_EqualTo_int, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_EqualTo_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_EqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_EqualTo_gRational, 1, "y", 
			porRATIONAL );

  FuncObj->SetFuncInfo( GSM_EqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_gString, 0, "x", 
			porTEXT );
  FuncObj->SetParamInfo( GSM_EqualTo_gString, 1, "y", 
			porTEXT );

  FuncObj->SetFuncInfo( GSM_EqualTo_bool, 2 );
  FuncObj->SetParamInfo( GSM_EqualTo_bool, 0, "x", 
			porBOOL );
  FuncObj->SetParamInfo( GSM_EqualTo_bool, 1, "y", 
			porBOOL );

  gsm->AddFunction( FuncObj );


  /*-------------------------- NotEqual ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "NotEqual" ); 

  FuncObj->SetFuncInfo( GSM_NotEqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_NotEqualTo_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_int, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_NotEqualTo_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gRational, 1, "y", 
			porRATIONAL );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gString, 0, "x", 
			porTEXT );
  FuncObj->SetParamInfo( GSM_NotEqualTo_gString, 1, "y", 
			porTEXT );

  FuncObj->SetFuncInfo( GSM_NotEqualTo_bool, 2 );
  FuncObj->SetParamInfo( GSM_NotEqualTo_bool, 0, "x", 
			porBOOL );
  FuncObj->SetParamInfo( GSM_NotEqualTo_bool, 1, "y", 
			porBOOL );

  gsm->AddFunction( FuncObj );


  /*-------------------------- Greater ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "Greater" );

  FuncObj->SetFuncInfo( GSM_GreaterThan_double, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_GreaterThan_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_GreaterThan_int, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_GreaterThan_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_GreaterThan_gRational, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_GreaterThan_gRational, 1, "y", 
			porRATIONAL );

  FuncObj->SetFuncInfo( GSM_GreaterThan_gString, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThan_gString, 0, "x", 
			porTEXT );
  FuncObj->SetParamInfo( GSM_GreaterThan_gString, 1, "y", 
			porTEXT );

  gsm->AddFunction( FuncObj );


  /*-------------------------- Less ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "Less" );

  FuncObj->SetFuncInfo( GSM_LessThan_double, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_LessThan_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_LessThan_int, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_LessThan_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_LessThan_gRational, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_LessThan_gRational, 1, "y", 
			porRATIONAL );

  FuncObj->SetFuncInfo( GSM_LessThan_gString, 2 );
  FuncObj->SetParamInfo( GSM_LessThan_gString, 0, "x", 
			porTEXT );
  FuncObj->SetParamInfo( GSM_LessThan_gString, 1, "y", 
			porTEXT );

  gsm->AddFunction( FuncObj );


  /*---------------------- GreaterThanOrEqualTo ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "GreaterEqual" );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_int, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gRational, 1, "y", 
			porRATIONAL );

  FuncObj->SetFuncInfo( GSM_GreaterThanOrEqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gString, 0, "x", 
			porTEXT );
  FuncObj->SetParamInfo( GSM_GreaterThanOrEqualTo_gString, 1, "y", 
			porTEXT );

  gsm->AddFunction( FuncObj );


  /*----------------------- LessThanOrEqualTo ---------------------------*/

  FuncObj = new FuncDescObj( (gString) "LessEqual" );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_double, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_double, 0, "x", 
			porFLOAT );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_double, 1, "y", 
			porFLOAT );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_int, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_int, 0, "x", 
			porINTEGER );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_int, 1, "y", 
			porINTEGER );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_gRational, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gRational, 0, "x", 
			porRATIONAL );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gRational, 1, "y", 
			porRATIONAL );

  FuncObj->SetFuncInfo( GSM_LessThanOrEqualTo_gString, 2 );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gString, 0, "x", 
			porTEXT );
  FuncObj->SetParamInfo( GSM_LessThanOrEqualTo_gString, 1, "y", 
			porTEXT );

  gsm->AddFunction( FuncObj );




  /*--------------------- And -------------------------------*/

  FuncObj = new FuncDescObj( (gString) "And" );
  FuncObj->SetFuncInfo( GSM_AND, 2 );
  FuncObj->SetParamInfo( GSM_AND, 0, "x", 
			porBOOL );
  FuncObj->SetParamInfo( GSM_AND, 1, "y", 
			porBOOL );
  gsm->AddFunction( FuncObj );


  /*--------------------- Or -------------------------------*/

  FuncObj = new FuncDescObj( (gString) "Or" );
  FuncObj->SetFuncInfo( GSM_OR, 2 );
  FuncObj->SetParamInfo( GSM_OR, 0, "x", 
			porBOOL );
  FuncObj->SetParamInfo( GSM_OR, 1, "y", 
			porBOOL );
  gsm->AddFunction( FuncObj );


  /*--------------------- Not -------------------------------*/

  FuncObj = new FuncDescObj( (gString) "Not" );
  FuncObj->SetFuncInfo( GSM_NOT, 1 );
  FuncObj->SetParamInfo( GSM_NOT, 0, "x", 
			porBOOL );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj("Paren");
  FuncObj->SetFuncInfo(GSM_Paren, 1);
  FuncObj->SetParamInfo(GSM_Paren, 0, "x", porVALUE);
  gsm->AddFunction(FuncObj);

  /*-------------------- NewStream -------------------------*/

  FuncObj = new FuncDescObj( (gString) "Output" );
  FuncObj->SetFuncInfo( GSM_NewOutputStream, 1 );
  FuncObj->SetParamInfo( GSM_NewOutputStream, 0, "file",
			porTEXT );
  gsm->AddFunction( FuncObj );

  FuncObj = new FuncDescObj( (gString) "Input" );
  FuncObj->SetFuncInfo( GSM_NewInputStream, 1 );
  FuncObj->SetParamInfo( GSM_NewInputStream, 0, "file",
			porTEXT );
  gsm->AddFunction( FuncObj );

}


