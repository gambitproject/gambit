//
// FILE: gsmutils.cc -- Utility functions for GSM builtin functions
//
// $Id$
//

#include "portion.h"

Portion *ArrayToList(const gArray<double> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new FloatValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<gRational> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new RationalValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Action *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new ActionValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<EFPlayer *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new EfPlayerValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Infoset *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new InfosetValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Outcome *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new OutcomeValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Node *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodeValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<NFPlayer *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NfPlayerValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gArray<Strategy *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new StrategyValPortion(A[i]));
  return ret;
}

Portion *ArrayToList(const gList<Node *> &A)
{
  ListPortion *ret = new ListValPortion;
  for (int i = 1; i <= A.Length(); i++)
    ret->Append(new NodeValPortion(A[i]));
  return ret;
}

Portion *gDPVectorToList(const gDPVector<double> &A)
{
  ListPortion* p;
  ListPortion* s1;
  ListPortion* s2;
  p = new ListValPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListValPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListValPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new FloatValPortion(A(i, j, k)));
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
  p = new ListValPortion();
  int l = 1;
  for (int i = 1; i <= A.DPLengths().Length(); i++)  {
    s1 = new ListValPortion();
    for (int j = 1; j <= A.DPLengths()[i]; j++)  {
      s2 = new ListValPortion();
      for (int k = 1; k <= A.Lengths()[l]; k++)
	s2->Append(new RationalValPortion(A(i, j, k)));
      l++;
      s1->Append(s2);
    }
    p->Append(s1);
  }
  return p;
}


