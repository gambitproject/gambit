//
// FILE: gsmoper.cc -- implementations for GSM operator functions
//                     companion to GSM
//
// $Id$
//

#include <stdlib.h>
#include <ctype.h>

#include "gmisc.h"
#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "rational.h"
#include "gblock.h"

#include "mixedsol.h"
#include "behavsol.h"

#include "nfg.h"
#include "efg.h"

#include "system.h"
#include "gstack.h"
#include "gstring.h"


#include "gcmdline.h"


extern GSM* _gsm;




//---------------------------------------
//           IsNull
//---------------------------------------

Portion* GSM_IsNull(Portion** param)
{
  return new BoolPortion(param[0]->Spec().Type == porNULL);
}


//---------------------------------------
//           Null
//---------------------------------------

Portion* GSM_Null(Portion** param)
{
  return new NullPortion(param[0]->Spec().Type);
}



//---------------------------------------
//           Randomize
//---------------------------------------

long _idum = -1;

Portion* GSM_Randomize_Integer(Portion** param)
{
  long _RandomSeed = ((IntPortion*) param[1])->Value();
  if(_RandomSeed > 0)
    _RandomSeed = -_RandomSeed;
  long v;
  if(_RandomSeed != 0)
    _idum = _RandomSeed;
  v = ran1(&_idum);
  return new IntPortion(v);
}

Portion* GSM_Randomize_Number(Portion** param)
{
  long _RandomSeed = ((IntPortion*) param[1])->Value();
  if(_RandomSeed > 0)
    _RandomSeed = -_RandomSeed;
  double v;
  if(_RandomSeed != 0)
    _idum = _RandomSeed;
  v = ran1(&_idum) / IM;
  return new NumberPortion(v);
}



//-------------------------------------------
//              Assign
//-------------------------------------------

/*
Portion* GSM_Assign(Portion** param)
{
  Portion* p = 0;
  Portion* result = 0;
  if( param[1]->IsReference() )
  {
    p = param[1]->RefCopy();
    result = param[1]->RefCopy();
  }
  else
  {
    p = param[1]->ValCopy();
    result = param[1]->ValCopy();
  }
  _gsm->Assign(param[0], p);
  return result;
}

Portion* GSM_UnAssign(Portion** param)
{
  return _gsm->UnAssignExt(param[0]);
}
*/


//-------------------------------------------------------------------
//                      mathematical operators
//------------------------------------------------------------------


//---------------------- GSM_Add ---------------------------

Portion* GSM_Add_int(Portion** param)
{
  Portion* result = 0;
  result = new IntPortion
    (
     ((IntPortion*) param[0])->Value() +
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Add_Number(Portion** param)
{
  return new NumberPortion(((NumberPortion*) param[0])->Value() +
                           ((NumberPortion*) param[1])->Value());
}

Portion* GSM_Add_Mixed(Portion** param)
{
  if(((MixedPortion*) param[0])->Value()->Support() !=
     ((MixedPortion*) param[1])->Value()->Support())
    return new ErrorPortion("Support mismatch");

  Portion *result = param[0]->ValCopy();
  *((MixedPortion*) result)->Value() += *((MixedPortion*) param[1])->Value();
  return result;
}

Portion* GSM_Add_Behav(Portion** param)
{
  if(((BehavPortion*) param[0])->Value()->Support() !=
     ((BehavPortion*) param[1])->Value()->Support())
    return new ErrorPortion("Support mismatch");

  Portion *result = param[0]->ValCopy();
  *((BehavPortion*) result)->Value() += *((BehavPortion*) param[1])->Value();
  return result;
}

//--------------------- Concat ---------------------------

Portion* GSM_Concat_gString(Portion** param)
{
  Portion* result = 0;
  result = new TextPortion
    (
     ((TextPortion*) param[0])->Value() +
     ((TextPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Concat_List(Portion** param)
{
  Portion* result = 0;
  int i;
  int append_result;

  // gBlock<Portion*>& p_value = ((ListPortion*) param[1])->Value();
  gList<Portion*>& p_value = ((ListPortion*) param[1])->Value();
  result = new ListPortion(((ListPortion*) param[0])->Value());
  for(i = 1; i <= p_value.Length(); i++)
  {
    append_result = ((ListPortion*) result)->Append(p_value[i]->ValCopy());
    if(append_result == 0)
    {
      delete result;
      result = new ErrorPortion
	("Attempted concatenating lists of different types");
      break;
    }
  }
  return result;
}



//---------------------- GSM_Subtract ------------------------

Portion* GSM_Subtract_int(Portion** param)
{
  Portion* result = 0;
  result = new IntPortion
    (
     ((IntPortion*) param[0])->Value() -
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Subtract_Number(Portion** param)
{
  return new NumberPortion(((NumberPortion *) param[0])->Value() -
                           ((NumberPortion *) param[1])->Value());
}


Portion* GSM_Subtract_Mixed(Portion** param)
{
  if(((MixedPortion*) param[0])->Value()->Support() !=
     ((MixedPortion*) param[1])->Value()->Support())
    return new ErrorPortion("Support mismatch");

  Portion *result = param[0]->ValCopy();
  *((MixedPortion*) result)->Value() -= *((MixedPortion*) param[1])->Value();
  return result;
}


Portion* GSM_Subtract_Behav(Portion** param)
{
  if(((BehavPortion*) param[0])->Value()->Support() !=
     ((BehavPortion*) param[1])->Value()->Support())
    return new ErrorPortion("Support mismatch");

  Portion *result = param[0]->ValCopy();
  *((BehavPortion*) result)->Value() -= *((BehavPortion*) param[1])->Value();
  return result;
}


//------------------------- GSM_Multiply ---------------------------

Portion* GSM_Multiply_int(Portion** param)
{
  Portion* result = 0;
  result = new IntPortion
    (
     ((IntPortion*) param[0])->Value() *
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Multiply_Number(Portion** param)
{
  return new NumberPortion(((NumberPortion *) param[0])->Value() *
                           ((NumberPortion *) param[1])->Value());
}


Portion* GSM_Multiply_Mixed(Portion** param)
{
  Portion *result = param[1]->ValCopy();
  *((MixedPortion*) result)->Value() *= ((NumberPortion*) param[0])->Value();
  return result;
}

Portion* GSM_Multiply_Behav(Portion** param)
{
  Portion *result = param[1]->ValCopy();
  *((BehavPortion*) result)->Value() *= ((NumberPortion*) param[0])->Value();
  return result;
}

//---------------------------- GSM_Divide -------------------------------

Portion* GSM_Divide_int(Portion** param)
{
  Portion* result = 0;
  if(((IntPortion*) param[1])->Value() != 0)
  {
    result = new NumberPortion(((IntPortion*) param[0])->Value());
    ((NumberPortion*) result)->Value() /=
      ((IntPortion*) param[1])->Value();
  }
  else
  {
    result = new NullPortion(porNUMBER);
  }
  return result;
}



Portion* GSM_Divide_Number(Portion** param)
{
  Portion* result = 0;
  if(((NumberPortion*) param[1])->Value() != (gNumber)0)
  {
    result = new NumberPortion
      (
       ((NumberPortion*) param[0])->Value() /
       ((NumberPortion*) param[1])->Value()
      );
  }
  else
  {
    result = new NullPortion(porNUMBER);
  }
  return result;
}



//-------------------------- GSM_IntegerDivide -------------------------------

Portion* GSM_IntegerDivide_int(Portion** param)
{
  Portion* result = 0;
  if(((IntPortion*) param[1])->Value() != 0)
  {
    result = new IntPortion
      (
       ((IntPortion*) param[0])->Value() /
       ((IntPortion*) param[1])->Value()
      );
  }
  else
  {
    result = new NullPortion(porINTEGER);
    //result = new ErrorPortion("Division by zero");
  }
  return result;
}




//---------------------- GSM_Exp, GSM_Log --------------------

Portion *GSM_Exp(Portion **param)
{
  return new NumberPortion
    (exp((double) ((NumberPortion *) param[0])->Value()));
}

Portion *GSM_Log(Portion **param)
{
  double d = (double) ((NumberPortion *) param[0])->Value();
  if(d <= 0.0)
    return new NullPortion(porNUMBER);
  else
    return new NumberPortion(log(d));
}


//------------------------ GSM_Power ------------------------

Portion* GSM_Power_Int_Int(Portion** param)
{
  long base = ((IntPortion*) param[0])->Value();
  long exponent = ((IntPortion*) param[1])->Value();
  return new IntPortion((long) pow((double) base, (double) exponent));
}

Portion* GSM_Power_Number_Int(Portion** param)
{
  gRational base = ((NumberPortion*) param[0])->Value();
  long exponent = ((IntPortion*) param[1])->Value();
  return new NumberPortion(pow(base, exponent));
}


Portion* GSM_Power_Int_Number(Portion** param)
{
  long base = ((IntPortion*) param[0])->Value();
  double exponent = ((NumberPortion*) param[1])->Value();
  return new NumberPortion((double) pow((double) base, exponent));
}

Portion* GSM_Power_Number_Number(Portion** param)
{
  gRational base = ((NumberPortion*) param[0])->Value();
  double exponent = ((NumberPortion*) param[1])->Value();
  return new NumberPortion((double) pow(base, ((long) exponent)));
}




//------------------------ GSM_Negate ------------------------

Portion* GSM_Negate_int(Portion** param)
{
  Portion* result = 0;
  result = new IntPortion
    (
     -((IntPortion*) param[0])->Value()
    );
  return result;
}

Portion* GSM_Negate_Number(Portion** param)
{
  return new NumberPortion(-((NumberPortion *) param[0])->Value());
}



//--------------------------------------------------------------------
//                    integer math operators
//--------------------------------------------------------------------

//---------------------- GSM_Modulus ---------------------------

Portion* GSM_Modulus_int(Portion** param)
{
  Portion* result = 0;
  if(((IntPortion*) param[1])->Value() != 0)
  {
    result = new IntPortion
      (
       ((IntPortion*) param[0])->Value() %
       ((IntPortion*) param[1])->Value()
      );
  }
  else
  {
    result = new NullPortion(porINTEGER);
    //result = new ErrorPortion("Division by zero");
  }
  return result;
}



//-------------------------------------------------------------------
//                      relational operators
//------------------------------------------------------------------

//-----------
// Equal
//-----------

Portion* GSM_Equal_Integer(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }

  return new BoolPortion(((IntPortion *) param[0])->Value() ==
			    ((IntPortion *) param[1])->Value());
}

Portion* GSM_Equal_Number(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((NumberPortion *) param[0])->Value() ==
			    ((NumberPortion *) param[1])->Value());
}

Portion* GSM_Equal_Text(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((TextPortion *) param[0])->Value() ==
			    ((TextPortion *) param[1])->Value());
}

Portion* GSM_Equal_Boolean(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((BoolPortion *) param[0])->Value() ==
			    ((BoolPortion *) param[1])->Value());
}

Portion* GSM_Equal_Efg(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((EfgPortion *) param[0])->Value() ==
			    ((EfgPortion *) param[1])->Value());
}

Portion* GSM_Equal_EfPlayer(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((EfPlayerPortion *) param[0])->Value() ==
			    ((EfPlayerPortion *) param[1])->Value());
}

Portion* GSM_Equal_Node(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((NodePortion *) param[0])->Value() ==
			    ((NodePortion *) param[1])->Value());
}

Portion* GSM_Equal_Infoset(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((InfosetPortion *) param[0])->Value() ==
			    ((InfosetPortion *) param[1])->Value());
}

Portion* GSM_Equal_Outcome(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((EfOutcomePortion *) param[0])->Value() ==
			    ((EfOutcomePortion *) param[1])->Value());
}

Portion* GSM_Equal_Action(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((ActionPortion *) param[0])->Value() ==
			    ((ActionPortion *) param[1])->Value());
}

Portion* GSM_Equal_EfSupport(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion((*((EfSupportPortion *) param[0])->Value()) ==
			    (*((EfSupportPortion *) param[1])->Value()));
}

Portion* GSM_Equal_Behav(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion
    (*((BehavPortion *) param[0])->Value() ==
     *((BehavPortion *) param[1])->Value());
}

Portion *GSM_Equal_Nfg(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }

  return new BoolPortion(((NfgPortion *) param[0])->Value() ==
			      ((NfgPortion *) param[1])->Value());

}

Portion* GSM_Equal_NfPlayer(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((NfPlayerPortion *) param[0])->Value() ==
			    ((NfPlayerPortion *) param[1])->Value());
}

Portion* GSM_Equal_Strategy(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((StrategyPortion *) param[0])->Value() ==
			    ((StrategyPortion *) param[1])->Value());
}

Portion* GSM_Equal_NfSupport(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion((*((NfSupportPortion *) param[0])->Value()) ==
			    (*((NfSupportPortion *) param[1])->Value()));
}

Portion* GSM_Equal_Mixed(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion
    (*((MixedPortion *) param[0])->Value() ==
     *((MixedPortion *) param[1])->Value());
}


//------------
// NotEqual
//------------

Portion* GSM_NotEqual_Integer(Portion** param)
{
  return new BoolPortion(((IntPortion *) param[0])->Value() !=
			    ((IntPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion *) param[0])->Value() !=
			    ((NumberPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Text(Portion** param)
{
  return new BoolPortion(((TextPortion *) param[0])->Value() !=
			    ((TextPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Boolean(Portion** param)
{
  return new BoolPortion(((BoolPortion *) param[0])->Value() !=
			    ((BoolPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Efg(Portion** param)
{
  return new BoolPortion(((EfgPortion *) param[0])->Value() !=
			    ((EfgPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_EfPlayer(Portion** param)
{
  return new BoolPortion(((EfPlayerPortion *) param[0])->Value() !=
			    ((EfPlayerPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Node(Portion** param)
{
  return new BoolPortion(((NodePortion *) param[0])->Value() !=
			    ((NodePortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Infoset(Portion** param)
{
  return new BoolPortion(((InfosetPortion *) param[0])->Value() !=
			    ((InfosetPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Outcome(Portion** param)
{
  return new BoolPortion(((EfOutcomePortion *) param[0])->Value() !=
			    ((EfOutcomePortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Action(Portion** param)
{
  return new BoolPortion(((ActionPortion *) param[0])->Value() !=
			    ((ActionPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_EfSupport(Portion** param)
{
  return new BoolPortion((*((EfSupportPortion *) param[0])->Value()) !=
			    (*((EfSupportPortion *) param[1])->Value()));
}

Portion* GSM_NotEqual_Behav(Portion** param)
{
  return new BoolPortion
    (*((BehavPortion *) param[0])->Value() !=
     *((BehavPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_NfPlayer(Portion** param)
{
  return new BoolPortion(((NfPlayerPortion *) param[0])->Value() !=
			    ((NfPlayerPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Strategy(Portion** param)
{
  return new BoolPortion(((StrategyPortion *) param[0])->Value() !=
			    ((StrategyPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Nfg(Portion** param)
{
  if ((param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type != 
 			       param[1]->Spec().Type );
  }

  return new BoolPortion(((NfgPortion *) param[0])->Value() !=
			      ((NfgPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_NfSupport(Portion** param)
{
  return new BoolPortion((*((NfSupportPortion *) param[0])->Value()) !=
			    (*((NfSupportPortion *) param[1])->Value()));
}

Portion* GSM_NotEqual_Mixed(Portion** param)
{
  return new BoolPortion
    (*((MixedPortion *) param[0])->Value() !=
     *((MixedPortion *) param[1])->Value());
}


//-------------------------- GSM_GreaterThan -------------------------

Portion* GSM_Greater_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((IntPortion*) param[0])->Value() >
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Greater_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() >
                         ((NumberPortion*) param[1])->Value());
}

Portion* GSM_Greater_Text(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((TextPortion*) param[0])->Value() >
     ((TextPortion*) param[1])->Value()
    );
  return result;
}


//----------------------- GSM_LessThan -----------------------

Portion* GSM_Less_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((IntPortion*) param[0])->Value() <
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Less_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() <
                         ((NumberPortion*) param[1])->Value());
}

Portion* GSM_Less_Text(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((TextPortion*) param[0])->Value() <
     ((TextPortion*) param[1])->Value()
    );
  return result;
}


//--------------------- GSM_GreaterThanOrEqualTo --------------------

Portion* GSM_GreaterEqual_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((IntPortion*) param[0])->Value() >=
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_GreaterEqual_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() >=
                         ((NumberPortion*) param[1])->Value());
}

Portion* GSM_GreaterEqual_Text(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((TextPortion*) param[0])->Value() >=
     ((TextPortion*) param[1])->Value()
    );
  return result;
}


//--------------------- GSM_LessThanOrEqualTo ---------------------

Portion* GSM_LessEqual_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((IntPortion*) param[0])->Value() <=
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_LessEqual_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() <=
                         ((NumberPortion*) param[1])->Value());
}

Portion* GSM_LessEqual_Text(Portion** param)
{
  Portion* result = 0;
  result = new BoolPortion
    (
     ((TextPortion*) param[0])->Value() <=
     ((TextPortion*) param[1])->Value()
    );
  return result;
}




//----------------------------------------------------------------
//                       logical operators
//----------------------------------------------------------------



Portion* GSM_AND(Portion** param)
{
  Portion* result = 0;

  assert(param[0]->Spec().Type == porBOOL);
  
  result = new BoolPortion
    (
     ((BoolPortion*) param[0])->Value() &&
     ((BoolPortion*) param[1])->Value()
    );
  return result;
}



Portion* GSM_OR(Portion** param)
{
  Portion* result = 0;

  assert(param[0]->Spec().Type == porBOOL);
  
  result = new BoolPortion
    (
     ((BoolPortion*) param[0])->Value() ||
     ((BoolPortion*) param[1])->Value()
    );

  return result;
}



Portion* GSM_NOT(Portion** param)
{
  Portion* result = 0;

  assert(param[0]->Spec().Type == porBOOL);
  
  result = new BoolPortion(!((BoolPortion*) param[0])->Value());

  return result;
}


Portion *GSM_Parentheses(Portion **param)
{
  return param[0]->ValCopy();
}

//-----------------------------------------------------------------
//    NewStream function - possibly belong somewhere else
//-----------------------------------------------------------------


Portion* GSM_NewOutputStream(Portion** param)
{
  Portion* result = 0;
  gOutput* g;
  
  gString filename = ((TextPortion*) param[0])->Value();
  bool append = ((BoolPortion*) param[1])->Value();

  g = new gFileOutput(filename, append);
  
  if(g->IsValid())
    result = new OutputPortion(*g);    
  else
    result = new ErrorPortion((gString) "Error opening file \"" + 
			      ((TextPortion*) param[0])->Value() + "\"");
  return result;
}


Portion* GSM_NewInputStream(Portion** param)
{
  Portion* result = 0;
  gInput* g;
  
  assert(param[0]->Spec().Type == porTEXT);

  g = new gFileInput(((TextPortion*) param[0])->Value());

  if(g->IsValid())
    result = new InputPortion(*g);
  else
    result = new ErrorPortion((gString) "Error opening file \"" + 
			      ((TextPortion*) param[0])->Value() + "\"");
  
  return result;
}




//-----------------------------------------------------------------
//    Write and SetFormat function - possibly belong somewhere else
//-----------------------------------------------------------------

long _WriteWidth = 0;
long _WritePrecis = 6;
bool _WriteExpmode = false;
bool _WriteQuoted = true;
bool _WriteListBraces = true;
bool _WriteListCommas = true;
long _WriteListLF = 0;
long _WriteListIndent = 2;
long _WriteSolutionInfo = 1;


void GSM_SetWriteOptions(void)
{
  Portion::_SetWriteWidth(_WriteWidth);
  Portion::_SetWritePrecis(_WritePrecis);
  Portion::_SetWriteExpmode(_WriteExpmode);
  Portion::_SetWriteQuoted(_WriteQuoted);
  Portion::_SetWriteListBraces(_WriteListBraces);
  Portion::_SetWriteListCommas(_WriteListCommas);
  Portion::_SetWriteListLF(_WriteListLF);
  Portion::_SetWriteListIndent(_WriteListIndent);
  Portion::_SetWriteSolutionInfo(_WriteSolutionInfo);

  ToStringWidth( _WriteWidth );
  ToStringPrecision( _WritePrecis );
}


//------------------------------------------------
// *Format
//------------------------------------------------

Portion* GSM_ListFormat(Portion** param)
{
  _WriteListBraces = ((BoolPortion*) param[0])->Value();
  _WriteListCommas = ((BoolPortion*) param[1])->Value();
  _WriteListLF = ((IntPortion*) param[2])->Value();
  _WriteListIndent = ((IntPortion*) param[3])->Value();

  GSM_SetWriteOptions();

  return new BoolPortion(true);
}

Portion* GSM_IntFormat(Portion** param)
{
  _WriteWidth = ((IntPortion*) param[1])->Value();

  GSM_SetWriteOptions();

  return param[0]->RefCopy();
}

Portion* GSM_FloatFormat(Portion** param)
{
  _WriteWidth = ((IntPortion*) param[1])->Value();
  _WritePrecis = ((IntPortion*) param[2])->Value();
  _WriteExpmode = ((BoolPortion*) param[3])->Value();

  GSM_SetWriteOptions();

  return param[0]->RefCopy();
}


Portion* GSM_TextFormat(Portion** param)
{
  _WriteQuoted = ((BoolPortion*) param[1])->Value();

  GSM_SetWriteOptions();

  return param[0]->RefCopy();
}


Portion* GSM_SolutionFormat(Portion** param)
{
  _WriteSolutionInfo = ((IntPortion*) param[1])->Value();

  GSM_SetWriteOptions();

  return param[0]->RefCopy();
}




//-------------------------------------------------


Portion* GSM_SetVerbose(Portion** param)
{
  gcmdline.SetVerbose( ((BoolPortion*) param[0])->Value() );
  return new BoolPortion( gcmdline.Verbose() );
  // _gsm->SetVerbose( ((BoolPortion*) param[0])->Value() );
  // return new BoolPortion( _gsm->Verbose() );
}





Portion *GSM_Print(Portion **param)
{
  param[0]->Output(gout);
  gout << '\n';
  return param[0]->ValCopy();
}


Portion* GSM_Write_numerical(Portion** param)
{
  assert(param[1]->Spec().Type & (porINTEGER|porNUMBER));
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}


Portion* GSM_Write_gString(Portion** param)
{
  assert(param[1]->Spec().Type == porTEXT);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}


Portion* GSM_Write_Mixed(Portion** param)
{
  assert(param[1]->Spec().Type & porMIXED);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}


Portion* GSM_Write_Behav(Portion** param)
{
  assert(param[1]->Spec().Type & porBEHAV);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}


Portion* GSM_Write_NfSupport(Portion** param)
{
  assert(param[1]->Spec().Type & porNFSUPPORT);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}

Portion* GSM_Write_EfSupport(Portion** param)
{
  assert(param[1]->Spec().Type & porEFSUPPORT);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}

Portion* GSM_Write_Strategy(Portion** param)
{
  assert(param[1]->Spec().Type & porSTRATEGY);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}


Portion* GSM_Write_Nfg(Portion** param)
{
  assert(param[1]->Spec().Type & porNFG);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  Nfg *nfg = ((NfgPortion *) param[1])->Value();

  nfg->WriteNfgFile(s);

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}


Portion* GSM_Write_Efg(Portion** param)
{
  assert(param[1]->Spec().Type & porEFG);
  gOutput& s = ((OutputPortion*) param[0])->Value();

  Efg *efg = ((EfgPortion*) param[1])->Value();

  efg->WriteEfgFile(s);


  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}

Portion* GSM_Write_EfgRational(Portion** param)
{
  assert(param[1]->Spec().Type & porEFG);
  gOutput& s = ((OutputPortion*) param[0])->Value();

  Efg *efg = ((EfgPortion*) param[1])->Value();

  efg->WriteEfgFile(s);


  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}




Portion* GSM_Write_list(Portion** param)
{
  assert(param[1]->Spec().ListDepth > 0);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  // swap the first parameter with the return value, so things like
  //   Output["..."] << x << y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}



//------------------------------ Read --------------------------//





Portion* GSM_Read_Bool(Portion** param)
{
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();
  bool value = false;
  bool error = false;
  char c = ' ';

  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");
  }
  while(!input.eof() && isspace(c))
    input.get(c);
  if(c == 'T')
  {
    if(!input.eof()) input.get(c); if(c != 'r') error = true;
    if(!input.eof()) input.get(c); if(c != 'u') error = true;
    if(!input.eof()) input.get(c); if(c != 'e') error = true;
    value = true;
  }
  else if(c == 'F')
  {
    if(!input.eof()) input.get(c); if(c != 'a') error = true;
    if(!input.eof()) input.get(c); if(c != 'l') error = true;
    if(!input.eof()) input.get(c); if(c != 's') error = true;
    if(!input.eof()) input.get(c); if(c != 'e') error = true;
    value = false;
  }
  else
    error = true;

  if(error)
  {
    input.setpos(old_pos);
    return new ErrorPortion("No boolean data found");
  }

  ((BoolPortion*) param[1])->Value() = value;


  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}


Portion* GSM_Read_Integer(Portion** param)
{
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();
  int value;
  char c;

  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");
  }
  input >> value;
  if(!input.IsValid())
  {
    input.setpos(old_pos);
    return new ErrorPortion("File read error");
  }

  input.get(c);
  while(!input.eof() && isspace(c)) 
    input.get(c); 
  if(c == '/')
  {
    input.setpos(old_pos);
    return new ErrorPortion("Type mismatch: expected INTEGER, got RATIONAL");
  }
  else if(c == '.')
  {
    input.setpos(old_pos);
    return new ErrorPortion("Type mismatch: expected INTEGER, got FLOAT");
  }
  else
    input.unget(c);

  ((IntPortion*) param[1])->Value() = value;


  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}

Portion* GSM_Read_Number(Portion** param)
{
  gNumber value;
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");
  }
  input >> value;
  if(!input.IsValid())
  {
    input.setpos(old_pos);
    return new ErrorPortion("File read error");
  }

  ((NumberPortion*) param[1])->Value() = value;

  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}

Portion* GSM_Read_Text(Portion** param)
{
  char c = ' ';
  gString s;
  gString t;
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  while(!input.eof() && isspace(c))
    input.get(c);
  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");
  }
  if(!input.eof() && c == '\"')
    input.get(c); 
  else
  {
    input.unget(c);
    input.setpos(old_pos);
    return new ErrorPortion("File read error: missing starting \"");
  }

  while(!input.eof() && c != '\"')
  { t+=c; input.get(c); }
  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");  
  }
  
  ((TextPortion*) param[1])->Value() = t;
 

  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}



Portion* GSM_Read_List(Portion** param, PortionSpec spec,
		       Portion* (*func) (Portion**), bool ListFormat)
{
  Portion* p = NULL;
  Portion* sub_param[2];
  ListPortion* list = NULL;
  int i = 0;
  char c = ' ';  
  gInput& input = ((InputPortion*) param[0])->Value();
  list = ((ListPortion*) param[1]);
  long old_pos = input.getpos();


  while(!input.eof() && isspace(c))
    input.get(c);
  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");
  }
  if(!ListFormat)
  {
    if(c == '{')
      ListFormat = true;
    else
      input.unget(c);
  }
  else
  {
    if(c != '{')
    {
      input.setpos(old_pos);
      return new ErrorPortion("\'{\' expected");
    }
  }




  for(i=1; i <= list->Length(); i++)
  {
    assert((*list)[i]->Spec().Type==spec.Type);
    sub_param[0] = param[0];
    sub_param[1] = (*list)[i];

    if(i > 1) 
    {
      c = ' ';
      while(!input.eof() && isspace(c))
	input.get(c);
      if(c == ',')
      {
	if(!ListFormat)
	  input.unget(c);
      }
      else
	input.unget(c);      
    }

    if((*list)[i]->Spec() == spec)
      p = (*func)(sub_param);
    else
      p = GSM_Read_List(sub_param, spec, func, ListFormat);

    if(p->Spec().Type == porERROR)
    {
      input.setpos(old_pos);
      return p;
    }
    else
    {
      // okay, complicated things going on here
      // we want to delete the return value, but
      //   p is actually swapped with sub_param[0] in the
      //   GSM_Read() functions!  So, can't just delete p;
      //   need to swap p and sub_param[0] first.

      // just to make sure that the above description is still
      //   correct...
      assert( p == param[0] );
      assert( sub_param[0] != param[0] );
      
      // delete and swap
      delete sub_param[0];
      sub_param[0] = p;
      p = NULL;
    }
  }

  if(ListFormat)
  {
    c = ' ';
    while(!input.eof() && isspace(c))
      input.get(c);
    if(c != '}')
    {
      input.setpos(old_pos);
      return new ErrorPortion("Mismatched braces");
    }
    if(input.eof())
    {
      input.setpos(old_pos);
      return new ErrorPortion("End of file reached");
    }
  }


  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion* result = param[0];
  param[0] = result->RefCopy();
  return result;
}



Portion* GSM_Read_List_Bool(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  Portion* p = GSM_Read_List(param, porBOOL, GSM_Read_Bool, false);
  if(p->Spec().Type == porERROR)
    ((ListPortion*) param[1])->AssignFrom(temp);
  delete temp;
  return p;
}

Portion* GSM_Read_List_Integer(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  Portion* p = GSM_Read_List(param, porINTEGER, GSM_Read_Integer, false);
  if(p->Spec().Type == porERROR)
    ((ListPortion*) param[1])->AssignFrom(temp);
  delete temp;
  return p;
}

Portion* GSM_Read_List_Number(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  Portion* p = GSM_Read_List(param, porNUMBER, GSM_Read_Number, false);
  if(p->Spec().Type == porERROR)
    ((ListPortion*) param[1])->AssignFrom(temp);
  delete temp;
  return p;
}

Portion* GSM_Read_List_Text(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  Portion* p = GSM_Read_List(param, porTEXT, GSM_Read_Text, false);
  if(p->Spec().Type == porERROR)
    ((ListPortion*) param[1])->AssignFrom(temp);
  delete temp;
  return p;
}


//
// Don't know where this really belongs, but this seems like a good
// a place as any, at least for now.
//

Portion *GSM_Version(Portion **)
{
  return new NumberPortion(GCL_VERSION);
}


Portion* GSM_Help(Portion** param)
{
  return _gsm->Help(((TextPortion*) param[0])->Value(),
		    ((BoolPortion*) param[1])->Value(),
		    ((BoolPortion*) param[2])->Value() );
}

gString GetLine(gInput& f)
{
  char c = 0;
  gString result;
  while(f.IsValid())
  {
    f >> c;
    if(f.eof())
      break;
    if(c != '\n')
      result += c;
    else
      break;
  }
  return result;
}

Portion* GSM_Manual(Portion** param)
{
  gString txt = ((TextPortion*) param[0])->Value();
  gOutput& s = ((OutputPortion*) param[1])->Value();
  ListPortion* Prototypes = (ListPortion*) _gsm->Help(txt, true, true, true);
  int i;
  int body;

  for(i=1; i<=Prototypes->Length(); i++)
  {
	 assert(Prototypes->Spec().Type == porTEXT);
	 s << ((TextPortion*) (*Prototypes)[i])->Value() << '\n';
  }


//  gString name = "gcl.man";   This gives problems on BC (rdm)
  char * name = "gcl.man";    // this change necessary for BC
  gFileInput* f = NULL;





  // This section is very inelegantly adopted from gcompile.yy...
  // This section and its gcompile.yy parallel should be converted into
  // one function...

  extern char* _SourceDir;
  const char* SOURCE = _SourceDir;
  assert( SOURCE );

#ifdef __GNUG__
  const char SLASH1= '/';
  const char SLASH = '/';
#elif defined __BORLANDC__
  const char SLASH1= '\\';
  const char * SLASH = "\\";
#endif   // __GNUG__

  bool search = false;
  bool man_found = false;
  if( strchr( name, SLASH1 ) == NULL )
	 search = true;
  gString ManFileName;

  ManFileName = (gString) name;
  f = new gFileInput( ManFileName );
  if (!f->IsValid())
  {
	 delete f;
	 f = NULL;
  }
  else
  {  
    man_found = true;
  }

  if( search )
  {

    if( !man_found && (System::GetEnv( "HOME" ) != NULL) )
    {
      ManFileName = (gString) System::GetEnv( "HOME" ) + SLASH + name;
      f = new gFileInput( ManFileName );
      if (!f->IsValid())
      {
	delete f;
	f = NULL;
      }
      else
      {  
        man_found = true;
      }
    }

    if( !man_found && (System::GetEnv( "GCLLIB" ) != NULL) )
    {
      ManFileName = (gString) System::GetEnv( "GCLLIB" ) + SLASH + name;
      f = new gFileInput( ManFileName );
      if (!f->IsValid())
      {
	delete f;
	f = NULL;
      }
      else
      {
        man_found = true;
      }
    }

    if( !man_found && (SOURCE != NULL) )
    {
      ManFileName = (gString) SOURCE + SLASH + name;
      f = new gFileInput( ManFileName );
      if (!f->IsValid())
      {
        delete f;
	f = NULL;
      }
      else
      {  
        man_found = true;
      }
    }

  }

  // End bad section

  if (f == NULL)
    return new BoolPortion(false);


  gString line;
  gString line_out;
  bool found = false;
  while(f->IsValid() && !f->eof() && !found)
  {
    line = GetLine(*f);
    if(line.length() > txt.length())
      if( line.left(txt.length() + 1).dncase() == (txt + "[").dncase() )
	found = true;
  }
  if(found)
  {
    body = 0;
    while(f->IsValid() && !f->eof())
    {
      line = GetLine(*f);      
      if(line.length()>=3 && line.left(3) == "\\bd")
	body++;
      if(body > 0)
      {
	line_out = line;
	while(true)
	{
	  char* s;
	  int idx;
	  int numchars = 0;
	  if((s=strstr(line_out.stradr(), "\\bd")) != 0)
	    numchars = 3;
	  else if((s=strstr(line_out.stradr(), "\\ed")) != 0)
	    numchars = 3;
	  else if((s=strstr(line_out.stradr(), "\\item")) != 0)
	    numchars = 5;
	  else if((s=strstr(line_out.stradr(), "\\tt")) != 0)
	    numchars = 4;
	  else if((s=strstr(line_out.stradr(), "\\em")) != 0)
	    numchars = 4;
	  else if((s=strstr(line_out.stradr(), "$")) != 0)
	  {
	    idx = s - line_out.stradr();
	    line_out[idx] = '\'';
	    numchars = 0;
	  }
	  else if((s=strstr(line_out.stradr(), "\\verb")) != 0)
	  {
	    numchars = 5;
	    idx = s - line_out.stradr();
	    for(i=0; i<numchars; i++) 
	      line_out.remove(idx);
	    if(line_out.length()>idx)
	    {
	      char c;
	      c = line_out[idx];
	      line_out[idx] = '\"';
	      while(line_out.length()>idx)
	      {
		idx++;
		if(line_out[idx]==c)
		  break;
	      }
	      line_out[idx] = '\"';
	    }
	    numchars = 0;
	  }
	  else
	    break;
	  idx = s - line_out.stradr();
	  for(i=0; i<numchars; i++) 
	    line_out.remove(idx);
	  if(line_out.length()>idx && line_out[idx] == ' ')
	    line_out.remove(idx);
	}
	for(i=0; i<body; i++)
	  s << ' ';
	s << line_out << '\n';
      }
      if(line.length()>=3 && line.left(3) == "\\ed")
      {
	body--;
	if(body <= 0)
	  break;
      }
    }
  }

  /*
  if(!found)
  {
    s << '\n';
    s << "  " << "Manual not found\n";
    s << '\n';
  }
  */

  delete f;

  return new BoolPortion(found);
}

Portion* GSM_HelpVars(Portion** param)
{
  return _gsm->HelpVars(((TextPortion*) param[0])->Value());
}

Portion* GSM_Clear(Portion**)
{
  _gsm->Clear();
  return new BoolPortion(true);
}



Portion* GSM_GetEnv( Portion** param )
{
  if( ((TextPortion*) param[0])->Value().length() == 0 )
    return new ErrorPortion( "Invalid environment variable name" );

  return 
    new TextPortion( System::GetEnv( ((TextPortion*) param[0])->Value() ) );
}

Portion* GSM_SetEnv( Portion** param )
{
  if( ((TextPortion*) param[0])->Value().length() == 0 )
    return new ErrorPortion( "Invalid environment variable name" );

  int result = 0;
  result = System::SetEnv( ((TextPortion*) param[0])->Value(),
			   ((TextPortion*) param[1])->Value() );
  if( result == 0 )
    return new BoolPortion( true );
  else
    return new ErrorPortion( "Insufficient environment space" );
}

Portion* GSM_UnSetEnv( Portion** param )
{
  if( ((TextPortion*) param[0])->Value().length() == 0 )
    return new ErrorPortion( "Invalid environment variable name" );

  int result = 0;
  result = System::UnSetEnv( ((TextPortion*) param[0])->Value() );
  if( result == 0 )
    return new BoolPortion( true );
  else
    return new ErrorPortion( "Insufficient environment space" );
}

Portion* GSM_Shell( Portion** param )
{
  gString str = ((TextPortion*) param[0])->Value();
  bool spawn = ((BoolPortion*) param[1])->Value();

  int result = -1;
  if( !spawn )
  {
    if( str.length() > 0 )
      result = System::Shell( str );
    else
      result = System::Shell( 0 );
  }
  else
  {
    if( str.length() > 0 )
      result = System::Spawn( str );
    else
      result = System::Spawn( 0 );
  }

  return new IntPortion( result );
}


extern char* _ExePath;
Portion* GSM_ExePath( Portion** param)
{
#ifdef __GNUG__
  const char SLASH = '/';
#elif defined __BORLANDC__
  const char SLASH = '\\';
#endif   // __GNUG__
  bool file = ((BoolPortion*) param[0])->Value();
  bool path = ((BoolPortion*) param[1])->Value();

  assert( _ExePath );
  gString txt( _ExePath );

  if( file && path )
  {
  }
  else if( file )
  {
    if( txt.lastOccur( SLASH ) > 0 )
      txt = txt.right( txt.length() - txt.lastOccur( SLASH ) );
    else
      txt = "";
  }
  else if( path )
  {
    if( txt.lastOccur( SLASH ) > 0 )
      txt = txt.left( txt.lastOccur( SLASH ) );
  }
  if( !file && !path )
    txt = "";
  return new TextPortion( txt );
}



Portion* GSM_Platform( Portion** )
{
#ifdef __svr4__
  return new TextPortion( "SVR4" );
#elif defined sparc
  return new TextPortion( "SPARC" );
#elif defined sun
  return new TextPortion( "SUN" );
#elif defined _AIX32
  return new TextPortion( "AIX32" );
#elif defined _AIX
  return new TextPortion( "AIX" );
#elif defined hpux
  return new TextPortion( "HP UX" );
#elif defined hppa
  return new TextPortion( "HPPA" );
#elif defined __BORLANDC__
  return new TextPortion( "DOS/Windows" );
#else
  return new TextPortion( "Unknown" );
#endif
}


extern gStack<gString> GCL_InputFileNames;
Portion* GSM_GetPath( Portion** param )
{
#ifdef __GNUG__
  const char SLASH = '/';
#elif defined __BORLANDC__
  const char SLASH = '\\';
#endif   // __GNUG__
  bool file = ((BoolPortion*) param[0])->Value();
  bool path = ((BoolPortion*) param[1])->Value();
  if( GCL_InputFileNames.Depth() > 0 )
  {
    gString txt = GCL_InputFileNames.Peek();

    if( file && path )
    {
    }
    else if( file )
    {
      if( txt.lastOccur( SLASH ) > 0 )
	txt = txt.right( txt.length() - txt.lastOccur( SLASH ) );
      else
	txt = "";
    }
    else if( path )
    {
      if( txt.lastOccur( SLASH ) > 0 )
	txt = txt.left( txt.lastOccur( SLASH ) );
    }
    if( !file && !path )
      txt = "";
    
    return new TextPortion( txt );
  }
  else
    return new TextPortion( "" );
}



#include <sys/types.h>
#ifndef __BORLANDC__
#include <sys/time.h>
#endif
#include <time.h>

Portion* GSM_Date(Portion**)
{
  time_t now = time(0);
  gString AscTime = asctime(localtime(&now));
  return new TextPortion(AscTime.mid(11, 1) +
			    AscTime.mid(4, 21) + ", " + AscTime.mid(8, 12));
}


/*
Portion* GSM_CallFunction( Portion** param )
{
  _gsm->InitCallFunction( ( (TextPortion*) param[0] )->Value() );
  _gsm->Push( param[1]->ValCopy() );
  _gsm->BindVal();
  _gsm->Push( param[2]->ValCopy() );
  _gsm->BindVal();
  _gsm->CallFunction();
  return _gsm->PopValue();
}

*/






void Init_gsmoper(GSM* gsm)
{
  FuncDescObj* FuncObj;

  FuncObj = new FuncDescObj("Version", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Version, porNUMBER, 0));
  gsm->AddFunction(FuncObj);

  //---------------------- Assign ------------------------


  ParamInfoType xy_Int[] =
  {
    ParamInfoType("x", porINTEGER),
    ParamInfoType("y", porINTEGER)
  };

  ParamInfoType xy_Number[] =
  {
    ParamInfoType("x", porNUMBER),
    ParamInfoType("y", porNUMBER)
  };

  ParamInfoType xy_Text[] =
  {
    ParamInfoType("x", porTEXT),
    ParamInfoType("y", porTEXT)
  };

  ParamInfoType xy_Bool[] =
  {
    ParamInfoType("x", porBOOL),
    ParamInfoType("y", porBOOL)
  };


  ParamInfoType xy_NfSupport[] =
  {
    ParamInfoType("x", porNFSUPPORT),
    ParamInfoType("y", porNFSUPPORT)
  };

  ParamInfoType xy_Mixed[] =
  {
    ParamInfoType("x", porMIXED),
    ParamInfoType("y", porMIXED)
  };

 ParamInfoType xy_EfSupport[] =
  {
    ParamInfoType("x", porEFSUPPORT),
    ParamInfoType("y", porEFSUPPORT)
  };

  ParamInfoType xy_Behav[] =
  {
    ParamInfoType("x", porBEHAV),
    ParamInfoType("y", porBEHAV)
  };

  ParamInfoType xy_Efg[] =
  {
    ParamInfoType("x", porEFG),
    ParamInfoType("y", porEFG)
  };

  ParamInfoType xy_EfPlayer[] =
  {
    ParamInfoType("x", porEFPLAYER),
    ParamInfoType("y", porEFPLAYER)
  };

  ParamInfoType xy_NfPlayer[] =
  {
    ParamInfoType("x", porNFPLAYER),
    ParamInfoType("y", porNFPLAYER),
  };

  ParamInfoType xy_Node[] =
  {
    ParamInfoType("x", porNODE),
    ParamInfoType("y", porNODE)
  };

  ParamInfoType xy_Infoset[] =
  {
    ParamInfoType("x", porINFOSET),
    ParamInfoType("y", porINFOSET)
  };

  ParamInfoType xy_Outcome[] =
  {
    ParamInfoType("x", porEFOUTCOME),
    ParamInfoType("y", porEFOUTCOME)
  };

  ParamInfoType xy_Action[] =
  {
    ParamInfoType("x", porACTION),
    ParamInfoType("y", porACTION)
  };

  ParamInfoType xy_Strategy[] =
  {
    ParamInfoType("x", porSTRATEGY),
    ParamInfoType("y", porSTRATEGY)
  };

  ParamInfoType xy_Nfg[] =
  {
    ParamInfoType("x", porNFG),
    ParamInfoType("y", porNFG)
  };



  ParamInfoType x_Number[] =
  {
    ParamInfoType("x", porNUMBER)
  };


  ParamInfoType file_Text[] =
  {
    ParamInfoType("file", porTEXT)
  };


  //-------------------- Plus ----------------------------

  FuncObj = new FuncDescObj("Plus", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Add_int,
				       porINTEGER, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Add_Number,
				       porNUMBER, 2, xy_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Add_Mixed,
				       porMIXED, 2, xy_Mixed));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Add_Behav,
				       porBEHAV, 2, xy_Behav));
  gsm->AddFunction(FuncObj);

  //-------------------- Concat -------------------------
  
  FuncObj = new FuncDescObj("Concat", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Concat_gString, 
				       porTEXT, 2, xy_Text));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Concat_List, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("y", 
					    PortionSpec(porANYTYPE, NLIST)));
  gsm->AddFunction(FuncObj);

  //----------------------- Minus ------------------------

  FuncObj = new FuncDescObj("Minus", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Subtract_int,
				       porINTEGER, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Subtract_Number,
				       porNUMBER, 2, xy_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Subtract_Mixed,
				       porMIXED, 2, xy_Mixed));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Subtract_Behav,
				       porBEHAV, 2, xy_Behav));
  gsm->AddFunction(FuncObj);


  //----------------------- Times ----------------------------

  FuncObj = new FuncDescObj("Times", 4);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Multiply_int,
				       porINTEGER, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Multiply_Number,
				       porNUMBER, 2, xy_Number));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Multiply_Mixed,
				       porMIXED, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porNUMBER));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("y", porMIXED));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Multiply_Behav,
				       porBEHAV, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porNUMBER));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("y", porBEHAV));
  gsm->AddFunction(FuncObj);


  //----------------------- Divide -------------------------

  FuncObj = new FuncDescObj("Divide", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Divide_int,
				       porNUMBER, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Divide_Number,
				       porNUMBER, 2, xy_Number));
  gsm->AddFunction(FuncObj);  

  //----------------------- Power -------------------------

  FuncObj = new FuncDescObj("Power", 4);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Power_Int_Int, 
				       porINTEGER, 2, xy_Int));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Power_Number_Int,
				       porNUMBER, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNUMBER));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("y", porINTEGER));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Power_Int_Number,
				       porNUMBER, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("y", porNUMBER));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Power_Number_Number,
				       porNUMBER, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porNUMBER));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("y", porNUMBER));

  gsm->AddFunction(FuncObj);

  //----------------------- Exp, Log ---------------------

  FuncObj = new FuncDescObj("Exp", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Exp, porNUMBER, 1, x_Number));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Log", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Log, porNUMBER, 1, x_Number));
  gsm->AddFunction(FuncObj);


  //----------------------- Negate -----------------------

  FuncObj = new FuncDescObj("Negate", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Negate_int, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Negate_Number,
				       porNUMBER, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNUMBER));
  gsm->AddFunction(FuncObj);



  //----------------------- IntegerDivide ------------------------

  FuncObj = new FuncDescObj("IntegerDivide", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IntegerDivide_int, 
				       porINTEGER, 2, xy_Int));
  gsm->AddFunction(FuncObj);



  //----------------------- Modulus ------------------------

  FuncObj = new FuncDescObj("Modulus", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Modulus_int, 
				       porINTEGER, 2, xy_Int));
  gsm->AddFunction(FuncObj);


  //-------------------------- Equal ---------------------------

  FuncObj = new FuncDescObj("Equal", 17);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Equal_Number, porBOOL, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType( "x",
                              PortionSpec(porNUMBER, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(0, 1, ParamInfoType( "y",
                              PortionSpec(porNUMBER, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Equal_Integer, porBOOL, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType( "x",
                              PortionSpec(porINTEGER, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(1, 1, ParamInfoType( "y",
                              PortionSpec(porINTEGER, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Equal_Text, porBOOL, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType( "x",
                              PortionSpec(porTEXT, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(2, 1, ParamInfoType( "y",
                              PortionSpec(porTEXT, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Equal_Boolean, porBOOL, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType( "x",
                              PortionSpec(porBOOL, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(3, 1, ParamInfoType( "y",
                              PortionSpec(porBOOL, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Equal_Efg, porBOOL, 2));
  FuncObj->SetParamInfo(4, 0, ParamInfoType( "x",
                              PortionSpec(porEFG, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(4, 1, ParamInfoType( "y",
                              PortionSpec(porEFG, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Equal_EfPlayer, porBOOL, 2));
  FuncObj->SetParamInfo(5, 0, ParamInfoType( "x",
                              PortionSpec(porEFPLAYER, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(5, 1, ParamInfoType( "y",
                              PortionSpec(porEFPLAYER, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Equal_Node, porBOOL, 2));
  FuncObj->SetParamInfo(6, 0, ParamInfoType( "x",
                              PortionSpec(porNODE, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(6, 1, ParamInfoType( "y",
                              PortionSpec(porNODE, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Equal_Infoset, porBOOL, 2));
  FuncObj->SetParamInfo(7, 0, ParamInfoType( "x",
                              PortionSpec(porINFOSET, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(7, 1, ParamInfoType( "y",
                              PortionSpec(porINFOSET, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(8, FuncInfoType(GSM_Equal_Outcome, porBOOL, 2));
  FuncObj->SetParamInfo(8, 0, ParamInfoType( "x",
                              PortionSpec(porEFOUTCOME, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(8, 1, ParamInfoType( "y",
                              PortionSpec(porEFOUTCOME, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(9, FuncInfoType(GSM_Equal_Action, porBOOL, 2));
  FuncObj->SetParamInfo(9, 0, ParamInfoType( "x",
                              PortionSpec(porACTION, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(9, 1, ParamInfoType( "y",
                              PortionSpec(porACTION, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(10, FuncInfoType(GSM_Equal_EfSupport, porBOOL, 2));
  FuncObj->SetParamInfo(10, 0, ParamInfoType( "x",
                              PortionSpec(porEFSUPPORT, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(10, 1, ParamInfoType( "y",
                              PortionSpec(porEFSUPPORT, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(11, FuncInfoType(GSM_Equal_Behav, porBOOL, 2));
  FuncObj->SetParamInfo(11, 0, ParamInfoType( "x",
                              PortionSpec(porBEHAV, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(11, 1, ParamInfoType( "y",
                              PortionSpec(porBEHAV, 0, porNULLSPEC)) );

 FuncObj->SetFuncInfo(12, FuncInfoType(GSM_Equal_Nfg, porBOOL, 2));
  FuncObj->SetParamInfo(12, 0, ParamInfoType( "x",
                              PortionSpec(porNFG, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(12, 1, ParamInfoType( "y",
                              PortionSpec(porNFG, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(13, FuncInfoType(GSM_Equal_NfPlayer, porBOOL, 2));
  FuncObj->SetParamInfo(13, 0, ParamInfoType( "x",
                              PortionSpec(porNFPLAYER, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(13, 1, ParamInfoType( "y",
                              PortionSpec(porNFPLAYER, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(14, FuncInfoType(GSM_Equal_Strategy, porBOOL, 2));
  FuncObj->SetParamInfo(14, 0, ParamInfoType( "x",
                              PortionSpec(porSTRATEGY, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(14, 1, ParamInfoType( "y",
                              PortionSpec(porSTRATEGY, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(15, FuncInfoType(GSM_Equal_NfSupport, porBOOL, 2));
  FuncObj->SetParamInfo(15, 0, ParamInfoType( "x",
                              PortionSpec(porNFSUPPORT, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(15, 1, ParamInfoType( "y",
                              PortionSpec(porNFSUPPORT, 0, porNULLSPEC)) );

  FuncObj->SetFuncInfo(16, FuncInfoType(GSM_Equal_Mixed, porBOOL, 2));
  FuncObj->SetParamInfo(16, 0, ParamInfoType( "x",
                              PortionSpec(porMIXED, 0, porNULLSPEC)) );
  FuncObj->SetParamInfo(16, 1, ParamInfoType( "y",
                              PortionSpec(porMIXED, 0, porNULLSPEC)) );
  gsm->AddFunction(FuncObj);


  //-------------------------- NotEqual ---------------------------

  FuncObj = new FuncDescObj("NotEqual", 17);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NotEqual_Integer,
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NotEqual_Number,
				       porBOOL, 2, xy_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_NotEqual_Text,
				       porBOOL, 2, xy_Text));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_NotEqual_Boolean,
				       porBOOL, 2, xy_Bool));

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_NotEqual_Efg,
				       porBOOL, 2, xy_Efg));
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_NotEqual_EfPlayer,
				       porBOOL, 2, xy_EfPlayer));
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_NotEqual_Node,
				       porBOOL, 2, xy_Node));
  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_NotEqual_Infoset,
				       porBOOL, 2, xy_Infoset));
  FuncObj->SetFuncInfo(8, FuncInfoType(GSM_NotEqual_Outcome,
				       porBOOL, 2, xy_Outcome));
  FuncObj->SetFuncInfo(9, FuncInfoType(GSM_NotEqual_Action,
				       porBOOL, 2, xy_Action));
  FuncObj->SetFuncInfo(10, FuncInfoType(GSM_NotEqual_EfSupport,
					porBOOL, 2, xy_EfSupport));
  FuncObj->SetFuncInfo(11, FuncInfoType(GSM_NotEqual_Behav,
					porBOOL, 2, xy_Behav));

  FuncObj->SetFuncInfo(12, FuncInfoType(GSM_NotEqual_Nfg,
					porBOOL, 2, xy_Nfg));
  FuncObj->SetFuncInfo(13, FuncInfoType(GSM_NotEqual_NfPlayer,
					porBOOL, 2, xy_NfPlayer));
  FuncObj->SetFuncInfo(14, FuncInfoType(GSM_NotEqual_Strategy,
					porBOOL, 2, xy_Strategy));
  FuncObj->SetFuncInfo(15, FuncInfoType(GSM_NotEqual_NfSupport,
					porBOOL, 2, xy_NfSupport));
  FuncObj->SetFuncInfo(16, FuncInfoType(GSM_NotEqual_Mixed,
					porBOOL, 2, xy_Mixed));
  gsm->AddFunction(FuncObj);


  //-------------------------- Greater ---------------------------

  FuncObj = new FuncDescObj("Greater", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Greater_int,
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Greater_Number,
				       porBOOL, 2, xy_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Greater_Text,
				       porBOOL, 2, xy_Text));
  gsm->AddFunction(FuncObj);


  //-------------------------- Less ---------------------------

  FuncObj = new FuncDescObj("Less", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Less_int,
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Less_Number,
				       porBOOL, 2, xy_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Less_Text,
				       porBOOL, 2, xy_Text));
  gsm->AddFunction(FuncObj);


  //---------------------- GreaterThanOrEqualTo ---------------------------

  FuncObj = new FuncDescObj("GreaterEqual", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GreaterEqual_int,
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GreaterEqual_Number,
				       porBOOL, 2, xy_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_GreaterEqual_Text,
				       porBOOL, 2, xy_Text));
  gsm->AddFunction(FuncObj);


  //----------------------- LessThanOrEqualTo ---------------------------

  FuncObj = new FuncDescObj("LessEqual", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LessEqual_int,
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_LessEqual_Number,
				       porBOOL, 2, xy_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_LessEqual_Text,
				       porBOOL, 2, xy_Text));
  gsm->AddFunction(FuncObj);




  //--------------------- And -------------------------------

  FuncObj = new FuncDescObj("And", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_AND, porBOOL, 2, xy_Bool));
  gsm->AddFunction(FuncObj);


  //--------------------- Or -------------------------------

  FuncObj = new FuncDescObj("Or", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_OR, porBOOL, 2, xy_Bool));
  gsm->AddFunction(FuncObj);


  //--------------------- Not -------------------------------

  FuncObj = new FuncDescObj("Not", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NOT, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porBOOL));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Parentheses", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Parentheses, porANYTYPE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porANYTYPE));
  gsm->AddFunction(FuncObj);

  //-------------------- NewStream -------------------------

  FuncObj = new FuncDescObj("Output", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewOutputStream, 
				       porOUTPUT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("file", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("append", porBOOL,
					    new BoolPortion( false )));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Input", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewInputStream, 
				       porINPUT, 1, file_Text));
  gsm->AddFunction(FuncObj);

  //------------------- Formatted Output -------------------

  GSM_SetWriteOptions();

  FuncObj = new FuncDescObj("Print", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Print, porANYTYPE, 1, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porANYTYPE));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Print, PortionSpec(porANYTYPE, NLIST), 1, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porANYTYPE, NLIST)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Write", 10);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Write_numerical, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType
			("x", porBOOL | porINTEGER | porNUMBER));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Write_gString, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("x", porTEXT));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Write_Mixed, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("x", porMIXED));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Write_Behav,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("x", porBEHAV));
  
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Write_Nfg, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("x", porNFG,
					    REQUIRED ));
  
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Write_Efg,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("x", porEFG));

  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Write_list,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(6, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(6, 1, ParamInfoType
			("x", PortionSpec(porBOOL | porINTEGER | porNUMBER |
					  porTEXT | porMIXED | porBEHAV, 1)));

  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Write_NfSupport,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(7, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(7, 1, ParamInfoType("x", porNFSUPPORT));

  FuncObj->SetFuncInfo(8, FuncInfoType(GSM_Write_EfSupport,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(8, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(8, 1, ParamInfoType("x", porEFSUPPORT));

  FuncObj->SetFuncInfo(9, FuncInfoType(GSM_Write_Strategy,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(9, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(9, 1, ParamInfoType("x", porSTRATEGY));

  gsm->AddFunction(FuncObj);




  //---------------
  // *Format
  //---------------

  FuncObj = new FuncDescObj("ListFormat", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ListFormat, porBOOL, 4));
  FuncObj->SetParamInfo(0, 0, ParamInfoType
			("braces", porBOOL,
			 new BoolPortion(_WriteListBraces, true), BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType
			("commas", porBOOL,
			 new BoolPortion(_WriteListCommas, true), BYREF));
  FuncObj->SetParamInfo(0, 2, ParamInfoType
			("lf", porINTEGER,
			 new IntPortion(_WriteListLF, true), BYREF ));
  FuncObj->SetParamInfo(0, 3, ParamInfoType
			("indent", porINTEGER,
			 new IntPortion(_WriteListIndent, true), BYREF ));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Format", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IntFormat, porINTEGER, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porINTEGER) );
  FuncObj->SetParamInfo(0, 1, ParamInfoType
			("width", porINTEGER, 
			 new IntPortion(_WriteWidth, true), BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_FloatFormat, porNUMBER, 4));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNUMBER) );
  FuncObj->SetParamInfo(1, 1, ParamInfoType
			("width", porINTEGER, 
			 new IntPortion(_WriteWidth, true), BYREF));
  FuncObj->SetParamInfo(1, 2, ParamInfoType
			("precis", porINTEGER,
			 new IntPortion(_WritePrecis, true), BYREF));
  FuncObj->SetParamInfo(1, 3, ParamInfoType
			("expmode", porBOOL,
			 new BoolPortion(_WriteExpmode, true), BYREF));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_TextFormat, porTEXT, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porTEXT) );
  FuncObj->SetParamInfo(2, 1, ParamInfoType
			("quote", porBOOL,
			 new BoolPortion(_WriteQuoted, true), BYREF));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_SolutionFormat, 
				       porBEHAV | porMIXED, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porBEHAV | porMIXED) );
  FuncObj->SetParamInfo(3, 1, ParamInfoType
			("info", porINTEGER,
			 new IntPortion(_WriteSolutionInfo, true), BYREF));
  gsm->AddFunction(FuncObj);



  // ------------------ SetVerbose --------------------


  FuncObj = new FuncDescObj("SetVerbose", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetVerbose, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porBOOL) );
  gsm->AddFunction(FuncObj);


  //-------------------- Read --------------------------

  FuncObj = new FuncDescObj("Read", 8);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Read_Bool, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("input", porINPUT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("x", porBOOL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Read_List_Bool, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("x", PortionSpec(porBOOL,1), 
					    REQUIRED, BYREF));


  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Read_Integer, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("x", porINTEGER, 
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Read_List_Integer, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("x", PortionSpec(porINTEGER,1), 
					    REQUIRED, BYREF));
  

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Read_Number,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("x", porNUMBER,
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Read_List_Number,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("x", PortionSpec(porNUMBER,1),
					    REQUIRED, BYREF));
  
  
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Read_Text,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(6, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(6, 1, ParamInfoType("x", porTEXT,
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Read_List_Text,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(7, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(7, 1, ParamInfoType("x", PortionSpec(porTEXT,1),
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);

  

  //---------------- faked functions -----------------//

  FuncObj = new FuncDescObj("Help", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Help, PortionSpec(porTEXT, 1), 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("udf", porBOOL,
					    new BoolPortion( true )));
  FuncObj->SetParamInfo(0, 2, ParamInfoType("bif", porBOOL, 
					    new BoolPortion( true )));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Manual", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Manual, porBOOL, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("y", porOUTPUT,
					    new OutputPortion(gout)));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("HelpVars", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_HelpVars, PortionSpec(porTEXT, 1),
				       1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Clear", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Clear, porBOOL, 0));
  gsm->AddFunction(FuncObj);






/*
  FuncObj = new FuncDescObj("Assign", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Assign, porANYTYPE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("name", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("value", porANYTYPE, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("UnAssign", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_UnAssign, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("name", porTEXT));
  gsm->AddFunction(FuncObj);

  */

  FuncObj = new FuncDescObj("Date", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Date, porTEXT, 0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Randomize", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Randomize_Integer, porINTEGER, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("seed", porINTEGER,
					    new IntPortion(0)));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Randomize_Number, porNUMBER, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porNUMBER));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("seed", porINTEGER,
					    new IntPortion(0)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("IsNull", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsNull, porBOOL, 1, 0, 
				       funcLISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", 
					    PortionSpec(porANYTYPE, 0, 
							porNULLSPEC )));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Null", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Null, 
				       PortionSpec(porANYTYPE, 0, 
						   porNULLSPEC ),
				       1, 0, funcLISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porANYTYPE));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Shell", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Shell, porINTEGER, 2 ));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("command", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("spawn", porBOOL, 
					    new BoolPortion(true)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GetEnv", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GetEnv, porTEXT, 1 ));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("name", porTEXT ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetEnv", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetEnv, porBOOL, 2 ));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("name", porTEXT ) );
  FuncObj->SetParamInfo(0, 1, ParamInfoType("value", porTEXT ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("UnSetEnv", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_UnSetEnv, porBOOL, 1 ));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("name", porTEXT ) );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ExePath", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ExePath, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("file", porBOOL,
					    new BoolPortion( true ) ) );
  FuncObj->SetParamInfo(0, 1, ParamInfoType("path", porBOOL,
					    new BoolPortion( true ) ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Platform", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Platform, porTEXT, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GetPath", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GetPath, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("file", porBOOL,
					    new BoolPortion( true ) ) );
  FuncObj->SetParamInfo(0, 1, ParamInfoType("path", porBOOL,
					    new BoolPortion( true ) ) );
  gsm->AddFunction(FuncObj);

/*
  // ------------------- CallFunction -----------------------

  FuncObj = new FuncDescObj("CallFunction", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_CallFunction, porSTRATEGY, 3));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("func", porTEXT ) );
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porNFG ) );
  FuncObj->SetParamInfo(0, 2, ParamInfoType("h", 
					    PortionSpec( porSTRATEGY, 2 )));
  gsm->AddFunction(FuncObj);
  */


}







