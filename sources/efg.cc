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

  Node new_node = nodes.InsertNode(pl, iset, n);

  for (int i = 1; i < children; i++)
    nodes.CreateNode(dummy, CreateInfoset(dummy, 0), new_node);

  return n;
}


Node ExtForm::DeleteNode(const Node &n, int keep)
{
  if (!nodes.IsMember(n))    return Node(dummy, 1, 1);
  if (nodes.IsTerminal(n))    return n;

  Node foo(n), ret;

  for (int i = 1; i < keep; i++)
    foo = DeleteTerminalNode(DeleteTree(nodes.GetChildNumber(foo, 1)));
  while (nodes.NumChildren(foo) > 1)
    foo = DeleteTerminalNode(DeleteTree(nodes.GetChildNumber(foo, 2)));
  if (nodes.DeleteInternalNode(foo, ret))
    players.RemoveInfoset(foo[1], foo[2]);

  return ret;
}

Node ExtForm::JoinInfoset(const Node &new_node, const Node &to_iset)
{
  if (!nodes.IsMember(new_node) || !nodes.IsMember(to_iset))
    return Node(dummy, 1, 1);
  
  if (nodes.NumChildren(new_node) != nodes.NumChildren(to_iset))
    return new_node;

  Node ret(to_iset);

  if (nodes.MoveNode(new_node, ret))
    players.RemoveInfoset(new_node[1], new_node[2]);

  return ret;
}

Node ExtForm::LeaveInfoset(const Node &n)
{
  if (!nodes.IsMember(n))   return Node(dummy, 1, 1);

  if (nodes.NumNodes(n[1], n[2]) == 1)   return n;

  Node ret(n[1], CreateInfoset(n[1], nodes.NumChildren(n)), 1);

      // we already know this won't empty the infoset...
  nodes.MoveNode(n, ret);

  return ret;
}

Node ExtForm::MergeInfoset(const Node &from, const Node &into)
{
  if (!nodes.IsMember(from) || !nodes.IsMember(into))
    return Node(dummy, 1, 1);

  if (nodes.NumChildren(from) != nodes.NumChildren(into))
    return from;

  Node ret(nodes.MoveNodes(from, into));

  players.RemoveInfoset(from[1], from[2]);

  return ret;
}


void ExtForm::InsertBranch(const Node &n, int where, int number)
{
  if (!nodes.IsMember(n))   return;
  
  for (int i = 0; i < number; i++)  
    players.InsertBranch(n[1], n[2], where + i);


      // we have to remember to insert the branch in all members of the iset
  for (i = 0; i < number; i++)
    for (int j = 1; j <= nodes.NumNodes(n[1], n[2]); j++)   
      nodes.InsertChild(dummy, CreateInfoset(dummy, 0), 
			Node(n[1], n[2], j), where + i);

}


Node ExtForm::DeleteBranch(const Node &n, int which)
{
  if (!nodes.IsMember(n))    return Node(dummy, 1, 1);

  players.RemoveBranch(n[1], n[2], which);

  Node ret(n);

      // remember to remove branch from all members of iset
  for (int i = 1; i <= nodes.NumNodes(ret[1], ret[2]); i++)  
    ret = DeleteTerminalNode(
	     DeleteTree(nodes.GetChildNumber(Node(ret[1],ret[2],i), which)));

  return ret;
}

Node ExtForm::MoveTree(const Node &src, const Node &dest)
{
  if (!nodes.IsMember(src) || !nodes.IsMember(dest)) return Node(dummy, 1, 1);
  if (!nodes.IsTerminal(dest))   return src;

  Node ret(src);
  nodes.AdoptChildren(src, dest);
  nodes.MoveNode(src, dest, 1);
  nodes.MoveNode(Node(dest[1], dest[2], 1), ret);
 
  return ret;
}



// Delete a terminal node, and return the new ID of its parent
Node ExtForm::DeleteTerminalNode(const Node &n)
{
  Node parent;

  if (nodes.DeleteNode(n, parent))
    players.RemoveInfoset(n[1], n[2]);
  return parent;
}

Node ExtForm::DeleteSubtree(Node n)
{
  Node parent;

  while (nodes.NumChildren(n))
    n = DeleteSubtree(nodes.GetChildNumber(n, 1));

  parent = DeleteTerminalNode(n);

  return parent;
}


// DeleteTree -- delete tree rooted at n, leave n as a terminal node
Node ExtForm::DeleteTree(const Node &n)
{
  if (!nodes.IsMember(n))     return Node(dummy, 1, 1);
  if (nodes.IsTerminal(n))    return n;

  Node foo(n);

  while (nodes.NumChildren(foo))
    foo = DeleteSubtree(nodes.GetChildNumber(foo, 1));

  Node ret(dummy, CreateInfoset(dummy, 0), 1);

  if (nodes.MoveNode(foo, ret))
    players.RemoveInfoset(foo[1], foo[2]);

  return ret;
}


void ExtForm::WriteToFile(FILE *f) const
{
  players.WriteToFile(f);
  fprintf(f, "\n\n");
  nodes.WriteToFile(f);
}
