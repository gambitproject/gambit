// 
// FILE: efgutils.cc -- useful global functions for the extensive form
//
// $Id$
//

#include "efgutils.h"

// recursive functions

void NDoChild (Node *n, gList <Node *> &list)
{ 
  list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NDoChild ( n->GetChild(i), list);
}

void TNDoChild (Node *n, gList <Node *> &list)
{
  if (n->NumChildren() == 0) list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    TNDoChild ( n->GetChild(i), list);
}

void NTNDoChild (Node *n, gList <Node *> &list)
{
  if (n->NumChildren() != 0) list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NTNDoChild ( n->GetChild(i), list);
}

void SRDoChild(Node *n, gList<Node *> &list)
{
  for (int i = 1; i <= n->NumChildren(); i++)
    SRDoChild(n->GetChild(i), list);
  if (n->GetSubgameRoot() == n)  list.Append(n);
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

void Nodes (const BaseEfg &befg, Node *n, gList <Node *> &list)
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

void SubgameRoots(const BaseEfg &efg, gList<Node *> &list)
{
  list.Flush();
  SRDoChild(efg.RootNode(), list);
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



#include "gmisc.h"
#include "efg.h"


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



