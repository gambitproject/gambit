//
// FILE: gsmoper.cc -- implementations for GSM operator functions
//                     companion to GSM
//
// $Id$
//

#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#ifndef __BORLANDC__
#include <sys/time.h>
#endif
#include <time.h>

#include "base/base.h"
#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "math/rational.h"

#include "game/mixedsol.h"
#include "game/behavsol.h"

#include "game/nfg.h"
#include "game/efg.h"

#include "base/system.h"
#include "base/base.h"


#include "gcmdline.h"


//------------------------------------
// Utilities for formatting options
//------------------------------------

NumberPortion _WriteWidth(0);
NumberPortion _WritePrecis(6);
BoolPortion _WriteExpmode(triFALSE);
BoolPortion _WriteQuoted(triTRUE);
BoolPortion _WriteListBraces(triTRUE);
BoolPortion _WriteListCommas(triTRUE);
NumberPortion _WriteListLF(0);
NumberPortion _WriteListIndent(2);
BoolPortion _WriteSolutionInfo(triFALSE);
BoolPortion _WriteSolutionLabels(triFALSE);
NumberPortion _WriteGameDecimals(6);

static void GSM_SetWriteOptions(void)
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
  Portion::_SetWriteSolutionLabels(_WriteSolutionLabels.Value());

  ToTextWidth(_WriteWidth.Value());
  ToTextPrecision(_WritePrecis.Value());
}

//-------
// And
//-------

static Portion *GSM_And(GSM &, Portion **param)
{
  gTriState x = ((BoolPortion *) param[0])->Value();
  gTriState y = ((BoolPortion *) param[1])->Value();

  if (x == triTRUE && y == triTRUE)    
    return new BoolPortion(triTRUE);
  else if (x == triFALSE || y == triFALSE)
    return new BoolPortion(triFALSE); 
  else
    return new BoolPortion(triUNKNOWN);  
}

//---------
// Clear
//---------

static Portion *GSM_Clear(GSM &gsm, Portion **)
{
  gsm.Clear();
  return new BoolPortion(true);
}

//----------
// Divide
//----------

static Portion *GSM_Divide(GSM &, Portion **param)
{
  if(((NumberPortion*) param[1])->Value() != (gNumber)0)
    return new NumberPortion(((NumberPortion*) param[0])->Value() /
			     ((NumberPortion*) param[1])->Value());
  else
    return new NullPortion(porNUMBER);
}

//--------
// Equal
//--------

static Portion *GSM_Equal_Number(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((NumberPortion *) param[0])->Value() ==
			   ((NumberPortion *) param[1])->Value());
}

static Portion *GSM_Equal_Text(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((TextPortion *) param[0])->Value() ==
			   ((TextPortion *) param[1])->Value());
}

static Portion *GSM_Equal_Boolean(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((BoolPortion *) param[0])->Value() ==
			   ((BoolPortion *) param[1])->Value());
}

static Portion *GSM_Equal_Efg(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((EfgPortion *) param[0])->Value() ==
			   ((EfgPortion *) param[1])->Value());
}

static Portion *GSM_Equal_EfPlayer(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((EfPlayerPortion *) param[0])->Value() ==
			   ((EfPlayerPortion *) param[1])->Value());
}

static Portion *GSM_Equal_Node(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((NodePortion *) param[0])->Value() ==
			   ((NodePortion *) param[1])->Value());
}

static Portion *GSM_Equal_Infoset(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((InfosetPortion *) param[0])->Value() ==
			   ((InfosetPortion *) param[1])->Value());
}

static Portion *GSM_Equal_EfOutcome(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((EfOutcomePortion *) param[0])->Value() ==
			   ((EfOutcomePortion *) param[1])->Value());
}

static Portion *GSM_Equal_Action(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((ActionPortion *) param[0])->Value() ==
			   ((ActionPortion *) param[1])->Value());
}

static Portion *GSM_Equal_EfSupport(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion((*((EfSupportPortion *) param[0])->Value()) ==
			   (*((EfSupportPortion *) param[1])->Value()));
}

static Portion *GSM_Equal_Behav(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(*((BehavPortion *) param[0])->Value() ==
			   *((BehavPortion *) param[1])->Value());
}

static Portion *GSM_Equal_Nfg(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((NfgPortion *) param[0])->Value() ==
			   ((NfgPortion *) param[1])->Value());

}

static Portion *GSM_Equal_NfPlayer(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((NfPlayerPortion *) param[0])->Value() ==
			   ((NfPlayerPortion *) param[1])->Value());
}

static Portion *GSM_Equal_Strategy(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((StrategyPortion *) param[0])->Value() ==
			   ((StrategyPortion *) param[1])->Value());
}

static Portion *GSM_Equal_NfOutcome(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(((NfOutcomePortion *) param[0])->Value() ==
			   ((NfOutcomePortion *) param[1])->Value());
}

static Portion *GSM_Equal_NfSupport(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion((*((NfSupportPortion *) param[0])->Value()) ==
			   (*((NfSupportPortion *) param[1])->Value()));
}

static Portion *GSM_Equal_Mixed(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type == param[1]->Spec().Type);
  else
    return new BoolPortion(*((MixedPortion *) param[0])->Value() ==
			   *((MixedPortion *) param[1])->Value());
}


//--------
// Exp
//--------

static Portion *GSM_Exp(GSM &, Portion **param)
{
  return new NumberPortion
    (exp((double) ((NumberPortion *) param[0])->Value()));
}

//---------
// Float
//---------

static Portion *GSM_Float(GSM &, Portion **param)
{
  return new NumberPortion((double) ((NumberPortion *) param[0])->Value());
}

//------------
// Greater
//------------

static Portion *GSM_Greater_Number(GSM &, Portion **param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() >
                         ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_Greater_Text(GSM &, Portion **param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() >
			 ((TextPortion*) param[1])->Value());
}

//---------------
// GreaterEqual
//---------------

static Portion *GSM_GreaterEqual_Number(GSM &, Portion **param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() >=
                         ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_GreaterEqual_Text(GSM &, Portion **param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() >=
			 ((TextPortion*) param[1])->Value());
}

//--------
// Help
//--------

static Portion *GSM_Help(GSM &gsm, Portion **param)
{
  return gsm.Help(((TextPortion*) param[0])->Value(),
		  ((BoolPortion*) param[1])->Value(),
		  ((BoolPortion*) param[2])->Value());
}

//------------
// HelpVars
//------------

static Portion *GSM_HelpVars(GSM &gsm, Portion **param)
{
  return gsm.HelpVars(((TextPortion*) param[0])->Value());
}


//--------
// Input
//--------

static Portion *GSM_Input(GSM &, Portion **param)
{
  try {
    return new InputPortion(*new gFileInput(((TextPortion*) param[0])->Value()));
  }
  catch (gFileInput::OpenFailed &) {
    throw gclRuntimeError((gText) "Error opening file \"" +
			  ((TextPortion*) param[0])->Value() + "\"");
  }
}

//-----------------
// IntegerDivide
//-----------------

static Portion *GSM_IntegerDivide(GSM &, Portion **param)
{
  if (((NumberPortion *) param[1])->Value() != (gNumber) 0) {
    gRational x = ((NumberPortion *) param[0])->Value().operator gRational();
    gRational y = ((NumberPortion *) param[1])->Value().operator gRational();
    return new NumberPortion(x.numerator() / y.numerator());
  }
  else
    return new NullPortion(porNUMBER);
}

//-------------
// IsDefined
//-------------

static Portion *GSM_IsDefined(GSM &gsm, Portion **param)
{
  if (param[0]->Spec().Type != porREFERENCE)
    return new BoolPortion(true);
  else
    return new BoolPortion(gsm.VarIsDefined(((ReferencePortion *) param[0])->Value()));
}

static Portion *GSM_IsDefined_Undefined(GSM &, Portion **)
{
  return new BoolPortion(false);
}

//--------
// IsEof
//--------

static Portion *GSM_IsEof(GSM &, Portion **param)
{
  return new BoolPortion(((InputPortion *) param[0])->Value().eof());
}

//---------
// IsNull
//---------

static Portion *GSM_IsNull(GSM &, Portion **param)
{
  return new BoolPortion(param[0]->Spec().Type == porNULL);
}

//---------
// Less
//---------

static Portion *GSM_Less_Number(GSM &, Portion **param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() <
                         ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_Less_Text(GSM &, Portion **param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() <
			 ((TextPortion*) param[1])->Value());
}

//-------------
// LessEqual
//-------------

static Portion *GSM_LessEqual_Number(GSM &, Portion **param)
{
  return new BoolPortion(((NumberPortion*) param[0])->Value() <=
                         ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_LessEqual_Text(GSM &, Portion **param)
{
  return new BoolPortion(((TextPortion*) param[0])->Value() <=
			 ((TextPortion*) param[1])->Value());
}

//--------
// Log
//--------

static Portion *GSM_Log(GSM &, Portion **param)
{
  double d = (double) ((NumberPortion *) param[0])->Value();
  if(d <= 0.0)
    return new NullPortion(porNUMBER);
  else
    return new NumberPortion(log(d));
}


//---------
// Manual
//---------

static gText GetLine(gInput &f)
{
  char c = 0;
  gText result;
  bool valid = true;
  while (valid) {
    try  {
    f >> c;
    }
    catch (gFileInput::ReadFailed &) {
      valid = false;
    }

    if (f.eof())
      break;
    if (c != '\n')
      result += c;
    else
      break;
  }
  return result;
}

static Portion *GSM_Manual(GSM &gsm, Portion** param)
{
  gText txt = ((TextPortion*) param[0])->Value();
  gOutput& s = ((OutputPortion*) param[1])->Value();
  ListPortion *Prototypes = (ListPortion*) gsm.Help(txt, true, true, true);
  int i;
  int body = 0;

  for (i = 1; i <= Prototypes->Length(); i++) {
    s << ((TextPortion*) (*Prototypes)[i])->Value() << '\n';
  }


  char *name = "gcl.man";    // this change necessary for BC
  gFileInput* f = NULL;

  // This section is very inelegantly adopted from gcompile.yy...
  // This section and its gcompile.yy parallel should be converted into
  // one function...

  extern char* _SourceDir;
  const char* SOURCE = _SourceDir;

#ifdef __GNUG__
  const char SLASH1= '/';
  const char SLASH = '/';
#elif defined __BORLANDC__
  const char SLASH1= '\\';
  const char * SLASH = "\\";
#endif   // __GNUG__

  bool search = false;
  if (strchr( name, SLASH1 ) == NULL)
    search = true;
  gText ManFileName;

  ManFileName = (gText) name;
  bool man_found = 0;
  f = 0;
  try {
    f = new gFileInput(ManFileName);
  }
  catch (gFileInput::OpenFailed &) {
    if (search) {
      if (!man_found && System::GetEnv("HOME") != NULL) {
        ManFileName = ((gText) System::GetEnv("HOME")) + gText(SLASH) + name;
        try  {
          f = new gFileInput(ManFileName);
        }
        catch(gFileInput::OpenFailed &) { f = NULL; }
        if(f) { man_found = true;}
      }
      if( !man_found && (System::GetEnv( "GCLLIB" ) != NULL) ) {
        ManFileName = (gText) System::GetEnv("GCLLIB") + gText(SLASH) + name;
        try{
        f = new gFileInput( ManFileName );
        }
        catch(gFileInput::OpenFailed &) { f = NULL; }
        if (f) {man_found = true;}
      }
      if( !man_found && (SOURCE != NULL) ) {
        ManFileName = (gText) SOURCE + gText(SLASH) + name;
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
	  unsigned int idx;
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

  delete f;

  return new BoolPortion(found);
}

//---------
// Minus
//---------

static Portion *GSM_Minus_Number(GSM &, Portion **param)
{
  return new NumberPortion(((NumberPortion *) param[0])->Value() -
                           ((NumberPortion *) param[1])->Value());
}

static Portion *GSM_Minus_Mixed(GSM &, Portion **param)
{
  MixedPortion *result = new MixedPortion(new MixedSolution(*((MixedPortion *) param[0])->Value()));
  *result->Value() -= *((MixedPortion*) param[1])->Value();
  return result;
}

static Portion *GSM_Minus_Behav(GSM &, Portion** param)
{
  if(((BehavPortion*) param[0])->Value()->Support() !=
     ((BehavPortion*) param[1])->Value()->Support())
    throw gclRuntimeError("Support mismatch");

  BehavPortion *result = new BehavPortion(new BehavSolution(*((BehavPortion *) param[0])->Value()));
  *result->Value() -= *((BehavPortion*) param[1])->Value();
  return result;
}



//-----------
// Modulus
//-----------

static Portion *GSM_Modulus(GSM &, Portion **param)
{
  if (((NumberPortion *) param[1])->Value() != (gNumber) 0) {
    gRational x = ((NumberPortion *) param[0])->Value().operator gRational();
    gRational y = ((NumberPortion *) param[1])->Value().operator gRational();
    return new NumberPortion(x.numerator() % y.numerator());
  }
  else
    return new NullPortion(porNUMBER);
}

//----------
// Negate
//----------

static Portion *GSM_Negate(GSM &, Portion **param)
{
  return new NumberPortion(-((NumberPortion *) param[0])->Value());
}

//------
// Not
//------

static Portion *GSM_Not(GSM &, Portion **param)
{
  gTriState x = ((BoolPortion *) param[0])->Value();

  if (x == triTRUE)
    return new BoolPortion(triFALSE);
  else if (x == triFALSE)
    return new BoolPortion(triTRUE);
  else
    return new BoolPortion(triUNKNOWN); 
}

//-----------
// NotEqual
//-----------

static Portion *GSM_NotEqual_Number(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((NumberPortion *) param[0])->Value() !=
			   ((NumberPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Text(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((TextPortion *) param[0])->Value() !=
			   ((TextPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Boolean(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((BoolPortion *) param[0])->Value() !=
			   ((BoolPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Efg(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((EfgPortion *) param[0])->Value() !=
			   ((EfgPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_EfPlayer(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((EfPlayerPortion *) param[0])->Value() !=
			   ((EfPlayerPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Node(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((NodePortion *) param[0])->Value() !=
			   ((NodePortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Infoset(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((InfosetPortion *) param[0])->Value() !=
			   ((InfosetPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_EfOutcome(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((EfOutcomePortion *) param[0])->Value() !=
			   ((EfOutcomePortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Action(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((ActionPortion *) param[0])->Value() !=
			   ((ActionPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_EfSupport(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion((*((EfSupportPortion *) param[0])->Value()) !=
			   (*((EfSupportPortion *) param[1])->Value()));
}

static Portion *GSM_NotEqual_Behav(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(*((BehavPortion *) param[0])->Value() !=
			   *((BehavPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Nfg(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((NfgPortion *) param[0])->Value() !=
			   ((NfgPortion *) param[1])->Value());

}

static Portion *GSM_NotEqual_NfPlayer(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((NfPlayerPortion *) param[0])->Value() !=
			   ((NfPlayerPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_Strategy(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((StrategyPortion *) param[0])->Value() !=
			   ((StrategyPortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_NfOutcome(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(((NfOutcomePortion *) param[0])->Value() !=
			   ((NfOutcomePortion *) param[1])->Value());
}

static Portion *GSM_NotEqual_NfSupport(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion((*((NfSupportPortion *) param[0])->Value()) !=
			   (*((NfSupportPortion *) param[1])->Value()));
}

static Portion *GSM_NotEqual_Mixed(GSM &, Portion **param)
{
  if ((param[0]->Spec().Type == porNULL) || (param[1]->Spec().Type == porNULL))
    return new BoolPortion(param[0]->Spec().Type != param[1]->Spec().Type);
  else
    return new BoolPortion(*((MixedPortion *) param[0])->Value() !=
			   *((MixedPortion *) param[1])->Value());
}

//-------
// Null
//-------

static Portion *GSM_Null(GSM &, Portion** param)
{
  return new NullPortion(param[0]->Spec().Type);
}

//-----
// Or
//-----

static Portion *GSM_Or(GSM &, Portion** param)
{
  gTriState x = ((BoolPortion *) param[0])->Value();
  gTriState y = ((BoolPortion *) param[1])->Value();

  if (x == triTRUE || y == triTRUE)    
    return new BoolPortion(triTRUE);
  else if (x == triUNKNOWN || y == triUNKNOWN)
    return new BoolPortion(triUNKNOWN); 
  else
    return new BoolPortion(triFALSE);
}

//----------
// Output
//----------

static Portion *GSM_Output(GSM &, Portion** param)
{
  gText filename = ((TextPortion*) param[0])->Value();
  bool append = ((BoolPortion*) param[1])->Value();
  bool close = ((BoolPortion*) param[2])->Value();

  try  {
    return new OutputPortion(*new gFileOutput(filename, append, close));
  }
  catch (gFileOutput::OpenFailed &) {
    throw gclRuntimeError((gText) "Error opening file \"" +
			  ((TextPortion*) param[0])->Value() + "\"");
  }
}

//--------------
// Parentheses
//--------------

static Portion *GSM_Parentheses(GSM &, Portion **param)
{
  return param[0]->ValCopy();
}

//-------
// Plus
//-------

static Portion *GSM_Plus_Number(GSM &, Portion** param)
{
  return new NumberPortion(((NumberPortion*) param[0])->Value() +
                           ((NumberPortion*) param[1])->Value());
}

static Portion *GSM_Plus_Text(GSM &, Portion** param)
{
  return new TextPortion(((TextPortion*) param[0])->Value() +
			 ((TextPortion*) param[1])->Value());
}

static Portion *GSM_Plus_Mixed(GSM &, Portion** param)
{
  MixedPortion *result = new MixedPortion(new MixedSolution(*((MixedPortion *) param[0])->Value()));
  *result->Value() += *((MixedPortion*) param[1])->Value();
  return result;
}

static Portion *GSM_Plus_Behav(GSM &, Portion** param)
{
  if(((BehavPortion*) param[0])->Value()->Support() !=
     ((BehavPortion*) param[1])->Value()->Support())
    throw gclRuntimeError("Support mismatch");

  BehavPortion *result = new BehavPortion(new BehavSolution(*((BehavPortion *) param[0])->Value()));
  *result->Value() += *((BehavPortion*) param[1])->Value();
  return result;
}

//----------
// Power
//----------

static Portion *GSM_Power(GSM &, Portion** param)
{
  gNumber base = ((NumberPortion*) param[0])->Value();
  gNumber exponent = ((NumberPortion*) param[1])->Value();

  if (base.Precision() == precDOUBLE || 
      exponent.Precision() == precDOUBLE)
    return new NumberPortion(pow((double) base, (double) exponent));
  else if (base.Precision() == precRATIONAL && exponent.IsInteger())
    return new NumberPortion(pow(base.operator gRational(),
				 exponent.operator gRational().numerator()));
  else
    throw gclRuntimeError("Not implemented for rational base with non-integer exponent");
}

//-------------
// Precision
//-------------

static Portion *GSM_Precision(GSM &, Portion** param)
{
  return new PrecisionPortion(((NumberPortion *) param[0])->Value().Precision());
}

//--------
// Print
//--------

static Portion *GSM_Print(GSM &gsm, Portion **param)
{
  param[0]->Output(gsm.OutputStream());
  gsm.OutputStream() << '\n';
  return param[0]->ValCopy();
}

//-------
// Quit
//-------

static Portion *GSM_Quit(GSM &, Portion **param)
{
  throw gclQuitOccurred(((NumberPortion *) param[0])->Value());
}

//------------
// Randomize
//------------

long _idum = -1;

static Portion *GSM_Randomize(GSM &, Portion** param)
{
  long _RandomSeed = ((NumberPortion*) param[1])->Value();
  if(_RandomSeed > 0)
    _RandomSeed = -_RandomSeed;
  long v;
  if(_RandomSeed != 0)
    _idum = _RandomSeed;
  v = ran1(&_idum);
  gRational x(v,(long)IM);
  return new NumberPortion(x);
}

//-----------
// Rational
//-----------

static Portion *GSM_Rational(GSM &, Portion **param)
{
  return new NumberPortion
    (((NumberPortion *) param[0])->Value().operator gRational());
}

//--------
// Read
//--------

static Portion *GSM_Read_Bool(GSM &, Portion** param)
{
  gInput &input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();
  gTriState value = triUNKNOWN;
  bool error = false;
  char c = ' ';

  if (input.eof()) {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }

  while (!input.eof() && isspace(c))
    input.get(c);
  
  if (c == 'T')  {
    if (!input.eof()) input.get(c); if (c != 'r') error = true;
    if (!input.eof()) input.get(c); if (c != 'u') error = true;
    if (!input.eof()) input.get(c); if (c != 'e') error = true;
    value = triTRUE;
  }
  else if (c == 'F')  {
    if (!input.eof()) input.get(c); if (c != 'a') error = true;
    if (!input.eof()) input.get(c); if (c != 'l') error = true;
    if (!input.eof()) input.get(c); if (c != 's') error = true;
    if (!input.eof()) input.get(c); if (c != 'e') error = true;
    value = triFALSE;
  }
  else if (c == 'U')  {
    if (!input.eof()) input.get(c); if (c != 'n') error = true;
    if (!input.eof()) input.get(c); if (c != 'k') error = true;
    if (!input.eof()) input.get(c); if (c != 'n') error = true;
    if (!input.eof()) input.get(c); if (c != 'o') error = true;
    if (!input.eof()) input.get(c); if (c != 'w') error = true;
    if (!input.eof()) input.get(c); if (c != 'n') error = true;
    value = triUNKNOWN;
  }
  else
    error = true;

  if (error)  {
    input.setpos(old_pos);
    throw gclRuntimeError("No BOOLEAN data found");
  }

  ((BoolPortion*) param[1])->SetValue(value);

  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion *p = param[0];
  param[0] = p->RefCopy();
  return p;
}

static Portion *GSM_Read_Number(GSM &, Portion** param)
{
  gNumber value;
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  if(input.eof()) {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }
  try {
    input >> value;
  }
  catch (gFileInput::ReadFailed &) {
    input.setpos(old_pos);
    throw gclRuntimeError("No NUMBER data found");
  }

  ((NumberPortion*) param[1])->SetValue(value);

  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion *p = param[0];
  param[0] = p->RefCopy();
  return p;
}

static Portion *GSM_Read_Text(GSM &, Portion** param)
{
  char c = ' ';
  gText s, t;
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  while (!input.eof() && isspace(c))
    input.get(c);

  if (input.eof())  {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }
  if (c != '"') {
    input.setpos(old_pos);
    throw gclRuntimeError("No TEXT data found");
  }

  if (!input.eof() && c == '\"')
    input.get(c); 
  else {
    input.unget(c);
    input.setpos(old_pos);
    throw gclRuntimeError("File read error: missing starting \"");
  }

  while (!input.eof() && c != '\"') {
    t += c;
    input.get(c);
  }
  if (input.eof()) {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }
  
  ((TextPortion*) param[1])->SetValue(t);
 
  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion *p = param[0];
  param[0] = p->RefCopy();
  return p;
}

static Portion *GSM_ReadText_Number(GSM &, Portion** param)
{
  char c = ' ';
  gText t;
  gInput& input = ((InputPortion*) param[0])->Value();
  unsigned int n = ((NumberPortion *) param[2])->Value();

  long old_pos = input.getpos();

  while (t.Length()<n) {
    input.get(c);
    if (input.eof()) {
      input.setpos(old_pos);
      throw gclRuntimeError("End of file reached");
    }
    t += c;
  }

  ((TextPortion*) param[1])->SetValue(t);
 
  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion *p = param[0];
  param[0] = p->RefCopy();
  return p;
}

static Portion *GSM_ReadText_Text(GSM &, Portion** param)
{
  char c = ' ';
  gText t;
  gInput& input = ((InputPortion*) param[0])->Value();
  gText x = ((TextPortion *) param[2])->Value();
  int n = x.Length();

  long old_pos = input.getpos();

  while (t.Right(n)!=x) {
    input.get(c);
    if (input.eof()) {
      input.setpos(old_pos);
      throw gclRuntimeError("End of file reached");
    }
    t += c;
  }

  ((TextPortion*) param[1])->SetValue(t);
 
  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion *p = param[0];
  param[0] = p->RefCopy();
  return p;
}

static Portion *GSM_Read_List(GSM &gsm, Portion** param, PortionSpec spec,
			      Portion *(*func) (GSM &, Portion **),
			      bool ListFormat)
{
  Portion *sub_param[2];
  ListPortion *list;
  char c = ' ';
  gInput& input = ((InputPortion*) param[0])->Value();
  list = ((ListPortion*) param[1]);
  long old_pos = input.getpos();

  while (!input.eof() && isspace(c)) 
    input.get(c);

  if (input.eof()) {
    input.setpos(old_pos);
    throw gclRuntimeError("End of file reached");
  }

  if (!ListFormat)  {
    if (c == '{')
      ListFormat = true;
    else
      input.unget(c);
  }
  else {
    if (c != '{') {
      input.setpos(old_pos);
      throw gclRuntimeError("\'{\' expected");
    }
  }

  for (int i = 1; i <= list->Length(); i++) {
    Portion *p;

    sub_param[0] = param[0];
    sub_param[1] = (*list)[i];

    if (i > 1) { 
      c = ' ';
      while (!input.eof() && isspace(c))
	input.get(c);
      if (c == ',') {
	if (!ListFormat)
	  input.unget(c);
      }
      else
	input.unget(c);      
    }

    try  {
      if ((*list)[i]->Spec() == spec)
        p = (*func)(gsm, sub_param);
      else
        p = GSM_Read_List(gsm, sub_param, spec, func, ListFormat);
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

      // delete and swap
      delete sub_param[0];
      sub_param[0] = p;
  }

  if (ListFormat) {
    c = ' ';
    while (!input.eof() && isspace(c))
      input.get(c);
    if (c != '}') {
      input.setpos(old_pos);
      throw gclRuntimeError("Mismatched braces");
    }
    if (input.eof()) {
      input.setpos(old_pos);
      throw gclRuntimeError("End of file reached");
    }
  }

  // swap the first parameter with the return value, so things like
  //   Input["..."] >> x >> y  would work
  Portion *result = param[0];
  param[0] = result->RefCopy();
  return result;
}

static Portion *GSM_Read_List_Bool(GSM &gsm, Portion** param)
{
  Portion *temp = param[1]->ValCopy();
  try  {
    Portion *p = GSM_Read_List(gsm, param, porBOOLEAN, GSM_Read_Bool, false);
    delete temp;
    return p;
  }
  catch (gclRuntimeError &)  {
    ((ListPortion *) param[1])->AssignFrom(temp);
    delete temp;
    throw;
  }
}

static Portion *GSM_Read_List_Number(GSM &gsm, Portion** param)
{
  Portion *temp = param[1]->ValCopy();
  try  {
    Portion *p = GSM_Read_List(gsm, param, porNUMBER, GSM_Read_Number, false);
    delete temp;
    return p;
  }
  catch (gclRuntimeError &)  {
    ((ListPortion *) param[1])->AssignFrom(temp);
    throw;
  }
}

static Portion *GSM_Read_List_Text(GSM &gsm, Portion** param)
{
  Portion *temp = param[1]->ValCopy();
  try  {
    Portion *p = GSM_Read_List(gsm, param, porTEXT, GSM_Read_Text, false);
    delete temp;
    return p;
  }
  catch (gclRuntimeError &)  {
    ((ListPortion *) param[1])->AssignFrom(temp);
    throw;
  }
}

static Portion *GSM_Read_Undefined(GSM &gsm, Portion** param)
{
  /* will go through and try to read the input as different format until
     it succeeds */

  Portion *sub_param[2];
  char c = ' ';  
  gInput& input = ((InputPortion*) param[0])->Value();
  long old_pos = input.getpos();

  Portion *result = 0;

  while(!input.eof() && isspace(c))
    input.get(c);
  if (input.eof())
    throw gclRuntimeError("End of file reached");

  if (c == '{') {
    param[1] = new ListPortion;
    bool read_success = true;
    do {
      sub_param[0] = param[0];
      sub_param[1] = 0;
      try {
	result = GSM_Read_Undefined(gsm, sub_param);
	((ListPortion*) param[1])->Append(sub_param[1]);

	// okay, complicated things going on here
	// we want to delete the return value, but
	//   result is actually swapped with sub_param[0] in the
	//   GSM_Read() functions!  So, can't just delete p;
	//   need to swap result and sub_param[0] first.
	
	// delete and swap
	delete sub_param[0];
	sub_param[0] = result;
	result = NULL;
      }
      catch (gclRuntimeError &) {
	read_success = false;
      }

      c = ' ';
      while(!input.eof() && isspace(c))
	input.get(c);
      if(!input.eof() && c != ',')
	input.unget(c);

    } while (read_success && !input.eof());

    c = ' ';
    while(!input.eof() && isspace(c))
      input.get(c);
    if (input.eof()) {
      delete param[1];
      param[1] = 0;
      throw gclRuntimeError("End of file reached");
    }
    else if (c != '}') {
      delete param[1];
      param[1] = 0;
      throw gclRuntimeError("Mismatching braces");
    }
    else {
      // swap the first parameter with the return value, so things like
      //   Input["..."] >> x >> y  would work
      result = param[0];
      param[0] = result->RefCopy();
    }
  }
  else  { // not a list
    input.unget(c);
    param[1] = new BoolPortion(false);
    
    try {
      return GSM_Read_Bool(gsm, param);
    }
    catch (gclRuntimeError &) {
      delete param[1];
      param[1] = new NumberPortion(0);
    }

    try {
      return GSM_Read_Number(gsm, param);    
    }
    catch (gclRuntimeError &) {
      delete param[1];
      param[1] = new TextPortion("");
    }

    try {
      return GSM_Read_Text(gsm, param);    
    }
    catch (gclRuntimeError &) {
      delete param[1];
      param[1] = 0;
      gsm.OutputStream() << old_pos << '\n'; 
      // Not sure if this line is needed... if it's included,
      // segfaults occur.
      //      input.setpos(old_pos);
      throw gclRuntimeError("Cannot determine data type");
    }
  }

  return result;
}

//----------
// Times
//----------

static Portion *GSM_Times_Number(GSM &, Portion** param)
{
  return new NumberPortion(((NumberPortion *) param[0])->Value() *
                           ((NumberPortion *) param[1])->Value());
}

static Portion *GSM_Times_Mixed(GSM &, Portion** param)
{
  MixedPortion *result = new MixedPortion(new MixedSolution(*((MixedPortion *) param[1])->Value()));
  *result->Value() *= ((NumberPortion*) param[0])->Value();
  return result;
}

static Portion *GSM_Times_Behav(GSM &, Portion** param)
{
  BehavPortion *result = new BehavPortion(new BehavSolution(*((BehavPortion *) param[1])->Value()));
  *result->Value() *= ((NumberPortion*) param[0])->Value();
  return result;
}

//----------
// Version
//----------

static Portion *GSM_Version(GSM &, Portion **)
{
  return new NumberPortion(GCL_VERSION);
}

//--------
// Write
//--------

static Portion *GSM_Write(GSM &, Portion** param)
{
  gOutput& s = ((OutputPortion*) param[0])->Value();
  s << param[1];
  return param[0]->ValCopy();
}

static Portion *GSM_Write_Nfg(GSM &, Portion** param)
{
  gOutput &s = ((OutputPortion*) param[0])->Value();
  Nfg *nfg = ((NfgPortion *) param[1])->Value();
  nfg->WriteNfgFile(s, _WriteGameDecimals.Value());
  return param[0]->ValCopy();
}


static Portion *GSM_Write_Efg(GSM &, Portion** param)
{
  gOutput &s = ((OutputPortion*) param[0])->Value();
  FullEfg *efg = ((EfgPortion*) param[1])->Value();
  efg->WriteEfgFile(s, _WriteGameDecimals.Value());
  return param[0]->ValCopy();
}

//-----------------------------------------------------

//---------------------
// Formatting
//---------------------

//----------
// *Format
//----------

static Portion *GSM_ListFormat(GSM &, Portion** param)
{
  _WriteListBraces.SetValue(((BoolPortion*) param[0])->Value());
  _WriteListCommas.SetValue(((BoolPortion*) param[1])->Value());
  _WriteListLF.SetValue(((NumberPortion*) param[2])->Value());
  _WriteListIndent.SetValue(((NumberPortion*) param[3])->Value());

  GSM_SetWriteOptions();

  return new BoolPortion(triTRUE);
}

static Portion *GSM_GetListFormat(GSM &, Portion** param)
{
  ((BoolPortion*) param[0])->SetValue(_WriteListBraces.Value());
  ((BoolPortion*) param[1])->SetValue(_WriteListCommas.Value());
  ((NumberPortion*) param[2])->SetValue(_WriteListLF.Value());
  ((NumberPortion*) param[3])->SetValue(_WriteListIndent.Value());

  return new BoolPortion(triTRUE);
}

static Portion *GSM_NumberFormat(GSM &, Portion** param)
{
  _WriteWidth.SetValue(((NumberPortion*) param[1])->Value());
  _WritePrecis.SetValue(((NumberPortion*) param[2])->Value());
  _WriteExpmode.SetValue(((BoolPortion*) param[3])->Value());

  GSM_SetWriteOptions();

  return param[0]->ValCopy();
}

static Portion *GSM_GetNumberFormat(GSM &, Portion** param)
{
  ((NumberPortion*) param[1])->SetValue(_WriteWidth.Value());
  ((NumberPortion*) param[2])->SetValue(_WritePrecis.Value());
  ((BoolPortion*) param[3])->SetValue(_WriteExpmode.Value());

  return param[0]->ValCopy();
}

static Portion *GSM_TextFormat(GSM &, Portion** param)
{
  _WriteQuoted.SetValue(((BoolPortion*) param[1])->Value());

  GSM_SetWriteOptions();

  return param[0]->ValCopy();
}

static Portion *GSM_GetTextFormat(GSM &, Portion** param)
{
  ((BoolPortion*) param[1])->SetValue(_WriteQuoted.Value());

  return param[0]->ValCopy();
}

static Portion *GSM_SolutionFormat(GSM &, Portion** param)
{
  _WriteSolutionInfo.SetValue(((BoolPortion*) param[1])->Value());
  _WriteSolutionLabels.SetValue(((BoolPortion *) param[2])->Value());

  GSM_SetWriteOptions();

  return param[0]->ValCopy();
}

static Portion *GSM_GetSolutionFormat(GSM &, Portion** param)
{
  ((BoolPortion*) param[1])->SetValue(_WriteSolutionInfo.Value());

  return param[0]->ValCopy();
}

static Portion *GSM_GameFormat(GSM &, Portion **param)
{
  _WriteGameDecimals.SetValue(((NumberPortion *) param[1])->Value());
  GSM_SetWriteOptions();
  return param[0]->ValCopy();
}

static Portion *GSM_GetGameFormat(GSM &, Portion **param)
{
  ((NumberPortion *) param[1])->SetValue(_WriteGameDecimals.Value());
  return param[0]->ValCopy();
}

//--------------
// Environment
//--------------

static Portion *GSM_GetEnv(GSM &, Portion** param)
{
  if (((TextPortion*) param[0])->Value().Length() == 0)
    throw gclRuntimeError("Invalid environment variable name");

  return new TextPortion(System::GetEnv(((TextPortion *) param[0])->Value()));
}

static Portion *GSM_SetEnv(GSM &, Portion** param)
{
  if (((TextPortion*) param[0])->Value().Length() == 0)
    throw gclRuntimeError("Invalid environment variable name");

  if (System::SetEnv(((TextPortion*) param[0])->Value(),
		     ((TextPortion*) param[1])->Value()) == 0)
    return new BoolPortion(true);
  else
    throw gclRuntimeError("Insufficient environment space");
}

static Portion *GSM_UnSetEnv(GSM &, Portion** param)
{
  if (((TextPortion*) param[0])->Value().Length() == 0)
    throw gclRuntimeError("Invalid environment variable name");

  if (System::UnSetEnv(((TextPortion*) param[0])->Value()) == 0)
    return new BoolPortion(true);
  else
    throw gclRuntimeError("Insufficient environment space");
}

static Portion *GSM_Shell(GSM &, Portion** param)
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

static Portion *GSM_ExePath(GSM &, Portion** param)
{
#ifdef __GNUG__
  const char SLASH = '/';
#elif defined __BORLANDC__
  const char SLASH = '\\';
#endif   // __GNUG__
  bool file = ((BoolPortion*) param[0])->Value();
  bool path = ((BoolPortion*) param[1])->Value();

  gText txt(_ExePath);

  if (file && path) {
  }
  else if (file) {
    if (txt.LastOccur(SLASH) > 0)
      txt = txt.Right(txt.Length() - txt.LastOccur(SLASH));
    else
      txt = "";
  }
  else if (path) {
    if (txt.LastOccur(SLASH) > 0)
      txt = txt.Left(txt.LastOccur(SLASH));
  }
  if (!file && !path)
    txt = "";
  return new TextPortion(txt);
}


//-------------------------------------------
//              Assign
//-------------------------------------------


Portion *GSM_Assign(GSM &gsm, Portion** param)
{
  Portion *p = 0;
  Portion *result = 0;
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
  gText txt = ((TextPortion*) param[0])->Value();

  if (!isalpha(txt[0u])) 
      throw gclRuntimeError("Bad Variable Name");
  for(unsigned int i = 1;i<txt.Length();i++) 
    if(!isalnum(txt[i]))
      throw gclRuntimeError("Bad Variable Name");

  gsm.VarDefine(txt,p);
  return result;
}

static Portion *GSM_UnAssign(GSM &gsm, Portion **param)
{
  gText txt = ((TextPortion*) param[0])->Value();
  return new BoolPortion(gsm.VarRemove(txt));
}

static Portion *GSM_Platform(GSM &, Portion**)
{
#ifdef __svr4__
  return new TextPortion("SVR4");
#elif defined sparc
  return new TextPortion("SPARC");
#elif defined sun
  return new TextPortion("SUN");
#elif defined _AIX32
  return new TextPortion("AIX32");
#elif defined _AIX
  return new TextPortion("AIX");
#elif defined hpux
  return new TextPortion("HP UX");
#elif defined hppa
  return new TextPortion("HPPA");
#elif defined linux
  return new TextPortion("Linux");
#elif defined __BORLANDC__
  return new TextPortion("Windows 95/NT");
#else
  return new TextPortion("Unknown");
#endif
}
static Portion *GSM_Date(GSM &, Portion**)
{
  time_t now = time(0);
  gText AscTime = asctime(localtime(&now));
  return new TextPortion(AscTime.Mid(11, 1) +
			    AscTime.Mid(4, 21) + ", " + AscTime.Mid(8, 12));
}

static Portion *GSM_IsList(GSM &, Portion **param)
{
  return new BoolPortion(param[0]->Spec().ListDepth > 0);
}

void Init_gsmoper(GSM* gsm)
{
  gclFunction* FuncObj;

  static struct { char *sig; Portion *(*func)(GSM &, Portion **); } ftable[] =
    { { "And[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN", GSM_And },
      { "Divide[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Divide },
      { "Equal[x->BOOLEAN*, y->BOOLEAN*] =: BOOLEAN", GSM_Equal_Boolean },
      { "Equal[x->NUMBER*, y->NUMBER*] =: BOOLEAN", GSM_Equal_Number },
      { "Equal[x->TEXT*, y->TEXT*] =: BOOLEAN", GSM_Equal_Text },
      { "Equal[x->EFG*, y->EFG*] =: BOOLEAN", GSM_Equal_Efg },
      { "Equal[x->EFPLAYER*, y->EFPLAYER*] =: BOOLEAN", 
	GSM_Equal_EfPlayer },
      { "Equal[x->NODE*, y->NODE*] =: BOOLEAN", GSM_Equal_Node },
      { "Equal[x->INFOSET*, y->INFOSET*] =: BOOLEAN", GSM_Equal_Infoset },
      { "Equal[x->EFOUTCOME*, y->EFOUTCOME*] =: BOOLEAN",
	GSM_Equal_EfOutcome },
      { "Equal[x->ACTION*, y->ACTION*] =: BOOLEAN", GSM_Equal_Action },
      { "Equal[x->EFSUPPORT*, y->EFSUPPORT*] =: BOOLEAN", 
	GSM_Equal_EfSupport },
      { "Equal[x->BEHAV*, y->BEHAV*] =: BOOLEAN", GSM_Equal_Behav },
      { "Equal[x->NFG*, y->NFG*] =: BOOLEAN", GSM_Equal_Nfg },
      { "Equal[x->NFPLAYER*, y->NFPLAYER*] =: BOOLEAN",
	GSM_Equal_NfPlayer },
      { "Equal[x->STRATEGY*, y->STRATEGY*] =: BOOLEAN",
	GSM_Equal_Strategy },
      { "Equal[x->NFOUTCOME*, y->NFOUTCOME*] =: BOOLEAN",
	GSM_Equal_NfOutcome },
      { "Equal[x->NFSUPPORT*, y->NFSUPPORT*] =: BOOLEAN",
	GSM_Equal_NfSupport },
      { "Equal[x->MIXED*, y->MIXED*] =: BOOLEAN", GSM_Equal_Mixed },
      { "Exp[x->NUMBER] =: NUMBER", GSM_Exp },
      { "Float[x->NUMBER] =: NUMBER", GSM_Float },
      { "Greater[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_Greater_Number },
      { "Greater[x->TEXT, y->TEXT] =: BOOLEAN", GSM_Greater_Text },
      { "GreaterEqual[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_GreaterEqual_Number },
      { "GreaterEqual[x->TEXT, y->TEXT] =: BOOLEAN", GSM_GreaterEqual_Text },
      { "IntegerDivide[x->INTEGER, y->INTEGER] =: INTEGER", 
	GSM_IntegerDivide },
      { "Less[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_Less_Number },
      { "Less[x->TEXT, y->TEXT] =: BOOLEAN", GSM_Less_Text },
      { "LessEqual[x->NUMBER, y->NUMBER] =: BOOLEAN", GSM_LessEqual_Number },
      { "LessEqual[x->TEXT, y->TEXT] =: BOOLEAN", GSM_LessEqual_Text },
      { "Log[x->NUMBER] =: NUMBER", GSM_Log },
      { "Minus[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Minus_Number },
      { "Minus[x->MIXED, y->MIXED] =: MIXED", GSM_Minus_Mixed },
      { "Minus[x->BEHAV, y->BEHAV] =: BEHAV", GSM_Minus_Behav },
      { "Modulus[x->INTEGER, y->INTEGER] =: INTEGER", GSM_Modulus },
      { "Negate[x->NUMBER] =: NUMBER", GSM_Negate },
      { "Not[x->BOOLEAN] =: BOOLEAN", GSM_Not },
      { "NotEqual[x->BOOLEAN*, y->BOOLEAN*] =: BOOLEAN", GSM_NotEqual_Boolean },
      { "NotEqual[x->NUMBER*, y->NUMBER*] =: BOOLEAN", GSM_NotEqual_Number },
      { "NotEqual[x->TEXT*, y->TEXT*] =: BOOLEAN", GSM_NotEqual_Text },
      { "NotEqual[x->EFG*, y->EFG*] =: BOOLEAN", GSM_NotEqual_Efg },
      { "NotEqual[x->EFPLAYER*, y->EFPLAYER*] =: BOOLEAN", 
	GSM_NotEqual_EfPlayer },
      { "NotEqual[x->NODE*, y->NODE*] =: BOOLEAN", GSM_NotEqual_Node },
      { "NotEqual[x->INFOSET*, y->INFOSET*] =: BOOLEAN", GSM_NotEqual_Infoset },
      { "NotEqual[x->EFOUTCOME*, y->EFOUTCOME*] =: BOOLEAN",
	GSM_NotEqual_EfOutcome },
      { "NotEqual[x->ACTION*, y->ACTION*] =: BOOLEAN", GSM_NotEqual_Action },
      { "NotEqual[x->EFSUPPORT*, y->EFSUPPORT*] =: BOOLEAN", 
	GSM_NotEqual_EfSupport },
      { "NotEqual[x->BEHAV*, y->BEHAV*] =: BOOLEAN", GSM_NotEqual_Behav },
      { "NotEqual[x->NFG*, y->NFG*] =: BOOLEAN", GSM_NotEqual_Nfg },
      { "NotEqual[x->NFPLAYER*, y->NFPLAYER*] =: BOOLEAN",
	GSM_NotEqual_NfPlayer },
      { "NotEqual[x->STRATEGY*, y->STRATEGY*] =: BOOLEAN",
	GSM_NotEqual_Strategy },
      { "NotEqual[x->NFOUTCOME*, y->NFOUTCOME*] =: BOOLEAN",
	GSM_NotEqual_NfOutcome },
      { "NotEqual[x->NFSUPPORT*, y->NFSUPPORT*] =: BOOLEAN",
	GSM_NotEqual_NfSupport },
      { "NotEqual[x->MIXED*, y->MIXED*] =: BOOLEAN", GSM_NotEqual_Mixed },
      { "Or[x->BOOLEAN, y->BOOLEAN] =: BOOLEAN", GSM_Or },
      { "Parentheses[x->ANYTYPE*] =: ANYTYPE", GSM_Parentheses },
      { "Plus[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Plus_Number },
      { "Plus[x->TEXT, y->TEXT] =: TEXT", GSM_Plus_Text }, 
      { "Plus[x->MIXED, y->MIXED] =: MIXED", GSM_Plus_Mixed },
      { "Plus[x->BEHAV, y->BEHAV] =: BEHAV", GSM_Plus_Behav },
      { "Power[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Power },
      { "Rational[x->NUMBER] =: NUMBER", GSM_Rational },
      { "Times[x->NUMBER, y->NUMBER] =: NUMBER", GSM_Times_Number },
      { "Times[x->NUMBER, y->MIXED] =: MIXED", GSM_Times_Mixed },
      { "Times[x->NUMBER, y->BEHAV] =: BEHAV", GSM_Times_Behav },
      { "Version[] =: NUMBER", GSM_Version },
      { 0, 0 }
    };

  for (int i = 0; ftable[i].sig != 0; i++) {
    gsm->AddFunction(new gclFunction(*gsm, ftable[i].sig, ftable[i].func));
  }


  FuncObj = new gclFunction(*gsm, "Quit", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Quit, porBOOLEAN, 1, 0,
				       funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("value", porINTEGER,
					   new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  //-------------------- Streams -------------------------

  FuncObj = new gclFunction(*gsm, "Output", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Output, porOUTPUT, 3));
  FuncObj->SetParamInfo(0, 0, gclParameter("file", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("append", porBOOLEAN,
					    new BoolPortion( false )));
  FuncObj->SetParamInfo(0, 2, gclParameter("close", porBOOLEAN,
					    new BoolPortion( true )));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new gclFunction(*gsm, "Input", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Input, porINPUT, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("file", porTEXT));
  gsm->AddFunction(FuncObj);

  //------------------- Formatted Output -------------------

  GSM_SetWriteOptions();

  FuncObj = new gclFunction(*gsm, "Print", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Print, porANYTYPE, 1, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("x",
					    PortionSpec(porANYTYPE, 0, 
							porNULLSPEC)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Print, PortionSpec(porANYTYPE, NLIST), 1, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", 
					    PortionSpec(porANYTYPE, NLIST,
						      porNULLSPEC)));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Write", 4);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Write, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter
			("x", porBOOLEAN | porNUMBER | porTEXT |
			 porMIXED | porBEHAV | porNFSUPPORT | porEFSUPPORT |
			 porSTRATEGY | porEFBASIS));
  
  FuncObj->SetFuncInfo(1, gclSignature(GSM_Write_Nfg, 
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, gclParameter("x", porNFG));
  
  FuncObj->SetFuncInfo(2, gclSignature(GSM_Write_Efg,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(2, 1, gclParameter("x", porEFG));

  FuncObj->SetFuncInfo(3, gclSignature(GSM_Write,
				       porOUTPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(3, 0, gclParameter("output", porOUTPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(3, 1, gclParameter
			("x", PortionSpec(porBOOLEAN | porNUMBER |
					  porTEXT | porMIXED | porBEHAV |
					  porNFSUPPORT | porEFSUPPORT, 1)));
  gsm->AddFunction(FuncObj);




  //---------------
  // *Format
  //---------------

  FuncObj = new gclFunction(*gsm, "ListFormat", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ListFormat, porBOOLEAN, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter
			("braces", porBOOLEAN,
			 _WriteListBraces.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter
			("commas", porBOOLEAN,
			 _WriteListCommas.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("lf", porNUMBER,
			 _WriteListLF.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("indent", porNUMBER,
			 _WriteListIndent.RefCopy(), BYREF ));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "GetListFormat", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_GetListFormat, porBOOLEAN, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter
			("braces", porBOOLEAN,
			 _WriteListBraces.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter
			("commas", porBOOLEAN,
			 _WriteListCommas.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("lf", porNUMBER,
			 _WriteListLF.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("indent", porNUMBER,
			 _WriteListIndent.RefCopy(), BYREF));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Format", 4);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_NumberFormat, porNUMBER, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER) );
  FuncObj->SetParamInfo(0, 1, gclParameter
			("width", porNUMBER, 
			 _WriteWidth.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("decimals", porNUMBER,
			 _WritePrecis.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("expmode", porBOOLEAN,
			 _WriteExpmode.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_TextFormat, porTEXT, 2));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", porTEXT) );
  FuncObj->SetParamInfo(1, 1, gclParameter
			("quote", porBOOLEAN,
			 _WriteQuoted.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_SolutionFormat,
				       porBEHAV | porMIXED, 3));
  FuncObj->SetParamInfo(2, 0, gclParameter("x", porBEHAV | porMIXED) );
  FuncObj->SetParamInfo(2, 1, gclParameter
			("info", porBOOLEAN,
			 _WriteSolutionInfo.RefCopy(), BYREF));
  FuncObj->SetParamInfo(2, 2, gclParameter("names", porBOOLEAN,
					   _WriteSolutionLabels.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(3, gclSignature(GSM_GameFormat,
				       porEFG | porNFG, 2));
  FuncObj->SetParamInfo(3, 0, gclParameter("x", porEFG | porNFG));
  FuncObj->SetParamInfo(3, 1, gclParameter("decimals", porNUMBER,
					   _WriteGameDecimals.RefCopy(), BYREF));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "GetFormat", 4);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_GetNumberFormat, porNUMBER, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER) );
  FuncObj->SetParamInfo(0, 1, gclParameter
			("width", porNUMBER, 
			 _WriteWidth.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter
			("decimals", porNUMBER,
			 _WritePrecis.RefCopy(), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter
			("expmode", porBOOLEAN,
			 _WriteExpmode.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_GetTextFormat, porTEXT, 2));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", porTEXT) );
  FuncObj->SetParamInfo(1, 1, gclParameter
			("quote", porBOOLEAN,
			 _WriteQuoted.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_GetSolutionFormat,
				       porBEHAV | porMIXED, 3));
  FuncObj->SetParamInfo(2, 0, gclParameter("x", porBEHAV | porMIXED) );
  FuncObj->SetParamInfo(2, 1, gclParameter
			("info", porBOOLEAN,
			 _WriteSolutionInfo.RefCopy(), BYREF));
  FuncObj->SetParamInfo(2, 2, gclParameter("names", porBOOLEAN,
					   _WriteSolutionLabels.RefCopy(), BYREF));

  FuncObj->SetFuncInfo(3, gclSignature(GSM_GetGameFormat,
				       porEFG | porNFG, 2));
  FuncObj->SetParamInfo(3, 0, gclParameter("x", porEFG | porNFG));
  FuncObj->SetParamInfo(3, 1, gclParameter("decimals", porNUMBER,
					   _WriteGameDecimals.RefCopy(),
					   BYREF));
  gsm->AddFunction(FuncObj);



  //-------------------- Read --------------------------

  FuncObj = new gclFunction(*gsm, "Read", 7);

  FuncObj->SetFuncInfo(0, gclSignature(GSM_Read_Bool, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("input", porINPUT, 
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter("x", porBOOLEAN, 
					    REQUIRED, BYREF));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Read_List_Bool, 
				       porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, gclParameter("x", PortionSpec(porBOOLEAN,1), 
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

  FuncObj->SetFuncInfo(6, gclSignature(GSM_Read_Undefined, 
					porINPUT, 2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(6, 0, gclParameter("input", porINPUT,
					   REQUIRED, BYREF));
  FuncObj->SetParamInfo(6, 1, gclParameter("x", porUNDEFINED, 
					   REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "ReadText", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ReadText_Text,
				       porINPUT, 3, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter("x", porTEXT,
					    new TextPortion(""), BYREF));
  FuncObj->SetParamInfo(0, 2, gclParameter("until", porTEXT,
					    REQUIRED));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_ReadText_Number,
				       porINPUT, 3, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("input", porINPUT,
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(1, 1, gclParameter("x", porTEXT,
					    new TextPortion(""), BYREF));
  FuncObj->SetParamInfo(1, 2, gclParameter("n", porINTEGER,
					    REQUIRED));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "IsEof", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsEof, porBOOLEAN, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("input", porINPUT));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "IsDefined", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsDefined, porBOOLEAN, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porANYTYPE));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_IsDefined_Undefined, porBOOLEAN, 1));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", porUNDEFINED));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Help", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Help, PortionSpec(porTEXT, 1), 3));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("udf", porBOOLEAN,
					    new BoolPortion( true )));
  FuncObj->SetParamInfo(0, 2, gclParameter("bif", porBOOLEAN, 
					    new BoolPortion( true )));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Manual", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Manual, porBOOLEAN, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("y", porOUTPUT,
					    new OutputPortion(gsm->OutputStream())));
  gsm->AddFunction(FuncObj);
  
  FuncObj = new gclFunction(*gsm, "HelpVars", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_HelpVars, PortionSpec(porTEXT, 1),
				       1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Clear", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Clear, porBOOLEAN, 0));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Assign", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Assign, porANYTYPE, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("value", porANYTYPE, 
					    REQUIRED, BYREF));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "UnAssign", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_UnAssign, porBOOLEAN, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Date", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Date, porTEXT, 0));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Randomize", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Randomize, porNUMBER, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER));
  FuncObj->SetParamInfo(0, 1, gclParameter("seed", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Precision", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Precision, porPRECISION, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porNUMBER));

  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "IsNull", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsNull, porBOOLEAN, 1, 0, 
				       funcLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", 
					    PortionSpec(porANYTYPE, 0, 
							porNULLSPEC )));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Null", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Null, 
				       PortionSpec(porANYTYPE, 0, 
						   porNULLSPEC ),
				       1, 0, funcLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porANYTYPE));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Shell", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Shell, porNUMBER, 2 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("command", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 1, gclParameter("spawn", porBOOLEAN, 
					    new BoolPortion(true)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "GetEnv", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_GetEnv, porTEXT, 1 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "SetEnv", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_SetEnv, porBOOLEAN, 2 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT ) );
  FuncObj->SetParamInfo(0, 1, gclParameter("value", porTEXT ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "UnSetEnv", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_UnSetEnv, porBOOLEAN, 1 ));
  FuncObj->SetParamInfo(0, 0, gclParameter("name", porTEXT ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "ExePath", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ExePath, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("file", porBOOLEAN,
					    new BoolPortion( true ) ) );
  FuncObj->SetParamInfo(0, 1, gclParameter("path", porBOOLEAN,
					    new BoolPortion( true ) ) );
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Platform", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Platform, porTEXT, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "IsList", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsList, porBOOLEAN, 1,
				       NO_PREDEFINED_PARAMS, 
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porANYTYPE ));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_IsList, porBOOLEAN, 1, 
				       NO_PREDEFINED_PARAMS, 
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(1, 0, gclParameter("list", 
					    PortionSpec(porANYTYPE,NLIST)));
  gsm->AddFunction(FuncObj);
}







