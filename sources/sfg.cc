//
// FILE: sfg.cc -- Implementation of sequence form member functions
//
// $Id$ 
//

#include "sfg.h"
#include "sfstrat.h"
#include "garray.imp"
#include "gnarray.imp"

//----------------------------------------------------
// Sfg: Constructors, Destructors, Operators
//----------------------------------------------------


Sfg::Sfg(const EFSupport &S, const gArray<gNumber> &v)
  : EF(S.Game()), efsupp(S), seq(EF.NumPlayers()),isets(EF.NumPlayers()),
    values(v)
{ 
  int i;
  gArray<int> zero(EF.NumPlayers());
  gArray<int> one(EF.NumPlayers());
  EFSupport support(EF);

  for(i=1;i<=EF.NumPlayers();i++) {
    seq[i]=support.NumSequences(i);
    isets[i]=EF.Players()[i]->NumInfosets();
    zero[i]=0;
    one[i]=1;
  }

  gIndexOdometer index(seq);

  SF = new gNArray<gArray<gNumber> *>(seq);
  while (index.Turn()) {
    (*SF)[index.CurrentIndices()] = new gArray<gNumber>(EF.NumPlayers());
    for(i=1;i<=EF.NumPlayers();i++)
      (*(*SF)[index.CurrentIndices()])[i]=(gNumber)0;
  } 

  E = new gArray<gRectArray<gNumber> *> (EF.NumPlayers());
  for(i=1;i<=EF.NumPlayers();i++) {
    (*E)[i] = new gRectArray<gNumber>(isets[i]+1,seq[i]);
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

  for(int i=1;i<=EF.NumPlayers();i++) 
    delete (*E)[i];
  delete E;

  for(int i=1;i<=EF.NumPlayers();i++)
    delete (*sequences)[i];
  delete sequences;
}

void Sfg::
MakeSequenceForm(const Node *n, gNumber prob,gArray<int>seq, 
		 gArray<int> iset, gArray<Sequence *> parent) 
{ 
  int i,pl;

  if(n->GetOutcome()) {
    for(pl = 1;pl<=seq.Length();pl++)
      (*(*SF)[seq])[pl] +=
      prob * EF.Payoff(n->GetOutcome(),pl ).Evaluate(values);
  }
  if(n->GetInfoset()) {
    if(n->GetPlayer()->IsChance()) {
      for(i=1;i<=n->NumChildren();i++)
	MakeSequenceForm(n->GetChild(i),
		     prob * EF.GetChanceProb(n->GetInfoset(), i), seq,iset,parent);
    }
    else {
      int pl = n->GetPlayer()->GetNumber();

      iset[pl]=n->GetInfoset()->GetNumber();
      gArray<int> snew(seq);
      snew[pl]=1;
      for(i=1;i<iset[pl];i++)
	snew[pl]+=n->GetPlayer()->Infosets()[i]->NumActions();
    
      (*(*E)[pl])(iset[pl]+1,seq[pl]) = (gNumber)1;
      Sequence *myparent(parent[pl]);
      for(i=1;i<=n->NumChildren();i++) {
	snew[pl]+=1;
	if(n==(n->GetInfoset()->Members())[1]) {
	  Sequence* child;
	  child = new Sequence(n->GetPlayer(),(n->GetInfoset()->Actions())[i], 
			       myparent,snew[pl]);
	  parent[pl]=child;
//	  gout << (*child);;
	  ((*sequences)[pl])->AddSequence(child);
	}
	(*(*E)[pl])(iset[pl]+1,snew[pl]) = -(gNumber)1;
	MakeSequenceForm(n->GetChild(i),prob,snew,iset,parent);
      }
    }
    
  }
}

void Sfg::Dump(gOutput& out) const
{
  gIndexOdometer index(seq);

  out << "\nSequence Form: \n";
  while (index.Turn()) {
    out << "\nrow " << index.CurrentIndices() << ": " << (*(*SF)[index.CurrentIndices()]);
  } 
  
  out << "\nConstraint matrices: \n";
  for(int i=1;i<=EF.NumPlayers();i++) 
    out << "\nPlayer " << i << ":\n " << (*(*E)[i]);
}

int Sfg::InfosetNumber(int pl, int sequence) const 
{
  int i=1;
  while (Constraints(pl)(i,sequence) == 0) i++;
  return i-1;
}

int Sfg::ActionNumber(int pl, int sequence) const
{
  int j,s=1,act=0;

  j=InfosetNumber(pl,sequence);
  while (s<=sequence) { 
    if(Constraints(pl)(j+1,s) ==(gNumber)(-1))
      act++;
    s++;
  }
  return act;
}

BehavProfile<gNumber> Sfg::ToBehav(const gPVector<double> &x) const
{
  BehavProfile<gNumber> b(efsupp,values);

  Sequence *sij;
  const Sequence *parent;
  gNumber value;

  int i,j;
  for(i=1;i<=EF.NumPlayers();i++)
    for(j=2;j<=seq[i];j++) {
      sij = ((*sequences)[i]->GetSFSequenceSet())[j];
      int sn = sij->GetNumber();
      parent = sij->Parent();
      assert(x(i, parent->GetNumber())>(double)0);
      value = (gNumber)(x(i,sn)/x(i,parent->GetNumber()));
      b(i,sij->GetInfoset()->GetNumber(),sij->GetAction()->GetNumber())= value;
    }
  return b;
}
template class gNArray<gArray<gNumber> *>;
template class gArray<gRectArray<gNumber> *>;
template gOutput &operator<<(gOutput &, const gArray<gNumber> &);
