//
// FILE: listfunc.cc -- List and text oriented function implementations
//
// $Id$
//

#include "base/base.h"

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "math/rational.h"
#include "math/gsmatrix.h"

#include "nfg.h"
#include "efg.h"



//---------
// Concat
//---------

static Portion* GSM_Concat_List(GSM &, Portion** param)
{
  const gList<Portion*>& p_value = ((ListPortion*) param[1])->Value();
  Portion *result = new ListPortion(((ListPortion*) param[0])->Value());
  for (int i = 1; i <= p_value.Length(); i++)  {
    int append_result = ((ListPortion*) result)->Append(p_value[i]->ValCopy());
    if(append_result == 0)  {
      delete result;
      throw gclRuntimeError("Attempted concatenating lists of different types");
    }
  }
  return result;
}


//------------
// Index
//------------

static Portion* GSM_Index(GSM &, Portion** param)
{
  ListPortion* result = new ListPortion();
  ListPortion& list = *(ListPortion*) param[0];
  bool type_found = false;
  for (int i = 1; i <= list.Length(); i++)  {
    if (PortionEqual(list[i], param[1], type_found)) 
      result->Append(new NumberPortion(i));
  }
  return result;
}




//-------------------
// Flatten
//-------------------

static void GSM_Flatten_Top(ListPortion* list, int levels, int depth, 
			    ListPortion* result)
{
  int Length = list->Length();
  int i;
  if (levels < 0) {
    throw gclRuntimeError("Internal error in Flatten[]");
  }

  if(levels == 0) {
    for (i=1; i<=Length; i++)
      if((*list)[i]->Spec().ListDepth == 0)
	result->Append((*list)[i]->ValCopy());
      else
	GSM_Flatten_Top((ListPortion*)(*list)[i], levels, depth+1, result);
  }
  else if(levels > 0) {
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

static void GSM_Flatten_Bottom(ListPortion* list, int levels, int depth, 
			       ListPortion* result)
{
  int Length = result->Length();
  int i;
  if (levels <= 0 || list != 0) {
    throw gclRuntimeError("Internal error in Flatten[]");
  }
  if(depth >= levels-1) {
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

static Portion* GSM_Flatten(GSM &, Portion** param)
{
  // if levels > 0, flatten from bottom
  ListPortion* list;
  int levels = ((NumberPortion*) param[1])->Value();
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


static bool ListDimCheck(ListPortion* p0, ListPortion* p1)
{
  int i;
  int Length;
  if (p0->Spec().ListDepth <= 0 || p1->Spec().ListDepth <= 0) {
    throw gclRuntimeError("Internal error in ListDimCheck()");
  }
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

static ListPortion* GSM_Filter_Aid(ListPortion* p0, ListPortion* p1)
{
  int i;
  int Length = p0->Length();
  ListPortion* list = new ListPortion();
  for(i=1; i<=Length; i++)
    if((*p0)[i]->Spec().ListDepth == 0) {
      if ((*p1)[i]->Spec().Type != porBOOLEAN) {
	throw gclRuntimeError("Internal error in Filter[]");
      }
      if(((BoolPortion*) (*p1)[i])->Value())
	list->Append((*p0)[i]->ValCopy());
    }
    else
      list->Append(GSM_Filter_Aid((ListPortion*)(*p0)[i], 
				  (ListPortion*)(*p1)[i]));
  return list;
}

static Portion* GSM_Filter(GSM &, Portion** param)
{
  if(!ListDimCheck((ListPortion*) param[0], (ListPortion*) param[1]))
    throw gclRuntimeError("Mismatching list dimensions");
  ListPortion* list = GSM_Filter_Aid((ListPortion*) param[0], 
				     (ListPortion*) param[1]);
  list->SetDataType( param[0]->Spec().Type );
  return list;
}


static Portion* GSM_Sort(Portion** param, 
			 bool (*compfunc)(Portion*, Portion*), 
			 bool altsort = false)
{
  // If altsort, param[1] is rearranged using the sorted result of param[0]

  unsigned long n = ((ListPortion*) param[0])->Length();
  
  if (!param[0]) {
    throw gclRuntimeError("Internal error in Sort[]");
  }
  if (altsort && !param[1]) {
    throw gclRuntimeError("Internal error in Sort[]");
  }

  if(altsort)
    if(n != (unsigned) ((ListPortion*) param[1])->Length())
      throw gclRuntimeError("Mismatching list dimensions");
  
  Portion* *a=new Portion* [n+1];
  Portion* *b=new Portion* [n+1];
  unsigned long* c=new unsigned long[n+1];
  unsigned long i, j, inc;
  Portion* va; 
  unsigned long vc;
  bool no_sub_lists = true;

  for(i=1; i<=n; i++)
  {
    c[i] = i;
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
    // sort via Shell's method, adapted from 
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
	vc = c[i];
	j=i;
	// RDM 9/24/00: changed test to maintain original ordering  
	// under equality
	while(compfunc(a[j-inc], va) 
	      || (!compfunc(va,a[j-inc]) && (c[j-inc] > vc))) {
	  a[j] = a[j-inc];
	  c[j] = c[j-inc];
	  j -= inc;
	  if(j <= inc) break;
	}
	a[j] = va;
	c[j] = vc;
      }
    } while(inc > 1);
  }

  for(i=1; i<=n; i++)
  {
    ((ListPortion*) param[0])->Append(a[i]);
    if(altsort)
      ((ListPortion*) param[1])->Append(b[c[i]]);      
  }

  delete[] a;
  delete[] b;
  delete[] c;

  if(no_sub_lists)
  {
    if(!altsort)
      return param[0]->ValCopy();
    else
      return param[1]->ValCopy();
  }
  else
    throw gclRuntimeError("Cannot sort a nested list");
}


#ifdef UNUSED
static bool GSM_Compare_Integer(Portion* p1, Portion* p2)
{ return ((NumberPortion*) p1)->Value() > ((NumberPortion*) p2)->Value(); }

static Portion* GSM_Sort_Integer(GSM &, Portion** param)
{ return GSM_Sort(param, GSM_Compare_Integer); }

static Portion* GSM_Sort_By_Integer(GSM &, Portion** param)
{
  Portion* p[2]; 
  p[0] = param[1]; 
  p[1] = param[0];
  return GSM_Sort(p, GSM_Compare_Integer, true);
}
#endif  // UNUSED

bool GSM_Compare_Number(Portion* p1, Portion* p2)
{ return ((NumberPortion*) p1)->Value() > ((NumberPortion*) p2)->Value(); }
Portion* GSM_Sort_Number(GSM &, Portion** param)
{ return GSM_Sort(param, GSM_Compare_Number); }
static Portion* GSM_Sort_By_Number(GSM &, Portion** param)
{
  Portion* p[2]; 
  p[0] = param[1]; 
  p[1] = param[0];
  return GSM_Sort(p, GSM_Compare_Number, true);
}

static bool GSM_Compare_Text(Portion* p1, Portion* p2)
{ return ((TextPortion*) p1)->Value() > ((TextPortion*) p2)->Value(); }
static Portion* GSM_Sort_Text(GSM &, Portion** param)
{ return GSM_Sort(param, GSM_Compare_Text); }
static Portion* GSM_Sort_By_Text(GSM &, Portion** param)
{
  Portion* p[2]; 
  p[0] = param[1]; 
  p[1] = param[0];
  return GSM_Sort(p, GSM_Compare_Text, true);
}




static Portion *GSM_NthElement(GSM &, Portion **param)
{
  int n = ((NumberPortion *) param[1])->Value();
  if(n <= 0 || n > ((ListPortion *) param[0])->Length())
    throw gclRuntimeError("Subscript out of range");
  else
    return ((ListPortion *) param[0])->SubscriptCopy(n);
}

static Portion *GSM_Remove(GSM &, Portion **param)
{
  ListPortion *ret = (ListPortion *) param[0]->ValCopy();
  delete ret->Remove(((NumberPortion *) param[1])->Value());
  return ret;
}

static Portion *GSM_Contains(GSM &, Portion **param)
{
  return new BoolPortion(((ListPortion *) param[0])->Contains(param[1]));
}

static Portion *GSM_NumElements(GSM &, Portion **param)
{
  return new NumberPortion(((ListPortion *) param[0])->Length());
}

static Portion *GSM_LengthList(GSM &, Portion **param)
{
  return new NumberPortion(((ListPortion *) param[0])->Length());
}

static Portion *GSM_LengthText(GSM &, Portion **param)
{
  return new NumberPortion(((TextPortion *) param[0])->Value().Length());
}

static Portion *GSM_NthElementText(GSM &, Portion **param)
{
  gText text(((TextPortion *) param[0])->Value());
  int n = ((NumberPortion *) param[1])->Value();
  if (n <= 0 || n > text.Length())
    return 0;
  return new TextPortion(text[n-1]);
}


//--------------------------- Text ---------------------------

static Portion *GSM_Text_Number(GSM &, Portion **param)
{
  return new TextPortion(ToText(((NumberPortion *) param[0])->Value()));
}

static Portion *GSM_TextText(GSM &, Portion **param)
{
  return param[0]->ValCopy();
}


//------------------------ Integer --------------------------

static Portion *GSM_IntegerNumber(GSM &, Portion **param)
{
  return new NumberPortion((long) ((NumberPortion *) param[0])->Value());
}


gWatch _gcl_watch(0);

static Portion *GSM_StartWatch(GSM &, Portion **)
{
  _gcl_watch.Start();
  return new NumberPortion(0.0);
}

static Portion *GSM_StopWatch(GSM &, Portion **)
{
  _gcl_watch.Stop();
  return new NumberPortion(_gcl_watch.Elapsed());
}

static Portion *GSM_ElapsedTime(GSM &, Portion **)
{
  return new NumberPortion(_gcl_watch.Elapsed());
}

static Portion *GSM_IsWatchRunning(GSM &, Portion **)
{
  return new BoolPortion(_gcl_watch.IsRunning());
}

//--------------------------- List ------------------------------

static Portion* GSM_List(GSM &, Portion** param)
{
  ListPortion* p;
  int i;

  if( ((NumberPortion*) param[1])->Value() < gNumber(0))
    throw gclRuntimeError( "Invalid list Length" );

  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= (double) ((NumberPortion*) param[1])->Value(); i++ )
    p->Append( param[0]->ValCopy() );
  return p;
}

static Portion* GSM_List_List(GSM &, Portion **param)
{
  ListPortion* p;
  int i;

  if( ((NumberPortion*) param[1])->Value() < gNumber(0) )
    throw gclRuntimeError( "Invalid list Length" );

  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= (double) ((NumberPortion*) param[1])->Value(); i++ )
    p->Append( param[0]->ValCopy() );
  return p;
}

static Portion* GSM_List_Number(GSM &, Portion **param)
{
  ListPortion* p;
  int i;

  if( ((NumberPortion*) param[1])->Value() < gNumber(0) )
    throw gclRuntimeError( "Invalid list Length" );

  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );
  for( i = 1; i <= (double) ((NumberPortion*) param[1])->Value(); i++ )
    p->Append( new NumberPortion( ((NumberPortion*) param[0])->Value()+
				      (gNumber)(i-1)*
				      ((NumberPortion*) param[2])->Value()));
  return p;
}

static Portion* GSM_List_Nfg(GSM &, Portion **param)
{
  ListPortion* p;
  int i;

  if( ((NumberPortion*) param[1])->Value() < gNumber(0) )
    throw gclRuntimeError( "Invalid list Length" );


  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );

  Nfg& nfg =
      * (((NfgPortion *) param[0])->Value());
    for( i = 1; i <= (double) ((NumberPortion*) param[1])->Value(); i++ )
      p->Append(new NfgPortion(new Nfg(nfg)));
  return p;
}

static Portion* GSM_List_Efg(GSM &, Portion **param)
{
  ListPortion* p;
  int i;

  if( ((NumberPortion*) param[1])->Value() < gNumber(0) )
    throw gclRuntimeError( "Invalid list Length" );


  p = new ListPortion();
  p->SetDataType( param[0]->Spec().Type );

  FullEfg& efg =
      *(((EfgPortion*) param[0])->Value());
  for( i = 1; i <= (double) ((NumberPortion*) param[1])->Value(); i++ )
    p->Append( new EfgPortion( new FullEfg( efg ) ) );  

  return p;
}


static Portion* GSM_Dot_Check( ListPortion* p1, ListPortion* p2 )
{
  int i;
  if( p1->Length() != p2->Length() )
    throw gclRuntimeError( "Mismatched dimentionalities" );
  for( i = 1; i <= p1->Length(); i++ )
    if( (*p1)[ i ]->Spec().ListDepth > 0 )
      throw gclRuntimeError("Can only operate on 1-D lists");
  return 0;
}

static Portion* GSM_Dot(GSM &, Portion **param)
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
    ((NumberPortion*) p)->SetValue(((NumberPortion *) p)->Value() +
      (((NumberPortion*) (*p1)[i])->Value() *
       ((NumberPortion*) (*p2)[i])->Value()));
  }
  return p;
}

static Portion* GSM_ArgMax(GSM &, Portion **param)
{
  Portion* p;
  gRational max = 0;
  int index = 0;
  int i;

  if (param[0]->Spec().ListDepth <= 0) {
    throw gclRuntimeError("Internal error in ArgMax[]");
  }

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
      throw gclRuntimeError( "Bad dimensionality" );
  }
  return new NumberPortion( index );
}

static Portion* GSM_Transpose(GSM &, Portion **param)
{
  int i;
  int j;
  int Length;
  int width = 0;
  ListPortion* p;
  ListPortion* s;

  if (param[0]->Spec().ListDepth <= 0) {
    throw gclRuntimeError("Internal error in Transpose[]");
  }

  Length = ((ListPortion*) param[0])->Length();
  for( i = 1; i <= Length; i++ )
  {
    if( (*(ListPortion*) param[0])[i]->Spec().ListDepth == 0 )
      throw gclRuntimeError( "Bad dimensionality" );
    if( i == 1 )
      width = ((ListPortion*) (*(ListPortion*) param[0])[i])->Length();
    else 
      if( ((ListPortion*) (*(ListPortion*) param[0])[i])->Length() != width )
	throw gclRuntimeError( "Bad dimensionality" );
  }
  p = new ListPortion();
  for( i = 1; i <= width; i++ ) {
    s = new ListPortion();
    for( j = 1; j <= Length; j++ ) {
      s->Append( (*(ListPortion*)(*(ListPortion*) param[0])[j])[i]->ValCopy());
    }
    p->Append( s );
  }
  return p;
}

static Portion* GSM_Inverse(GSM &, Portion **param)
{
  int i;
  int Length;
  int width = 0;
  ListPortion *p;
  ListPortion *s;

  if (param[0]->Spec().ListDepth <= 0) {
    throw gclRuntimeError("Internal error in Inverse[]");
  }

  Length = ((ListPortion*) param[0])->Length();
  for( i = 1; i <= Length; i++ )
  {
    if( (*(ListPortion*) param[0])[i]->Spec().ListDepth == 0 )
      throw gclRuntimeError( "Bad dimensionality" );
    if( i == 1 ) {
      width = ((ListPortion*) (*(ListPortion*) param[0])[i])->Length();
      if(width != Length)
        throw gclRuntimeError( "Not a square matrix" );
    }
    else 
      if( ((ListPortion*) (*(ListPortion*) param[0])[i])->Length() != width )
	      throw gclRuntimeError( "Bad dimensionality" );
  }
  int j;
  gPrecision precis = precRATIONAL;
  for(i=1; precis==precRATIONAL && i <= Length; i++ )
    for(j=1; precis==precRATIONAL && j<=width; j++)
      if((((NumberPortion*) ((*(ListPortion*) (*(ListPortion*) param[0])[i])[j]))->Value()).Precision() != precRATIONAL) 
	precis = precDOUBLE;
  gNumber gn;
  if(precis == precRATIONAL) {
    gSquareMatrix<gRational> A(Length);
    for(i=1;i<=Length;i++)
      for(j=1;j<=width;j++) {
	gn = ((NumberPortion*) ((*(ListPortion*) (*(ListPortion*) param[0])[i])[j]))->Value();
	A(i,j) = gn.gNumber::operator gRational();
      }
    gSquareMatrix<gRational> AA(A.Inverse());
    p = new ListPortion();
    p->SetDataType( param[0]->Spec().Type );
    for( i = 1; i <= width; i++ ) {
      s = new ListPortion();
      s->SetDataType( param[0]->Spec().Type );
      for( j = 1; j <= Length; j++ )
	  s->Append( new NumberPortion( AA(i,j) ));
      p->Append( s );
    }
  }
  else {
    gSquareMatrix<double> B(Length);
    for(i=1;i<=Length;i++)
      for( j=1;j<=width;j++)
	B(i,j)= (double)(((NumberPortion*) ((*(ListPortion*) (*(ListPortion*) param[0])[i])[j]))->Value());
    gSquareMatrix<double> BB(B.Inverse());
    p = new ListPortion();
    p->SetDataType( param[0]->Spec().Type );
    for( i = 1; i <= width; i++ ) {
      s = new ListPortion();
      s->SetDataType( param[0]->Spec().Type );
      for( j = 1; j <= Length; j++ )
	  s->Append( new NumberPortion( BB(i,j) ));
      p->Append( s );
    }
  }
  return p;
}


void Init_listfunc(GSM *gsm)
{
  gclFunction *FuncObj;

  gclParameter x_Int[] =
  {
    gclParameter( "x", porNUMBER )
  };
  gclParameter x_Number[] =
  {
    gclParameter( "x", porNUMBER )
  };

  FuncObj = new gclFunction(*gsm, "Concat", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Concat_List, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, gclParameter("y", 
					    PortionSpec(porANYTYPE, NLIST)));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Index", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Index, 
				       PortionSpec(porINTEGER, 1), 2, 0,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, gclParameter("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, gclParameter("x", porANYTYPE));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_Index, 
				       PortionSpec(porINTEGER, 1), 2, 0,
				       funcNONLISTABLE ));
  FuncObj->SetParamInfo(1, 0, gclParameter("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(1, 1, gclParameter("x", PortionSpec(porANYTYPE, 1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "NumElements", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_NumElements, porINTEGER, 1,
				       0, funcNONLISTABLE ));
  FuncObj->SetParamInfo(0, 0, gclParameter("list", 
                              PortionSpec(porANYTYPE, 1, porNULLSPEC )));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Length", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_LengthList, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("list", 
                              PortionSpec(porANYTYPE, 1, porNULLSPEC )));
  gsm->AddFunction(FuncObj);
			
  FuncObj = new gclFunction(*gsm, "NumChars", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_LengthText, porINTEGER, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("text", porTEXT));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Contains", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Contains, porBOOLEAN, 2,
				       0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, gclParameter("x", porANYTYPE));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Contains, porBOOLEAN, 2,
				       0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("list", 
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(1, 1, gclParameter("x", 
					    PortionSpec(porANYTYPE,1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "NthElement", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_NthElement, porANYTYPE, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("list", 
					    PortionSpec(porANYTYPE, NLIST),
					    REQUIRED, BYREF));
  FuncObj->SetParamInfo(0, 1, gclParameter("n", porINTEGER));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Remove", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Remove, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("list",
					    PortionSpec(porANYTYPE, NLIST)));
  FuncObj->SetParamInfo(0, 1, gclParameter("n", porNUMBER));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "NthElement", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_NthElementText, porTEXT, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("text", porTEXT));
  FuncObj->SetParamInfo(0, 1, gclParameter("n", porINTEGER));
  gsm->AddFunction(FuncObj);


  //--------------------------- Text -----------------------
  
  FuncObj = new gclFunction(*gsm, "Text", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Text_Number, porTEXT, 1, x_Number));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_TextText, porTEXT, 1));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", porTEXT));
  gsm->AddFunction(FuncObj);
  

  //-------------------------- Integer ------------------------

  FuncObj = new gclFunction(*gsm, "Integer", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IntegerNumber, porNUMBER,
				       1, x_Number));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "StartWatch", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_StartWatch, porNUMBER, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "StopWatch", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_StopWatch, porNUMBER, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "ElapsedTime", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ElapsedTime, porNUMBER, 0));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "IsWatchRunning", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_IsWatchRunning, porBOOLEAN, 0));
  gsm->AddFunction(FuncObj);


  //-------------------------- List -----------------------------

  FuncObj = new gclFunction(*gsm, "List", 5);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_List, PortionSpec(porANYTYPE, 1), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", porANYTYPE & 
					    ~(porNUMBER | porNUMBER | porNFG | porEFG )));
  FuncObj->SetParamInfo(0, 1, gclParameter("length", porNUMBER, 
					    new NumberPortion(1)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_List_List, 
				       PortionSpec(porANYTYPE, 2), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(1, 1, gclParameter("length", porNUMBER, 
					    new NumberPortion(1)));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_List_Number,
				       PortionSpec(porNUMBER, 1),
				       3, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(2, 0, gclParameter("x", porNUMBER));
  FuncObj->SetParamInfo(2, 1, gclParameter("length", porNUMBER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(2, 2, gclParameter("delta", porNUMBER,
					    new NumberPortion(0)));

  FuncObj->SetFuncInfo(3, gclSignature(GSM_List_Nfg,
				       PortionSpec(porNFG, 1), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(3, 0, gclParameter("x", porNFG));
  FuncObj->SetParamInfo(3, 1, gclParameter("length", porNUMBER,
					    new NumberPortion(1)));

  FuncObj->SetFuncInfo(4, gclSignature(GSM_List_Efg,
				       PortionSpec(porEFG, 1), 
				       2, 0, funcNONLISTABLE));
  FuncObj->SetParamInfo(4, 0, gclParameter("x", porEFG));
  FuncObj->SetParamInfo(4, 1, gclParameter("length", porNUMBER,
					    new NumberPortion(1)));

  gsm->AddFunction(FuncObj);

  //--------------------------- Dot ----------------------------

  FuncObj = new gclFunction(*gsm, "Dot", 1);
  
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Dot, porNUMBER, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", PortionSpec(porNUMBER,1)));
  FuncObj->SetParamInfo(0, 1, gclParameter("y", PortionSpec(porNUMBER,1)));
  gsm->AddFunction(FuncObj);

  //----------------------- ArgMax ------------------------

  FuncObj = new gclFunction(*gsm, "ArgMax", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_ArgMax, porNUMBER, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", PortionSpec(porNUMBER,1)));
  gsm->AddFunction(FuncObj);

  //------------------ Transpose -----------------------
  FuncObj = new gclFunction(*gsm, "Transpose", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Transpose, 
				       PortionSpec(porANYTYPE, 2), 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", PortionSpec(porANYTYPE,2), 
					    REQUIRED, BYVAL));
  gsm->AddFunction(FuncObj);

  //------------------ Inverse -----------------------
  FuncObj = new gclFunction(*gsm, "Inverse", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Inverse, 
				       PortionSpec(porNUMBER,2), 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", PortionSpec(porNUMBER,2), 
					    REQUIRED, BYVAL));
  gsm->AddFunction(FuncObj);

  //------------------ Sort -----------------------
  FuncObj = new gclFunction(*gsm, "Sort", 4);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Sort_Number,
				       PortionSpec(porNUMBER, 1), 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", PortionSpec(porNUMBER,1)));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_Sort_Text,
				       PortionSpec(porTEXT, 1), 1));
  FuncObj->SetParamInfo(1, 0, gclParameter("x", PortionSpec(porTEXT,1)));

  /*  FuncObj->SetFuncInfo(2, gclSignature(GSM_Sort_By_Integer,
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(2, 0, gclParameter("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(2, 1, gclParameter("by", PortionSpec(porNUMBER,1)));
  */
  FuncObj->SetFuncInfo(2, gclSignature(GSM_Sort_By_Number,
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(2, 0, gclParameter("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(2, 1, gclParameter("by", PortionSpec(porNUMBER,1)));
  FuncObj->SetFuncInfo(3, gclSignature(GSM_Sort_By_Text,
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(3, 0, gclParameter("x", PortionSpec(porANYTYPE,1)));
  FuncObj->SetParamInfo(3, 1, gclParameter("by", PortionSpec(porTEXT,1)));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Filter", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Filter, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", 
                              PortionSpec(porANYTYPE, NLIST, porNULLSPEC )));
  FuncObj->SetParamInfo(0, 1, gclParameter("y", 
					    PortionSpec(porBOOLEAN, NLIST)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Flatten", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Flatten, 
				       PortionSpec(porANYTYPE, 1), 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("x", 
                              PortionSpec(porANYTYPE, NLIST, porNULLSPEC )));
  FuncObj->SetParamInfo(0, 1, gclParameter("levels", porINTEGER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

}




