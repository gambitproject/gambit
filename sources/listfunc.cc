//
// FILE: listfunc.cc -- List and text oriented function implementations
//
// $Id$
//

#include <assert.h>

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "gmisc.h"
#include "rational.h"

#include "nfg.h"
#include "efg.h"



//---------
// Concat
//---------

static Portion* GSM_Concat_List(Portion** param)
{
  const gList<Portion*>& p_value = ((ListPortion*) param[1])->Value();
  Portion *result = new ListPortion(((ListPortion*) param[0])->Value());
  for (int i = 1; i <= p_value.Length(); i++)  {
    int append_result = ((ListPortion*) result)->Append(p_value[i]->ValCopy());
    if(append_result == 0)  {
      delete result;
      result = new ErrorPortion
	("Attempted concatenating lists of different types");
      break;
    }
  }
  return result;
}


//------------
// Index
//------------

Portion* GSM_Index( Portion** param )
{
  int i = 0;
  ListPortion* result = new ListPortion();
  ListPortion& list = *(ListPortion*) param[0];
  bool type_found = false;
  for( i = 1; i <= list.Length(); i++ )
  {
    if( PortionEqual( list[i], param[1], type_found ) ) 
      result->Append( new IntPortion( i ) );
  }
  return result;
}




//-------------------
// Flatten
//-------------------


void GSM_Flatten_Top(ListPortion* list, int levels, int depth, 
		     ListPortion* result)
{
  int Length = list->Length();
  int i;
  assert(levels >= 0);
  if(levels == 0)
  {
    for(i=1; i<=Length; i++)
      if((*list)[i]->Spec().ListDepth == 0)
	result->Append((*list)[i]->ValCopy());
      else
	GSM_Flatten_Top((ListPortion*)(*list)[i], levels, depth+1, result);
  }
  else if(levels > 0)
  {
    if(depth >= levels)
      for(i=1; i<=Length; i++)
	result->Append((*list)[i]->ValCopy());
    else
      for(i=1; i<=Length; i++)
	if((*list)[i]->Spec().ListDepth == 0)
	  result->Append((*list)[i]->ValCopy());
	else
	  GSM_Flatten_Top((ListPortion*)(*list)[i], levels, depth+1, result);
  }
}

void GSM_Flatten_Bottom(ListPortion* list, int levels, int depth, 
			ListPortion* result)
{
  int Length = result->Length();
  int i;
  assert(levels > 0);
  assert(list == 0);
  if(depth >= levels-1)
  {
    for(i=1; i<=Length; i++)
      if((*result)[i]->Spec().ListDepth == 0)
	;
      else
      {
	ListPortion* oldsublist = (ListPortion*) result->Remove(i);
	ListPortion* newsublist = new ListPortion();
	GSM_Flatten_Top(oldsublist, 0, depth+1, newsublist);
	result->Insert(newsublist, i);
	delete oldsublist;
      }
  }
  else
  {
    for(i=1; i<=Length; i++)
      if((*result)[i]->Spec().ListDepth == 0)
	;
      else
	GSM_Flatten_Bottom(0, levels, depth+1, (ListPortion*)(*result)[i]);
  }
}

Portion* GSM_Flatten(Portion** param)
{
  // if levels > 0, flatten from bottom
  ListPortion* list;
  int levels = ((IntPortion*) param[1])->Value();
  if(levels >= 0)
  {
    list = new ListPortion();
    GSM_Flatten_Top((ListPortion*) param[0], levels, 0, list);
  }
  else
  {
    list = (ListPortion*) ((ListPortion*) param[0])->ValCopy();
    GSM_Flatten_Bottom(0, -levels, 0, list);
  }
    
  return list;
}


bool ListDimCheck(ListPortion* p0, ListPortion* p1)
{
  int i;
  int Length;
  assert(p0->Spec().ListDepth > 0);
  assert(p1->Spec().ListDepth > 0);
  Length = p0->Length();
  if(Length != p1->Length())
    return false;
  for(i = 1; i <= Length; i++)
  {
    if((*p0)[i]->Spec().ListDepth != (*p1)[i]->Spec().ListDepth)
      return false;
    if((*p0)[i]->Spec().ListDepth > 0)
      if(!ListDimCheck((ListPortion*)(*p0)[i], (ListPortion*)(*p1)[i]))
	return false;
  }
  return true;
}

ListPortion* GSM_Filter_Aid(ListPortion* p0, ListPortion* p1)
{
  int i;
  int Length = p0->Length();
  ListPortion* list = new ListPortion();
  for(i=1; i<=Length; i++)
    if((*p0)[i]->Spec().ListDepth == 0)
    {
      assert((*p1)[i]->Spec().Type == porBOOL);
      if(((BoolPortion*) (*p1)[i])->Value())
	list->Append((*p0)[i]->ValCopy());
    }
    else
      list->Append(GSM_Filter_Aid((ListPortion*)(*p0)[i], 
				  (ListPortion*)(*p1)[i]));
  return list;
}

Portion* GSM_Filter(Portion** param)
{
  if(!ListDimCheck((ListPortion*) param[0], (ListPortion*) param[1]))
    return new ErrorPortion("Mismatching list dimensions");
  ListPortion* list = GSM_Filter_Aid((ListPortion*) param[0], 
				     (ListPortion*) param[1]);
  list->SetDataType( param[0]->Spec().Type );
  return list;
}


Portion* GSM_Sort(Portion** param, bool (*compfunc)(Portion*, Portion*), 
		  bool altsort = false)
{
  // If altsort, param[1] is rearranged using the sorted result of param[0]

  unsigned long n = ((ListPortion*) param[0])->Length();
  
  assert(param[0]);
  if(altsort)
    assert(param[1]);

  if(altsort)
    if(n != (unsigned) ((ListPortion*) param[1])->Length())
      return new ErrorPortion("Mismatching list dimensions");
  
  Portion* *a=new Portion* [n+1];
  Portion* *b=new Portion* [n+1];
  unsigned long i, j, inc;
  Portion* va; 
  Portion* vb; 
  bool no_sub_lists = true;

  for(i=1; i<=n; i++)
  {
    a[i] = ((ListPortion*) param[0])->Remove(1);
    if(a[i]->Spec().ListDepth > 0)
      no_sub_lists = false;
    if(altsort)
    {
      b[i] = ((ListPortion*) param[1])->Remove(1);
      if(b[i]->Spec().ListDepth > 0)
	no_sub_lists = false;
    }
  }

  if(no_sub_lists)
  {
    // sort via Shell's method, adopted from 
    // _Numerical_Recipes_in_C_, 2nd Edition, p. 332
    inc = 1;
    do {
      inc *= 3;
      inc++;
    } while(inc <= n);
    do {
      inc /= 3;
      for(i=inc+1; i<=n; i++) {
	va = a[i];
	vb = b[i];
	j=i;
	while(compfunc(a[j-inc], va)) {
	  a[j] = a[j-inc];
	  b[j] = b[j-inc];
	  j -= inc;
	  if(j <= inc) break;
	}
	a[j] = va;
	b[j] = vb;
      }
    } while(inc > 1);
  }

  for(i=1; i<=n; i++)
  {
    ((ListPortion*) param[0])->Append(a[i]);
    if(altsort)
      ((ListPortion*) param[1])->Append(b[i]);      
  }

  delete[] a;
  delete[] b;

  if(no_sub_lists)
  {
    if(!altsort)
      return param[0]->ValCopy();
    else
      return param[1]->ValCopy();
  }
  else
    return new ErrorPortion("Cannot sort a nested list");
}


bool GSM_Compare_Integer(Portion* p1, Portion* p2)
{ return ((IntPortion*) p1)->Value() > ((IntPortion*) p2)->Value(); }
Portion* GSM_Sort_Integer(Portion** param)
{ return GSM_Sort(param, GSM_Compare_Integer); }
Portion* GSM_Sort_By_Integer(Portion** param)
{
  Portion* p[2]; 
  p[0] = param[1]; 
  p[1] = param[0];
  return GSM_Sort(p, GSM_Compare_Integer, true);
}

bool GSM_Compare_Number(Portion* p1, Portion* p2)
{ return ((NumberPortion*) p1)->Value() > ((NumberPortion*) p2)->Value(); }
Portion* GSM_Sort_Number(Portion** param)
{ return GSM_Sort(param, GSM_Compare_Number); }
Portion* GSM_Sort_By_Number(Portion** param)
{
  Portion* p[2]; 
  p[0] = param[1]; 
  p[1] = param[0];
  return GSM_Sort(p, GSM_Compare_Number, true);
}

bool GSM_Compare_Text(Portion* p1, Portion* p2)
{ return ((TextPortion*) p1)->Value() > ((TextPortion*) p2)->Value(); }
Portion* GSM_Sort_Text(Portion** param)
{ return GSM_Sort(param, GSM_Compare_Text); }
Portion* GSM_Sort_By_Text(Portion** param)
{
  Portion* p[2]; 
  p[0] = param[1]; 
  p[1] = param[0];
  return GSM_Sort(p, GSM_Compare_Text, true);
}




Portion *GSM_NthElement(Portion **param)
{
  int n = ((IntPortion *) param[1])->Value();
  if(n <= 0 || n > ((ListPortion *) param[0])->Length())
    return new ErrorPortion("Subscript out of range");
  else
    return ((ListPortion *) param[0])->SubscriptCopy(n);
}

Portion *GSM_Remove(Portion **param)
{
  ListPortion *ret = (ListPortion *) param[0]->ValCopy();
  delete ret->Remove(((IntPortion *) param[1])->Value());
  return ret;
}



Portion *GSM_Contains(Portion **param)
{
  return new BoolPortion(((ListPortion *) param[0])->Contains(param[1]));
}

Portion *GSM_NumElements(Portion **param)
{
  return new IntPortion(((ListPortion *) param[0])->Length());
}

Portion *GSM_LengthList(Portion **param)
{
  return new IntPortion(((ListPortion *) param[0])->Length());
}

Portion *GSM_LengthText(Portion **param)
{
  return new IntPortion(((TextPortion *) param[0])->Value().Length());
}

Portion *GSM_NthChar(Portion **param)
{
  gText text(((TextPortion *) param[0])->Value());
  int n = ((IntPortion *) param[1])->Value();
  if (n <= 0 || n > text.Length())
    return 0;
  return new TextPortion(text[n-1]);
}


//--------------------------- Text ---------------------------

Portion *GSM_TextInt(Portion **param)
{
  return new TextPortion(ToText(((IntPortion *) param[0])->Value()));
}

Portion *GSM_Text_Number(Portion **param)
{
  return new TextPortion(ToText(((NumberPortion *) param[0])->Value()));
}

Portion *GSM_TextText(Portion **param)
{
  return param[0]->ValCopy();
}


//------------------------ Integer --------------------------

Portion *GSM_IntegerNumber(Portion **param)
{
  return new IntPortion((long) ((NumberPortion *) param[0])->Value());
}

Portion *GSM_IntegerInteger(Portion **param)
{
  return param[0]->ValCopy();
}


//------------------------- Number ------------------------

Portion *GSM_NumberInteger(Portion **param)
{
  return new NumberPortion(((IntPortion *) param[0])->Value());
}

Portion *GSM_NumberNumber(Portion **param)
{
  return param[0]->ValCopy();
}



//----------------------------- Stop Watch ----------------------

#include "gwatch.h"

gWatch _gcl_watch(0);

Portion *GSM_StartWatch(Portion **)
{
  _gcl_watch.Start();
  return new NumberPortion(0.0);
}

Portion *GSM_StopWatch(Portion **)
{
  _gcl_watch.Stop();
  return new NumberPortion(_gcl_watch.Elapsed());
}

Portion *GSM_ElapsedTime(Portion **)
{
  return new NumberPortion(_gcl_watch.Elapsed());
}

Portion *GSM_IsWatchRunning(Portion **)
{
  return new BoolPortion(_gcl_watch.IsRunning());
}

//--------------------------- List ------------------------------

Portion* GSM_List( Portion** param )
{
  ListPortion* p;
  int i;
  assert( param[0]->Spec().Type != porERROR );  

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list Length" );

  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );
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
    return new ErrorPortion( "Invalid list Length" );

  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( param[0]->ValCopy() );
  return p;
}


Portion* GSM_List_Integer( Portion** param )
{
  ListPortion* p;
  int i;

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list Length" );

  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( new IntPortion( ((IntPortion*) param[0])->Value()+ 
				 (i-1)*((IntPortion*) param[2])->Value() ));
  return p;
}

Portion* GSM_List_Number( Portion** param )
{
  ListPortion* p;
  int i;

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list Length" );

  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
    p->Append( new NumberPortion( ((NumberPortion*) param[0])->Value()+
				      (gNumber)(i-1)*
				      ((NumberPortion*) param[2])->Value()));
  return p;
}


Portion* GSM_List_Nfg( Portion** param )
{
  ListPortion* p;
  int i;
  assert( param[0]->Spec().Type != porERROR );  

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list Length" );


  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );

  Nfg& nfg =
      * (((NfgPortion *) param[0])->Value());
    for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
      p->Append(new NfgPortion(new Nfg(nfg)));
  return p;
}


Portion* GSM_List_Efg( Portion** param )
{
  ListPortion* p;
  int i;
  assert( param[0]->Spec().Type != porERROR );  

  if( ((IntPortion*) param[1])->Value() < 0 )
    return new ErrorPortion( "Invalid list Length" );


  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );

  Efg& efg =
      *(((EfgPortion*) param[0])->Value());
    for( i = 1; i <= ((IntPortion*) param[1])->Value(); i++ )
      p->Append( new EfgPortion( new Efg( efg ) ) );  

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

  p = new IntPortion( (long) 0 );
  for( i = 1; i <= p1->Length(); i++ )
  {
    ((IntPortion*) p)->Value() += 
      (((IntPortion*) (*p1)[i])->Value() * 
       ((IntPortion*) (*p2)[i])->Value());
  }
  return p;
}


Portion* GSM_Dot_Number(Portion **param)
{
  int i;
  Portion* p;
  ListPortion* p1 = (ListPortion*) param[0];
  ListPortion* p2 = (ListPortion*) param[1];

  p = GSM_Dot_Check( p1, p2 );
  if( p != 0 )
    return p;

  p = new NumberPortion( (gRational) 0 );
  for( i = 1; i <= p1->Length(); i++ )
  {
    ((NumberPortion*) p)->Value() +=
      (((NumberPortion*) (*p1)[i])->Value() *
       ((NumberPortion*) (*p2)[i])->Value());
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
    if( p->Spec() == porINTEGER )
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
  return new IntPortion( index );
}

Portion* GSM_ArgMax_Number( Portion** param )
{
  Portion* p;
  gRational max = 0;
  int index = 0;
  int i;
  assert( param[0]->Spec().ListDepth > 0 );
  for( i = ((ListPortion*) param[0])->Length(); i >= 1; i-- )
  {
    p = (*(ListPortion*) param[0])[i];
    if( p->Spec() == porNUMBER )
    {
      if( ((NumberPortion*) p)->Value() >= gNumber(max) ||
	 i == ((ListPortion*) param[0])->Length() )
      {
	max = ((NumberPortion*) p)->Value();
	index = i;
      }
    }
    else
      return new ErrorPortion( "Bad dimensionality" );
  }
  return new IntPortion( index );
}


//--------------------------- Transpose -------------------------

Portion* GSM_Transpose( Portion** param )
{
  int i;
  int j;
  int Length = 0;
  int width = 0;
  ListPortion* p;
  ListPortion* s;
  assert( param[0]->Spec().ListDepth > 0 );
  Length = ((ListPortion*) param[0])->Length();
  for( i = 1; i <= Length; i++ )
  {
    if( (*(ListPortion*) param[0])[i]->Spec().ListDepth == 0 )
      return new ErrorPortion( "Bad dimensionality" );
    if( i == 1 )
      width = ((ListPortion*) (*(ListPortion*) param[0])[i])->Length();
    else 
      if( ((ListPortion*) (*(ListPortion*) param[0])[i])->Length() != width )
	return new ErrorPortion( "Bad dimensionality" );
  }
  p = new ListPortion();
  for( i = 1; i <= width; i++ )
  {
    s = new ListPortion();
    for( j = 1; j <= Length; j++ )
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

  ParamInfoType x_Int[] =
  {
    ParamInfoType( "x", porINTEGER )
  };
  ParamInfoType x_Number[] =
  {
    ParamInfoType( "x", porNUMBER )
  };



  FuncObj = new FuncDescObj("Concat", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Concat_List, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("y", 
					    PortionSpec(porANYTYPE, NLIST)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Index", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Index, 
				       PortionSpec(porINTEGER, 1), 2, 0,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("x", porANYTYPE));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Index, 
				       PortionSpec(porINTEGER, 1), 2, 0,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("x", PortionSpec(porANYTYPE, 1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NumElements", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NumElements, porINTEGER, 1,
				       0, funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", 
                              PortionSpec(porANYTYPE, 1, porNULLSPEC )));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Length", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LengthList, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", 
                              PortionSpec(porANYTYPE, 1, porNULLSPEC )));
  gsm->AddFunction(FuncObj);
			
  FuncObj = new FuncDescObj("NumChars", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_LengthText, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("text", porTEXT));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Contains", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Contains, porBOOL, 2,
				       0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("x", porANYTYPE));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Contains, porBOOL, 2,
				       0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("x", 
					    PortionSpec(porANYTYPE,1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("NthElement", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NthElement, porANYTYPE, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list", 
					    PortionSpec(porANYTYPE, NLIST),
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Remove", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Remove, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("list",
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("NthChar", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NthChar, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("text", porTEXT));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("n", porINTEGER));
  gsm->AddFunction(FuncObj);


  //--------------------------- Text -----------------------
  
  FuncObj = new FuncDescObj("Text", 3);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_TextInt, porTEXT, 1, x_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Text_Number, porTEXT, 1, x_Number));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_TextText, porTEXT, 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porTEXT));
  gsm->AddFunction(FuncObj);
  
  //-------------------------- Integer ------------------------

  FuncObj = new FuncDescObj("Integer", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IntegerNumber, porINTEGER,
				       1, x_Number));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_IntegerInteger, porINTEGER,
				       1, x_Int));
  gsm->AddFunction(FuncObj);

  //------------------------ Number --------------------------

  FuncObj = new FuncDescObj("Number", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_NumberInteger,
				       porNUMBER, 1, x_Int));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_NumberNumber,
				       porNUMBER, 1, x_Number));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StartWatch", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StartWatch, porNUMBER, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("StopWatch", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_StopWatch, porNUMBER, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("ElapsedTime", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ElapsedTime, porNUMBER, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("IsWatchRunning", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_IsWatchRunning, porBOOL, 0));
  gsm->AddFunction(FuncObj);


  //-------------------------- List -----------------------------

  FuncObj = new FuncDescObj("List", 6);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_List, PortionSpec(porANYTYPE, 1), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", porANYTYPE & 
					    ~(porINTEGER | porNUMBER | porNFG | porEFG )));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("length", porINTEGER, 
					    new IntPortion(1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_List_List, 
				       PortionSpec(porANYTYPE, 2), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("length", porINTEGER, 
					    new IntPortion(1)));

  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_List_Integer, 
				       PortionSpec(porINTEGER, 1), 
				       3, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", porINTEGER));
  FuncObj->SetParamInfo(2, 1, ParamInfoType("length", porINTEGER, 
					    new IntPortion(1)));
  FuncObj->SetParamInfo(2, 2, ParamInfoType("delta", porINTEGER, 
					    new IntPortion(0)));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_List_Number,
				       PortionSpec(porNUMBER, 1),
				       3, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", porNUMBER));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("length", porINTEGER,
					    new IntPortion(1)));
  FuncObj->SetParamInfo(3, 2, ParamInfoType("delta", porNUMBER,
					    new NumberPortion(0)));

  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_List_Nfg,
				       PortionSpec(porNFG, 1), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("x", porNFG));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("length", porINTEGER,
					    new IntPortion(1)));

  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_List_Efg,
				       PortionSpec(porEFG, 1), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("x", porEFG));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("length", porINTEGER,
					    new IntPortion(1)));

  gsm->AddFunction(FuncObj);

  //--------------------------- Dot ----------------------------

  FuncObj = new FuncDescObj("Dot", 2);
  
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Dot_Integer, porINTEGER, 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porINTEGER,1)));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("y", PortionSpec(porINTEGER,1)));

  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Dot_Number, porNUMBER, 2));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porNUMBER,1)));
  FuncObj->SetParamInfo(1, 1, ParamInfoType("y", PortionSpec(porNUMBER,1)));
  gsm->AddFunction(FuncObj);

  //----------------------- ArgMax ------------------------
  FuncObj = new FuncDescObj("ArgMax", 2);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_ArgMax_Integer, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porINTEGER,1)));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_ArgMax_Number, porINTEGER, 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porNUMBER,1)));
  gsm->AddFunction(FuncObj);

  //------------------ Transpose -----------------------
  FuncObj = new FuncDescObj("Transpose", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Transpose, 
				       PortionSpec(porANYTYPE, 2), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porANYTYPE,2), 
					    REQUIRED, BYVAL));
  gsm->AddFunction(FuncObj);


  //------------------ Sort -----------------------
  FuncObj = new FuncDescObj("Sort", 6);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Sort_Integer, 
				       PortionSpec(porINTEGER, 1), 1));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", PortionSpec(porINTEGER,1)));
  FuncObj->SetFuncInfo(1, FuncInfoType(GSM_Sort_Number,
				       PortionSpec(porNUMBER, 1), 1));
  FuncObj->SetParamInfo(1, 0, ParamInfoType("x", PortionSpec(porNUMBER,1)));
  FuncObj->SetFuncInfo(2, FuncInfoType(GSM_Sort_Text,
				       PortionSpec(porTEXT, 1), 1));
  FuncObj->SetParamInfo(2, 0, ParamInfoType("x", PortionSpec(porTEXT,1)));

  FuncObj->SetFuncInfo(3, FuncInfoType(GSM_Sort_By_Integer,
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(3, 0, ParamInfoType("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(3, 1, ParamInfoType("by", PortionSpec(porINTEGER,1)));
  FuncObj->SetFuncInfo(4, FuncInfoType(GSM_Sort_By_Number,
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(4, 0, ParamInfoType("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(4, 1, ParamInfoType("by", PortionSpec(porNUMBER,1)));
  FuncObj->SetFuncInfo(5, FuncInfoType(GSM_Sort_By_Text,
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(5, 0, ParamInfoType("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(5, 1, ParamInfoType("by", PortionSpec(porTEXT,1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new FuncDescObj("Filter", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Filter, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", 
                              PortionSpec(porANYTYPE, NLIST, porNULLSPEC )));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("y", 
					    PortionSpec(porBOOL, NLIST)));
  gsm->AddFunction(FuncObj);

  FuncObj = new FuncDescObj("Flatten", 1);
  FuncObj->SetFuncInfo(0, FuncInfoType(GSM_Flatten, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, ParamInfoType("x", 
                              PortionSpec(porANYTYPE, NLIST, porNULLSPEC )));
  FuncObj->SetParamInfo(0, 1, ParamInfoType("levels", porINTEGER,
					    new IntPortion(0)));
  gsm->AddFunction(FuncObj);

}




