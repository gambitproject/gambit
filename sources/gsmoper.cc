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
#include "gtext.h"


#include "gcmdline.h"


extern GSM* _gsm;






//-------------
// Precision
//-------------

Portion* GSM_Precision(Portion** param)
{
  return new PrecisionPortion(((NumberPortion *) param[0])->Value().GetPrecision());
}

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

Portion* GSM_Randomize(Portion** param)
{
  long _RandomSeed = ((NumberPortion*) param[1])->Value();
  if(_RandomSeed > 0)
    _RandomSeed = -_RandomSeed;
  long v;
  if(_RandomSeed != 0)
    _idum = _RandomSeed;
  v = ran1(&_idum);
  return new NumberPortion(v);
}


//-------------------------------------------------------------------
//                      mathematical operators
//------------------------------------------------------------------

//---------
// Plus
//---------

static Portion *GSM_Plus_Number(Portion** param)
{
  return new NumberPortion(((NumberPortion*) param[0])->Value() +
                           ((NumberPortion*) param[1])->Value());
}

static Portion* GSM_Plus_Mixed(Portion** param)
{
  MixedPortion *result = new MixedPortion(new MixedSolution(*((MixedPortion *) param[0])->Value()));
  *result->Value() += *((MixedPortion*) param[1])->Value();
  return result;
}

static Portion *GSM_Plus_Behav(Portion** param)
{
  if(((BehavPortion*) param[0])->Value()->Support() !=
     ((BehavPortion*) param[1])->Value()->Support())
    throw gclRuntimeError("Support mismatch");

  BehavPortion *result = new BehavPortion(new BehavSolution(*((BehavPortion *) param[0])->Value()));
  *result->Value() += *((BehavPortion*) param[1])->Value();
  return result;
}


//-----------
// Concat
//-----------

static Portion* GSM_Concat_Text(Portion** param)
{
  return new TextPortion(((TextPortion*) param[0])->Value() +
			 ((TextPortion*) param[1])->Value());
}


//---------
// Minus
//---------

static Portion *GSM_Minus_Number(Portion** param)
{
  return new NumberPortion(((NumberPortion *) param[0])->Value() -
                           ((NumberPortion *) param[1])->Value());
}

static Portion *GSM_Minus_Mixed(Portion** param)
{
  MixedPortion *result = new MixedPortion(new MixedSolution(*((MixedPortion *) param[0])->Value()));
  *result->Value() -= *((MixedPortion*) param[1])->Value();
  return result;
}

static Portion *GSM_Minus_Behav(Portion** param)
{
  if(((BehavPortion*) param[0])->Value()->Support() !=
     ((BehavPortion*) param[1])->Value()->Support())
    throw gclRuntimeError("Support mismatch");

  BehavPortion *result = new BehavPortion(new BehavSolution(*((BehavPortion *) param[0])->Value()));
  *result->Value() -= *((BehavPortion*) param[1])->Value();
  return result;
}


//----------
// Times
//----------

static Portion *GSM_Times_Number(Portion** param)
{
  return new NumberPortion(((NumberPortion *) param[0])->Value() *
                           ((NumberPortion *) param[1])->Value());
}

static Portion *GSM_Times_Mixed(Portion** param)
{
  MixedPortion *result = new MixedPortion(new MixedSolution(*((MixedPortion *) param[1])->Value()));
  *result->Value() *= ((NumberPortion*) param[0])->Value();
  return result;
}

static Portion *GSM_Times_Behav(Portion** param)
{
  BehavPortion *result = new BehavPortion(new BehavSolution(*((BehavPortion *) param[1])->Value()));
  *result->Value() *= ((NumberPortion*) param[0])->Value();
  return result;
}

//----------
// Divide
//----------

static Portion *GSM_Divide(Portion** param)
{
  if(((NumberPortion*) param[1])->Value() != (gNumber)0)
    return new NumberPortion(((NumberPortion*) param[0])->Value() /
			     ((NumberPortion*) param[1])->Value());
  else
    return new NullPortion(porNUMBER);
}


//--------
// Exp
//--------

static Portion *GSM_Exp(Portion **param)
{
  return new NumberPortion
    (exp((double) ((NumberPortion *) param[0])->Value()));
}

//--------
// Log
//--------

static Portion *GSM_Log(Portion **param)
{
  double d = (double) ((NumberPortion *) param[0])->Value();
  if(d <= 0.0)
    return new NullPortion(porNUMBER);
  else
    return new NumberPortion(log(d));
}


//----------
// Power
//----------

static Portion *GSM_Power(Portion** param)
{
  gNumber base = ((NumberPortion*) param[0])->Value();
  gNumber exponent = ((NumberPortion*) param[1])->Value();
  // Note, the below should eventually be corrected to do rational exponentiation
  // correctly
  return new NumberPortion(pow((double)base, (double)exponent));
}


//----------
// Negate
//----------

static Portion *GSM_Negate(Portion** param)
{
  return new NumberPortion(-((NumberPortion *) param[0])->Value());
}

//-----------
// Modulus
//-----------

static Portion *GSM_Modulus(Portion** param)
{
  if (((NumberPortion*) param[1])->Value() != gNumber(0))
    return new NumberPortion((long) ((NumberPortion*) param[0])->Value() %
			  (long) ((NumberPortion*) param[1])->Value());
  else
    return new NullPortion(porNUMBER);
}



//-----------
// Equal
//-----------

static Portion* GSM_Equal_Number(Portion** param)
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

static Portion* GSM_Equal_Text(Portion** param)
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

static Portion* GSM_Equal_Boolean(Portion** param)
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

static Portion* GSM_Equal_Efg(Portion** param)
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

static Portion* GSM_Equal_EfPlayer(Portion** param)
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

static Portion* GSM_Equal_Node(Portion** param)
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

static Portion* GSM_Equal_Infoset(Portion** param)
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

static Portion* GSM_Equal_EfOutcome(Portion** param)
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

static Portion* GSM_Equal_Action(Portion** param)
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

static Portion* GSM_Equal_EfSupport(Portion** param)
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

static Portion* GSM_Equal_Behav(Portion** param)
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

static Portion* GSM_Equal_NfPlayer(Portion** param)
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

static Portion* GSM_Equal_Strategy(Portion** param)
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

static Portion* GSM_Equal_NfOutcome(Portion** param)
{
  if( (param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )
  {
    return new BoolPortion( param[0]->Spec().Type == 
 			       param[1]->Spec().Type );
  }
  return new BoolPortion(((NfOutcomePortion *) param[0])->Value() ==
			    ((NfOutcomePortion *) param[1])->Value());
}

static Portion* GSM_Equal_NfSupport(Portion** param)
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

static Portion* GSM_Equal_Mixed(Portion** param)
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

static Portion* GSM_NotEqual_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion *) param[0])->Value() !=
			    ((NumberPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Text(Portion** param)
{
  return new BoolPortion(((TextPortion *) param[0])->Value() !=
			    ((TextPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Boolean(Portion** param)
{
  return new BoolPortion(((BoolPortion *) param[0])->Value() !=
			    ((BoolPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Efg(Portion** param)
{
  return new BoolPortion(((EfgPortion *) param[0])->Value() !=
			    ((EfgPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_EfPlayer(Portion** param)
{
  return new BoolPortion(((EfPlayerPortion *) param[0])->Value() !=
			    ((EfPlayerPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Node(Portion** param)
{
  return new BoolPortion(((NodePortion *) param[0])->Value() !=
			    ((NodePortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Infoset(Portion** param)
{
  return new BoolPortion(((InfosetPortion *) param[0])->Value() !=
			    ((InfosetPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_EfOutcome(Portion** param)
{
  return new BoolPortion(((EfOutcomePortion *) param[0])->Value() !=
			    ((EfOutcomePortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Action(Portion** param)
{
  return new BoolPortion(((ActionPortion *) param[0])->Value() !=
			    ((ActionPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_EfSupport(Portion** param)
{
  return new BoolPortion((*((EfSupportPortion *) param[0])->Value()) !=
			    (*((EfSupportPortion *) param[1])->Value()));
}

static Portion* GSM_NotEqual_Behav(Portion** param)
{
  if (*((BehavPortion *) param[0])->Value() !=
      *((BehavPortion *) param[1])->Value())
    return new BoolPortion(triTRUE);
  else
    return new BoolPortion(triFALSE);
}

static Portion* GSM_NotEqual_NfPlayer(Portion** param)
{
  return new BoolPortion(((NfPlayerPortion *) param[0])->Value() !=
			    ((NfPlayerPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Strategy(Portion** param)
{
  return new BoolPortion(((StrategyPortion *) param[0])->Value() !=
			    ((StrategyPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Nfg(Portion** param)
{
  if ((param[0]->Spec().Type == porNULL) || 
      (param[1]->Spec().Type == porNULL) )  {
    return new BoolPortion( param[0]->Spec().Type != 
 			       param[1]->Spec().Type );
  }

  return new BoolPortion(((NfgPortion *) param[0])->Value() !=
			      ((NfgPortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_NfSupport(Portion** param)
{
  return new BoolPortion((*((NfSupportPortion *) param[0])->Value()) !=
			    (*((NfSupportPortion *) param[1])->Value()));
}

static Portion* GSM_NotEqual_NfOutcome(Portion** param)
{
  return new BoolPortion(((NfOutcomePortion *) param[0])->Value() !=
			    ((NfOutcomePortion *) param[1])->Value());
}

static Portion* GSM_NotEqual_Mixed(Portion** param)
{
  if (*((MixedPortion *) param[0])->Value() !=
      *((MixedPortion *) param[1])->Value())
    return new BoolPortion(triTRUE);
  else
    return new BoolPortion(triFALSE);
}

//------------
// Greater
//------------

static Portion *GSM_Greater_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() >
                         ((NumberPortion*) param[1])->Value());
}

static Portion* GSM_Greater_Text(Portion** param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() >
			 ((TextPortion*) param[1])->Value());
}


//---------
// Less
//---------

static Portion *GSM_Less_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() <
                         ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_Less_Text(Portion** param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() <
			 ((TextPortion*) param[1])->Value());
}


//---------------
// GreaterEqual
//---------------

static Portion *GSM_GreaterEqual_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() >=
                         ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_GreaterEqual_Text(Portion** param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() >=
			 ((TextPortion*) param[1])->Value());
}


//-------------
// LessEqual
//-------------

static Portion *GSM_LessEqual_Number(Portion** param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() <=
                         ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_LessEqual_Text(Portion** param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() <=
			 ((TextPortion*) param[1])->Value());
}




//----------------------------------------------------------------
//                       logical operators
//----------------------------------------------------------------


static Portion *GSM_And(Portion** param)
{
  gTriState x = ((BoolPortion *) param[0])->Value();
  gTriState y = ((BoolPortion *) param[1])->Value();

  if (x == triTRUE && y == triTRUE)    
    return new BoolPortion(triTRUE);
  else if (x == triFALSE && y == triFALSE)
    return new BoolPortion(triFALSE); 
  else
    return new BoolPortion(triMAYBE);  
}

static Portion *GSM_Or(Portion** param)
{
  gTriState x = ((BoolPortion *) param[0])->Value();
  gTriState y = ((BoolPortion *) param[1])->Value();

  if (x == triTRUE || y == triTRUE)    
    return new BoolPortion(triTRUE);
  else if (x == triMAYBE || y == triMAYBE)
    return new BoolPortion(triMAYBE); 
  else
    return new BoolPortion(triFALSE);
}

static Portion *GSM_Not(Portion** param)
{
  gTriState x = ((BoolPortion *) param[0])->Value();

  if (x == triTRUE)
    return new BoolPortion(triFALSE);
  else if (x == triFALSE)
    return new BoolPortion(triTRUE);
  else
    return new BoolPortion(triMAYBE); 
}


static Portion *GSM_Parentheses(Portion **param)
{
  return param[0]->ValCopy();
}


//----------
// Output
//----------

Portion* GSM_Output(Portion** param)
{
  gText filename = ((TextPortion*) param[0])->Value();
  bool append = ((BoolPortion*) param[1])->Value();

  try  {
    return new OutputPortion(*new gFileOutput(filename, append));
  }
  catch (gFileInput::OpenFailed &) {
    throw gclRuntimeError((gText) "Error opening file \"" +
			      ((TextPortion*) param[0])->Value() + "\"");
  }
}


//--------
// Input
//--------

Portion* GSM_Input(Portion** param)
{
  try {
    return new InputPortion(*new gFileInput(((TextPortion*) param[0])->Value()));
  }
  catch (gFileInput::OpenFailed &) {
    throw gclRuntimeError((gText) "Error opening file \"" +
			      ((TextPortion*) param[0])->Value() + "\"");
  }
}




//-----------------------------------------------------------------
//    Write and SetFormat function - possibly belong somewhere else
//-----------------------------------------------------------------

NumberPortion _WriteWidth(0);
NumberPortion _WritePrecis(6);
BoolPortion _WriteExpmode(triFALSE);
BoolPortion _WriteQuoted(triTRUE);
BoolPortion _WriteListBraces(triTRUE);
BoolPortion _WriteListCommas(triTRUE);
NumberPortion _WriteListLF(0);
NumberPortion _WriteListIndent(2);
NumberPortion _WriteSolutionInfo(1);


void GSM_SetWriteOptions(void)
{
  Portion::_SetWriteWidth(_WriteWidth.Value());
  Portion::_SetWritePrecis(_WritePrecis.Value());
  Portion::_SetWriteExpmode(_WriteExpmode.Value());
  Portion::_SetWriteQuoted(_WriteQuoted.Value());
  Portion::_SetWriteListBraces(_WriteListBraces.Value());
  Portion::_SetWriteListCommas(_WriteListCommas.Value());
  Portion::_SetWriteListLF(_WriteListLF.Value());
  Portion::_SetWriteListIndent(_WriteListIndent.Value());
  Portion::_SetWriteSolutionInfo(_WriteSolutionInfo.Value());

  ToTextWidth(_WriteWidth.Value());
  ToTextPrecision(_WritePrecis.Value());
}


//------------------------------------------------
// *Format
//------------------------------------------------

Portion* GSM_ListFormat(Portion** param)
{
  _WriteListBraces = ((BoolPortion*) param[0])->Value();
  _WriteListCommas = ((BoolPortion*) param[1])->Value();
  _WriteListLF = ((NumberPortion*) param[2])->Value();
  _WriteListIndent = ((NumberPortion*) param[3])->Value();

  GSM_SetWriteOptions();

  return new BoolPortion(triTRUE);
}

Portion* GSM_GetListFormat(Portion** param)
{
  ((BoolPortion*) param[0])->SetValue(_WriteListBraces.Value());
  ((BoolPortion*) param[1])->SetValue(_WriteListCommas.Value());
  ((NumberPortion*) param[2])->SetValue(_WriteListLF.Value());
  ((NumberPortion*) param[3])->SetValue(_WriteListIndent.Value());

  return new BoolPortion(triTRUE);
}

Portion* GSM_FloatFormat(Portion** param)
{
  _WriteWidth = ((NumberPortion*) param[1])->Value();
  _WritePrecis = ((NumberPortion*) param[2])->Value();
  _WriteExpmode = ((BoolPortion*) param[3])->Value();

  GSM_SetWriteOptions();

  return param[0]->RefCopy();
}

Portion* GSM_GetFloatFormat(Portion** param)
{
  ((NumberPortion*) param[1])->SetValue(_WriteWidth.Value());
  ((NumberPortion*) param[2])->SetValue(_WritePrecis.Value());
  ((BoolPortion*) param[3])->SetValue(_WriteExpmode.Value());

  return param[0]->RefCopy();
}

Portion* GSM_TextFormat(Portion** param)
{
  _WriteQuoted = ((BoolPortion*) param[1])->Value();

  GSM_SetWriteOptions();

  return param[0]->RefCopy();
}

Portion* GSM_GetTextFormat(Portion** param)
{
  ((BoolPortion*) param[1])->SetValue(_WriteQuoted.Value());

  return param[0]->RefCopy();
}


Portion* GSM_SolutionFormat(Portion** param)
{
  _WriteSolutionInfo = ((NumberPortion*) param[1])->Value();

  GSM_SetWriteOptions();

  return param[0]->RefCopy();
}

Portion* GSM_GetSolutionFormat(Portion** param)
{
  ((NumberPortion*) param[1])->SetValue(_WriteSolutionInfo.Value());

  return param[0]->RefCopy();
}


Portion *GSM_Print(Portion **param)
{
  param[0]->Output(gout);
  gout << '\n';
  return param[0]->ValCopy();
}


Portion* GSM_Write(Portion** param)
{
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];

  return param[0]->ValCopy();
}

Portion* GSM_Write_Nfg(Portion** param)
{
  gOutput &s = ((OutputPortion*) param[0])->Value();
  Nfg *nfg = ((NfgPortion *) param[1])->Value();

  nfg->WriteNfgFile(s);

  return param[0]->ValCopy();
}


Portion* GSM_Write_Efg(Portion** param)
{
  gOutput &s = ((OutputPortion*) param[0])->Value();
  Efg *efg = ((EfgPortion*) param[1])->Value();

  efg->WriteEfgFile(s);

  return param[0]->ValCopy();
}


//------------------------------ Read --------------------------//


Portion* GSM_Read_Bool(Portion** param)
{
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();
  gTriState value = triMAYBE;
  bool error = false;
  char c = ' ';

  if(input.eof())
  {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }
  while(!input.eof() && isspace(c))
    input.get(c);
  if (c == 'T')  {
    if(!input.eof()) input.get(c); if(c != 'r') error = true;
    if(!input.eof()) input.get(c); if(c != 'u') error = true;
    if(!input.eof()) input.get(c); if(c != 'e') error = true;
    value = triTRUE;
  }
  else if (c == 'F')  {
    if(!input.eof()) input.get(c); if(c != 'a') error = true;
    if(!input.eof()) input.get(c); if(c != 'l') error = true;
    if(!input.eof()) input.get(c); if(c != 's') error = true;
    if(!input.eof()) input.get(c); if(c != 'e') error = true;
    value = triFALSE;
  }
  else if (c == 'M')  {
    if(!input.eof()) input.get(c); if(c != 'a') error = true;
    if(!input.eof()) input.get(c); if(c != 'y') error = true;
    if(!input.eof()) input.get(c); if(c != 'b') error = true;
    if(!input.eof()) input.get(c); if(c != 'e') error = true;
    value = triMAYBE;
  }
  else
    error = true;

  if(error)
  {
    input.setpos(old_pos);
    throw gclRuntimeError("No boolean data found");
  }

  ((BoolPortion*) param[1])->SetValue(value);


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
    throw gclRuntimeError("End of file reached");
  }
  try {
    input >> value;
  }
  catch(gFileInput::ReadFailed &) {
    input.setpos(old_pos);
    throw gclRuntimeError("File read error");
  }

  ((NumberPortion*) param[1])->SetValue(value);

  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}

Portion* GSM_Read_Text(Portion** param)
{
  char c = ' ';
  gText s;
  gText t;
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  while(!input.eof() && isspace(c))
    input.get(c);
  if(input.eof())
  {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }
  if(!input.eof() && c == '\"')
    input.get(c); 
  else
  {
    input.unget(c);
    input.setpos(old_pos);
    throw gclRuntimeError("File read error: missing starting \"");
  }

  while(!input.eof() && c != '\"')
  { t+=c; input.get(c); }
  if(input.eof())
  {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }
  
  ((TextPortion*) param[1])->SetValue(t);
 

  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion* p = param[0];
  param[0] = p->RefCopy();
  return p;
}



Portion* GSM_Read_List(Portion** param, PortionSpec spec,
		       Portion* (*func) (Portion**), bool ListFormat)
{
  Portion* sub_param[2];
  ListPortion* list;
  char c = ' ';
  gInput& input = ((InputPortion*) param[0])->Value();
  list = ((ListPortion*) param[1]);
  long old_pos = input.getpos();


  while(!input.eof() && isspace(c))
    input.get(c);
  if(input.eof())
  {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
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
      throw gclRuntimeError("\'{\' expected");
    }
  }




  for(int i=1; i <= list->Length(); i++)
  {
    Portion* p;

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

    try  {
      if ((*list)[i]->Spec() == spec)
        p = (*func)(sub_param);
      else
        p = GSM_Read_List(sub_param, spec, func, ListFormat);
    }
    catch (...)  {
      input.setpos(old_pos);
      throw;
    }

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
  }

  if(ListFormat)
  {
    c = ' ';
    while(!input.eof() && isspace(c))
      input.get(c);
    if(c != '}')
    {
      input.setpos(old_pos);
      throw gclRuntimeError("Mismatched braces");
    }
    if(input.eof())
    {
      input.setpos(old_pos);
      throw gclRuntimeError("End of file reached");
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
  try  {
    Portion* p = GSM_Read_List(param, porBOOL, GSM_Read_Bool, false);
    delete temp;
    return p;
  }
  catch (gclRuntimeError &)  {
    ((ListPortion *) param[1])->AssignFrom(temp);
    delete temp;
    throw;
  }
}

Portion* GSM_Read_List_Number(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  try  {
    Portion* p = GSM_Read_List(param, porNUMBER, GSM_Read_Number, false);
    delete temp;
    return p;
  }
  catch (gclRuntimeError &)  {
    ((ListPortion *) param[1])->AssignFrom(temp);
    throw;
  }
}

Portion* GSM_Read_List_Text(Portion** param)
{
  Portion* temp = param[1]->ValCopy();
  try  {
    Portion* p = GSM_Read_List(param, porTEXT, GSM_Read_Text, false);
    delete temp;
    return p;
  }
  catch (gclRuntimeError &)  {
    ((ListPortion *) param[1])->AssignFrom(temp);
    throw;
  }
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

gText GetLine(gInput& f)
{
  char c = 0;
  gText result;
  bool valid = true;
  while(valid)
  {
    try{
    f >> c;
    }
    catch(gFileInput::ReadFailed &) {valid = false;}

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
  gText txt = ((TextPortion*) param[0])->Value();
  gOutput& s = ((OutputPortion*) param[1])->Value();
  ListPortion* Prototypes = (ListPortion*) _gsm->Help(txt, true, true, true);
  int i;
  int body;

  for(i=1; i<=Prototypes->Length(); i++)
  {
	 assert(Prototypes->Spec().Type == porTEXT);
	 s << ((TextPortion*) (*Prototypes)[i])->Value() << '\n';
  }


//  gText name = "gcl.man";   This gives problems on BC (rdm)
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
  gText ManFileName;

  ManFileName = (gText) name;
  f = 0;
  try {
    f = new gFileInput( ManFileName );
  }
  catch(gFileInput::OpenFailed &) {
    if( search ) {
      if(!man_found && (System::GetEnv( "HOME" ) != NULL))  {
        ManFileName = (gText) System::GetEnv( "HOME" ) + SLASH + name;
        try{
        f = new gFileInput( ManFileName );
        }
        catch(gFileInput::OpenFailed &) { f = NULL; }
        if(f) { man_found = true;}
      }
      if( !man_found && (System::GetEnv( "GCLLIB" ) != NULL) ) {
        ManFileName = (gText) System::GetEnv( "GCLLIB" ) + SLASH + name;
        try{
        f = new gFileInput( ManFileName );
        }
        catch(gFileInput::OpenFailed &) { f = NULL; }
        if (f) {man_found = true;}
      }
      if( !man_found && (SOURCE != NULL) ) {
        ManFileName = (gText) SOURCE + SLASH + name;
        try{
        f = new gFileInput( ManFileName );
        }
        catch(gFileInput::OpenFailed &) { f = NULL; }
        if (f) {man_found = true;}
      }
    }
  }

  // End bad section

  if (f == NULL)
    return new BoolPortion(false);

  gText line;
  gText line_out;
  bool found = false;
  bool valid = true;
  while(valid && !f->eof() && !found) {
    try{
      line = GetLine(*f);
    }
    catch(gFileInput::ReadFailed &) {valid=false;}
    if(line.Length() > txt.Length())
      if( line.Left(txt.Length() + 1).Dncase() == (txt + "[").Dncase() )
	found = true;
  }
  if(found) {
    body = 0;
    bool valid = true;
    while(valid && !f->eof()) {
      try{
	line = GetLine(*f);
      }
      catch(gFileInput::ReadFailed &) {valid = false;}
      
      if(line.Length()>=3 && line.Left(3) == "\\bd")
	body++;
      if(body > 0) {
	line_out = line;
	while(true) {
	  char* s;
	  int idx;
	  int numchars;
	  if((s=strstr((char *) line_out, "\\bd")) != 0)
	    numchars = 3;
	  else if((s=strstr((char *) line_out, "\\ed")) != 0)
	    numchars = 3;
	  else if((s=strstr((char *) line_out, "\\item")) != 0)
	    numchars = 5;
	  else if((s=strstr((char *) line_out, "\\tt")) != 0)
	    numchars = 4;
	  else if((s=strstr((char *) line_out, "\\em")) != 0)
	    numchars = 4;
	  else if((s=strstr((char *) line_out, "$")) != 0) {
	    idx = s - (char *) line_out;
	    line_out[idx] = '\'';
	    numchars = 0;
	  }
	  else if((s=strstr((char *) line_out, "\\verb")) != 0) {
	    numchars = 5;
	    idx = s - (char *) line_out;
	    for(i=0; i<numchars; i++) 
	      line_out.Remove(idx);
	    if(line_out.Length()>idx) {
	      char c;
	      c = line_out[idx];
	      line_out[idx] = '\"';
	      while(line_out.Length()>idx) {
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
	  idx = s - (char *) line_out;
	  if(idx>= 0) {    // this is necessary to prevent case where Remove() makes idx neg
	    for(i=0; i<numchars; i++) 
	      line_out.Remove(idx);
	    if(line_out.Length()>idx && line_out[idx] == ' ')
	      line_out.Remove(idx);
	  }
	}
	for(i=0; i<body; i++)
	  s << ' ';
	s << line_out << '\n';
      }
      if(line.Length()>=3 && line.Left(3) == "\\ed") {
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
  if( ((TextPortion*) param[0])->Value().Length() == 0 )
    throw gclRuntimeError( "Invalid environment variable name" );

  return 
    new TextPortion( System::GetEnv( ((TextPortion*) param[0])->Value() ) );
}

Portion* GSM_SetEnv( Portion** param )
{
  if( ((TextPortion*) param[0])->Value().Length() == 0 )
    throw gclRuntimeError( "Invalid environment variable name" );

  if (System::SetEnv(((TextPortion*) param[0])->Value(),
			               ((TextPortion*) param[1])->Value()) == 0)
    return new BoolPortion(true);
  else
    throw gclRuntimeError( "Insufficient environment space" );
}

Portion* GSM_UnSetEnv( Portion** param )
{
  if( ((TextPortion*) param[0])->Value().Length() == 0 )
    throw gclRuntimeError( "Invalid environment variable name" );

  if (System::UnSetEnv(((TextPortion*) param[0])->Value()) == 0)
    return new BoolPortion( true );
  else
    throw gclRuntimeError( "Insufficient environment space" );
}

Portion* GSM_Shell( Portion** param )
{
  gText str = ((TextPortion*) param[0])->Value();
  bool spawn = ((BoolPortion*) param[1])->Value();

  if (!spawn)  {
    if (str.Length() > 0)
      return new NumberPortion(System::Shell(str));
    else
      return new NumberPortion(System::Shell(0));
  }
  else  {
    if (str.Length() > 0)
      return new NumberPortion(System::Spawn(str));
    else
      return new NumberPortion(System::Spawn(0));
  }
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
  gText txt( _ExePath );

  if( file && path )
  {
  }
  else if( file )
  {
    if( txt.LastOccur( SLASH ) > 0 )
      txt = txt.Right( txt.Length() - txt.LastOccur( SLASH ) );
    else
      txt = "";
  }
  else if( path )
  {
    if( txt.LastOccur( SLASH ) > 0 )
      txt = txt.Left( txt.LastOccur( SLASH ) );
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


extern gStack<gText> GCL_InputFileNames;
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
    gText txt = GCL_InputFileNames.Peek();

    if( file && path )
    {
    }
    else if( file )
    {
      if( txt.LastOccur( SLASH ) > 0 )
	txt = txt.Right( txt.Length() - txt.LastOccur( SLASH ) );
      else
	txt = "";
    }
    else if( path )
    {
      if( txt.LastOccur( SLASH ) > 0 )
	txt = txt.Left( txt.LastOccur( SLASH ) );
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
  gText AscTime = asctime(localtime(&now));
  return new TextPortion(AscTime.Mid(11, 1) +
			    AscTime.Mid(4, 21) + ", " + AscTime.Mid(8, 12));
}


Portion* GSM_IsList( Portion** p )
{
  return new BoolPortion( p[0]->Spec().ListDepth > 0 );
}




Portion* GSM_SaveGlobalVar( Portion** param )
{
  const gText &varname = ((TextPortion*) param[0])->Value();
  _gsm->GlobalVarDefine( varname, 
			 param[1]->ValCopy() );
  return _gsm->GlobalVarValue( varname )->RefCopy();
}

Portion* GSM_LoadGlobalVar( Portion** param )
{
  const gText &varname = ((TextPortion*) param[0])->Value();
  if( _gsm->GlobalVarIsDefined( varname ) )
    return _gsm->GlobalVarValue( varname )->RefCopy();
  else
    throw gclRuntimeError( "Variable not found" );
}

Portion* GSM_ExistsGlobalVar( Portion** param )
{
  const gText &varname = ((TextPortion*) param[0])->Value();
  if( _gsm->GlobalVarIsDefined( varname ) )
    return new BoolPortion( true );
  else
    return new BoolPortion( false );
}


Portion* GSM_SaveLocalVar( Portion** param )
{
  gText varname =
    _gsm->UserFuncName() + gText((char) 1) + ((TextPortion*) param[0])->Value();
  _gsm->GlobalVarDefine( varname, 
			 param[1]->ValCopy() );
  return _gsm->GlobalVarValue( varname )->RefCopy();
}

Portion* GSM_LoadLocalVar( Portion** param )
{
  gText varname =
    _gsm->UserFuncName() + gText((char) 1) + ((TextPortion*) param[0])->Value();
  if( _gsm->GlobalVarIsDefined( varname ) )
    return _gsm->GlobalVarValue( varname )->RefCopy();
  else
    throw gclRuntimeError( "Variable not found" );
}


Portion* GSM_ExistsLocalVar( Portion** param )
{
  gText varname =
    _gsm->UserFuncName() + gText((char) 1) + ((TextPortion*) param[0])->Value();
  if( _gsm->GlobalVarIsDefined( varname ) )
    return new BoolPortion( true );

  else
    return new BoolPortion( false ); 
}






void Init_gsmoper(GSM* gsm)
{
  FuncDescObj* FuncObj;

  static struct { char *sig; Portion *(*func)(Portion **); } ftable[] =
    { { "And[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN", GSM_And },
      { "Concat[x->TEXT, y->TEXT] =: TEXT", GSM_Concat_Text },
      { "Divide[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Divide },
      { "Equal[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN", GSM_Equal_Boolean },
      { "Equal[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_Equal_Number },
      { "Equal[x->TEXT, y->TEXT] =: BOOLEAN", GSM_Equal_Text },
      { "Equal[x->EFG, y->EFG] =: BOOLEAN", GSM_Equal_Efg },
      { "Equal[x->EFPLAYER, y->EFPLAYER] =: BOOLEAN", 
	GSM_Equal_EfPlayer },
      { "Equal[x->NODE, y->NODE] =: BOOLEAN", GSM_Equal_Node },
      { "Equal[x->INFOSET, y->INFOSET] =: BOOLEAN", GSM_Equal_Infoset },
      { "Equal[x->EFOUTCOME, y->EFOUTCOME] =: BOOLEAN",
	GSM_Equal_EfOutcome },
      { "Equal[x->ACTION, y->ACTION] =: BOOLEAN", GSM_Equal_Action },
      { "Equal[x->EFSUPPORT, y->EFSUPPORT] =: BOOLEAN", 
	GSM_Equal_EfSupport },
      { "Equal[x->BEHAV, y->BEHAV] =: BOOLEAN", GSM_Equal_Behav },
      { "Equal[x->NFG, y->NFG] =: BOOLEAN", GSM_Equal_Nfg },
      { "Equal[x->NFPLAYER, y->NFPLAYER] =: BOOLEAN",
	GSM_Equal_NfPlayer },
      { "Equal[x->STRATEGY, y->STRATEGY] =: BOOLEAN",
	GSM_Equal_Strategy },
      { "Equal[x->NFOUTCOME, y->NFOUTCOME] =: BOOLEAN",
	GSM_Equal_NfOutcome },
      { "Equal[x->NFSUPPORT, y->NFSUPPORT] =: BOOLEAN",
	GSM_Equal_NfSupport },
      { "Equal[x->MIXED, y->MIXED] =: BOOLEAN", GSM_Equal_Mixed },
      { "Exp[x->NUMBER] =: NUMBER", GSM_Exp },
      { "Greater[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_Greater_Number },
      { "Greater[x->TEXT, y->TEXT] =: BOOLEAN", GSM_Greater_Text },
      { "GreaterEqual[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_GreaterEqual_Number },
      { "GreaterEqual[x->TEXT, y->TEXT] =: BOOLEAN", GSM_GreaterEqual_Text },
      { "Less[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_Less_Number },
      { "Less[x->TEXT, y->TEXT] =: BOOLEAN", GSM_Less_Text },
      { "LessEqual[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_LessEqual_Number },
      { "LessEqual[x->TEXT, y->TEXT] =: BOOLEAN", GSM_LessEqual_Text },
      { "Log[x->NUMBER] =: NUMBER", GSM_Log },
      { "Minus[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Minus_Number },
      { "Minus[x->MIXED, y->MIXED] =: MIXED", GSM_Minus_Mixed },
      { "Minus[x->BEHAV, y->BEHAV] =: BEHAV", GSM_Minus_Behav },
      { "Modulus[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Modulus },
      { "Negate[x->NUMBER] =: NUMBER", GSM_Negate },
      { "Not[x->BOOLEAN] =: BOOLEAN", GSM_Not },
      { "NotEqual[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN", GSM_NotEqual_Boolean },
      { "NotEqual[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_NotEqual_Number },
      { "NotEqual[x->TEXT, y->TEXT] =: BOOLEAN", GSM_NotEqual_Text },
      { "NotEqual[x->EFG, y->EFG] =: BOOLEAN", GSM_NotEqual_Efg },
      { "NotEqual[x->EFPLAYER, y->EFPLAYER] =: BOOLEAN", 
	GSM_NotEqual_EfPlayer },
      { "NotEqual[x->NODE, y->NODE] =: BOOLEAN", GSM_NotEqual_Node },
      { "NotEqual[x->INFOSET, y->INFOSET] =: BOOLEAN", GSM_NotEqual_Infoset },
      { "NotEqual[x->EFOUTCOME, y->EFOUTCOME] =: BOOLEAN",
	GSM_NotEqual_EfOutcome },
      { "NotEqual[x->ACTION, y->ACTION] =: BOOLEAN", GSM_NotEqual_Action },
      { "NotEqual[x->EFSUPPORT, y->EFSUPPORT] =: BOOLEAN", 
	GSM_NotEqual_EfSupport },
      { "NotEqual[x->BEHAV, y->BEHAV] =: BOOLEAN", GSM_NotEqual_Behav },
      { "NotEqual[x->NFG, y->NFG] =: BOOLEAN", GSM_NotEqual_Nfg },
      { "NotEqual[x->NFPLAYER, y->NFPLAYER] =: BOOLEAN",
	GSM_NotEqual_NfPlayer },
      { "NotEqual[x->STRATEGY, y->STRATEGY] =: BOOLEAN",
	GSM_NotEqual_Strategy },
      { "NotEqual[x->NFOUTCOME, y->NFOUTCOME] =: BOOLEAN",
	GSM_NotEqual_NfOutcome },
      { "NotEqual[x->NFSUPPORT, y->NFSUPPORT] =: BOOLEAN",
	GSM_NotEqual_NfSupport },
      { "NotEqual[x->MIXED, y->MIXED] =: BOOLEAN", GSM_NotEqual_Mixed },
      { "Or[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN", GSM_Or },
      { "Parentheses[x->ANYTYPE] =: ANYTYPE", GSM_Parentheses },
      { "Plus[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Plus_Number },
      { "Plus[x->MIXED, y->MIXED] =: MIXED", GSM_Plus_Mixed },
      { "Plus[x->BEHAV, y->BEHAV] =: BEHAV", GSM_Plus_Behav },
      { "Power[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Power },
      { "Times[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Times_Number },
      { "Times[x->NUMBER, y->MIXED] =: MIXED", GSM_Times_Mixed },
      { "Times[x->NUMBER, y->BEHAV] =: BEHAV", GSM_Times_Behav },
      { "Version[] =: NUMBER", GSM_Version },
      { 0, 0 }
    };

  for (int i = 0; ftable[i].sig != 0; i++)
    gsm->AddFunction(new FuncDescObj(ftable[i].sig, ftable[i].func));

  //-------------------- Streams -------------------------

  FuncObj = new FuncDescObj("Output", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Output, porOUTPUT, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("file", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("append", porBOOL,
					    new BoolPortion( false )));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("Input", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Input, porINPUT, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("file", porTEXT));
  gsm->AddFunction(FuncObj);

  //------------------- Formatted Output -------------------

  GSM_SetWriteOptions();

  FuncObj = new FuncDescObj("Print", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Print, porANYTYPE, 1, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("x",
					    PortionSpec(porANYTYPE, 0, 
							porNULLSPEC)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Print, PortionSpec(porANYTYPE, NLIST), 1, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", 
					    PortionSpec(porANYTYPE, NLIST,
						      porNULLSPEC)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Write", 11);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Write, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter
			("x", porBOOL | porNUMBER | porNUMBER));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Write, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, gclParameter("x", porTEXT));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_Write, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, gclParameter("x", porMIXED));

  FuncObj->SetFuncInfo(3, gclSignature(GSM_Write,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(3, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, gclParameter("x", porBEHAV));
  
  FuncObj->SetFuncInfo(4, gclSignature(GSM_Write_Nfg, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(4, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(4, 1, gclParameter("x", porNFG,
					    REQUIRED ));
  
  FuncObj->SetFuncInfo(5, gclSignature(GSM_Write_Efg,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(5, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(5, 1, gclParameter("x", porEFG));

  FuncObj->SetFuncInfo(6, gclSignature(GSM_Write,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(6, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(6, 1, gclParameter
			("x", PortionSpec(porBOOL | porNUMBER |
					  porTEXT | porMIXED | porBEHAV, 1)));

  FuncObj->SetFuncInfo(7, gclSignature(GSM_Write,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(7, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(7, 1, gclParameter("x", porNFSUPPORT));

  FuncObj->SetFuncInfo(8, gclSignature(GSM_Write,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(8, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(8, 1, gclParameter("x", porEFSUPPORT));

  FuncObj->SetFuncInfo(9, gclSignature(GSM_Write,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(9, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(9, 1, gclParameter("x", porSTRATEGY));

  FuncObj->SetFuncInfo(10, gclSignature(GSM_Write,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(10, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(10, 1, gclParameter("x", porEFBASIS));

  gsm->AddFunction(FuncObj);




  //---------------
  // *Format
  //---------------

  FuncObj = new FuncDescObj("ListFormat", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ListFormat, porBOOL, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter
			("braces", porBOOL,
			 _WriteListBraces.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter
			("commas", porBOOL,
			 _WriteListCommas.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("lf", porNUMBER,
			 _WriteListLF.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("indent", porNUMBER,
			 _WriteListIndent.RefCopy(), BYREF ));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GetListFormat", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_GetListFormat, porBOOL, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter
			("braces", porBOOL,
			 _WriteListBraces.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter
			("commas", porBOOL,
			 _WriteListCommas.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("lf", porNUMBER,
			 _WriteListLF.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("indent", porNUMBER,
			 _WriteListIndent.RefCopy(), BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Format", 3);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_FloatFormat, porNUMBER, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER) );
  FuncObj->SetParamInfo(0, 1, gclParameter
			("width", porNUMBER, 
			 _WriteWidth.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("precis", porNUMBER,
			 _WritePrecis.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("expmode", porBOOL,
			 _WriteExpmode.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_TextFormat, porTEXT, 2));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", porTEXT) );
  FuncObj->SetParamInfo(1, 1, gclParameter
			("quote", porBOOL,
			 _WriteQuoted.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_SolutionFormat,
				       porBEHAV | porMIXED, 2));
  FuncObj->SetParamInfo(2, 0, gclParameter("x", porBEHAV | porMIXED) );
  FuncObj->SetParamInfo(2, 1, gclParameter
			("info", porNUMBER,
			 _WriteSolutionInfo.RefCopy(), BYREF));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GetFormat", 3);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_GetFloatFormat, porNUMBER, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER) );
  FuncObj->SetParamInfo(0, 1, gclParameter
			("width", porNUMBER, 
			 _WriteWidth.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("precis", porNUMBER,
			 _WritePrecis.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("expmode", porBOOL,
			 _WriteExpmode.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_GetTextFormat, porTEXT, 2));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", porTEXT) );
  FuncObj->SetParamInfo(1, 1, gclParameter
			("quote", porBOOL,
			 _WriteQuoted.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_GetSolutionFormat,
				       porBEHAV | porMIXED, 2));
  FuncObj->SetParamInfo(2, 0, gclParameter("x", porBEHAV | porMIXED) );
  FuncObj->SetParamInfo(2, 1, gclParameter
			("info", porNUMBER,
			 _WriteSolutionInfo.RefCopy(), BYREF));
  gsm->AddFunction(FuncObj);



  //-------------------- Read --------------------------

  FuncObj = new FuncDescObj("Read", 6);

  FuncObj->SetFuncInfo(0, gclSignature(GSM_Read_Bool, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("input", porINPUT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter("x", porBOOL, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Read_List_Bool, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, gclParameter("x", PortionSpec(porBOOL,1), 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_Read_Number,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, gclParameter("x", porNUMBER,
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(3, gclSignature(GSM_Read_List_Number,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(3, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, gclParameter("x", PortionSpec(porNUMBER,1),
					    REQUIRED, BYREF));
  
  
  FuncObj->SetFuncInfo(4, gclSignature(GSM_Read_Text,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(4, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(4, 1, gclParameter("x", porTEXT,
					    REQUIRED, BYREF));
  
  FuncObj->SetFuncInfo(5, gclSignature(GSM_Read_List_Text,
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(5, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(5, 1, gclParameter("x", PortionSpec(porTEXT,1),
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Help", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Help, PortionSpec(porTEXT, 1), 3));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("udf", porBOOL,
					    new BoolPortion( true )));
  FuncObj->SetParamInfo(0, 2, gclParameter("bif", porBOOL, 
					    new BoolPortion( true )));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Manual", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Manual, porBOOL, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("y", porOUTPUT,
					    new OutputPortion(gout)));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new FuncDescObj("HelpVars", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_HelpVars, PortionSpec(porTEXT, 1),
				       1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Clear", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Clear, porBOOL, 0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Date", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Date, porTEXT, 0));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Randomize", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Randomize, porNUMBER, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER));
  FuncObj->SetParamInfo(0, 1, gclParameter("seed", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Precision", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Precision, porPRECISION, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER));

  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsNull", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsNull, porBOOL, 1, 0, 
				       funcLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", 
					    PortionSpec(porANYTYPE, 0, 
							porNULLSPEC )));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Null", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Null, 
				       PortionSpec(porANYTYPE, 0, 
						   porNULLSPEC ),
				       1, 0, funcLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porANYTYPE));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("Shell", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Shell, porNUMBER, 2 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("command", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 1, gclParameter("spawn", porBOOL, 
					    new BoolPortion(true)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("GetEnv", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_GetEnv, porTEXT, 1 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("SetEnv", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_SetEnv, porBOOL, 2 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT ) );
  FuncObj->SetParamInfo(0, 1, gclParameter("value", porTEXT ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("UnSetEnv", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_UnSetEnv, porBOOL, 1 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT ) );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("ExePath", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ExePath, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("file", porBOOL,
					    new BoolPortion( true ) ) );
  FuncObj->SetParamInfo(0, 1, gclParameter("path", porBOOL,
					    new BoolPortion( true ) ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Platform", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Platform, porTEXT, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("GetPath", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_GetPath, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("file", porBOOL,
					    new BoolPortion( true ) ) );
  FuncObj->SetParamInfo(0, 1, gclParameter("path", porBOOL,
					    new BoolPortion( true ) ) );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("IsList", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsList, porBOOL, 1,
				       NO_PREDEFINED_PARAMS, 
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porANYTYPE ));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_IsList, porBOOL, 1, 
				       NO_PREDEFINED_PARAMS, 
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(1, 0, gclParameter("list", 
					    PortionSpec(porANYTYPE,NLIST)));
  gsm->AddFunction(FuncObj);



  FuncObj = new FuncDescObj("SaveGlobalVar", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_SaveGlobalVar, porANYTYPE, 2,
				       NO_PREDEFINED_PARAMS,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT) );
  FuncObj->SetParamInfo(0, 1, gclParameter("var", porANYTYPE) );

  FuncObj->SetFuncInfo(1, gclSignature(GSM_SaveGlobalVar, 
				       PortionSpec(porANYTYPE, NLIST), 2,
				       NO_PREDEFINED_PARAMS,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(1, 0, gclParameter("name", porTEXT) );
  FuncObj->SetParamInfo(1, 1, gclParameter("var", 
					    PortionSpec(porANYTYPE, NLIST)) );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LoadGlobalVar", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_LoadGlobalVar, porANYTYPE, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ExistsGlobalVar", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ExistsGlobalVar, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT) );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("SaveLocalVar", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_SaveLocalVar, porANYTYPE, 2,
				       NO_PREDEFINED_PARAMS,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT) );
  FuncObj->SetParamInfo(0, 1, gclParameter("var", porANYTYPE) );

  FuncObj->SetFuncInfo(1, gclSignature(GSM_SaveLocalVar, 
				       PortionSpec(porANYTYPE, NLIST), 2,
				       NO_PREDEFINED_PARAMS,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(1, 0, gclParameter("name", porTEXT) );
  FuncObj->SetParamInfo(1, 1, gclParameter("var", 
					    PortionSpec(porANYTYPE, NLIST)) );
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("LoadLocalVar", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_LoadLocalVar, porANYTYPE, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT) );
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ExistsLocalVar", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ExistsLocalVar, porBOOL, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT) );
  gsm->AddFunction(FuncObj);


}







