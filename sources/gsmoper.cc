//
// FILE: gsmoper.cc -- implementations for GSM operator functions
//                     companion to GSM
//
// @(#)gsmoper.cc	1.89 8/19/96
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

extern GSM* _gsm;




//---------------------------------------
//           IsNull
//---------------------------------------

Portion* GSM_IsNull(Portion** param)
{
  return new BoolValPortion(param[0]->Spec().Type == porNULL);
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
  return new IntValPortion(v);
}

Portion* GSM_Randomize_Float(Portion** param)
{
  long _RandomSeed = ((IntPortion*) param[1])->Value();
  if(_RandomSeed > 0)
    _RandomSeed = -_RandomSeed;
  double v;
  if(_RandomSeed != 0)
    _idum = _RandomSeed;
  v = (double) ran1(&_idum) / IM;
  return new FloatValPortion(v);
}

Portion* GSM_Randomize_Rational(Portion** param)
{
  long _RandomSeed = ((IntPortion*) param[1])->Value();
  if(_RandomSeed > 0)
    _RandomSeed = -_RandomSeed;
  gRational v;
  if(_RandomSeed != 0)
    _idum = _RandomSeed;
  v = (gRational) ran1(&_idum) / (gRational) IM; 
  return new RationalValPortion(v);
}



//-------------------------------------------
//              Assign
//-------------------------------------------


Portion* GSM_Assign(Portion** param)
{
  _gsm->PushRef(((TextPortion*) param[0])->Value());
  Portion* p = param[1]->ValCopy();
  _gsm->Push(p);
  _gsm->Assign();
  return p->RefCopy();
}

Portion* GSM_UnAssign(Portion** param)
{
  _gsm->PushRef(((TextPortion*) param[0])->Value());
  return _gsm->UnAssignExt();
}

Portion* GSM_Fake(Portion**)
{
  assert(0);
  return 0;
}

//-------------------------------------------------------------------
//                      mathematical operators
//------------------------------------------------------------------


//---------------------- GSM_Add ---------------------------

Portion* GSM_Add_double(Portion** param)
{
  Portion* result = 0;
  result = new FloatValPortion
    (
     ((FloatPortion*) param[0])->Value() +
     ((FloatPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Add_int(Portion** param)
{
  Portion* result = 0;
  result = new IntValPortion
    (
     ((IntPortion*) param[0])->Value() +
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Add_gRational(Portion** param)
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     ((RationalPortion*) param[0])->Value() +
     ((RationalPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Add_MixedFloat(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[0])->Value() == 0 ||
     ((MixedPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");

  result = param[0]->ValCopy();
  (* (MixedSolution<double>*) ((MixedPortion*) result)->Value()) +=
    (* (MixedSolution<double>*) ((MixedPortion*) param[1])->Value());
  return result;
}

Portion* GSM_Add_MixedRational(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[0])->Value() == 0 ||
     ((MixedPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");

  result = param[0]->ValCopy();
  (* (MixedSolution<gRational>*) ((MixedPortion*) result)->Value()) +=
    (* (MixedSolution<gRational>*) ((MixedPortion*) param[1])->Value());
  return result;
}


Portion* GSM_Add_BehavFloat(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[0])->Value() == 0 ||
     ((BehavPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Behav value");

  result = param[0]->ValCopy();
  (* (BehavSolution<double>*) ((BehavPortion*) result)->Value()) +=
    (* (BehavSolution<double>*) ((BehavPortion*) param[1])->Value());
  return result;
}

Portion* GSM_Add_BehavRational(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[0])->Value() == 0 ||
     ((BehavPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");

  result = param[0]->ValCopy();
  (* (BehavSolution<gRational>*) ((BehavPortion*) result)->Value()) +=
    (* (BehavSolution<gRational>*) ((BehavPortion*) param[1])->Value());
  return result;
}


//--------------------- Concat ---------------------------

Portion* GSM_Concat_gString(Portion** param)
{
  Portion* result = 0;
  result = new TextValPortion
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
  result = new ListValPortion(((ListPortion*) param[0])->Value());
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

Portion* GSM_Subtract_double(Portion** param)
{
  Portion* result = 0;
  result = new FloatValPortion
    (
     ((FloatPortion*) param[0])->Value() -
     ((FloatPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Subtract_int(Portion** param)
{
  Portion* result = 0;
  result = new IntValPortion
    (
     ((IntPortion*) param[0])->Value() -
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Subtract_gRational(Portion** param)
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     ((RationalPortion*) param[0])->Value() -
     ((RationalPortion*) param[1])->Value()
    );
  return result;
}


Portion* GSM_Subtract_MixedFloat(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[0])->Value() == 0 ||
     ((MixedPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");

  result = param[0]->ValCopy();
  (* (MixedSolution<double>*) ((MixedPortion*) result)->Value()) -=
    (* (MixedSolution<double>*) ((MixedPortion*) param[1])->Value());
  return result;
}

Portion* GSM_Subtract_MixedRational(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[0])->Value() == 0 ||
     ((MixedPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");

  result = param[0]->ValCopy();
  (* (MixedSolution<gRational>*) ((MixedPortion*) result)->Value()) -=
    (* (MixedSolution<gRational>*) ((MixedPortion*) param[1])->Value());
  return result;
}


Portion* GSM_Subtract_BehavFloat(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[0])->Value() == 0 ||
     ((BehavPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Behav value");

  result = param[0]->ValCopy();
  (* (BehavSolution<double>*) ((BehavPortion*) result)->Value()) -=
    (* (BehavSolution<double>*) ((BehavPortion*) param[1])->Value());
  return result;
}

Portion* GSM_Subtract_BehavRational(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[0])->Value() == 0 ||
     ((BehavPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");

  result = param[0]->ValCopy();
  (* (BehavSolution<gRational>*) ((BehavPortion*) result)->Value()) -=
    (* (BehavSolution<gRational>*) ((BehavPortion*) param[1])->Value());
  return result;
}



//------------------------- GSM_Multiply ---------------------------

Portion* GSM_Multiply_double(Portion** param)
{
  Portion* result = 0;
  result = new FloatValPortion
    (
     ((FloatPortion*) param[0])->Value() *
     ((FloatPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Multiply_int(Portion** param)
{
  Portion* result = 0;
  result = new IntValPortion
    (
     ((IntPortion*) param[0])->Value() *
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_Multiply_gRational(Portion** param)
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     ((RationalPortion*) param[0])->Value() *
     ((RationalPortion*) param[1])->Value()
    );
  return result;
}


Portion* GSM_Multiply_MixedFloat1(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");
  
  result = param[1]->ValCopy();
  (* (MixedSolution<double>*) ((MixedPortion*) result)->Value()) *=
    ((FloatPortion*) param[0])->Value();
  return result;
}

Portion* GSM_Multiply_MixedFloat2(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[0])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");
  
  result = param[0]->ValCopy();
  (* (MixedSolution<double>*) ((MixedPortion*) result)->Value()) *=
    ((FloatPortion*) param[1])->Value();
  return result;
}


Portion* GSM_Multiply_MixedRational1(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");
  
  result = param[1]->ValCopy();
  (* (MixedSolution<gRational>*) ((MixedPortion*) result)->Value()) *=
    ((RationalPortion*) param[0])->Value();
  return result;
}

Portion* GSM_Multiply_MixedRational2(Portion** param)
{
  Portion* result = 0;
  if(((MixedPortion*) param[0])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Mixed value");
  
  result = param[0]->ValCopy();
  (* (MixedSolution<gRational>*) ((MixedPortion*) result)->Value()) *=
    ((RationalPortion*) param[1])->Value();
  return result;
}


Portion* GSM_Multiply_BehavFloat1(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Behav value");
  
  result = param[1]->ValCopy();
  (* (BehavSolution<double>*) ((BehavPortion*) result)->Value()) *=
    ((FloatPortion*) param[0])->Value();
  return result;
}

Portion* GSM_Multiply_BehavFloat2(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[0])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Behav value");
  
  result = param[0]->ValCopy();
  (* (BehavSolution<double>*) ((BehavPortion*) result)->Value()) *=
    ((FloatPortion*) param[1])->Value();
  return result;
}


Portion* GSM_Multiply_BehavRational1(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[1])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Behav value");
  
  result = param[1]->ValCopy();
  (* (BehavSolution<gRational>*) ((BehavPortion*) result)->Value()) *=
    ((RationalPortion*) param[0])->Value();
  return result;
}

Portion* GSM_Multiply_BehavRational2(Portion** param)
{
  Portion* result = 0;
  if(((BehavPortion*) param[0])->Value() == 0)
    return new ErrorPortion("Cannot operate on a null Behav value");
  
  result = param[0]->ValCopy();
  (* (BehavSolution<gRational>*) ((BehavPortion*) result)->Value()) *=
    ((RationalPortion*) param[1])->Value();
  return result;
}



//---------------------------- GSM_Divide -------------------------------

Portion* GSM_Divide_double(Portion** param)
{
  Portion* result = 0;
  if(((FloatPortion*) param[1])->Value() != 0)
  {
    result = new FloatValPortion
      (
       ((FloatPortion*) param[0])->Value() /
       ((FloatPortion*) param[1])->Value()
      );
  }
  else
  {
    result = new NullPortion(porFLOAT);
    //result = new ErrorPortion("Division by zero");
  }
  return result;
}


Portion* GSM_Divide_int(Portion** param)
{
  Portion* result = 0;
  if(((IntPortion*) param[1])->Value() != 0)
  {
    result = new RationalValPortion(((IntPortion*) param[0])->Value());
    ((RationalPortion*) result)->Value() /= 
      ((IntPortion*) param[1])->Value();
  }
  else
  {
    result = new NullPortion(porRATIONAL);
    //result = new ErrorPortion("Division by zero");
  }
  return result;
}



Portion* GSM_Divide_gRational(Portion** param)
{
  Portion* result = 0;
  if(((RationalPortion*) param[1])->Value() != (gRational)0)
  {
    result = new RationalValPortion
      (
       ((RationalPortion*) param[0])->Value() /
       ((RationalPortion*) param[1])->Value()
      );
  }
  else
  {
    result = new NullPortion(porRATIONAL);
    //result = new ErrorPortion("Division by zero");
  }
  return result;
}



//-------------------------- GSM_IntegerDivide -------------------------------

Portion* GSM_IntegerDivide_int(Portion** param)
{
  Portion* result = 0;
  if(((IntPortion*) param[1])->Value() != 0)
  {
    result = new IntValPortion
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
  return new FloatValPortion
    (exp(((FloatPortion *) param[0])->Value()));
}

Portion *GSM_Log(Portion **param)
{
  double d = ((FloatPortion *) param[0])->Value();
  if(d <= 0.0)
    return new NullPortion(porFLOAT);
    //return new ErrorPortion("Argument must be greater than zero");
  else
    return new FloatValPortion(log(d));
}


//------------------------ GSM_Power ------------------------

Portion* GSM_Power_Int_Int(Portion** param)
{
  long base = ((IntPortion*) param[0])->Value();
  long exponent = ((IntPortion*) param[1])->Value();
  return new IntValPortion((long) pow((double) base, (double) exponent));
}

Portion* GSM_Power_Float_Int(Portion** param)
{
  double base = ((FloatPortion*) param[0])->Value();
  long exponent = ((IntPortion*) param[1])->Value();
  return new FloatValPortion(pow(base, (double) exponent));
}

Portion* GSM_Power_Rational_Int(Portion** param)
{
  gRational base = ((RationalPortion*) param[0])->Value();
  long exponent = ((IntPortion*) param[1])->Value();
  return new RationalValPortion(pow(base, exponent));
}


Portion* GSM_Power_Int_Float(Portion** param)
{
  long base = ((IntPortion*) param[0])->Value();
  double exponent = ((FloatPortion*) param[1])->Value();
  return new FloatValPortion((double) pow((double) base, exponent));
}

Portion* GSM_Power_Float_Float(Portion** param)
{
  double base = ((FloatPortion*) param[0])->Value();
  double exponent = ((FloatPortion*) param[1])->Value();
  return new FloatValPortion((double) pow(base, exponent));
}

Portion* GSM_Power_Rational_Float(Portion** param)
{
  gRational base = ((RationalPortion*) param[0])->Value();
  double exponent = ((FloatPortion*) param[1])->Value();
  return new FloatValPortion((double) pow(base, ((long) exponent)));
}




//------------------------ GSM_Negate ------------------------

Portion* GSM_Negate_double(Portion** param)
{
  Portion* result = 0;
  result = new FloatValPortion
    (
     -((FloatPortion*) param[0])->Value()
    );
  return result;
}

Portion* GSM_Negate_int(Portion** param)
{
  Portion* result = 0;
  result = new IntValPortion
    (
     -((IntPortion*) param[0])->Value()
    );
  return result;
}

Portion* GSM_Negate_gRational(Portion** param)
{
  Portion* result = 0;
  result = new RationalValPortion
    (
     -((RationalPortion*) param[0])->Value()
    );
  return result;
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
    result = new IntValPortion
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

Portion* GSM_Equal_Float(Portion** param)
{
  return new BoolValPortion(((FloatPortion *) param[0])->Value() ==
			    ((FloatPortion *) param[1])->Value());
}

Portion* GSM_Equal_Integer(Portion** param)
{
  return new BoolValPortion(((IntPortion *) param[0])->Value() ==
			    ((IntPortion *) param[1])->Value());
}

Portion* GSM_Equal_Rational(Portion** param)
{
  return new BoolValPortion(((RationalPortion *) param[0])->Value() ==
			    ((RationalPortion *) param[1])->Value());
}

Portion* GSM_Equal_Text(Portion** param)
{
  return new BoolValPortion(((TextPortion *) param[0])->Value() ==
			    ((TextPortion *) param[1])->Value());
}

Portion* GSM_Equal_Boolean(Portion** param)
{
  return new BoolValPortion(((BoolPortion *) param[0])->Value() ==
			    ((BoolPortion *) param[1])->Value());
}

Portion* GSM_Equal_EfPlayer(Portion** param)
{
  return new BoolValPortion(((EfPlayerPortion *) param[0])->Value() ==
			    ((EfPlayerPortion *) param[1])->Value());
}

Portion* GSM_Equal_Node(Portion** param)
{
  return new BoolValPortion(((NodePortion *) param[0])->Value() ==
			    ((NodePortion *) param[1])->Value());
}

Portion* GSM_Equal_Infoset(Portion** param)
{
  return new BoolValPortion(((InfosetPortion *) param[0])->Value() ==
			    ((InfosetPortion *) param[1])->Value());
}

Portion* GSM_Equal_Outcome(Portion** param)
{
  return new BoolValPortion(((OutcomePortion *) param[0])->Value() ==
			    ((OutcomePortion *) param[1])->Value());
}

Portion* GSM_Equal_Action(Portion** param)
{
  return new BoolValPortion(((ActionPortion *) param[0])->Value() ==
			    ((ActionPortion *) param[1])->Value());
}

Portion* GSM_Equal_EfSupport(Portion** param)
{
  return new BoolValPortion((*((EfSupportPortion *) param[0])->Value()) ==
			    (*((EfSupportPortion *) param[1])->Value()));
}

Portion* GSM_Equal_BehavFloat(Portion** param)
{
  return new BoolValPortion
    ((*(BehavSolution<double> *) ((BehavPortion *) param[0])->Value()) ==
     (*(BehavSolution<double> *) ((BehavPortion *) param[1])->Value()));
}

Portion* GSM_Equal_BehavRational(Portion** param)
{
  return new BoolValPortion
    ((*(BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value()) ==
     (*(BehavSolution<gRational> *) ((BehavPortion *) param[1])->Value()));
}

Portion* GSM_Equal_NfPlayer(Portion** param)
{
  return new BoolValPortion(((NfPlayerPortion *) param[0])->Value() ==
			    ((NfPlayerPortion *) param[1])->Value());
}

Portion* GSM_Equal_Strategy(Portion** param)
{
  return new BoolValPortion(((StrategyPortion *) param[0])->Value() ==
			    ((StrategyPortion *) param[1])->Value());
}

Portion* GSM_Equal_NfSupport(Portion** param)
{
  return new BoolValPortion((*((NfSupportPortion *) param[0])->Value()) ==
			    (*((NfSupportPortion *) param[1])->Value()));
}

Portion* GSM_Equal_MixedFloat(Portion** param)
{
  return new BoolValPortion
    ((*(MixedSolution<double> *) ((MixedPortion *) param[0])->Value()) ==
     (*(MixedSolution<double> *) ((MixedPortion *) param[1])->Value()));
}

Portion* GSM_Equal_MixedRational(Portion** param) 
{
  return new BoolValPortion
    ((*(MixedSolution<gRational> *) ((MixedPortion *) param[0])->Value()) ==
     (*(MixedSolution<gRational> *) ((MixedPortion *) param[1])->Value()));
}

//------------
// NotEqual
//------------

Portion* GSM_NotEqual_Float(Portion** param)
{
  return new BoolValPortion(((FloatPortion *) param[0])->Value() !=
			    ((FloatPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Integer(Portion** param)
{
  return new BoolValPortion(((IntPortion *) param[0])->Value() !=
			    ((IntPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Rational(Portion** param)
{
  return new BoolValPortion(((RationalPortion *) param[0])->Value() !=
			    ((RationalPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Text(Portion** param)
{
  return new BoolValPortion(((TextPortion *) param[0])->Value() !=
			    ((TextPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Boolean(Portion** param)
{
  return new BoolValPortion(((BoolPortion *) param[0])->Value() !=
			    ((BoolPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_EfPlayer(Portion** param)
{
  return new BoolValPortion(((EfPlayerPortion *) param[0])->Value() !=
			    ((EfPlayerPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Node(Portion** param)
{
  return new BoolValPortion(((NodePortion *) param[0])->Value() !=
			    ((NodePortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Infoset(Portion** param)
{
  return new BoolValPortion(((InfosetPortion *) param[0])->Value() !=
			    ((InfosetPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Outcome(Portion** param)
{
  return new BoolValPortion(((OutcomePortion *) param[0])->Value() !=
			    ((OutcomePortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Action(Portion** param)
{
  return new BoolValPortion(((ActionPortion *) param[0])->Value() !=
			    ((ActionPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_EfSupport(Portion** param)
{
  return new BoolValPortion((*((EfSupportPortion *) param[0])->Value()) !=
			    (*((EfSupportPortion *) param[1])->Value()));
}

Portion* GSM_NotEqual_BehavFloat(Portion** param)
{
  return new BoolValPortion
    ((*(BehavSolution<double> *) ((BehavPortion *) param[0])->Value()) !=
     (*(BehavSolution<double> *) ((BehavPortion *) param[1])->Value()));
}

Portion* GSM_NotEqual_BehavRational(Portion** param)
{
  return new BoolValPortion
    ((*(BehavSolution<gRational> *) ((BehavPortion *) param[0])->Value()) !=
     (*(BehavSolution<gRational> *) ((BehavPortion *) param[1])->Value()));
}

Portion* GSM_NotEqual_NfPlayer(Portion** param)
{
  return new BoolValPortion(((NfPlayerPortion *) param[0])->Value() !=
			    ((NfPlayerPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_Strategy(Portion** param)
{
  return new BoolValPortion(((StrategyPortion *) param[0])->Value() !=
			    ((StrategyPortion *) param[1])->Value());
}

Portion* GSM_NotEqual_NfSupport(Portion** param)
{
  return new BoolValPortion((*((NfSupportPortion *) param[0])->Value()) !=
			    (*((NfSupportPortion *) param[1])->Value()));
}

Portion* GSM_NotEqual_MixedFloat(Portion** param)
{
  return new BoolValPortion
    ((*(MixedSolution<double> *) ((MixedPortion *) param[0])->Value()) !=
     (*(MixedSolution<double> *) ((MixedPortion *) param[1])->Value()));
}

Portion* GSM_NotEqual_MixedRational(Portion** param) 
{
  return new BoolValPortion
    ((*(MixedSolution<gRational> *) ((MixedPortion *) param[0])->Value()) !=
     (*(MixedSolution<gRational> *) ((MixedPortion *) param[1])->Value()));
}





//-------------------------- GSM_GreaterThan -------------------------

Portion* GSM_GreaterThan_double(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((FloatPortion*) param[0])->Value() >
     ((FloatPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_GreaterThan_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((IntPortion*) param[0])->Value() >
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_GreaterThan_gRational(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((RationalPortion*) param[0])->Value() >
     ((RationalPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_GreaterThan_gString(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((TextPortion*) param[0])->Value() >
     ((TextPortion*) param[1])->Value()
    );
  return result;
}


//----------------------- GSM_LessThan -----------------------

Portion* GSM_LessThan_double(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((FloatPortion*) param[0])->Value() <
     ((FloatPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_LessThan_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((IntPortion*) param[0])->Value() <
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_LessThan_gRational(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((RationalPortion*) param[0])->Value() <
     ((RationalPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_LessThan_gString(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((TextPortion*) param[0])->Value() <
     ((TextPortion*) param[1])->Value()
    );
  return result;
}


//--------------------- GSM_GreaterThanOrEqualTo --------------------

Portion* GSM_GreaterThanOrEqualTo_double(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((FloatPortion*) param[0])->Value() >=
     ((FloatPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((IntPortion*) param[0])->Value() >=
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_gRational(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((RationalPortion*) param[0])->Value() >=
     ((RationalPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_GreaterThanOrEqualTo_gString(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((TextPortion*) param[0])->Value() >=
     ((TextPortion*) param[1])->Value()
    );
  return result;
}


//--------------------- GSM_LessThanOrEqualTo ---------------------

Portion* GSM_LessThanOrEqualTo_double(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((FloatPortion*) param[0])->Value() <=
     ((FloatPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_LessThanOrEqualTo_int(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((IntPortion*) param[0])->Value() <=
     ((IntPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_LessThanOrEqualTo_gRational(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
    (
     ((RationalPortion*) param[0])->Value() <=
     ((RationalPortion*) param[1])->Value()
    );
  return result;
}

Portion* GSM_LessThanOrEqualTo_gString(Portion** param)
{
  Portion* result = 0;
  result = new BoolValPortion
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
  
  result = new BoolValPortion
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
  
  result = new BoolValPortion
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
  
  result = new BoolValPortion(!((BoolPortion*) param[0])->Value());

  return result;
}


Portion *GSM_Paren(Portion **param)
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
  
  assert(param[0]->Spec().Type == porTEXT);

  g = new gFileOutput(((TextPortion*) param[0])->Value());
  result = new OutputValPortion(*g);
  
  return result;
}


Portion* GSM_NewInputStream(Portion** param)
{
  Portion* result = 0;
  gInput* g;
  
  assert(param[0]->Spec().Type == porTEXT);

  g = new gFileInput(((TextPortion*) param[0])->Value());

  if(g->IsValid())
    result = new InputValPortion(*g);
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
}


Portion* GSM_SetFormat(Portion** param)
{
  _WriteWidth = ((IntPortion*) param[0])->Value();
  _WritePrecis = ((IntPortion*) param[1])->Value();
  _WriteExpmode = ((BoolPortion*) param[2])->Value();
  _WriteQuoted = ((BoolPortion*) param[3])->Value();
  _WriteListBraces = ((BoolPortion*) param[4])->Value();
  _WriteListCommas = ((BoolPortion*) param[5])->Value();
  _WriteListLF = ((IntPortion*) param[6])->Value();
  _WriteListIndent = ((IntPortion*) param[7])->Value();
  _WriteSolutionInfo = ((IntPortion*) param[8])->Value();

  GSM_SetWriteOptions();

  return new BoolValPortion(true);
}





Portion* GSM_Write_numerical(Portion** param)
{
  assert(param[1]->Spec().Type & (porINTEGER|porFLOAT|porRATIONAL));
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}


Portion* GSM_Write_gString(Portion** param)
{
  assert(param[1]->Spec().Type == porTEXT);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}


Portion* GSM_Write_Mixed(Portion** param)
{
  assert(param[1]->Spec().Type & porMIXED);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}


Portion* GSM_Write_Behav(Portion** param)
{
  assert(param[1]->Spec().Type & porBEHAV);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}


Portion* GSM_Write_NfSupport(Portion** param)
{
  assert(param[1]->Spec().Type & porNF_SUPPORT);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}

Portion* GSM_Write_EfSupport(Portion** param)
{
  assert(param[1]->Spec().Type & porEF_SUPPORT);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}

Portion* GSM_Write_Strategy(Portion** param)
{
  assert(param[1]->Spec().Type & porSTRATEGY);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}


Portion* GSM_Write_Nfg(Portion** param)
{
  assert(param[1]->Spec().Type & porNFG);
  gOutput& s = ((OutputPortion*) param[0])->Value();

  BaseNfg* nfg = ((NfgPortion*) param[1])->Value();
  // nfg->DisplayNfg(s);

  s << "Temporary hack; still waiting for DisplayNfg() to finish\n";
  nfg->WriteNfgFile(s);

  return param[0]->RefCopy();
}


Portion* GSM_Write_Efg(Portion** param)
{
  assert(param[1]->Spec().Type & porEFG);
  gOutput& s = ((OutputPortion*) param[0])->Value();

  BaseEfg* efg = ((EfgPortion*) param[1])->Value();
  // efg->DisplayEfg(s);

  s << "Temporary hack; still waiting for DisplayEfg() to finish\n";
  efg->WriteEfgFile(s);

  return param[0]->RefCopy();
}




Portion* GSM_Write_list(Portion** param)
{
  assert(param[1]->Spec().ListDepth > 0);
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->RefCopy();
}



//------------------------------ Read --------------------------//





Portion* GSM_Read_Bool(Portion** param)
{
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();
  bool value;
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
  return param[0]->RefCopy();
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

  return param[0]->RefCopy();
}

Portion* GSM_Read_Float(Portion** param)
{
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();
  double value;
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
    return new ErrorPortion("Type mismatch: expected FLOAT, got RATIONAL");
  }
  else
    input.unget(c);

  ((FloatPortion*) param[1])->Value() = value;

  return param[0]->RefCopy();
}

Portion* GSM_Read_Rational(Portion** param)
{
  int numerator = 0;
  int denominator = 0;
  char c = ' ';
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");
  }
  input >> numerator;
  if(!input.IsValid())
  {
    input.setpos(old_pos);
    return new ErrorPortion("File read error");
  }

  input.get(c);
  while(!input.eof() && isspace(c)) 
    input.get(c); 
  if(!input.eof() && c == '.')
  {
    input.setpos(old_pos);
    return new ErrorPortion("Expected RATIONAL, got FLOAT");
  }
  if(input.eof() || c != '/')
  {
    input.setpos(old_pos);
    return new ErrorPortion("Expected RATIONAL, got INTEGER");
  }

  if(input.eof())
  {
    input.setpos(old_pos);
    return new ErrorPortion("End of file reached");
  }
  input >> denominator;
  if(!input.IsValid())
  {
    input.setpos(old_pos);
    return new ErrorPortion("File read error");
  }

  if(denominator == 0)
  {
    input.setpos(old_pos);
    return new ErrorPortion("Division by zero");
  }

  ((RationalPortion*) param[1])->Value() = numerator;
  ((RationalPortion*) param[1])->Value() /= denominator;
 
  return param[0]->RefCopy();
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
 
  return param[0]->RefCopy();
}



Portion* GSM_Read_List(Portion** param, PortionSpec spec,
		       Portion* (*func) (Portion**), bool ListFormat)
{
  Portion* p;
  Portion** sub_param;
  ListPortion* list;
  int i;
  char c = ' ';  
  gInput& input = ((InputPortion*) param[0])->Value();
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


  sub_param = new Portion*[2];
  list = ((ListPortion*) param[1]);


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
      delete[] sub_param;
      input.setpos(old_pos);
      return p;
    }
    else
      delete p;
  }
  delete[] sub_param;

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

  return param[0]->RefCopy();
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

Portion* GSM_Read_List_Float(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  Portion* p = GSM_Read_List(param, porFLOAT, GSM_Read_Float, false);
  if(p->Spec().Type == porERROR)
    ((ListPortion*) param[1])->AssignFrom(temp);
  delete temp;
  return p;
}

Portion* GSM_Read_List_Rational(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  Portion* p = GSM_Read_List(param, porRATIONAL, GSM_Read_Rational, false);
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




extern Portion* GSM_ListForm_MixedFloat(Portion** param);
extern Portion* GSM_Mixed_NfgFloat(Portion **param);

Portion* GSM_Read_MixedFloat(Portion** param)
{
  Portion** sub_param = new Portion*[2];
  //Portion* owner = param[1]->Owner();
  Portion* owner = 
    new NfgValPortion(((MixedSolution<double>*) 
		       ((MixedPortion*) param[1])->Value())->BelongsTo());
  sub_param[0] = param[1];
  sub_param[1] = 0;
  Portion* list = GSM_ListForm_MixedFloat(sub_param);
  sub_param[0] = param[0];
  sub_param[1] = list;
  Portion* result = GSM_Read_List_Float(sub_param);
  sub_param[0] = owner;
  sub_param[1] = list;
  Portion* p = GSM_Mixed_NfgFloat(sub_param);
  (*((MixedSolution<double>*) ((MixedPortion*) param[1])->Value())) = 
    (*((MixedSolution<double>*) ((MixedPortion*) p)->Value()));
  delete list;
  delete p;
  delete owner;
  delete[] sub_param;
  return result;
}


extern Portion* GSM_ListForm_MixedRational(Portion** param);
extern Portion* GSM_Mixed_NfgRational(Portion **param);

Portion* GSM_Read_MixedRational(Portion** param)
{
  Portion** sub_param = new Portion*[2];
  //Portion* owner = param[1]->Owner();
  Portion* owner = 
    new NfgValPortion(((MixedSolution<gRational>*) 
		       ((MixedPortion*) param[1])->Value())->BelongsTo());
  sub_param[0] = param[1];
  sub_param[1] = 0;
  Portion* list = GSM_ListForm_MixedRational(sub_param);
  sub_param[0] = param[0];
  sub_param[1] = list;
  Portion* result = GSM_Read_List_Rational(sub_param);
  sub_param[0] = owner;
  sub_param[1] = list;
  Portion* p = GSM_Mixed_NfgRational(sub_param);
  (*((MixedSolution<gRational>*) ((MixedPortion*) param[1])->Value())) = 
    (*((MixedSolution<gRational>*) ((MixedPortion*) p)->Value()));
  delete list;
  delete p;
  delete owner;
  delete[] sub_param;
  return result;
}



extern Portion* GSM_ListForm_BehavFloat(Portion** param);
extern Portion* GSM_Behav_EfgFloat(Portion **param);

Portion* GSM_Read_BehavFloat(Portion** param)
{
  Portion** sub_param = new Portion*[2];
  //Portion* owner = param[1]->Owner();
  Portion* owner = 
    new EfgValPortion(((BehavSolution<double>*) 
		       ((BehavPortion*) param[1])->Value())->BelongsTo());
  sub_param[0] = param[1];
  sub_param[1] = 0;
  Portion* list = GSM_ListForm_BehavFloat(sub_param);
  sub_param[0] = param[0];
  sub_param[1] = list;
  Portion* result = GSM_Read_List_Float(sub_param);
  sub_param[0] = owner;
  sub_param[1] = list;
  Portion* p = GSM_Behav_EfgFloat(sub_param);
  (*((BehavSolution<double>*) ((BehavPortion*) param[1])->Value())) = 
    (*((BehavSolution<double>*) ((BehavPortion*) p)->Value()));
  delete list;
  delete p;
  delete[] sub_param;
  return result;
}


extern Portion* GSM_ListForm_BehavRational(Portion** param);
extern Portion* GSM_Behav_EfgRational(Portion **param);

Portion* GSM_Read_BehavRational(Portion** param)
{
  Portion** sub_param = new Portion*[2];
  //Portion* owner = param[1]->Owner();
  Portion* owner = 
    new EfgValPortion(((BehavSolution<gRational>*) 
		       ((BehavPortion*) param[1])->Value())->BelongsTo());
  sub_param[0] = param[1];
  sub_param[1] = 0;
  Portion* list = GSM_ListForm_BehavRational(sub_param);
  sub_param[0] = param[0];
  sub_param[1] = list;
  Portion* result = GSM_Read_List_Rational(sub_param);
  sub_param[0] = owner;
  sub_param[1] = list;
  Portion* p = GSM_Behav_EfgRational(sub_param);
  (*((BehavSolution<gRational>*) ((BehavPortion*) param[1])->Value())) = 
    (*((BehavSolution<gRational>*) ((BehavPortion*) p)->Value()));
  delete list;
  delete p;
  delete[] sub_param;
  return result;
}




Portion* GSM_Read_Undefined(Portion** param)
{
  /* will go through and try to read the input as different format until
     it succeeds */

  Portion** sub_param;
  char c = ' ';  
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  Portion* result = 0;

  assert(param[1] == 0);


  while(!input.eof() && isspace(c))
    input.get(c);
  if(input.eof())
    return new ErrorPortion("End of file reached");
  if(c == '{')
  {
    param[1] = new ListValPortion();

    sub_param = new Portion*[2];
    
    do
    {
      sub_param[0] = param[0];
      sub_param[1] = 0;
      result = GSM_Read_Undefined(sub_param);
      if(result->Spec().Type != porERROR)
	((ListPortion*) param[1])->Append(sub_param[1]);
      else
      {
	delete result;
	result = 0;
      }

      c = ' ';
      while(!input.eof() && isspace(c))
	input.get(c);
      if(!input.eof() && c != ',')
	input.unget(c);

    } while(result != 0 && !input.eof());

    delete[] sub_param;

    c = ' ';
    while(!input.eof() && isspace(c))
      input.get(c);
    if(input.eof())
    {
      delete result;
      delete param[1];
      param[1] = 0;
      result = new ErrorPortion("End of file reached");
    }
    else if(c != '}')
    {
      delete result;
      delete param[1];
      param[1] = 0;
      result = new ErrorPortion("Mismatching braces");
    }
    else
    {
      assert(result == 0);
      result = param[0]->RefCopy();
    }

  }
  else // not a list
  {
    input.unget(c);
    param[1] = new BoolValPortion(false);
    result = GSM_Read_Bool(param);

    if(result->Spec().Type == porERROR)
    {
      delete param[1];
      delete result;
      param[1] = new IntValPortion(0);
      result = GSM_Read_Integer(param);    
    }
    if(result->Spec().Type == porERROR)
    {
      delete param[1];
      delete result;
      param[1] = new FloatValPortion(0);
      result = GSM_Read_Float(param);    
    }
    if(result->Spec().Type == porERROR)
    {
      delete param[1];
      delete result;
      param[1] = new RationalValPortion(0);
      result = GSM_Read_Rational(param);    
    }
    if(result->Spec().Type == porERROR)
    {
      delete param[1];
      delete result;
      param[1] = new TextValPortion((gString) "");
      result = GSM_Read_Text(param);    
    }
    if(result->Spec().Type == porERROR)
    {
      delete param[1];
      delete result;
      param[1] = 0;
      result = new ErrorPortion("Cannot determine data type");
      input.setpos(old_pos);
    }

  }

  return result;
}

//
// Don't know where this really belongs, but this seems like a good
// a place as any, at least for now.
//

Portion *GSM_Version(Portion **)
{
  return new FloatValPortion(GCL_VERSION);
}


Portion* GSM_Help(Portion** param)
{
  return _gsm->Help(((TextPortion*) param[0])->Value());
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
  ListPortion* Prototypes = (ListPortion*) _gsm->Help(txt);
  int i;
  int body;

  for(i=1; i<=Prototypes->Length(); i++)
  {
    assert(Prototypes->Spec().Type == porTEXT);
    s << ((TextPortion*) (*Prototypes)[i])->Value() << '\n';
  }

  gFileInput f("gcl.man");
  gString line;
  gString line_out;
  bool found = false;
  while(f.IsValid() && !f.eof() && !found)
  {
    line = GetLine(f);
    if(line.length() >= txt.length())
      if(line.left(txt.length())==txt)
	found = true;
  }
  if(found)
  {
    body = 0;
    while(f.IsValid() && !f.eof())
    {
      line = GetLine(f);      
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

  if(!found)
  {
    s << '\n';
    s << "  " << "Manual not found\n";
    s << '\n';
  }
  return new BoolValPortion(found);
}

Portion* GSM_HelpVars(Portion** param)
{
  return _gsm->HelpVars(((TextPortion*) param[0])->Value());
}

Portion* GSM_Clear(Portion**)
{
  _gsm->Clear();
  return new BoolValPortion(true);
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
  return new TextValPortion(AscTime.mid(11, 1) +
			    AscTime.mid(4, 21) + ", " + AscTime.mid(8, 12));
}





void Init_gsmoper(GSM* gsm)
{
  FuncDescObj* FuncObj;

  FuncObj = new FuncDescObj("Version", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Version, porFLOAT, 0));
  gsm->AddFunction(FuncObj);

  //---------------------- Assign ------------------------


  ParamInfoType xy_Int[] =
  {
    ParamInfoType("x", porINTEGER),
    ParamInfoType("y", porINTEGER)
  };

  ParamInfoType xy_Float[] =
  {
    ParamInfoType("x", porFLOAT),
    ParamInfoType("y", porFLOAT)
  };

  ParamInfoType xy_Rational[] =
  {
    ParamInfoType("x", porRATIONAL),
    ParamInfoType("y", porRATIONAL)
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
    ParamInfoType("x", porNF_SUPPORT),
    ParamInfoType("y", porNF_SUPPORT)
  };

  ParamInfoType xy_MixedFloat[] =
  {
    ParamInfoType("x", porMIXED_FLOAT),
    ParamInfoType("y", porMIXED_FLOAT)
  };

  ParamInfoType xy_MixedRational[] =
  {
    ParamInfoType("x", porMIXED_RATIONAL),
    ParamInfoType("y", porMIXED_RATIONAL)
  };

  ParamInfoType xy_EfSupport[] =
  {
    ParamInfoType("x", porEF_SUPPORT),
    ParamInfoType("y", porEF_SUPPORT)
  };

  ParamInfoType xy_BehavFloat[] =
  {
    ParamInfoType("x", porBEHAV_FLOAT),
    ParamInfoType("y", porBEHAV_FLOAT)
  };

  ParamInfoType xy_BehavRational[] =
  {
    ParamInfoType("x", porBEHAV_RATIONAL),
    ParamInfoType("y", porBEHAV_RATIONAL)
  };


  ParamInfoType xy_EfPlayer[] =
  {
    ParamInfoType("x", porPLAYER_EFG),
    ParamInfoType("y", porPLAYER_EFG)
  };

  ParamInfoType xy_NfPlayer[] =
  {
    ParamInfoType("x", porPLAYER_NFG),
    ParamInfoType("y", porPLAYER_NFG),
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
    ParamInfoType("x", porOUTCOME),
    ParamInfoType("y", porOUTCOME)
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


  ParamInfoType x_Float[] =
  {
    ParamInfoType("x", porFLOAT)
  };


  ParamInfoType file_Text[] =
  {
    ParamInfoType("file", porTEXT)
  };


  //-------------------- Plus ----------------------------

  FuncObj = new FuncDescObj("Plus", 7);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Add_double, 
				       porFLOAT, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Add_int, 
				       porINTEGER, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Add_gRational, 
				       porRATIONAL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Add_MixedFloat, 
				       porMIXED_FLOAT, 2, xy_MixedFloat));
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Add_MixedRational,
				       porMIXED_RATIONAL, 2,xy_MixedRational));
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Add_BehavFloat, 
				       porBEHAV_FLOAT, 2, xy_BehavFloat));
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Add_BehavRational, 
				       porBEHAV_RATIONAL, 2,xy_BehavRational));
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

  FuncObj = new FuncDescObj("Minus", 7);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Subtract_double, 
				       porFLOAT, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Subtract_int, 
				       porINTEGER, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Subtract_gRational, 
				       porRATIONAL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Subtract_MixedFloat, 
				       porMIXED_FLOAT, 2, xy_MixedFloat));
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Subtract_MixedRational, 
				       porMIXED_RATIONAL, 2,xy_MixedRational));
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Subtract_BehavFloat, 
				       porBEHAV_FLOAT, 2, xy_BehavFloat));
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Subtract_BehavRational, 
				       porBEHAV_RATIONAL, 2,xy_BehavRational));
  gsm->AddFunction(FuncObj);


  //----------------------- Times ----------------------------

  FuncObj = new FuncDescObj("Times", 11);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Multiply_double, 
				       porFLOAT, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Multiply_int, 
				       porINTEGER, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Multiply_gRational, 
				       porRATIONAL, 2, xy_Rational));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Multiply_MixedFloat1, 
				       porMIXED_FLOAT, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porFLOAT));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("y", porMIXED_FLOAT));
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Multiply_MixedFloat2, 
				       porMIXED_FLOAT, 2));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("x", porMIXED_FLOAT));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("y", porFLOAT));

  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Multiply_MixedRational1, 
				       porMIXED_RATIONAL, 2));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("x", porRATIONAL));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("y", porMIXED_RATIONAL));
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Multiply_MixedRational2, 
				       porMIXED_RATIONAL, 2));
  FuncObj->SetParamInfo(6, 0, ParamInfoType("x", porMIXED_RATIONAL));
  FuncObj->SetParamInfo(6, 1, ParamInfoType("y", porRATIONAL));

  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Multiply_BehavFloat1, 
				       porBEHAV_FLOAT, 2));
  FuncObj->SetParamInfo(7, 0, ParamInfoType("x", porFLOAT));
  FuncObj->SetParamInfo(7, 1, ParamInfoType("y", porBEHAV_FLOAT));
  FuncObj->SetFuncInfo(8, FuncInfoType(GSM_Multiply_BehavFloat2, 
				       porBEHAV_FLOAT, 2));
  FuncObj->SetParamInfo(8, 0, ParamInfoType("x", porBEHAV_FLOAT));
  FuncObj->SetParamInfo(8, 1, ParamInfoType("y", porFLOAT));

  FuncObj->SetFuncInfo(9, FuncInfoType(GSM_Multiply_BehavRational1, 
				       porBEHAV_RATIONAL, 2));
  FuncObj->SetParamInfo(9, 0, ParamInfoType("x", porRATIONAL));
  FuncObj->SetParamInfo(9, 1, ParamInfoType("y", porBEHAV_RATIONAL));
  FuncObj->SetFuncInfo(10, FuncInfoType(GSM_Multiply_BehavRational2, 
					porBEHAV_RATIONAL, 2));
  FuncObj->SetParamInfo(10, 0, ParamInfoType("x", porBEHAV_RATIONAL));
  FuncObj->SetParamInfo(10, 1, ParamInfoType("y", porRATIONAL));

  gsm->AddFunction(FuncObj);


  //----------------------- Divide -------------------------

  FuncObj = new FuncDescObj("Divide", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Divide_double, porFLOAT, 
				       2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Divide_int, 
				       porRATIONAL, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Divide_gRational, 
				       porRATIONAL, 2, xy_Rational));
  gsm->AddFunction(FuncObj);  

  //----------------------- Power -------------------------

  FuncObj = new FuncDescObj("Power", 6);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Power_Int_Int, 
				       porINTEGER, 2, xy_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Power_Float_Int, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porFLOAT));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("y", porINTEGER));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Power_Rational_Int, 
				       porRATIONAL, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porRATIONAL));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("y", porINTEGER));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Power_Int_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("y", porFLOAT));
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Power_Float_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("x", porFLOAT));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("y", porFLOAT));
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Power_Rational_Float, 
				       porFLOAT, 2));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("x", porRATIONAL));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("y", porFLOAT));

  gsm->AddFunction(FuncObj);


  //----------------------- Exp, Log ---------------------

  FuncObj = new FuncDescObj("Exp", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Exp, porFLOAT, 1, x_Float));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Log", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Log, porFLOAT, 1, x_Float));
  gsm->AddFunction(FuncObj);


  //----------------------- Negate -----------------------

  FuncObj = new FuncDescObj("Negate", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Negate_double, 
				       porFLOAT, 1, x_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Negate_int, porINTEGER, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Negate_gRational, 
				       porRATIONAL, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porRATIONAL));
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

  FuncObj = new FuncDescObj("Equal", 18);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Equal_Float, 
				       porBOOL, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Equal_Integer, 
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Equal_Rational, 
				       porBOOL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Equal_Text, 
				       porBOOL, 2, xy_Text));
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Equal_Boolean, 
				       porBOOL, 2, xy_Bool));

  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Equal_EfPlayer, 
				       porBOOL, 2, xy_EfPlayer));
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Equal_Node, 
				       porBOOL, 2, xy_Node));
  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Equal_Infoset, 
				       porBOOL, 2, xy_Infoset));
  FuncObj->SetFuncInfo(8, FuncInfoType(GSM_Equal_Outcome, 
				       porBOOL, 2, xy_Outcome));
  FuncObj->SetFuncInfo(9, FuncInfoType(GSM_Equal_Action, 
				       porBOOL, 2, xy_Action));
  FuncObj->SetFuncInfo(10, FuncInfoType(GSM_Equal_EfSupport,
					porBOOL, 2, xy_EfSupport));
  FuncObj->SetFuncInfo(11, FuncInfoType(GSM_Equal_BehavFloat, 
					porBOOL, 2, xy_BehavFloat));
  FuncObj->SetFuncInfo(12, FuncInfoType(GSM_Equal_BehavRational, 
					porBOOL, 2, xy_BehavRational));

  FuncObj->SetFuncInfo(13, FuncInfoType(GSM_Equal_NfPlayer,
					porBOOL, 2, xy_NfPlayer));
  FuncObj->SetFuncInfo(14, FuncInfoType(GSM_Equal_Strategy,
					porBOOL, 2, xy_Strategy));
  FuncObj->SetFuncInfo(15, FuncInfoType(GSM_Equal_NfSupport,
					porBOOL, 2, xy_NfSupport));
  FuncObj->SetFuncInfo(16, FuncInfoType(GSM_Equal_MixedFloat, 
					porBOOL, 2, xy_MixedFloat));
  FuncObj->SetFuncInfo(17, FuncInfoType(GSM_Equal_MixedRational, 
					porBOOL, 2, xy_MixedRational));
  gsm->AddFunction(FuncObj);


  //-------------------------- NotEqual ---------------------------

  FuncObj = new FuncDescObj("NotEqual", 18);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NotEqual_Float, 
				       porBOOL, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NotEqual_Integer, 
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_NotEqual_Rational, 
				       porBOOL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_NotEqual_Text, 
				       porBOOL, 2, xy_Text));
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_NotEqual_Boolean, 
				       porBOOL, 2, xy_Bool));

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
  FuncObj->SetFuncInfo(11, FuncInfoType(GSM_NotEqual_BehavFloat, 
					porBOOL, 2, xy_BehavFloat));
  FuncObj->SetFuncInfo(12, FuncInfoType(GSM_NotEqual_BehavRational, 
					porBOOL, 2, xy_BehavRational));

  FuncObj->SetFuncInfo(13, FuncInfoType(GSM_NotEqual_NfPlayer,
					porBOOL, 2, xy_NfPlayer));
  FuncObj->SetFuncInfo(14, FuncInfoType(GSM_NotEqual_Strategy,
					porBOOL, 2, xy_Strategy));
  FuncObj->SetFuncInfo(15, FuncInfoType(GSM_NotEqual_NfSupport,
					porBOOL, 2, xy_NfSupport));
  FuncObj->SetFuncInfo(16, FuncInfoType(GSM_NotEqual_MixedFloat, 
					porBOOL, 2, xy_MixedFloat));
  FuncObj->SetFuncInfo(17, FuncInfoType(GSM_NotEqual_MixedRational, 
					porBOOL, 2, xy_MixedRational));
  gsm->AddFunction(FuncObj);


  //-------------------------- Greater ---------------------------

  FuncObj = new FuncDescObj("Greater", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GreaterThan_double, 
				       porBOOL, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GreaterThan_int, 
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_GreaterThan_gRational, 
				       porBOOL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_GreaterThan_gString, 
				       porBOOL, 2, xy_Text));
  gsm->AddFunction(FuncObj);


  //-------------------------- Less ---------------------------

  FuncObj = new FuncDescObj("Less", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LessThan_double, 
				       porBOOL, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_LessThan_int, 
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_LessThan_gRational, 
				       porBOOL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_LessThan_gString, 
				       porBOOL, 2, xy_Text));
  gsm->AddFunction(FuncObj);


  //---------------------- GreaterThanOrEqualTo ---------------------------

  FuncObj = new FuncDescObj("GreaterEqual", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_GreaterThanOrEqualTo_double, 
				       porBOOL, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_GreaterThanOrEqualTo_int,
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_GreaterThanOrEqualTo_gRational, 
				       porBOOL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_GreaterThanOrEqualTo_gString, 
				       porBOOL, 2, xy_Text));
  gsm->AddFunction(FuncObj);


  //----------------------- LessThanOrEqualTo ---------------------------

  FuncObj = new FuncDescObj("LessEqual", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LessThanOrEqualTo_double, 
				       porBOOL, 2, xy_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_LessThanOrEqualTo_int, 
				       porBOOL, 2, xy_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_LessThanOrEqualTo_gRational, 
				       porBOOL, 2, xy_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_LessThanOrEqualTo_gString, 
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



  FuncObj = new FuncDescObj("Paren", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Paren, porANYTYPE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porANYTYPE));
  gsm->AddFunction(FuncObj);

  //-------------------- NewStream -------------------------

  FuncObj = new FuncDescObj("Output", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewOutputStream, 
				       porOUTPUT, 1, file_Text));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Input", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NewInputStream, 
				       porINPUT, 1, file_Text));
  gsm->AddFunction(FuncObj);

  //------------------- Formatted Output -------------------

  GSM_SetWriteOptions();

  FuncObj = new FuncDescObj("Write", 10);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Write_numerical, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType
			("x", porBOOL | porINTEGER | porFLOAT | porRATIONAL));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Write_gString, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("x", porTEXT));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Write_Mixed, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("x", porMIXED));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Write_Behav, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("x", porBEHAV));
  
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Write_Nfg, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("x", porNFG,
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Write_Efg, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("x", porEFG,
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Write_list, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(6, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(6, 1, ParamInfoType
			("x", PortionSpec(porBOOL | porINTEGER | porFLOAT | 
					  porTEXT | porRATIONAL | porMIXED |
					  porBEHAV, 1)));

  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Write_NfSupport, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(7, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(7, 1, ParamInfoType("x", porNF_SUPPORT));

  FuncObj->SetFuncInfo(8, FuncInfoType(GSM_Write_EfSupport, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(8, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(8, 1, ParamInfoType("x", porEF_SUPPORT));

  FuncObj->SetFuncInfo(9, FuncInfoType(GSM_Write_Strategy, 
				       porOUTPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(9, 0, ParamInfoType("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(9, 1, ParamInfoType("x", porSTRATEGY));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SetFormat", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_SetFormat, porBOOL, 9));
  FuncObj->SetParamInfo(0, 0, ParamInfoType
			("width", porINTEGER, 
			 new IntRefPortion(_WriteWidth)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType
			("precis", porINTEGER,
			 new IntRefPortion(_WritePrecis)));
  FuncObj->SetParamInfo(0, 2, ParamInfoType
			("expmode", porBOOL,
			 new BoolRefPortion(_WriteExpmode)));
  FuncObj->SetParamInfo(0, 3, ParamInfoType
			("quote", porBOOL,
			 new BoolRefPortion(_WriteQuoted)));
  FuncObj->SetParamInfo(0, 4, ParamInfoType
			("listBraces", porBOOL,
			 new BoolRefPortion(_WriteListBraces)));
  FuncObj->SetParamInfo(0, 5, ParamInfoType
			("listCommas", porBOOL,
			 new BoolRefPortion(_WriteListCommas)));
  FuncObj->SetParamInfo(0, 6, ParamInfoType
			("listLF", porINTEGER,
			 new IntRefPortion(_WriteListLF)));
  FuncObj->SetParamInfo(0, 7, ParamInfoType
			("listIndent", porINTEGER,
			 new IntRefPortion(_WriteListIndent)));
  FuncObj->SetParamInfo(0, 8, ParamInfoType
			("solutionInfo", porINTEGER,
			 new IntRefPortion(_WriteSolutionInfo)));
  gsm->AddFunction(FuncObj);



  //-------------------- Read --------------------------

  FuncObj = new FuncDescObj("Read", 15);

  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Read_Bool, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("input", porINPUT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("x", porBOOL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Read_List_Bool, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("x", PortionSpec(porBOOL,1), 
					    REQUIRED, BYREF));


  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Read_Integer, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("x", porINTEGER, 
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Read_List_Integer, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("x", PortionSpec(porINTEGER,1), 
					    REQUIRED, BYREF));
  
  
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Read_Float, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("x", porFLOAT,  
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Read_List_Float, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("x", PortionSpec(porFLOAT,1), 
					    REQUIRED, BYREF));
  
  
  FuncObj->SetFuncInfo(6, FuncInfoType(GSM_Read_Rational, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(6, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(6, 1, ParamInfoType("x", porRATIONAL, 
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(7, FuncInfoType(GSM_Read_List_Rational, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(7, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(7, 1, ParamInfoType("x", PortionSpec(porRATIONAL,1), 
					    REQUIRED, BYREF));
  
  
  FuncObj->SetFuncInfo(8, FuncInfoType(GSM_Read_Text, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(8, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(8, 1, ParamInfoType("x", porTEXT, 
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(9, FuncInfoType(GSM_Read_List_Text, 
				       porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(9, 0, ParamInfoType("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(9, 1, ParamInfoType("x", PortionSpec(porTEXT,1), 
					    REQUIRED, BYREF));


  FuncObj->SetFuncInfo(10, FuncInfoType(GSM_Read_MixedFloat, 
					porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(10, 0, ParamInfoType("input", porINPUT,
					     REQUIRED, BYREF));
  FuncObj->SetParamInfo(10, 1, ParamInfoType("x", porMIXED_FLOAT, 
					     REQUIRED, BYREF));

  FuncObj->SetFuncInfo(11, FuncInfoType(GSM_Read_MixedRational, 
					porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(11, 0, ParamInfoType("input", porINPUT,
					     REQUIRED, BYREF));
  FuncObj->SetParamInfo(11, 1, ParamInfoType("x", porMIXED_RATIONAL, 
					     REQUIRED, BYREF));


  FuncObj->SetFuncInfo(12, FuncInfoType(GSM_Read_BehavFloat, 
					porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(12, 0, ParamInfoType("input", porINPUT,
					     REQUIRED, BYREF));
  FuncObj->SetParamInfo(12, 1, ParamInfoType("x", porBEHAV_FLOAT, 
					     REQUIRED, BYREF));

  FuncObj->SetFuncInfo(13, FuncInfoType(GSM_Read_BehavRational,
					porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(13, 0, ParamInfoType("input", porINPUT,
					     REQUIRED, BYREF));
  FuncObj->SetParamInfo(13, 1, ParamInfoType("x", porBEHAV_RATIONAL, 
					     REQUIRED, BYREF));


  FuncObj->SetFuncInfo(14, FuncInfoType(GSM_Read_Undefined, 
					porINPUT, 2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(14, 0, ParamInfoType("input", porINPUT,
					     REQUIRED, BYREF));
  FuncObj->SetParamInfo(14, 1, ParamInfoType("x", porUNDEFINED, 
					     REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);

  

  //---------------- faked functions -----------------//

  FuncObj = new FuncDescObj("Help", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Help, PortionSpec(porTEXT, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Manual", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Manual, porBOOL, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("y", porOUTPUT,
					    new OutputRefPortion(gout)));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("HelpVars", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_HelpVars, PortionSpec(porTEXT, 1),
				       1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Clear", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Clear, porBOOL, 0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Assign", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Assign, porANYTYPE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("v", porANYTYPE, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("UnAssign", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_UnAssign, porANYTYPE, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porTEXT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Date", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Date, porTEXT, 0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Randomize", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Randomize_Integer, porINTEGER, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("seed", porINTEGER,
					    new IntValPortion(0)));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Randomize_Float, porFLOAT, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", porFLOAT));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("seed", porINTEGER,
					    new IntValPortion(0)));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Randomize_Rational, porRATIONAL,2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porRATIONAL));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("seed", porINTEGER,
					    new IntValPortion(0)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("IsNull", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsNull, porBOOL, 1, 0, LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", 
					    PortionSpec(porANYTYPE, 0, true)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Null", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Null, 
				       PortionSpec(porANYTYPE, 0, true),
				       1, 0, LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porANYTYPE));
  gsm->AddFunction(FuncObj);

}




