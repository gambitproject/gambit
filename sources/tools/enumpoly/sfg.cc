//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of sequence form classes
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

#include "sfg.h"
#include "sfstrat.h"
#include "gnarray.imp"
#include "libgambit/libgambit.h"

//----------------------------------------------------
// Sfg: Constructors, Destructors, Operators
//----------------------------------------------------


Sfg::Sfg(const gbtEfgSupport &S)
  : EF(S.GetGame()), efsupp(S), seq(EF->NumPlayers()), isetFlag(S.GetGame()->NumInfosets()),
    isetRow(S.GetGame()->NumInfosets()), infosets(EF->NumPlayers())
{ 
  int i;
  gbtArray<Gambit::GameInfoset> zero(EF->NumPlayers());
  gbtArray<int> one(EF->NumPlayers());

  gbtEfgSupport support(EF);

  for(i=1;i<=EF->NumPlayers();i++) {
    seq[i]=1;
    zero[i]=0;
    one[i]=1;
  }

  isetFlag = 0;
  isetRow = 0;

  GetSequenceDims(EF->GetRoot());

  isetFlag = 0;

  gIndexOdometer index(seq);

  SF = new gNArray<gbtArray<gbtRational> *>(seq);
  while (index.Turn()) {
    (*SF)[index.CurrentIndices()] = new gbtArray<gbtRational>(EF->NumPlayers());
    for(i=1;i<=EF->NumPlayers();i++)
      (*(*SF)[index.CurrentIndices()])[i]=(gbtRational)0;
  } 

  E = new gbtArray<gbtRectArray<gbtRational> *> (EF->NumPlayers());
  for(i=1;i<=EF->NumPlayers();i++) {
    (*E)[i] = new gbtRectArray<gbtRational>(infosets[i].Length()+1,seq[i]);
    for(int j = (*(*E)[i]).MinRow();j<=(*(*E)[i]).MaxRow();j++)
      for(int k = (*(*E)[i]).MinCol();k<=(*(*E)[i]).MaxCol();k++)
	(*(*E)[i])(j,k)=(gbtRational)0;
    (*(*E)[i])(1,1)=(gbtRational)1;
  } 

  sequences = new gbtArray<SFSequenceSet *>(EF->NumPlayers());
  for(i=1;i<=EF->NumPlayers();i++)
    (*sequences)[i] = new SFSequenceSet(EF->GetPlayer(i));

  gbtArray<Sequence *> parent(EF->NumPlayers());
  for(i=1;i<=EF->NumPlayers();i++)
    parent[i] = (((*sequences)[i])->GetSFSequenceSet())[1];

  MakeSequenceForm(EF->GetRoot(),(gbtRational)1,one,zero,parent);
}

Sfg::~Sfg()
{
  gIndexOdometer index(seq);

  while (index.Turn()) 
    delete (*SF)[index.CurrentIndices()];
  delete SF;

  int i;

  for(i=1;i<=EF->NumPlayers();i++)
    delete (*E)[i];
  delete E;

  for(i=1;i<=EF->NumPlayers();i++)
    delete (*sequences)[i];
  delete sequences;
}

void Sfg::
MakeSequenceForm(const Gambit::GameNode &n, gbtRational prob,gbtArray<int>seq, 
		 gbtArray<Gambit::GameInfoset> iset, gbtArray<Sequence *> parent) 
{ 
  int i,pl;

  if (n->GetOutcome()) {
    for(pl = 1;pl<=seq.Length();pl++)
      (*(*SF)[seq])[pl] += prob * n->GetOutcome()->GetPayoff(pl);
  }
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	MakeSequenceForm(n->GetChild(i),
		     prob * n->GetInfoset()->GetActionProb(i), seq,iset,parent);
    }
    else {
      int pl = n->GetPlayer()->GetNumber();
      iset[pl]=n->GetInfoset();
      int isetnum = iset[pl]->GetNumber();
      gbtArray<int> snew(seq);
      snew[pl]=1;
      for(i=1;i<isetnum;i++)
	if(isetRow(pl,i)) 
	  snew[pl]+=efsupp.NumActions(pl,i);

      (*(*E)[pl])(isetRow(pl,isetnum),seq[pl]) = (gbtRational)1;
      Sequence *myparent(parent[pl]);

      bool flag = false;
      if(!isetFlag(pl,isetnum)) {   // on first visit to iset, create new sequences
	isetFlag(pl,isetnum)=1;
	flag =true;
      }
      for(i=1;i<=n->NumChildren();i++) {
	if(efsupp.Find(n->GetInfoset()->GetAction(i))) {
	  snew[pl]+=1;
	  if(flag) {
	    Sequence* child;
	    child = new Sequence(n->GetPlayer(),
				 n->GetInfoset()->GetAction(i), 
				 myparent,snew[pl]);
	    parent[pl]=child;
	    ((*sequences)[pl])->AddSequence(child);
	    
	  }

	  (*(*E)[pl])(isetRow(pl,isetnum),snew[pl]) = -(gbtRational)1;
	  MakeSequenceForm(n->GetChild(i),prob,snew,iset,parent);
	}
      }
    }
    
  }
}

void Sfg::
GetSequenceDims(const Gambit::GameNode &n) 
{ 
  int i;

  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	GetSequenceDims(n->GetChild(i));
    }
    else {
      int pl = n->GetPlayer()->GetNumber();
      int isetnum = n->GetInfoset()->GetNumber();
    
      bool flag = false;
      if(!isetFlag(pl,isetnum)) {   // on first visit to iset, create new sequences
	infosets[pl].Append(n->GetInfoset());
	isetFlag(pl,isetnum)=1;
	isetRow(pl,isetnum)=infosets[pl].Length()+1;
	flag =true;
      }
      for(i=1;i<=n->NumChildren();i++) {
	if(efsupp.Find(n->GetInfoset()->GetAction(i))) {
	  if(flag) {
	    seq[pl]++;
	  }
	  GetSequenceDims(n->GetChild(i));
	}
      }
    }
  }
}

int Sfg::TotalNumSequences() const 
{
  int tot=0;
  for(int i=1;i<=seq.Length();i++)
    tot+=seq[i];
  return tot;
}

int Sfg::NumPlayerInfosets() const 
{
  int tot=0;
  for(int i=1;i<=infosets.Length();i++)
    tot+=infosets[i].Length();
  return tot;
}

int Sfg::InfosetRowNumber(int pl, int j) const 
{
  if(j==1) return 0;
  int isetnum = (*sequences)[pl]->Find(j)->GetInfoset()->GetNumber();
  return isetRow(pl,isetnum);
}

int Sfg::ActionNumber(int pl, int j) const
{
  if(j==1) return 0;
  int isetnum = (*sequences)[pl]->Find(j)->GetInfoset()->GetNumber();
  return efsupp.Find(pl,isetnum,GetAction(pl,j));
}

Gambit::GameInfoset Sfg::GetInfoset(int pl, int j) const 
{
  if(j==1) return 0;
  return (*sequences)[pl]->Find(j)->GetInfoset();
}

Gambit::GameAction Sfg::GetAction(int pl, int j) const
{
  if(j==1) return 0;
  return (*sequences)[pl]->Find(j)->GetAction();
}

gbtBehavProfile<double> Sfg::ToBehav(const gbtPVector<double> &x) const
{
  gbtBehavProfile<double> b(efsupp);

  b = (gbtRational) 0;

  Sequence *sij;
  const Sequence *parent;
  gbtRational value;

  int i,j;
  for(i=1;i<=EF->NumPlayers();i++)
    for(j=2;j<=seq[i];j++) {
      sij = ((*sequences)[i]->GetSFSequenceSet())[j];
      int sn = sij->GetNumber();
      parent = sij->Parent();

      // gout << "\ni,j,sn,iset,act: " << i << " " << j << " " << sn << " ";
      // gout << sij->GetInfoset()->GetNumber() << " " << sij->GetAction()->GetNumber();

      if(x(i, parent->GetNumber())>(double)0)
	value = (x(i,sn)/x(i,parent->GetNumber()));
      else
	value = 0;

      b(i,sij->GetInfoset()->GetNumber(),efsupp.Find(sij->GetAction()))= value;
    }
  return b;
}

gbtRational Sfg::Payoff(const gbtArray<int> & index,int pl) const 
{
  return Payoffs(index)[pl];
}


template class gNArray<gbtArray<gbtRational> *>;
