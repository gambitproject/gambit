//
// FILE: sfg.cc -- Implementation of sequence form member functions
//
// $Id$ 
//

#include "sfg.h"
#include "sfstrat.h"
#include "base/garray.imp"
#include "gnarray.imp"
#include "base/grarray.imp"
#include "base/glist.imp"
#include "behav.h"

//----------------------------------------------------
// Sfg: Constructors, Destructors, Operators
//----------------------------------------------------


Sfg::Sfg(const EFSupport &S)
  : EF(S.GetGame()), efsupp(S), seq(EF.NumPlayers()), isetFlag(S.GetGame().NumInfosets()),
    isetRow(S.GetGame().NumInfosets()), infosets(EF.NumPlayers())
{ 
  int i;
  gArray<Infoset *> zero(EF.NumPlayers());
  gArray<int> one(EF.NumPlayers());

  EFSupport support(EF);

  for(i=1;i<=EF.NumPlayers();i++) {
    seq[i]=1;
    zero[i]=0;
    one[i]=1;
  }

  isetFlag = 0;
  isetRow = 0;

  GetSequenceDims(EF.RootNode());

  isetFlag = 0;

  gIndexOdometer index(seq);

  SF = new gNArray<gArray<gNumber> *>(seq);
  while (index.Turn()) {
    (*SF)[index.CurrentIndices()] = new gArray<gNumber>(EF.NumPlayers());
    for(i=1;i<=EF.NumPlayers();i++)
      (*(*SF)[index.CurrentIndices()])[i]=(gNumber)0;
  } 

  E = new gArray<gRectArray<gNumber> *> (EF.NumPlayers());
  for(i=1;i<=EF.NumPlayers();i++) {
    (*E)[i] = new gRectArray<gNumber>(infosets[i].Length()+1,seq[i]);
    for(int j = (*(*E)[i]).MinRow();j<=(*(*E)[i]).MaxRow();j++)
      for(int k = (*(*E)[i]).MinCol();k<=(*(*E)[i]).MaxCol();k++)
	(*(*E)[i])(j,k)=(gNumber)0;
    (*(*E)[i])(1,1)=(gNumber)1;
  } 

  sequences = new gArray<SFSequenceSet *>(EF.NumPlayers());
  for(i=1;i<=EF.NumPlayers();i++)
    (*sequences)[i] = new SFSequenceSet( (EF.Players())[i] );

  gArray<Sequence *> parent(EF.NumPlayers());
  for(i=1;i<=EF.NumPlayers();i++)
    parent[i] = (((*sequences)[i])->GetSFSequenceSet())[1];

  MakeSequenceForm(EF.RootNode(),(gNumber)1,one,zero,parent);
}

Sfg::~Sfg()
{
  gIndexOdometer index(seq);

  while (index.Turn()) 
    delete (*SF)[index.CurrentIndices()];
  delete SF;

  int i;

  for(i=1;i<=EF.NumPlayers();i++)
    delete (*E)[i];
  delete E;

  for(i=1;i<=EF.NumPlayers();i++)
    delete (*sequences)[i];
  delete sequences;
}

void Sfg::
MakeSequenceForm(const Node *n, gNumber prob,gArray<int>seq, 
		 gArray<Infoset *> iset, gArray<Sequence *> parent) 
{ 
  int i,pl;

  if (!EF.GetOutcome(n).IsNull()) {
    for(pl = 1;pl<=seq.Length();pl++)
      (*(*SF)[seq])[pl] += prob * EF.Payoff(n, EF.Players()[pl]);
  }
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=EF.NumChildren(n);i++)
	MakeSequenceForm(n->GetChild(i),
		     prob * EF.GetChanceProb(n->GetInfoset(), i), seq,iset,parent);
    }
    else {
      int pl = n->GetPlayer()->GetNumber();
      iset[pl]=n->GetInfoset();
      int isetnum = iset[pl]->GetNumber();
      gArray<int> snew(seq);
      snew[pl]=1;
      for(i=1;i<isetnum;i++)
	if(isetRow(pl,i)) 
	  snew[pl]+=efsupp.NumActions(pl,i);

      (*(*E)[pl])(isetRow(pl,isetnum),seq[pl]) = (gNumber)1;
      Sequence *myparent(parent[pl]);

      bool flag = false;
      if(!isetFlag(pl,isetnum)) {   // on first visit to iset, create new sequences
	isetFlag(pl,isetnum)=1;
	flag =true;
      }
      for(i=1;i<=EF.NumChildren(n);i++) {
	if(efsupp.Find(n->GetInfoset()->Actions()[i])) {
	  snew[pl]+=1;
	  if(flag) {
	    Sequence* child;
	    child = new Sequence(n->GetPlayer(),(n->GetInfoset()->Actions())[i], 
				 myparent,snew[pl]);
	    parent[pl]=child;
	    ((*sequences)[pl])->AddSequence(child);
	    
	  }

	  (*(*E)[pl])(isetRow(pl,isetnum),snew[pl]) = -(gNumber)1;
	  MakeSequenceForm(n->GetChild(i),prob,snew,iset,parent);
	}
      }
    }
    
  }
}

void Sfg::
GetSequenceDims(const Node *n) 
{ 
  int i;

  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=EF.NumChildren(n);i++)
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
      for(i=1;i<=EF.NumChildren(n);i++) {
	if(efsupp.Find(n->GetInfoset()->Actions()[i])) {
	  if(flag) {
	    seq[pl]++;
	  }
	  GetSequenceDims(n->GetChild(i));
	}
      }
    }
  }
}

void Sfg::Dump(gOutput& out) const
{
  gIndexOdometer index(seq);

  out << "\nseq: " << seq;

  out << "\nSequence Form: \n";
  while (index.Turn()) {
    out << "\nrow " << index.CurrentIndices() << ": " << (*(*SF)[index.CurrentIndices()]);
  } 
  
  out << "\nConstraint matrices: \n";
  for(int i=1;i<=EF.NumPlayers();i++) 
    out << "\nPlayer " << i << ":\n " << (*(*E)[i]);
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
  return efsupp.Find(pl,isetnum,(Action *)GetAction(pl,j));
}

const Infoset*  Sfg::GetInfoset(int pl, int j) const 
{
  if(j==1) return 0;
  return (*sequences)[pl]->Find(j)->GetInfoset();
}

const Action* Sfg::GetAction(int pl, int j) const
{
  if(j==1) return 0;
  return (*sequences)[pl]->Find(j)->GetAction();
}

BehavProfile<gNumber> Sfg::ToBehav(const gPVector<double> &x) const
{
  BehavProfile<gNumber> b(efsupp);

  b.gDPVector<gNumber>::operator=((gNumber)0);

  Sequence *sij;
  const Sequence *parent;
  gNumber value;

  int i,j;
  for(i=1;i<=EF.NumPlayers();i++)
    for(j=2;j<=seq[i];j++) {
      sij = ((*sequences)[i]->GetSFSequenceSet())[j];
      int sn = sij->GetNumber();
      parent = sij->Parent();

      // gout << "\ni,j,sn,iset,act: " << i << " " << j << " " << sn << " ";
      // gout << sij->GetInfoset()->GetNumber() << " " << sij->GetAction()->GetNumber();

      if(x(i, parent->GetNumber())>(double)0)
	value = (gNumber)(x(i,sn)/x(i,parent->GetNumber()));
      else
	value = (gNumber)0;

      b(i,sij->GetInfoset()->GetNumber(),efsupp.Find(sij->GetAction()))= value;
    }
  return b;
}

gNumber Sfg::Payoff(const gArray<int> & index,int pl) const 
{
  return Payoffs(index)[pl];
}


template class gNArray<gArray<gNumber> *>;
template class gArray<gRectArray<gNumber> *>;
template gOutput &operator<<(gOutput &, const gArray<gNumber> &);
template class gArray<gList<Infoset *> >;
template gOutput &operator<<(gOutput &, const gArray<gList<Infoset *> > &);
template gOutput &operator<<(gOutput &, const gList<Infoset *> &);
