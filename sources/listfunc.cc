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
  Portion* *a=new Portion* [n+1];
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


  FuncObj = new FuncDescObj("Length", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LengthList, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", 
					    PortionSpec(porANYTYPE,1)));
  gsm->AddFunction(FuncObj);
			
  FuncObj = new FuncDescObj("NumChars", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LengthText, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("text", porTEXT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Contains", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Contains, porBOOL, 2,
				       0, NON_LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", 
					    PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("x", porANYTYPE));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NthElement", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NthElement, porANYTYPE, 
				       2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", PortionSpec(porANYTYPE, 1),
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Remove", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Remove, 
				       PortionSpec(porANYTYPE, 1), 
				       2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list",
					    PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthChar", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NthChar, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("text", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);


  //--------------------------- Text -----------------------
  
  FuncObj = new FuncDescObj("Text", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_TextInt, porTEXT, 1, x_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_TextFloat, porTEXT, 1, x_Float));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_TextRat, porTEXT, 1, x_Rational));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_TextText, porTEXT, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porTEXT));
  gsm->AddFunction(FuncObj);
  
  //-------------------------- Integer ------------------------

  FuncObj = new FuncDescObj("Integer", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IntegerRational, porINTEGER, 
				       1, x_Rational));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IntegerInteger, porINTEGER,
				       1, x_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_IntegerFloat, porINTEGER,
				       1, x_Float));
  gsm->AddFunction(FuncObj);

  //-------------------------- Float ------------------------

  FuncObj = new FuncDescObj("Float", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_FloatRational, porFLOAT, 
				       1, x_Rational));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_FloatInteger, porFLOAT,
				       1, x_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_FloatFloat, porFLOAT, 
				       1, x_Float));
  gsm->AddFunction(FuncObj);

  //------------------------ Rational --------------------------

  FuncObj = new FuncDescObj("Rational", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_RationalFloat, 
				       porRATIONAL, 1, x_Float));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_RationalInteger, 
				       porRATIONAL, 1, x_Int));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_RationalRational, 
				       porRATIONAL, 1, x_Rational));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StartWatch", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StartWatch, porFLOAT, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StopWatch", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StopWatch, porFLOAT, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ElapsedTime", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElapsedTime, porFLOAT, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsWatchRunning", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsWatchRunning, porFLOAT, 0));
  gsm->AddFunction(FuncObj);


  //-------------------------- List -----------------------------

  FuncObj = new FuncDescObj("List", 5);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_List, PortionSpec(porANYTYPE, 1), 
				       2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porANYTYPE & 
					    ~(porINTEGER | porFLOAT | 
					      porRATIONAL)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("length", porINTEGER, 
					    new IntValPortion(1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_List_List, 
				       PortionSpec(porANYTYPE, 2), 
				       2, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("length", porINTEGER, 
					    new IntValPortion(1)));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_List_Integer, 
				       PortionSpec(porINTEGER, 1), 
				       3, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("length", porINTEGER, 
					    new IntValPortion(1)));
  FuncObj->SetParamInfo(2, 2, ParamInfoType("delta", porINTEGER, 
					    new IntValPortion(0)));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_List_Float, 
				       PortionSpec(porFLOAT, 1), 
				       3, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porFLOAT));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("length", porINTEGER, 
					    new IntValPortion(1)));
  FuncObj->SetParamInfo(3, 2, ParamInfoType("delta", porFLOAT, 
					    new FloatValPortion(0)));

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_List_Rational, 
				       PortionSpec(porRATIONAL, 1), 
				       3, 0, NON_LISTABLE));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("x", porRATIONAL));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("length", porINTEGER, 
					    new IntValPortion(1)));
  FuncObj->SetParamInfo(4, 2, ParamInfoType("delta", porRATIONAL, 
					    new RationalValPortion(0)));

  gsm->AddFunction(FuncObj);

  //--------------------------- Dot ----------------------------

  FuncObj = new FuncDescObj("Dot", 3);
  
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Dot_Integer, porINTEGER, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porINTEGER,1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("y", PortionSpec(porINTEGER,1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Dot_Float, porFLOAT, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porFLOAT,1)));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("y", PortionSpec(porFLOAT,1)));
  
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Dot_Rational, porRATIONAL, 2));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", PortionSpec(porRATIONAL,1)));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("y", PortionSpec(porRATIONAL,1)));
  gsm->AddFunction(FuncObj);

  //----------------------- ArgMax ------------------------
  FuncObj = new FuncDescObj("ArgMax", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ArgMax_Integer, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porINTEGER,1)));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ArgMax_Float, porINTEGER, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porFLOAT,1)));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_ArgMax_Rational, porINTEGER, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", PortionSpec(porRATIONAL,1)));
  gsm->AddFunction(FuncObj);

  //------------------ Transpose -----------------------
  FuncObj = new FuncDescObj("Transpose", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Transpose, 
				       PortionSpec(porANYTYPE, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porANYTYPE,2), 
					    REQUIRED, BYVAL));
  gsm->AddFunction(FuncObj);


  //------------------ Sort -----------------------
  FuncObj = new FuncDescObj("Sort", 4);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Sort_Integer, 
				       PortionSpec(porINTEGER, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porINTEGER,1)));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Sort_Float, 
				       PortionSpec(porFLOAT, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porFLOAT,1)));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Sort_Rational, 
				       PortionSpec(porRATIONAL, 1), 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", PortionSpec(porRATIONAL,1)));
  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Sort_Text, porTEXT, 1));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", PortionSpec(porTEXT,1)));
  gsm->AddFunction(FuncObj);

}




