// 
//  FILE efgutils.cc -- useful global functions for the extensive form
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
    NDoChild ( n->GetChild(i), list);
}

void NTNDoChild (Node *n, gList <Node *> &list)
{
  if (n->NumChildren() != 0) list.Append(n);
  for (int i = 1; i <= n->NumChildren(); i++)
    NDoChild ( n->GetChild(i), list);
}

int CountChildren (Node *n)
{
  int num = 1;
  for (int i = 1; i <= n->NumChildren(); i++)
    num += CountChildren (n->GetChild(i));
  return num;
}


// Public Functions

void Nodes (const BaseEfg &befg, gList <Node *> &list)
{
  list.Flush();
  NDoChild(befg.RootNode(), list); 
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

int NumNodes (const BaseEfg &befg)
{
  return (CountChildren(befg.RootNode()));
}


#include "gmisc.h"
#include "efg.h"

// prototype in efg.h

template <class T> void RandomEfg(Efg<T> &efg)
{
  for (int i = 1; i <= efg.NumPlayers(); i++)
    for (int j = 1; j <= efg.NumOutcomes(); j++)
      ((OutcomeVector<T>&) *efg.outcomes[j])[i] = (T) Uniform();
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "rational.h"

TEMPLATE void RandomEfg(Efg<double> &efg);
TEMPLATE void RandomEfg(Efg<gRational> &efg);



