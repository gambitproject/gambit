//
// FILE: funclist.cc -- List functions for GEL
//
// $Id$
//

#include "exprtree.h"


//----------
// Concat
//----------

DECLARE_BINARY_LIST(gelfuncConcatNumber, gNumber *, gNumber *, gNumber *)

gNestedList<gNumber *>
gelfuncConcatNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber *> list1 = op1->Evaluate( vt );
  gNestedList< gNumber *> list2 = op2->Evaluate( vt );

  assert( list1.Dim().Length() >= 2 );
  assert( list2.Dim().Length() >= 2 );

  gList<int> dim( list1.Dim() + list2.Dim() );
  dim[list1.Dim().Length()] = 
    abs( dim[list1.Dim().Length()  ] ) + 
    abs( dim[list1.Dim().Length()+2] ) - 1;
  if( dim[list1.Dim().Length()+2] < 0 )
    dim[list1.Dim().Length()] = -dim[list1.Dim().Length()];
  dim.Remove( list1.Dim().Length()+1 );
  dim.Remove( list1.Dim().Length()+1 );
  

  gNestedList< gNumber *> ret( dim );
  for (int i = 1; i <= list1.Data().Length(); ++i )
    ret[i] = list1.Data()[i];
  for (int i = 1; i <= list2.Data().Length(); ++i )
    ret[i + list1.Data().Length()] = list2.Data()[i];
  return ret;
}


//------------
// Contains
//------------

DECLARE_BINARY_LIST(gelfuncContainsNumber, gNumber *, gNumber *, gTriState *)

gNestedList<gTriState *>
gelfuncContainsNumber::Evaluate(gelVariableTable *vt) const
{
  gNestedList<gNumber *> list1 = op1->Evaluate(vt);
  gNestedList<gNumber *> x = op2->Evaluate(vt);
  return gNestedList<gTriState *>(new gTriState((list1.Contains(x) ? triTRUE : triFALSE)));
}


//---------
// Index
//---------

DECLARE_BINARY_LIST(gelfuncIndexNumber, gNumber *, gNumber *, gNumber *)

gNestedList<gNumber *>
gelfuncIndexNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber *> list1 = op1->Evaluate( vt );
  gNestedList< gNumber *> list2 = op2->Evaluate( vt );
  gList<int> dim;
  dim.Append( 1 );
  dim.Append( -1 );
  gList<gNumber *> data;

  for(int i = 1; i <= list1.NumElements(); ++i )
  {
    if( list1.NthElement( i ) == list2 )
    {
      data.Append(new gNumber(i));
      --dim[2];
    }
  }
  return gNestedList< gNumber *>( data, dim );
}

//--------
// List
//--------


DECLARE_BINARY_LIST(gelfuncListNumber, gNumber *, gNumber *, gNumber *)

gNestedList<gNumber *>
gelfuncListNumber::Evaluate( gelVariableTable *vt ) const
{
  int x      = *op1->Evaluate( vt ).Data()[1];
  int length = *op2->Evaluate( vt ).Data()[1];

  gList<int> dim;
  dim.Append( 1 );
  dim.Append( -(length + 1) );

  gNestedList<gNumber *> ret(dim);
  for (int i = 1; i <= length; i++)
    ret[i] = new gNumber(x);
  return ret;
}


//--------------
// NthElement
//--------------

DECLARE_BINARY_LIST(gelfuncNthElementNumber, gNumber *, gNumber *, gNumber *)

gNestedList<gNumber *>
gelfuncNthElementNumber::Evaluate(gelVariableTable *vt) const
{
  gNestedList< gNumber *> list = op1->Evaluate( vt );
  int el = *op2->Evaluate( vt ).Data()[1];

  assert( list.Dim().Length() >= 2 );

  return list.NthElement( el );
}


//--------------
// NumElements
//--------------

DECLARE_UNARY_LIST(gelfuncNumElementsNumber, gNumber *, gNumber *)

gNestedList<gNumber *>
gelfuncNumElementsNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber *> list = op1->Evaluate( vt );
  return gNestedList<gNumber *>(new gNumber(list.NumElements()));
}


//---------
// Remove
//---------


DECLARE_BINARY_LIST(gelfuncRemoveNumber, gNumber *, gNumber *, gNumber *)

gNestedList<gNumber *>
gelfuncRemoveNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber *> list = op1->Evaluate( vt );
  int el = *op2->Evaluate( vt ).Data()[1];
  list.Remove( el );
  return list;
}


gelExpr* GEL_ConcatNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncConcatNumber( (gelExpression<gNumber *> *) params[1],
				  (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_ContainsNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncContainsNumber( (gelExpression<gNumber *> *) params[1],
				    (gelExpression<gNumber *> *) params[2]);
}

gelExpr* GEL_IndexNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncIndexNumber( (gelExpression<gNumber *> *) params[1],
				 (gelExpression<gNumber *> *) params[2]);
}

gelExpr *GEL_ListNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncListNumber( (gelExpression<gNumber *> *) params[1],
				(gelExpression<gNumber *> *) params[2]);
}

gelExpr* GEL_NthElementNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncNthElementNumber( (gelExpression<gNumber *> *) params[1],
				      (gelExpression<gNumber *> *) params[2]);
}

gelExpr* GEL_NumElementsNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncNumElementsNumber( (gelExpression<gNumber *> *) params[1] );
}

gelExpr* GEL_RemoveNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncRemoveNumber( (gelExpression<gNumber *> *) params[1],
				  (gelExpression<gNumber *> *) params[2]);
}




#include "match.h"

void gelListInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_ConcatNumber, "Concat[list1->LIST(NUMBER), list2->LIST(NUMBER)] =: LIST(NUMBER)" },
    { GEL_ContainsNumber, "Contains[list->LIST(NUMBER), x->NUMBER] =: BOOLEAN" },
    { GEL_IndexNumber, "Index[list1->LIST(NUMBER), list2->NUMBER] =: NUMBER" },
    { GEL_ListNumber, "List[x->NUMBER, length->NUMBER] =: NUMBER" },
    { GEL_NthElementNumber, "NthElement[list->LIST(NUMBER), el->NUMBER] =: NUMBER" },
    { GEL_NumElementsNumber, "NumElements[list->LIST(NUMBER)] =: NUMBER" },
    { GEL_RemoveNumber, "Remove[list->LIST(NUMBER), el->NUMBER] =: LIST(NUMBER)" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)  
    env->Register(sigarray[i].func, sigarray[i].sig);
}












