//#
//# FILE: extform.cc -- Implementation of extensive form data type
//#
//# $Id$
//#

#include "extform.h"
#include "player.h"
#include "infoset.h"
#include "node.h"
#include "outcome.h"
#include "gconvert.h"
#include <assert.h>

Player::~Player()
{
  while (infosets.Length())   delete infosets.Remove(1);
}

Infoset *Player::GetInfoset(const gString &name) const
{
  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->name == name)   return infosets[i];
  return 0;
}

//------------------------------------------------------------------------
//      BaseExtForm: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

BaseExtForm::BaseExtForm(void) : title("UNTITLED"), chance(new Player(0))
{ }

BaseExtForm::~BaseExtForm()
{
  delete root;
  delete chance;
  while (players.Length())    delete players.Remove(1);
  while (outcomes.Length())   delete outcomes.Remove(1);
}

//------------------------------------------------------------------------
//               BaseExtForm: Title access and manipulation
//------------------------------------------------------------------------

void BaseExtForm::SetTitle(const gString &s)
{ title = s; }

const gString &BaseExtForm::GetTitle(void) const
{ return title; }

//------------------------------------------------------------------------
//                    BaseExtForm: Writing data files
//------------------------------------------------------------------------

void BaseExtForm::DisplayTree(gOutput &f, Node *n) const
{
  f << "{ " << n << ' ';
  for (int i = 1; i <= n->children.Length(); DisplayTree(f, n->children[i++]));
  f << "} ";
}

void BaseExtForm::DisplayTree(gOutput &f) const
{
  DisplayTree(f, root);
}

void BaseExtForm::WriteEfgFile(gOutput &f, Node *n) const
{
  if (n->children.Length() == 0)   {
    f << "t \"" << n->name << "\" \"";
    if (n->outcome)  {
      f << n->outcome->name << "\" ";
      n->outcome->PrintValues(f);
      f << '\n';
    }
    else
      f << "\"\n";
  }
  
  else if (n->infoset->player->number)   {
    f << "p \"" << n->name << "\" \"" << n->infoset->player->name << "\" ";
    f << '"' << n->infoset->name << "\" ";
    n->infoset->PrintActions(f);
    f << " \"";
    if (n->outcome)  {
      f << n->outcome->name << "\" ";
      n->outcome->PrintValues(f);
      f << '\n';
    }
    else
      f << "\"\n";
  }
  
  else   {    // chance node
    f << "c \"" << n->name << "\" ";
    f << '"' << n->infoset->name << "\" ";
    n->infoset->PrintActions(f);
    f << " \"";
    if (n->outcome)  {
      f << n->outcome->name << "\" ";
      n->outcome->PrintValues(f);
      f << '\n';
    }
    else
      f << "\"\n";
  }

  for (int i = 1; i <= n->children.Length(); i++)
    WriteEfgFile(f, n->children[i]);
}

void BaseExtForm::WriteEfgFile(gOutput &f) const
{
  f << "EFG 1 " << ((Type() == DOUBLE) ? 'D' : 'R');
  f << " \"" << title << "\" { ";
  for (int i = 1; i <= players.Length(); i++)
    f << '"' << players[i]->name << "\" ";
  f << "}\n\n";

  WriteEfgFile(f, root);
}


//------------------------------------------------------------------------
//                    ExtForm<T>: General data access
//------------------------------------------------------------------------

int BaseExtForm::NumPlayers(void) const
{ return players.Length(); }

int BaseExtForm::NumOutcomes(void) const
{ return outcomes.Length(); }

Node *BaseExtForm::RootNode(void) const
{ return root; }

bool BaseExtForm::IsSuccessor(const Node *n, const Node *from) const
{ return false; }

bool BaseExtForm::IsPredecessor(const Node *n, const Node *of) const
{ return false; }

//------------------------------------------------------------------------
//                    ExtForm<T>: Operations on players
//------------------------------------------------------------------------

Player *BaseExtForm::GetChance(void) const
{
  return chance;
}

Player *BaseExtForm::GetPlayer(const gString &name) const
{
  for (int i = 1; i <= players.Length(); i++)
    if (players[i]->name == name)   return players[i];
  return 0;
}

Player *BaseExtForm::NewPlayer(void)
{
  Player *ret = new Player(players.Length() + 1);
  players.Append(ret);
  root->Resize(players.Length());
  return ret;
}

Infoset *BaseExtForm::AppendNode(Node *n, Player *p, int count)
{
  assert(n && p && count > 0);

  if (n->children.Length() == 0)   {
    n->infoset = CreateInfoset(p->infosets.Length() + 1, p, count);
    n->infoset->members.Append(n);
    p->infosets.Append(n->infoset);
    while (count--)
      n->children.Append(CreateNode(n));
  }

  return n->infoset;
}  

Infoset *BaseExtForm::AppendNode(Node *n, Infoset *s)
{
  assert(n && s);
  
  if (n->children.Length() == 0)   {
    n->infoset = s;
    s->members.Append(n);
    for (int i = 1; i <= s->actions.Length(); i++)
      n->children.Append(CreateNode(n));
  }

  return s;
}
  
Node *BaseExtForm::DeleteNode(Node *n, Node *keep)
{
  return n;
}

Infoset *BaseExtForm::InsertNode(Node *n, Player *p, int count)
{
  return 0;
}

Infoset *BaseExtForm::InsertNode(Node *n, Infoset *s)
{
  return 0;
}

Infoset *BaseExtForm::JoinInfoset(Infoset *s, Node *n)
{
  return s;
}

Infoset *BaseExtForm::LeaveInfoset(Node *n)
{
  return n->infoset;
}

Infoset *BaseExtForm::MergeInfoset(Infoset *to, Infoset *from)
{
  return to;
}

Infoset *BaseExtForm::SwitchPlayer(Infoset *s, Player *p)
{
  return s;
}

Infoset *BaseExtForm::SwitchPlayer(Node *n, Player *p)
{
  return n->infoset;
}

Node *BaseExtForm::CopyTree(Node *src, Node *dest)
{
  return dest;
}

Node *BaseExtForm::MoveTree(Node *src, Node *dest)
{
  return dest;
}

Node *BaseExtForm::DeleteTree(Node *n)
{
  return n;
}

Infoset *BaseExtForm::InsertAction(Infoset *s, Action *a, int count)
{
  return s;
}

Infoset *BaseExtForm::DeleteAction(Infoset *s, Action *a)
{
  return s;
}

//========================================================================

//---------------------------------------------------------------------------
//                    BaseBehavProfile member functions
//---------------------------------------------------------------------------

BaseBehavProfile::BaseBehavProfile(const BaseExtForm &EF, bool trunc)
  : E(&EF), truncated(trunc)  { }

BaseBehavProfile::~BaseBehavProfile()   { }

BaseBehavProfile &BaseBehavProfile::operator=(const BaseBehavProfile &p)
{
  E = p.E;
  truncated = p.truncated;
  return *this;
}

DataType BaseBehavProfile::Type(void) const
{
  return E->Type();
}


#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "glist.imp"

TEMPLATE class gList<Outcome *>;
TEMPLATE class gNode<Outcome *>;

TEMPLATE class gList<Node *>;
TEMPLATE class gNode<Node *>;

#include "garray.imp"
#include "gblock.imp"

TEMPLATE class gArray<Player *>;
TEMPLATE class gBlock<Player *>;

TEMPLATE class gArray<Infoset *>;
TEMPLATE class gBlock<Infoset *>;

TEMPLATE class gArray<Node *>;
TEMPLATE class gBlock<Node *>;

TEMPLATE class gArray<Action *>;
TEMPLATE class gBlock<Action *>;




