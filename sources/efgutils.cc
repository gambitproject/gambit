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
  if (n->BelongsTo()->IsLegalSubgame(n))   list.Append(n);
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

void Nodes (const BaseEfg &befg, gList <Node *> &list)
{
  list.Flush();
  NDoChild(befg.RootNode(), list); 
}

void Nodes (const BaseEfg &, Node *n, gList <Node *> &list)
{
  list.Flush();
  NDoChild(n, list);
}

void TerminalNodes (const BaseEfg &befg, gList <Node *> &list)
{
  list.Flush();
  TNDoChild(befg.RootNode(), list);
}

void NonTerminalNodes (const BaseEfg &befg, gList <Node *> &list)
{
  list.Flush();
  NTNDoChild(befg.RootNode(), list);
}

void MarkedSubgameRoots(const BaseEfg &efg, gList<Node *> &list)
{
  list.Flush();
  MSRDoChild(efg.RootNode(), list);
}

void LegalSubgameRoots(const BaseEfg &efg, gList<Node *> &list)
{
  list.Flush();
  LSRDoChild(efg.RootNode(), list);
}

void LegalSubgameRoots(Node *n, gList<Node *> &list)
{
  list.Flush();
  LSRDoChild(n, list);
}

bool AllSubgamesMarked(const BaseEfg &efg)
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

int NumNodes (const BaseEfg &befg)
{
  return (CountNodes(befg.RootNode()));
}

Action* LastAction( Node* node )
{
  Node* parent;
  Infoset* infoset;
  int childnum;
  int i;

  parent = node->GetParent();
  if( parent == 0 )
    return 0;
  infoset = parent->GetInfoset();
  for( i = 1; i <= parent->NumChildren(); i++ )
  {
    if( parent->GetChild( i ) == node )
    {
      childnum = i;
      break;
    }
  }
  return infoset->GetActionList()[ childnum ];
}


bool IsPerfectRecall(const BaseEfg &efg, Infoset *&s1, Infoset *&s2)
{
  for (int pl = 1; pl <= efg.NumPlayers(); pl++)   {
    EFPlayer *player = efg.PlayerList()[pl];
    
    for (int i = 1; i <= player->NumInfosets(); i++)  {
      Infoset *iset1 = player->InfosetList()[i];
      for (int j = 1; j <= player->NumInfosets(); j++)   {
	Infoset *iset2 = player->InfosetList()[j];

	bool precedes = false;
	int action = 0;
	
	for (int m = 1; m <= iset2->NumMembers(); m++)  {
	  int n;
	  for (n = 1; n <= iset1->NumMembers(); n++)  {
	    if (efg.IsPredecessor(iset1->GetMemberList()[n],
				  iset2->GetMemberList()[m]) &&
	        iset1->GetMemberList()[n] != iset2->GetMemberList()[m])  {
	      precedes = true;
	      for (int act = 1; act <= iset1->NumActions(); act++)  {
		if (efg.IsPredecessor(iset1->GetMemberList()[n]->GetChild(act),
				      iset2->GetMemberList()[m]))  {
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

template <class T> Efg<T> *CompressEfg(const Efg<T> &efg, const EFSupport &S)
{
  Efg<T> *newefg = new Efg<T>(efg);

  for (int pl = 1; pl <= newefg->NumPlayers(); pl++)   { 
    EFPlayer *player = newefg->PlayerList()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *infoset = player->InfosetList()[iset];
      for (int act = infoset->NumActions(); act >= 1; act--)  {
	Action *oldact = efg.PlayerList()[pl]->InfosetList()[iset]->GetActionList()[act];
	if (!S.Find(oldact))
	  newefg->DeleteAction(infoset, infoset->GetActionList()[act]);
      }
    }
  }

  return newefg;
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"
// prototype in efg.h

template <class T> void RandomEfg(Efg<T> &efg)
{
  for (int i = 1; i <= efg.NumPlayers(); i++)
    for (int j = 1; j <= efg.NumOutcomes(); j++)
      ((OutcomeVector<T>&) *efg.outcomes[j])[i] = (T) Uniform();
}

TEMPLATE void RandomEfg(Efg<double> &efg);
TEMPLATE void RandomEfg(Efg<gRational> &efg);

TEMPLATE Efg<double> *CompressEfg(const Efg<double> &, const EFSupport &);
TEMPLATE Efg<gRational> *CompressEfg(const Efg<gRational> &, const EFSupport &);


