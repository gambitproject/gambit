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
  int x      = op1->Evaluate( vt )[1];
  int length = op2->Evaluate( vt )[1];

  gList<int> dim;
  dim.Append( 1 );
  dim.Append( -(length + 1) );

  gNestedList< gNumber > ret( dim );
  for( int i = 0; i < length; ++i )
    ret.Append( x );
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
  for( i = 1; i <= list1.Length(); ++i )
    ret.Append( list1[i] );
  for( i = 1; i <= list2.Length(); ++i )
    ret.Append( list2[i] );
  return ret;
}

gelExpr* GEL_ConcatNumber(const gArray<gelExpr *> &params)
{
  return new gelfuncConcatNumber( (gelExpression<gNumber> *) params[1],
				  (gelExpression<gNumber> *) params[2]);
}






#include "match.h"

void gelListInit(gelEnvironment *env)
{
  struct  { gelAdapter *func; char *sig; }  sigarray[] = {
    { GEL_ListNumber, "List[x->NUMBER, length->NUMBER] =: NUMBER" },
    { GEL_ConcatNumber, "Concat[list1->NUMBER, list2->NUMBER] =: NUMBER" },
    { 0, 0 } };

  for (int i = 0; sigarray[i].func; i++)  
    env->Register(sigarray[i].func, sigarray[i].sig);
}












