//#
//# FILE: listfunc.cc -- List and text oriented function implementations
//#
//# $Id$
//#

#include <assert.h>

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "gmisc.h"
#include "rational.h"



Portion* GSM_Sort(Portion** param, bool (*compfunc)(Portion*, Portion*))
{
  unsigned long n = ((ListPortion*) param[0])->Length();
  Portion* a[n+1];
  unsigned long i, j, inc;
  Portion* v; 
  bool no_sub_lists = true;

  for(i=1; i<=n; i++)
  {
    a[i] = ((ListPortion*) param[0])->Remove(1);
    if(a[i]->Spec().ListDepth > 0)
      no_sub_lists = false;
  }

  if(no_sub_lists)
  {
    // insertion sort, adopted from _Numerical_Recipes_in_C_
    inc = 1;
    do {
      inc *= 3;
      inc++;
    } while(inc <= n);
    do {
      inc /= 3;
      for(i=inc+1; i<=n; i++) {
	v = a[i];
	j=i;
	while(compfunc(a[j-inc], v)) {
	  a[j] = a[j-inc];
	  j -= inc;
	  if(j <= inc) break;
	}
	a[j] = v;
      }
    } while(inc > 1);
  }

  for(i=1; i<=n; i++)
    ((ListPortion*) param[0])->Append(a[i]);  

  if(no_sub_lists)
    return param[0]->ValCopy();
  else
    return new ErrorPortion("Cannot sort a nested list");
}


bool GSM_Compare_Integer(Portion* p1, Portion* p2)
{ return ((IntPortion*) p1)->Value() > ((IntPortion*) p2)->Value(); }
Portion* GSM_Sort_Integer(Portion** param)
{ return GSM_Sort(param, GSM_Compare_Integer); }

bool GSM_Compare_Float(Portion* p1, Portion* p2)
{ return ((FloatPortion*) p1)->Value() > ((FloatPortion*) p2)->Value(); }
Portion* GSM_Sort_Float(Portion** param)
{ return GSM_Sort(param, GSM_Compare_Float); }

bool GSM_Compare_Rational(Portion* p1, Portion* p2)
{ return ((RationalPortion*) p1)->Value() > ((RationalPortion*) p2)->Value(); }
Portion* GSM_Sort_Rational(Portion** param)
{ return GSM_Sort(param, GSM_Compare_Rational); }

bool GSM_Compare_Text(Portion* p1, Portion* p2)
{ return ((TextPortion*) p1)->Value() > ((TextPortion*) p2)->Value(); }
Portion* GSM_Sort_Text(Portion** param)
{ return GSM_Sort(param, GSM_Compare_Text); }




Portion *GSM_NthElement(Portion **param)
{
  int n = ((IntPortion *) param[1])->Value();
  if(n < 0 || n > ((ListPortion *) param[0])->Length())
    return new ErrorPortion("Subscript out of range");
  else
    return ((ListPortion *) param[0])->Subscript(n);
}

Portion *GSM_Remove(Portion **param)
{
  ListPortion *ret = (ListPortion *) param[0]->ValCopy();
  delete ret->Remove
    (((IntPortion *) param[1])->Value());
  return ret;
}



Portion *GSM_Contains(Portion **param)
{
  return new BoolValPortion(((ListPortion *) param[0])->Contains(param[1]));
}


Portion *GSM_LengthList(Portion **param)
{
  return new IntValPortion(((ListPortion *) param[0])->Length());
}

Portion *GSM_LengthText(Portion **param)
{
  return new IntValPortion(((TextPortion *) param[0])->Value().length());
}

Portion *GSM_NthChar(Portion **param)
{
  gString text(((TextPortion *) param[0])->Value());
  int n = ((IntPortion *) param[1])->Value();
  if (n <= 0 || n > text.length())
    return 0;
  return new TextValPortion(text[n-1]);
}


//--------------------------- Text ---------------------------

Portion *GSM_TextInt(Portion **param)
{
  return new TextValPortion(ToString(((IntPortion *) param[0])->Value()));
}

Portion *GSM_TextFloat(Portion **param)
{
  return new TextValPortion(ToString(((FloatPortion *) param[0])->Value()));
}

Portion *GSM_TextRat(Portion **param)
{
  return new TextValPortion(ToString(((RationalPortion *) param[0])->Value()));
}

Portion *GSM_TextText(Portion **param)
{
  return param[0]->ValCopy();
}


//------------------------ Integer --------------------------

Portion *GSM_IntegerRational(Portion **param)
{
  return new IntValPortion((long) ((RationalPortion *) param[0])->Value());
}

Portion *GSM_IntegerInteger(Portion **param)
{
  return param[0]->ValCopy();
}

Portion *GSM_IntegerFloat(Portion **param)
{
  return new IntValPortion((long) ((FloatPortion *) param[0])->Value());
}


//------------------------ Float --------------------------

Portion *GSM_FloatRational(Portion **param)
{
  return new FloatValPortion((double) ((RationalPortion *) param[0])->Value());
}

Portion *GSM_FloatInteger(Portion **param)
{
  return new FloatValPortion((double) ((IntPortion *) param[0])->Value());
}

Portion *GSM_FloatFloat(Portion **param)
{
  return param[0]->ValCopy();
}

//------------------------- Rational ------------------------

Portion *GSM_RationalFloat(Portion **param)
{
  return new RationalValPortion(((FloatPortion *) param[0])->Value());
}

Portion *GSM_RationalInteger(Portion **param)
{
  return new RationalValPortion(((IntPortion *) param[0])->Value());
}

Portion *GSM_RationalRational(Portion **param)
{
  return param[0]->ValCopy();
}



//----------------------------- Stop Watch ----------------------

#include "gwatch.h"

gWatch _gcl_watch(0);

Portion *GSM_StartWatch(Portion **)
{
  _gcl_watch.Start();
  return new FloatValPortion(0.0);
}

Portion *GSM_StopWatch(Portion **)
{
  _gcl_watch.Stop();
  return new FloatValPortion(_gcl_watch.Elapsed());
}

Portion *GSM_ElapsedTime(Portion **)
{
  return new FloatValPortion(_gcl_watch.Elapsed());
}

Portion *GSM_IsWatchRunning(Portion **)
{
  return new BoolValPortion(_gcl_watch.IsRunning());
}

//--------------------------- List ------------------------------

Portion* GSM_List( Portion** param )
{
  ListPortion* p;
  int i;
  assert( param[0]->Spec().Type != porERROR );  

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list length" );

  p = new ListValPortion();
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( param[0]->ValCopy() );
  return p;
}

Portion* GSM_List_List( Portion** param )
{
  ListPortion* p;
  int i;
  assert( param[0]->Spec().Type != porERROR );  

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list length" );

  p = new ListValPortion();
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( param[0]->ValCopy() );
  return p;
}


Portion* GSM_List_Integer( Portion** param )
{
  ListPortion* p;
  int i;

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list length" );

  p = new ListValPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( new IntValPortion( ((IntPortion*) param[0])->Value()+ 
				 (i-1)*((IntPortion*) param[2])->Value() ));
  return p;
}

Portion* GSM_List_Float( Portion** param )
{
  ListPortion* p;
  int i;

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list length" );

  p = new ListValPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( new FloatValPortion( ((FloatPortion*) param[0])->Value()+ 
				   (i-1)*((FloatPortion*) param[2])->Value()));
  return p;
}

Portion* GSM_List_Rational( Portion** param )
{
  ListPortion* p;
  int i;

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list length" );

  p = new ListValPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( new RationalValPortion( ((RationalPortion*) param[0])->Value()+ 
				      (gRational)(i-1)*
				      ((RationalPortion*) param[2])->Value()));
  return p;
}


//------------------------ Dot --------------------------------


Portion* GSM_Dot_Check( ListPortion* p1, ListPortion* p2 )
{
  int i;
  if( p1->Length() != p2->Length() )
    return new ErrorPortion( "Mismatched dimentionalities" );
  for( i = 1; i <= p1->Length(); i++ )
    if( (*p1)[ i ]->Spec().ListDepth > 0 )
      return new ErrorPortion("Can only operate on 1-D lists");
  return 0;
}


Portion* GSM_Dot_Integer(Portion **param)
{
  int i;
  Portion* p;
  ListPortion* p1 = (ListPortion*) param[0];
  ListPortion* p2 = (ListPortion*) param[1];

  p = GSM_Dot_Check( p1, p2 );
  if( p != 0 )
    return p;

  p = new IntValPortion( (long) 0 );
  for( i = 1; i <= p1->Length(); i++ )
  {
    ((IntPortion*) p)->Value() += 
      (((IntPortion*) (*p1)[i])->Value() * 
       ((IntPortion*) (*p2)[i])->Value());
  }
  return p;
}

Portion* GSM_Dot_Float(Portion **param)
{
  int i;
  Portion* p;
  ListPortion* p1 = (ListPortion*) param[0];
  ListPortion* p2 = (ListPortion*) param[1];

  p = GSM_Dot_Check( p1, p2 );
  if( p != 0 )
    return p;

  p = new FloatValPortion( (double) 0 );
  for( i = 1; i <= p1->Length(); i++ )
  {
    ((FloatPortion*) p)->Value() += 
      (((FloatPortion*) (*p1)[i])->Value() * 
       ((FloatPortion*) (*p2)[i])->Value());
  }
  return p;
}

Portion* GSM_Dot_Rational(Portion **param)
{
  int i;
  Portion* p;
  ListPortion* p1 = (ListPortion*) param[0];
  ListPortion* p2 = (ListPortion*) param[1];

  p = GSM_Dot_Check( p1, p2 );
  if( p != 0 )
    return p;

  p = new RationalValPortion( (gRational) 0 );
  for( i = 1; i <= p1->Length(); i++ )
  {
    ((RationalPortion*) p)->Value() += 
      (((RationalPortion*) (*p1)[i])->Value() * 
       ((RationalPortion*) (*p2)[i])->Value());
  }
  return p;
}


//--------------------------- ArgMax -------------------------

Portion* GSM_ArgMax_Integer( Portion** param )
{
  Portion* p;
  long max = 0;
  int index = 0;
  int i;
  assert( param[0]->Spec().ListDepth > 0 );
  for( i = ((ListPortion*) param[0])->Length(); i >= 1; i-- )
  {
    p = (*(ListPortion*) param[0])[i];
    if( p->Spec().Type == porINTEGER )
    {
      if( ((IntPortion*) p)->Value() >= max ||
	 i == ((ListPortion*) param[0])->Length() )
      {
	max = ((IntPortion*) p)->Value();
	index = i;
      }
    }
    else
      return new ErrorPortion( "Bad dimensionality" );
  }
  return new IntValPortion( index );
}

Portion* GSM_ArgMax_Float( Portion** param )
{
  Portion* p;
  double max = 0;
  int index = 0;
  int i;
  assert( param[0]->Spec().ListDepth > 0 );
  for( i = ((ListPortion*) param[0])->Length(); i >= 1; i-- )
  {
    p = (*(ListPortion*) param[0])[i];
    if( p->Spec().Type == porFLOAT )
    {
      if( ((FloatPortion*) p)->Value() >= max || 
	 i == ((ListPortion*) param[0])->Length() )
      {
	max = ((FloatPortion*) p)->Value();
	index = i;
      }
    }
    else
      return new ErrorPortion( "Bad dimensionality" );
  }
  return new IntValPortion( index );
}

Portion* GSM_ArgMax_Rational( Portion** param )
{
  Portion* p;
  gRational max = 0;
  int index = 0;
  int i;
  assert( param[0]->Spec().ListDepth > 0 );
  for( i = ((ListPortion*) param[0])->Length(); i >= 1; i-- )
  {
    p = (*(ListPortion*) param[0])[i];
    if( p->Spec().Type == porRATIONAL )
    {
      if( ((RationalPortion*) p)->Value() >= max ||
	 i == ((ListPortion*) param[0])->Length() )
      {
	max = ((RationalPortion*) p)->Value();
	index = i;
      }
    }
    else
      return new ErrorPortion( "Bad dimensionality" );
  }
  return new IntValPortion( index );
}


//--------------------------- Transpose -------------------------

Portion* GSM_Transpose( Portion** param )
{
  int i;
  int j;
  int length = 0;
  int width = 0;
  ListPortion* p;
  ListPortion* s;
  assert( param[0]->Spec().ListDepth > 0 );
  length = ((ListPortion*) param[0])->Length();
  for( i = 1; i <= length; i++ )
  {
    if( (*(ListPortion*) param[0])[i]->Spec().ListDepth == 0 )
      return new ErrorPortion( "Bad dimensionality" );
    if( i == 1 )
      width = ((ListPortion*) (*(ListPortion*) param[0])[i])->Length();
    else 
      if( ((ListPortion*) (*(ListPortion*) param[0])[i])->Length() != width )
	return new ErrorPortion( "Bad dimensionality" );
  }
  p = new ListValPortion();
  for( i = 1; i <= width; i++ )
  {
    s = new ListValPortion();
    for( j = 1; j <= length; j++ )
    {
      s->Append( (*(ListPortion*)(*(ListPortion*) param[0])[j])[i]->ValCopy());
    }
    p->Append( s );
  }
  return p;
}




//--------------------------- Init_listfunc ------------------------------

void Init_listfunc(GSM *gsm)
{
  FuncDescObj *FuncObj;


  ParamInfoType x_Float[] =
  {
    ParamInfoType( "x", porFLOAT )
  };
  ParamInfoType x_Int[] =
  {
    ParamInfoType( "x", porINTEGER )
  };
  ParamInfoType x_Rational[] =
  {
    ParamInfoType( "x", porRATIONAL )
  };


  FuncObj = new FuncDescObj("Length");
  FuncObj->SetFuncInfo(GSM_LengthList, 1, NO_PREDEFINED_PARAMS);
  FuncObj->SetParamInfo(GSM_LengthList, 0, "list", PortionSpec(porANYTYPE,1),
			NO_DEFAULT_VALUE);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NumChars");
  FuncObj->SetFuncInfo(GSM_LengthText, 1);
  FuncObj->SetParamInfo(GSM_LengthText, 0, "text", porTEXT);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Contains");
  FuncObj->SetFuncInfo(GSM_Contains, 2, NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_Contains, 0, "list", PortionSpec(porANYTYPE,1));
  FuncObj->SetParamInfo(GSM_Contains, 1, "x", porANYTYPE);
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NthElement");
  FuncObj->SetFuncInfo(GSM_NthElement, 2, NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_NthElement, 0, "list", PortionSpec(porANYTYPE, 1),
			NO_DEFAULT_VALUE, PASS_BY_REFERENCE );
  FuncObj->SetParamInfo(GSM_NthElement, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Remove");
  FuncObj->SetFuncInfo(GSM_Remove, 2, NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_Remove, 0, "list", PortionSpec(porANYTYPE,1));
  FuncObj->SetParamInfo(GSM_Remove, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthChar");
  FuncObj->SetFuncInfo(GSM_NthChar, 2);
  FuncObj->SetParamInfo(GSM_NthChar, 0, "text", porTEXT);
  FuncObj->SetParamInfo(GSM_NthChar, 1, "n", porINTEGER);
  gsm->AddFunction(FuncObj);


  //--------------------------- Text -----------------------

  FuncObj = new FuncDescObj("Text");
  FuncObj->SetFuncInfo(GSM_TextInt, 1, x_Int);
  FuncObj->SetFuncInfo(GSM_TextFloat, 1, x_Float);
  FuncObj->SetFuncInfo(GSM_TextRat, 1, x_Rational);
  FuncObj->SetFuncInfo(GSM_TextText, 1);
  FuncObj->SetParamInfo(GSM_TextText, 0, "x", porTEXT);
  gsm->AddFunction(FuncObj);
  
  //-------------------------- Integer ------------------------

  FuncObj = new FuncDescObj("Integer");
  FuncObj->SetFuncInfo(GSM_IntegerRational, 1, x_Rational);
  FuncObj->SetFuncInfo(GSM_IntegerInteger, 1, x_Int);
  FuncObj->SetFuncInfo(GSM_IntegerFloat, 1, x_Float);
  gsm->AddFunction(FuncObj);

  //-------------------------- Float ------------------------

  FuncObj = new FuncDescObj("Float");
  FuncObj->SetFuncInfo(GSM_FloatRational, 1, x_Rational);
  FuncObj->SetFuncInfo(GSM_FloatInteger, 1, x_Int);
  FuncObj->SetFuncInfo(GSM_FloatFloat, 1, x_Float);
  gsm->AddFunction(FuncObj);

  //------------------------ Rational --------------------------

  FuncObj = new FuncDescObj("Rational");
  FuncObj->SetFuncInfo(GSM_RationalFloat, 1, x_Float);
  FuncObj->SetFuncInfo(GSM_RationalInteger, 1, x_Int);
  FuncObj->SetFuncInfo(GSM_RationalRational, 1, x_Rational);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StartWatch");
  FuncObj->SetFuncInfo(GSM_StartWatch, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StopWatch");
  FuncObj->SetFuncInfo(GSM_StopWatch, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ElapsedTime");
  FuncObj->SetFuncInfo(GSM_ElapsedTime, 0);
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsWatchRunning");
  FuncObj->SetFuncInfo(GSM_IsWatchRunning, 0);
  gsm->AddFunction(FuncObj);


  //-------------------------- List -----------------------------

  FuncObj = new FuncDescObj("List");
  FuncObj->SetFuncInfo(GSM_List, 2, NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_List, 0, "x", porANYTYPE & 
			~(porINTEGER|porFLOAT|porRATIONAL));
  FuncObj->SetParamInfo(GSM_List, 1, "length", porINTEGER, 
			new IntValPortion(1));

  FuncObj->SetFuncInfo(GSM_List_List, 2, NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_List_List, 0, "x", PortionSpec(porANYTYPE,1));
  FuncObj->SetParamInfo(GSM_List_List, 1, "length", porINTEGER, 
			new IntValPortion(1));

  FuncObj->SetFuncInfo(GSM_List_Integer, 3, 
		       NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_List_Integer, 0, "x", porINTEGER);
  FuncObj->SetParamInfo(GSM_List_Integer, 1, "length", porINTEGER, 
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_List_Integer, 2, "delta", porINTEGER, 
			new IntValPortion(0));

  FuncObj->SetFuncInfo(GSM_List_Float, 3, NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_List_Float, 0, "x", porFLOAT);
  FuncObj->SetParamInfo(GSM_List_Float, 1, "length", porINTEGER, 
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_List_Float, 2, "delta", porFLOAT, 
			new FloatValPortion(0));

  FuncObj->SetFuncInfo(GSM_List_Rational, 3,
		       NO_PREDEFINED_PARAMS, NON_LISTABLE);
  FuncObj->SetParamInfo(GSM_List_Rational, 0, "x", porRATIONAL);
  FuncObj->SetParamInfo(GSM_List_Rational, 1, "length", porINTEGER, 
			new IntValPortion(1));
  FuncObj->SetParamInfo(GSM_List_Rational, 2, "delta", porRATIONAL, 
			new RationalValPortion(0));

  gsm->AddFunction(FuncObj);

  //--------------------------- Dot ----------------------------

  FuncObj = new FuncDescObj("Dot");
  
  FuncObj->SetFuncInfo(GSM_Dot_Integer, 2);
  FuncObj->SetParamInfo(GSM_Dot_Integer, 0, "x", PortionSpec(porINTEGER,1));
  FuncObj->SetParamInfo(GSM_Dot_Integer, 1, "y", PortionSpec(porINTEGER,1));

  FuncObj->SetFuncInfo(GSM_Dot_Float, 2);
  FuncObj->SetParamInfo(GSM_Dot_Float, 0, "x", PortionSpec(porFLOAT,1));
  FuncObj->SetParamInfo(GSM_Dot_Float, 1, "y", PortionSpec(porFLOAT,1));
  
  FuncObj->SetFuncInfo(GSM_Dot_Rational, 2);
  FuncObj->SetParamInfo(GSM_Dot_Rational, 0, "x", PortionSpec(porRATIONAL,1));
  FuncObj->SetParamInfo(GSM_Dot_Rational, 1, "y", PortionSpec(porRATIONAL,1));
  gsm->AddFunction(FuncObj);

  //----------------------- ArgMax ------------------------
  FuncObj = new FuncDescObj( "ArgMax" );
  FuncObj->SetFuncInfo(GSM_ArgMax_Integer, 1);
  FuncObj->SetParamInfo(GSM_ArgMax_Integer, 0, "x", PortionSpec(porINTEGER,1));
  FuncObj->SetFuncInfo(GSM_ArgMax_Float, 1);
  FuncObj->SetParamInfo(GSM_ArgMax_Float, 0, "x", PortionSpec(porFLOAT,1));
  FuncObj->SetFuncInfo(GSM_ArgMax_Rational, 1);
  FuncObj->SetParamInfo(GSM_ArgMax_Rational, 0, "x", 
			PortionSpec(porRATIONAL,1));
  gsm->AddFunction(FuncObj);

  //------------------ Transpose -----------------------
  FuncObj = new FuncDescObj( "Transpose" );
  FuncObj->SetFuncInfo(GSM_Transpose, 1);
  FuncObj->SetParamInfo(GSM_Transpose,
			0, "x", PortionSpec(porANYTYPE,2), 
			NO_DEFAULT_VALUE, PASS_BY_VALUE);
  gsm->AddFunction(FuncObj);


  //------------------ Sort -----------------------
  FuncObj = new FuncDescObj( "Sort" );
  FuncObj->SetFuncInfo(GSM_Sort_Integer, 1);
  FuncObj->SetParamInfo(GSM_Sort_Integer, 0, "x", PortionSpec(porINTEGER,1));
  FuncObj->SetFuncInfo(GSM_Sort_Float, 1);
  FuncObj->SetParamInfo(GSM_Sort_Float, 0, "x", PortionSpec(porFLOAT,1));
  FuncObj->SetFuncInfo(GSM_Sort_Rational, 1);
  FuncObj->SetParamInfo(GSM_Sort_Rational, 0, "x", PortionSpec(porRATIONAL,1));
  FuncObj->SetFuncInfo(GSM_Sort_Text, 1);
  FuncObj->SetParamInfo(GSM_Sort_Text, 0, "x", PortionSpec(porTEXT,1));
  gsm->AddFunction(FuncObj);

}




