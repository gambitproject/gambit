//
// FILE: sfg.cc -- Implementation of sequence form member functions
//              -- and Implementation of NFPlayer member functions
//
// $Id$ 
//

#include "sfg.h"
#include "garray.imp"
#include "gnarray.imp"

//----------------------------------------------------
// Sfg: Constructors, Destructors, Operators
//----------------------------------------------------


Sfg::Sfg(const EFSupport &S, const gArray<gNumber> &v)
  : EF(S.Game()), support(S), seq(EF.NumPlayers()),isets(EF.NumPlayers()),
    values(v)
{ 
  int i;
  gArray<int> zero(EF.NumPlayers());
  gArray<int> one(EF.NumPlayers());

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

  MakeSequenceForm(EF.RootNode(),(gNumber)1,one,zero);

  Dump();
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
}

void Sfg::
MakeSequenceForm(const Node *n, gNumber prob,gArray<int> seq, gArray<int> iset)
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
		     prob * EF.GetChanceProb(n->GetInfoset(), i), seq,iset);
    }
    else {
      int pl = n->GetPlayer()->GetNumber();

      iset[pl]=n->GetInfoset()->GetNumber();
      gArray<int> snew(seq);
      snew[pl]=1;
      for(i=1;i<iset[pl];i++)
	snew[pl]+=n->GetPlayer()->Infosets()[i]->NumActions();
    
      (*(*E)[pl])(iset[pl]+1,seq[pl]) = (gNumber)1;
      for(i=1;i<=n->NumChildren();i++) {
	snew[pl]+=1;
	(*(*E)[pl])(iset[pl]+1,snew[pl]) = -(gNumber)1;
	MakeSequenceForm(n->GetChild(i),prob,snew,iset);
      }
    }
    
  }
}

void Sfg::Dump()
{
  gIndexOdometer index(seq);

  gout << "\nSequence Form: \n";
  do {
    gout << "\nrow " << index.CurrentIndices() << ": " << (*(*SF)[index.CurrentIndices()]);
  } 
  while (index.Turn());

  gout << "\nConstraint matrices: \n";
  for(int i=1;i<=EF.NumPlayers();i++) 
    gout << "\nPlayer " << i << ":\n " << (*(*E)[i]);
}


template class gNArray<gArray<gNumber> *>;
template class gArray<gRectArray<gNumber> *>;
template gOutput &operator<<(gOutput &, const gArray<gNumber> &);

