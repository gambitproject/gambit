//
// FILE: extform.cc -- Implementation of extensive form data type
//
// $Id$
//

#include "extform.h"


const int dummy = -1;

void ExtForm::AddPlayer(int p)
{
  nodes.AddPlayer(p);
  players.AddPlayer(p);
}

int ExtForm::CreateInfoset(int p, int kids)
{
  nodes.CreateInfoset(p);
  return players.CreateInfoset(p, kids);
}

ExtForm &ExtForm::operator=(const ExtForm &ef)
{
  if (this != &ef)  {
    nodes = ef.nodes;
    players = ef.players;
  }
  return *this;
}


Node ExtForm::AddNode(const Node &n, int player, int child_count)
{
  if (!nodes.IsMember(n))   return Node(dummy, 1, 1);
  if (!nodes.IsTerminal(n))   return n;

  AddPlayer(player);

  int iset_no = CreateInfoset(player, child_count);

  Node ret(player, iset_no, 0);
  
  if (nodes.MoveNode(n, ret))  
    players.RemoveInfoset(n[1], n[2]);
  
  for (int i = 1; i <= child_count; i++)   
    nodes.CreateNode(dummy, CreateInfoset(dummy, 0), ret);
  return ret;
}


Node ExtForm::InsertNode(const Node &n, int pl, int children)
{
  if (!nodes.IsMember(n))   return Node(dummy, 1, 1);

  AddPlayer(pl);

  int iset = CreateInfoset(pl, children);

  nodes.CreateNode(pl, iset, nodes.GetParent(n));
  nodes.SetParent(n, Node(pl, iset, 1));

  for (int i = 1; i < children; i++)
    nodes.CreateNode(dummy, CreateInfoset(dummy, 0), Node(pl, iset, 1));

  return n;
}


Node ExtForm::DeleteNode(const Node &n)
{
  return n;
}

Node ExtForm::JoinInfoset(const Node &new_node, const Node &to_iset)
{
  return new_node;
}

Node ExtForm::MergeInfoset(const Node &from, const Node &into)
{
  return from;
}

void ExtForm::InsertBranch(const Node &n, int where, int number)
{
}

void ExtForm::DeleteBranch(const Node &n, int which)
{
}

Node ExtForm::MoveTree(const Node &from, const Node &dest)
{
  return from;
}

Node ExtForm::CopyTree(const Node &from, const Node &dest)
{
  return from;
}

Node ExtForm::DeleteTree(const Node &n)
{
  return n;
}



void ExtForm::WriteToFile(FILE *f) const
{
  players.WriteToFile(f);
  fprintf(f, "\n\n");
  nodes.WriteToFile(f);
}
