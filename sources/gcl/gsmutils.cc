//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Utility functions for constructing GCL data types
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "portion.h"
#include "math/rational.h"
#include "math/gvector.h"
#include "math/gdpvect.h"
#include "math/gmatrix.h"

template <class T> Portion *ArrayToList(const gArray<T> &);
template <class T> Portion *ArrayToList(const gList<T> &);
template <class T> Portion *gDPVectorToList(const gDPVector<T> &);

template<>
Portion *ArrayToList(const gArray<double> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NumberPortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gArray<gRational> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NumberPortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gArray<gNumber> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NumberPortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gArray<Action *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new ActionPortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gArray<EFPlayer *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new EfPlayerPortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gArray<Infoset *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new InfosetPortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gArray<Node *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodePortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gArray<Strategy *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new StrategyPortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gList<Node *> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodePortion(A[i]));
  return ret;
}

template<>
Portion *ArrayToList(const gList<const EFSupport> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++) {
    EFSupport sup = A[i];
    ret->Append(new EfSupportPortion(sup));
  }
  return ret;
}

template<>
Portion *ArrayToList(const gList<const gbtNfgSupport> &A)
{
  ListPortion *ret = new ListPortion;
  for (int i = 1; i <= A.Length(); i++) {
    gbtNfgSupport sup = A[i];
    ret->Append(new NfSupportPortion(sup));
  }
  return ret;
}

template<>
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

template<>
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

template<>
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
