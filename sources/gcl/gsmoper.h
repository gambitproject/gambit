//#
//# FILE: gsmoper.h -- header file for gsmoper.cc, general mathmatical and
//#                    other functions
//#
//# $Id$
//#


#include "portion.h"



//-------------------------------------------------------------------
//                      mathematical operators
//------------------------------------------------------------------


//---------------------- GSM_Add ---------------------------

Portion* GSM_Add_double( Portion** param );
Portion* GSM_Add_int( Portion** param );
Portion* GSM_Add_gRational( Portion** param );
Portion* GSM_Add_gString( Portion** param );
Portion* GSM_Add_List( Portion** param );
Portion* GSM_Add_MixedFloat( Portion** param );
Portion* GSM_Add_MixedRational( Portion** param );
Portion* GSM_Add_BehavFloat( Portion** param );
Portion* GSM_Add_BehavRational( Portion** param );


//---------------------- GSM_Subtract ------------------------

Portion* GSM_Subtract_double( Portion** param );
Portion* GSM_Subtract_int( Portion** param );
Portion* GSM_Subtract_gRational( Portion** param );
Portion* GSM_Subtract_MixedFloat( Portion** param );
Portion* GSM_Subtract_MixedRational( Portion** param );
Portion* GSM_Subtract_BehavFloat( Portion** param );
Portion* GSM_Subtract_BehavRational( Portion** param );



//------------------------- GSM_Multiply ---------------------------

Portion* GSM_Multiply_double( Portion** param );
Portion* GSM_Multiply_int( Portion** param );
Portion* GSM_Multiply_gRational( Portion** param );
Portion* GSM_Multiply_MixedFloat1( Portion** param );
Portion* GSM_Multiply_MixedFloat2( Portion** param );
Portion* GSM_Multiply_MixedRational1( Portion** param );
Portion* GSM_Multiply_MixedRational2( Portion** param );
Portion* GSM_Multiply_BehavFloat1( Portion** param );
Portion* GSM_Multiply_BehavFloat2( Portion** param );
Portion* GSM_Multiply_BehavRational1( Portion** param );
Portion* GSM_Multiply_BehavRational2( Portion** param );


//---------------------------- GSM_Divide -------------------------------

Portion* GSM_Divide_double( Portion** param );
Portion* GSM_Divide_int( Portion** param );
Portion* GSM_Divide_gRational( Portion** param );


//---------------------------- GSM_IntegerDivide -----------------------


Portion* GSM_IntegerDivide_int( Portion** param );


//---------------------- GSM_Exp, GSM_Log --------------------

Portion *GSM_Exp(Portion **param);
Portion *GSM_Log(Portion **param);


//------------------------ GSM_Power ------------------------

Portion* GSM_Power_Int_Int(Portion** param);
Portion* GSM_Power_Float_Int(Portion** param);
Portion* GSM_Power_Rational_Int(Portion** param);
Portion* GSM_Power_Int_Float(Portion** param);
Portion* GSM_Power_Float_Float(Portion** param);
Portion* GSM_Power_Rational_Float(Portion** param);


//------------------------ GSM_Negate ------------------------

Portion* GSM_Negate_double( Portion** param );
Portion* GSM_Negate_int( Portion** param );
Portion* GSM_Negate_gRational( Portion** param );


//--------------------------------------------------------------------
//                    integer math operators
//--------------------------------------------------------------------

//---------------------- GSM_Modulus ---------------------------

Portion* GSM_Modulus_int( Portion** param );
