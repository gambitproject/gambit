//
// FILE: gsmutils.cc -- Utility functions for GSM builtin functions
//
// $Id$
//

#include "portion.h"
#include "rational.h"
#include "gvector.h"
#include "gdpvect.h"
#include "gmatrix.h"

Portion *ArrayToList(const gArray<double> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NumberPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<gRational> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NumberPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<gNumber> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NumberPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Action *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new ActionPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<EFPlayer *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new EfPlayerPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Infoset *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new InfosetPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<EFOutcome *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new EfOutcomePortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Node *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodePortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<NFPlayer *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NfPlayerPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<NFOutcome *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NfOutcomePortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Strategy *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new StrategyPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gList<Node *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodePortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gList<const EFSupport> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++) {
    EFSupport sup = A[i];
    ret->Append(new EfSupportPortion(sup));
  }
  return ret;
}

Portion *ArrayToList(const gList<const NFSupport> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++) {
    NFSupport sup = A[i];
    ret->Append(new NfSupportPortion(sup));
  }
  return ret;
}

Portion *gDPVectorToList(const gDPVector<double> &A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new NumberPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
}

Portion* gDPVectorToList(const gDPVector<gRational> &A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new NumberPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
}

Portion* gDPVectorToList(const gDPVector<gNumber> &A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new NumberPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
}



gVector<double>* ListToVector_Float(ListPortion* list)
{
  int length = list->Length();
  int i;
  gVector<double>* vector = 0;
  bool flat = true;
  for(i=1; i<=length; i++)
  {
    if((*list)[i]->Spec().Type != porNUMBER)
      flat = false;
  }
  if(flat)
  {
    vector = new gVector<double>(length);
    for(i=1; i<=length; i++)
      (*vector)[i] = ((NumberPortion*)(*list)[i])->Value();
  }
  return vector;      
}

gVector<gRational>* ListToVector_Rational(ListPortion* list)
{
  int length = list->Length();
  int i;
  gVector<gRational>* vector = 0;
  bool flat = true;
  for(i=1; i<=length; i++)
  {
    if((*list)[i]->Spec().Type != porNUMBER)
      flat = false;
  }
  if(flat)
  {
    vector = new gVector<gRational>(length);
    for(i=1; i<=length; i++)
      (*vector)[i] = ((NumberPortion*)(*list)[i])->Value();
  }
  return vector;      
}





gMatrix<double>* ListToMatrix_Float(ListPortion* list)
{
  int rows = list->Length();
  int cols = 0;
  int i;
  int j;
  gMatrix<double>* matrix = 0;
  bool rect = true;
  for(i=1; i<=rows; i++)
  {
    if((*list)[i]->Spec().ListDepth == 0)
      rect = false;
    else if(cols==0)
      cols = ((ListPortion*) (*list)[i])->Length();
    else if(cols != ((ListPortion*) (*list)[i])->Length())
      rect = false;
    if(rect)
      for(j=1; j<=cols; j++)
	if((*((ListPortion*) (*list)[i]))[j]->Spec().Type != porNUMBER)
	  rect = false;
  }

  if(rect)
  {
    matrix = new gMatrix<double>(rows, cols);
    for(i=1; i<=rows; i++)
      for(j=1; j<=cols; j++)
	(*matrix)(i,j) = 
	  ((NumberPortion*)(*((ListPortion*)(*list)[i]))[j])->Value();
  }
  return matrix;
}


gMatrix<gRational>* ListToMatrix_Rational(ListPortion* list)
{
  int rows = list->Length();
  int cols = 0;
  int i;
  int j;
  gMatrix<gRational>* matrix = 0;
  bool rect = true;
  for(i=1; i<=rows; i++)
  {
    if((*list)[i]->Spec().ListDepth == 0)
      rect = false;
    else if(cols==0)
      cols = ((ListPortion*) (*list)[i])->Length();
    else if(cols != ((ListPortion*) (*list)[i])->Length())
      rect = false;
    if(rect)
      for(j=1; j<=cols; j++)
	if((*((ListPortion*) (*list)[i]))[j]->Spec().Type != porNUMBER)
	  rect = false;
  }

  if(rect)
  {
    matrix = new gMatrix<gRational>(rows, cols);
    for(i=1; i<=rows; i++)
      for(j=1; j<=cols; j++)
	(*matrix)(i,j) = 
	  ((NumberPortion*)(*((ListPortion*)(*list)[i]))[j])->Value();
  }
  return matrix;
}
