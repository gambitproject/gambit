//
// FILE: efgutils.cc -- useful global functions for the extensive form
//
// $Id$
//

#include "efgutils.h"

// recursive functions

static void NDoChild (Node *n, gList <Node *> &list)
{ 
  list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NDoChild ( n->GetChild(i), list);
}

static void TNDoChild (Node *n, gList <Node *> &list)
{
  if (n->NumChildren() == 0) list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    TNDoChild ( n->GetChild(i), list);
}

static void NTNDoChild (Node *n, gList <Node *> &list)
{
  if (n->NumChildren() != 0) list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NTNDoChild ( n->GetChild(i), list);
}

static void MSRDoChild(Node *n, gList<Node *> &list)
{
  for (int i = 1; i <= n->NumChildren(); i++)
    MSRDoChild(n->GetChild(i), list);
  if (n->GetSubgameRoot() == n)  list.Append(n);
}

static void LSRDoChild(Node *n, gList<Node *> &list)
{
  for (int i = 1; i <= n->NumChildren(); i++)
    LSRDoChild(n->GetChild(i), list);
  if (n->Game()->IsLegalSubgame(n))   list.Append(n);
}

static void CSDoChild(Node *n, gList<Node *> &list)
{
  if (n->GetSubgameRoot() == n)
    list.Append(n);
  else
    for (int i = 1; i <= n->NumChildren(); i++)
      CSDoChild(n->GetChild(i), list);
}

// Public Functions
 
int CountNodes (Node *n)
{
  int num = 1;
  for (int i = 1; i <= n->NumChildren(); i++)
    num += CountNodes (n->GetChild(i));
  return num;
}

void Nodes (const Efg &befg, gList <Node *> &list)
{
  list.Flush();
  NDoChild(befg.RootNode(), list); 
}

void Nodes (const Efg &, Node *n, gList <Node *> &list)
{
  list.Flush();
  NDoChild(n, list);
}

void TerminalNodes (const Efg &befg, gList <Node *> &list)
{
  list.Flush();
  TNDoChild(befg.RootNode(), list);
}

void NonTerminalNodes (const Efg &befg, gList <Node *> &list)
{
  list.Flush();
  NTNDoChild(befg.RootNode(), list);
}

void MarkedSubgameRoots(const Efg &efg, gList<Node *> &list)
{
  list.Flush();
  MSRDoChild(efg.RootNode(), list);
}

void LegalSubgameRoots(const Efg &efg, gList<Node *> &list)
{
  list.Flush();
  LSRDoChild(efg.RootNode(), list);
}

void LegalSubgameRoots(Node *n, gList<Node *> &list)
{
  list.Flush();
  LSRDoChild(n, list);
}

bool HasSubgames(const Efg &efg)
{
  gList<Node *> list;
  LegalSubgameRoots(efg, list);
  return list.Length()>1;
}

bool HasSubgames(Node * n)
{
  gList<Node *> list;
  LegalSubgameRoots(n, list);
  if(n->Game()->IsLegalSubgame(n))
    return list.Length()>1;
  return list.Length()>0;
}

bool AllSubgamesMarked(const Efg &efg)
{
  gList<Node *> marked, valid;

  LegalSubgameRoots(efg, valid);
  MarkedSubgameRoots(efg, marked);

  return (marked == valid);
}


void ChildSubgames(Node *n, gList<Node *> &list)
{
  list.Flush();
  for (int i = 1; i <= n->NumChildren(); i++)
    CSDoChild(n->GetChild(i), list);
}

int NumNodes (const Efg &befg)
{
  return (CountNodes(befg.RootNode()));
}

Action *LastAction(Node *node)
{
  Node *parent = node->GetParent();
  if (parent == 0)  return 0;
  for (int i = 1; i <= parent->NumChildren(); i++) 
    if (parent->GetChild(i) == node)  
      return parent->GetInfoset()->Actions()[i];
  return 0;
}

bool IsPerfectRecall(const Efg &p_efg)
{
  Infoset *s1, *s2;
  return IsPerfectRecall(p_efg, s1, s2);
}

bool IsPerfectRecall(const Efg &efg, Infoset *&s1, Infoset *&s2)
{
  for (int pl = 1; pl <= efg.NumPlayers(); pl++)   {
    EFPlayer *player = efg.Players()[pl];
    
    for (int i = 1; i <= player->NumInfosets(); i++)  {
      Infoset *iset1 = player->Infosets()[i];
      for (int j = 1; j <= player->NumInfosets(); j++)   {
	Infoset *iset2 = player->Infosets()[j];

	bool precedes = false;
	int action = 0;
	
	for (int m = 1; m <= iset2->NumMembers(); m++)  {
	  int n;
	  for (n = 1; n <= iset1->NumMembers(); n++)  {
	    if (efg.IsPredecessor(iset1->Members()[n],
				  iset2->Members()[m]) &&
	        iset1->Members()[n] != iset2->Members()[m])  {
	      precedes = true;
	      for (int act = 1; act <= iset1->NumActions(); act++)  {
		if (efg.IsPredecessor(iset1->Members()[n]->GetChild(act),
				      iset2->Members()[m]))  {
		  if (action != 0 && action != act)  {
		    s1 = iset1;
		    s2 = iset2;
		    return false;
		  }
		  action = act;
		}
	      }
	      break;
	    }
	  }
	  
	  if (i == j && precedes)  {
	    s1 = iset1;
	    s2 = iset2;
	    return false;
	  }

	  if (n > iset1->NumMembers() && precedes)  {
	    s1 = iset1;
	    s2 = iset2;
	    return false;
	  }
	}
	

      }
    }
  }

  return true;
}

FullEfg *CompressEfg(const FullEfg &efg, const EFSupport &S)
{
  FullEfg *newefg = new FullEfg(efg);

  for (int pl = 1; pl <= newefg->NumPlayers(); pl++)   { 
    EFPlayer *player = newefg->Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->Infosets()[iset];
      for (int act = infoset->NumActions(); act >= 1; act--)  {
	Action *oldact = efg.Players()[pl]->Infosets()[iset]->Actions()[act];
	if (!S.Find(oldact))
	  newefg->DeleteAction(infoset, infoset->Actions()[act]);
      }
    }
  }

  return newefg;
}


#include "rational.h"
// prototype in efg.h

void RandomEfg(FullEfg &efg)
{
  for (int i = 1; i <= efg.NumPlayers(); i++)
    for (int j = 1; j <= efg.NumOutcomes(); j++)
      efg.SetPayoff(efg.outcomes[j], i, gNumber(Uniform()));
}

