//
// FILE: funclist.cc -- List functions for GEL
//
// $Id$
//

#include "exprtree.h"



//-------------------------------------------------------------
//   List
//-------------------------------------------------------------


DECLARE_BINARY_LIST(gelfuncListNumber, gNumber, gNumber, gNumber)

gNestedList<gNumber>
gelfuncListNumber::Evaluate( gelVariableTable *vt ) const
{
  int x      = op1->Evaluate( vt ).Data()[1];
  int length = op2->Evaluate( vt ).Data()[1];

  gList<int> dim;
  dim.Append( 1 );
  dim.Append( -(length + 1) );

  gNestedList< gNumber > ret( dim );
  for( int i = 0; i < length; ++i )
    ret.Data().Append( x );
  return ret;
}

gelExpr* GEL_ListNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncListNumber( (gelExpression<gNumber> *) params[1],
				(gelExpression<gNumber> *) params[2]);
}




//-------------------------------------------------------------
//   Concat
//-------------------------------------------------------------


DECLARE_BINARY_LIST(gelfuncConcatNumber, gNumber, gNumber, gNumber)

gNestedList<gNumber>
gelfuncConcatNumber::Evaluate( gelVariableTable *vt ) const
{
  int i = 0;
  gNestedList< gNumber > list1 = op1->Evaluate( vt );
  gNestedList< gNumber > list2 = op2->Evaluate( vt );

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
  

  gNestedList< gNumber > ret( dim );
  for( i = 1; i <= list1.Data().Length(); ++i )
    ret.Data().Append( list1.Data()[i] );
  for( i = 1; i <= list2.Data().Length(); ++i )
    ret.Data().Append( list2.Data()[i] );
  return ret;
}

gelExpr* GEL_ConcatNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncConcatNumber( (gelExpression<gNumber> *) params[1],
				  (gelExpression<gNumber> *) params[2]);
}



//-------------------------------------------------------------
//   Contains
//-------------------------------------------------------------


DECLARE_BINARY_LIST(gelfuncContainsNumber, gNumber, gNumber, gTriState)

gNestedList<gTriState>
gelfuncContainsNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber > list1 = op1->Evaluate( vt );
  gNestedList< gNumber > list2 = op2->Evaluate( vt );  
  gNestedList< gTriState > ret;
  if( list1.Contains( list2 ) )
    ret.Data().Append( triTRUE );
  else
    ret.Data().Append( triFALSE );
  return ret;
}

gelExpr* GEL_ContainsNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncContainsNumber( (gelExpression<gNumber> *) params[1],
				    (gelExpression<gNumber> *) params[2]);
}


//-------------------------------------------------------------
//   Index
//-------------------------------------------------------------


DECLARE_BINARY_LIST(gelfuncIndexNumber, gNumber, gNumber, gNumber)

gNestedList<gNumber>
gelfuncIndexNumber::Evaluate( gelVariableTable *vt ) const
{
  int i = 0;
  gNestedList< gNumber > list1 = op1->Evaluate( vt );
  gNestedList< gNumber > list2 = op2->Evaluate( vt );  
  gList<int> dim;
  dim.Append( 1 );
  dim.Append( -1 );
  gList<gNumber> data;

  for( i = 1; i <= list1.NumElements(); ++i )
  {
    if( list1.NthElement( i ) == list2 )
    {
      data.Append( i );
      --dim[2];
    }
  }
  return gNestedList< gNumber >( data, dim );
}

gelExpr* GEL_IndexNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncIndexNumber( (gelExpression<gNumber> *) params[1],
				 (gelExpression<gNumber> *) params[2]);
}


//-------------------------------------------------------------
//   NthElement
//-------------------------------------------------------------


DECLARE_BINARY_LIST(gelfuncNthElementNumber, gNumber, gNumber, gNumber)

gNestedList<gNumber>
gelfuncNthElementNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber > list = op1->Evaluate( vt );
  int el = op2->Evaluate( vt ).Data()[1];

  assert( list.Dim().Length() >= 2 );

  return list.NthElement( el );
}

gelExpr* GEL_NthElementNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncNthElementNumber( (gelExpression<gNumber> *) params[1],
				      (gelExpression<gNumber> *) params[2]);
}


//-------------------------------------------------------------
//   NumElements
//-------------------------------------------------------------


DECLARE_UNARY_LIST(gelfuncNumElementsNumber, gNumber, gNumber)

gNestedList<gNumber>
gelfuncNumElementsNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber > list = op1->Evaluate( vt );
  gNestedList< gNumber > ret;
  ret.Data().Append( list.NumElements() );
  return ret;
}

gelExpr* GEL_NumElementsNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncNumElementsNumber( (gelExpression<gNumber> *) params[1] );
}



//-------------------------------------------------------------
//   Remove
//-------------------------------------------------------------


DECLARE_BINARY_LIST(gelfuncRemoveNumber, gNumber, gNumber, gNumber)

gNestedList<gNumber>
gelfuncRemoveNumber::Evaluate( gelVariableTable *vt ) const
{
  gNestedList< gNumber > list = op1->Evaluate( vt );
  int el = op2->Evaluate( vt ).Data()[1];
  list.Remove( el );
  return list;
}

gelExpr* GEL_RemoveNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncRemoveNumber( (gelExpression<gNumber> *) params[1],
				  (gelExpression<gNumber> *) params[2]);
}









#include "match.h"

void gelListInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_ListNumber, "List[x->NUMBER, length->NUMBER] =: NUMBER" },
    { GEL_ConcatNumber, "Concat[list1->NUMBER, list2->NUMBER] =: NUMBER" },
    { GEL_ContainsNumber, "Contains[list1->NUMBER, list2->NUMBER] =: BOOL" },
    { GEL_IndexNumber, "Index[list1->NUMBER, list2->NUMBER] =: NUMBER" },
    { GEL_NthElementNumber, "NthElement[list->NUMBER, el->NUMBER] =: NUMBER" },
    { GEL_NumElementsNumber, "NumElements[list->NUMBER] =: NUMBER" },
    { GEL_RemoveNumber, "Remove[list->NUMBER, el->NUMBER] =: NUMBER" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)  
    env->Register(sigarray[i].func, sigarray[i].sig);
}












