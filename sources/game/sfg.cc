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
#include "base/garray.imp"
#include "base/gnarray.imp"
#include "base/grarray.imp"
#include "base/glist.imp"

//----------------------------------------------------
// gbtSfgGame: Constructors, Destructors, Operators
//----------------------------------------------------


gbtSfgGame::gbtSfgGame(const gbtGame &p_efg)
  : m_efg(p_efg), seq(m_efg->NumPlayers()),
    isetFlag(m_efg->NumInfosets()),
    isetRow(m_efg->NumInfosets()), infosets(m_efg->NumPlayers())
{ 
  int i;
  gbtArray<gbtGameInfoset> zero(m_efg->NumPlayers());
  gbtArray<int> one(m_efg->NumPlayers());

  for(i=1;i<=m_efg->NumPlayers();i++) {
    seq[i]=1;
    zero[i]=0;
    one[i]=1;
  }

  isetFlag = 0;
  isetRow = 0;

  GetSequenceDims(m_efg->GetRoot());

  isetFlag = 0;

  gbtIndexOdometer index(seq);

  SF = new gbtNDArray<gbtArray<gbtRational> *>(seq);
  while (index.Turn()) {
    (*SF)[index.CurrentIndices()] = new gbtArray<gbtRational>(m_efg->NumPlayers());
    for(i=1;i<=m_efg->NumPlayers();i++)
      (*(*SF)[index.CurrentIndices()])[i]=(gbtRational)0;
  } 

  E = new gbtArray<gbtRectArray<gbtRational> *> (m_efg->NumPlayers());
  for(i=1;i<=m_efg->NumPlayers();i++) {
    (*E)[i] = new gbtRectArray<gbtRational>(infosets[i].Length()+1,seq[i]);
    for(int j = (*(*E)[i]).MinRow();j<=(*(*E)[i]).MaxRow();j++)
      for(int k = (*(*E)[i]).MinCol();k<=(*(*E)[i]).MaxCol();k++)
	(*(*E)[i])(j,k)=(gbtRational)0;
    (*(*E)[i])(1,1)=(gbtRational)1;
  } 

  sequences = new gbtArray<gbtSfgSequenceSet *>(m_efg->NumPlayers());
  for (i=1;i<=m_efg->NumPlayers();i++) {
    (*sequences)[i] = new gbtSfgSequenceSet( m_efg->GetPlayer(i) );
  }

  gbtArray<gbtSfgSequence *> parent(m_efg->NumPlayers());
  for(i=1;i<=m_efg->NumPlayers();i++)
    parent[i] = (((*sequences)[i])->GetSFSequenceSet())[1];

  MakeSequenceForm(m_efg->GetRoot(),(gbtRational)1,one,zero,parent);
}

gbtSfgGame::~gbtSfgGame()
{
  gbtIndexOdometer index(seq);

  while (index.Turn()) 
    delete (*SF)[index.CurrentIndices()];
  delete SF;

  int i;

  for(i=1;i<=m_efg->NumPlayers();i++)
    delete (*E)[i];
  delete E;

  for(i=1;i<=m_efg->NumPlayers();i++)
    delete (*sequences)[i];
  delete sequences;
}

void 
gbtSfgGame::MakeSequenceForm(const gbtGameNode &n, gbtRational prob,gbtArray<int>seq, 
		      gbtArray<gbtGameInfoset> iset, gbtArray<gbtSfgSequence *> parent) 
{ 
  int i,pl;

  if (!n->GetOutcome().IsNull()) {
    for(pl = 1;pl<=seq.Length();pl++)
      (*(*SF)[seq])[pl] += prob * n->GetOutcome()->GetPayoff(m_efg->GetPlayer(pl));
  }
  if (!n->GetInfoset().IsNull()) {
    if (n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	MakeSequenceForm(n->GetChild(i),
		     prob * n->GetInfoset()->GetAction(i)->GetChanceProb(), seq,iset,parent);
    }
    else {
      int pl = n->GetPlayer()->GetId();
      iset[pl]=n->GetInfoset();
      int isetnum = iset[pl]->GetId();
      gbtArray<int> snew(seq);
      snew[pl]=1;
      for(i=1;i<isetnum;i++)
	if(isetRow(pl,i)) 
	  snew[pl]+=m_efg->GetPlayer(pl)->GetInfoset(i)->NumActions();

      (*(*E)[pl])(isetRow(pl,isetnum),seq[pl]) = (gbtRational)1;
      gbtSfgSequence *myparent(parent[pl]);

      bool flag = false;
      if(!isetFlag(pl,isetnum)) {   // on first visit to iset, create new sequences
	isetFlag(pl,isetnum)=1;
	flag =true;
      }
      for(i=1;i<=n->NumChildren();i++) {
	snew[pl]+=1;
	if(flag) {
	  gbtSfgSequence* child;
	  child = new gbtSfgSequence(n->GetPlayer(), n->GetInfoset()->GetAction(i), 
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

void gbtSfgGame::GetSequenceDims(const gbtGameNode &n) 
{ 
  int i;

  if (!n->GetInfoset().IsNull()) {
    if (n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	GetSequenceDims(n->GetChild(i));
    }
    else {
      int pl = n->GetPlayer()->GetId();
      int isetnum = n->GetInfoset()->GetId();
    
      bool flag = false;
      if(!isetFlag(pl,isetnum)) {   // on first visit to iset, create new sequences
	infosets[pl].Append(n->GetInfoset());
	isetFlag(pl,isetnum)=1;
	isetRow(pl,isetnum)=infosets[pl].Length()+1;
	flag =true;
      }
      for(i=1;i<=n->NumChildren();i++) {
	if(flag) {
	  seq[pl]++;
	}
	GetSequenceDims(n->GetChild(i));
      }
    }
  }
}

int gbtSfgGame::TotalNumSequences() const 
{
  int tot=0;
  for(int i=1;i<=seq.Length();i++)
    tot+=seq[i];
  return tot;
}

int gbtSfgGame::NumPlayerInfosets() const 
{
  int tot=0;
  for(int i=1;i<=infosets.Length();i++)
    tot+=infosets[i].Length();
  return tot;
}

int gbtSfgGame::InfosetRowNumber(int pl, int j) const 
{
  if(j==1) return 0;
  int isetnum = (*sequences)[pl]->Find(j)->GetInfoset()->GetId();
  return isetRow(pl,isetnum);
}

int gbtSfgGame::ActionNumber(int pl, int j) const
{
  if(j==1) return 0;
  return GetAction(pl,j)->GetId();
}

gbtGameInfoset gbtSfgGame::GetInfoset(int pl, int j) const 
{
  if(j==1) return 0;
  return (*sequences)[pl]->Find(j)->GetInfoset();
}

gbtGameAction gbtSfgGame::GetAction(int pl, int j) const
{
  if(j==1) return 0;
  return (*sequences)[pl]->Find(j)->GetAction();
}

gbtBehavProfile<double> gbtSfgGame::ToBehav(const gbtPVector<double> &x) const
{
  gbtBehavProfile<double> b = m_efg->NewBehavProfile(0.0);

  for (int i = 1; i <= b->BehaviorProfileLength(); i++) {
    b[i] = 0;
  }

  gbtSfgSequence *sij;
  const gbtSfgSequence *parent;
  gbtRational value;

  int i,j;
  for(i=1;i<=m_efg->NumPlayers();i++)
    for(j=2;j<=seq[i];j++) {
      sij = ((*sequences)[i]->GetSFSequenceSet())[j];
      int sn = sij->GetNumber();
      parent = sij->Parent();

      // gout << "\ni,j,sn,iset,act: " << i << " " << j << " " << sn << " ";
      // gout << sij->GetInfoset()->GetNumber() << " " << sij->GetAction()->GetNumber();

      if (x(i, parent->GetNumber()) > 0.0) {
	value = x(i,sn) / x(i,parent->GetNumber());
      }
      else {
	value = 0.0;
      }

      b(i,sij->GetInfoset()->GetId(),sij->GetAction()->GetId()) = value;
    }
  return b;
}

gbtRational gbtSfgGame::Payoff(const gbtArray<int> & index,int pl) const 
{
  return Payoffs(index)[pl];
}


template class gbtNDArray<gbtArray<gbtRational> *>;
template class gbtArray<gbtRectArray<gbtRational> *>;

template class gbtArray<gbtList<gbtGameInfoset> >;
