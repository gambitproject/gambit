//
// FILE: efg.cc -- Implementation of extensive form data type
//
// $Id$
//

class Node;
class EFPlayer;
class Infoset;
class Action;
class Outcome;
#include "garray.h"
#include "rational.h"
#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gArray<int>;
class gArray<gArray<int> *>;
class gArray<double>;
class gArray<gRational>;
template <class T> class gBlock;
class gBlock<double>;
class gBlock<gRational>;
#define TEMPLATE
#pragma option -Jgd
#endif   // __GNUG__, __BORLANDC__

#include "garray.imp"
#include "gblock.imp"

TEMPLATE class gArray<EFPlayer *>;
TEMPLATE class gBlock<EFPlayer *>;

TEMPLATE class gArray<Infoset *>;
TEMPLATE class gBlock<Infoset *>;

TEMPLATE class gArray<Node *>;
TEMPLATE class gBlock<Node *>;

TEMPLATE class gArray<Action *>;
TEMPLATE class gBlock<Action *>;

TEMPLATE class gArray<Outcome *>;
TEMPLATE class gBlock<Outcome *>;

class EFActionSet;
class EFActionArrays;

TEMPLATE bool operator==(const gArray<Action *> &, const gArray<Action *> &);
TEMPLATE class gArray<EFActionSet *>;
TEMPLATE class gArray<EFActionArrays *>;
//TEMPLATE class gArray<gBlock <Action *> *>;
//TEMPLATE class gArray<gArray <Action *> *>;
#include "glist.imp"

TEMPLATE class gList<Node *>;
TEMPLATE class gNode<Node *>;

#pragma -Jgx

#include "efg.h"
#include "efgutils.h"
#include <assert.h>

EFPlayer::~EFPlayer()
{
  while (infosets.Length())   delete infosets.Remove(1);
}

bool EFPlayer::IsInfosetDefined(const gString &s) const
{
  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->name == s)   return true;
  return false;
}

Infoset *EFPlayer::GetInfoset(const gString &name) const
{
  for (int i = 1; i <= infosets.Length(); i++)
    if (infosets[i]->name == name)   return infosets[i];
  return 0;
}

//------------------------------------------------------------------------
//      BaseExtForm: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

#ifdef MEMCHECK
int BaseEfg::_NumObj = 0;
#endif // MEMCHECK

BaseEfg::BaseEfg(void)
  : sortisets(true), title("UNTITLED"), chance(new EFPlayer(this, 0)),
    lexicon(0)
{
#ifdef MEMCHECK
  _NumObj++;
  gout << "--- BaseEfg Ctor: " << _NumObj << "\n";
#endif // MEMCHECK
}

BaseEfg::BaseEfg(const BaseEfg &E)
  : sortisets(false), title(E.title),
    players(E.players.Length()), chance(new EFPlayer(this, 0)),
    lexicon(0)
{
  for (int i = 1; i <= players.Length(); i++)  {
    (players[i] = new EFPlayer(this, i))->name = E.players[i]->name;
    for (int j = 1; j <= E.players[i]->infosets.Length(); j++)   {
      Infoset *s = new Infoset(this, j, players[i],
			       E.players[i]->infosets[j]->actions.Length());
      s->name = E.players[i]->infosets[j]->name;
      for (int k = 1; k <= s->actions.Length(); k++)
	s->actions[k]->name = E.players[i]->infosets[j]->actions[k]->name;
      players[i]->infosets.Append(s);
    }      
  }

#ifdef MEMCHECK
  _NumObj++;
  gout << "--- BaseEfg Ctor: " << _NumObj << "\n";
#endif // MEMCHECK
}

BaseEfg::~BaseEfg()
{
  delete root;
  delete chance;
  DeleteLexicon();
  int i;

  for (i = 1; i <= players.Length(); delete players[i++]);
  for (i = 1; i <= outcomes.Length(); delete outcomes[i++]);

  for (i = 1; i <= dead_nodes.Length(); delete dead_nodes[i++]);
  for (i = 1; i <= dead_infosets.Length(); delete dead_infosets[i++]);
  for (i = 1; i <= dead_outcomes.Length(); delete dead_outcomes[i++]);

#ifdef MEMCHECK
  _NumObj--;
  gout << "--- BaseEfg Dtor: " << _NumObj << "\n";
#endif // MEMCHECK
}

//------------------------------------------------------------------------
//                  BaseEfg: Private member functions
//------------------------------------------------------------------------

void BaseEfg::ScrapNode(Node *n)
{
  n->children.Flush();
  n->valid = false;
  dead_nodes.Append(n);
}

void BaseEfg::ScrapInfoset(Infoset *s)
{
  s->members.Flush();
  s->valid = false;
  dead_infosets.Append(s);
}

void BaseEfg::ScrapOutcome(Outcome *c)
{
  dead_outcomes.Append(c);
}

Infoset *BaseEfg::GetInfosetByIndex(EFPlayer *p, int index) const
{
  for (int i = 1; i <= p->infosets.Length(); i++)
    if (p->infosets[i]->number == index)   return p->infosets[i];
  return 0;
}

Outcome *BaseEfg::GetOutcomeByIndex(int index) const
{
  for (int i = 1; i <= outcomes.Length(); i++)
    if (outcomes[i]->number == index)   return outcomes[i];
  return 0;
}

void BaseEfg::Reindex(void)
{
  int i;

  for (i = 1; i <= players.Length(); i++)  {
    EFPlayer *p = players[i];
    for (int j = 1; j <= p->infosets.Length(); j++)
      p->infosets[j]->number = j;
  }

  for (i = 1; i <= outcomes.Length(); i++)
    outcomes[i]->number = i;
}

void BaseEfg::SortInfosets(void)
{
  if (!sortisets)  return;

  int pl;

  for (pl = 1; pl <= players.Length(); pl++)  {
    gList<Node *> nodes;

    Nodes(*this, nodes);

    int i, isets = 0;

    // First, move all empty infosets to the back of the list so
    // we don't "lose" them
    int foo = players[pl]->infosets.Length();
    i = 1;
    while (i < foo)   {
      if (players[pl]->infosets[i]->members.Length() == 0)  {
	Infoset *bar = players[pl]->infosets[i];
	players[pl]->infosets[i] = players[pl]->infosets[foo];
	players[pl]->infosets[foo--] = bar;
      }
      else
	i++;
    }

    // This will give empty infosets their proper number; the nonempty
    // ones will be renumbered by the next loop
    for (i = 1; i <= players[pl]->infosets.Length(); i++)
      if (players[pl]->infosets[i]->members.Length() == 0)
	players[pl]->infosets[i]->number = i;
      else
	players[pl]->infosets[i]->number = 0;
  
    for (i = 1; i <= nodes.Length(); i++)  {
      Node *n = nodes[i];
      if (n->GetPlayer() == players[pl] && n->GetInfoset()->number == 0)  {
	n->GetInfoset()->number = ++isets;
	players[pl]->infosets[isets] = n->GetInfoset();
      }
    }  

    assert(isets == players[pl]->infosets.Length() ||
	   players[pl]->infosets[isets + 1]->members.Length() == 0);
//    assert(isets == players[pl]->infosets.Length());
  }

  // Now, we sort the nodes within the infosets
  
  gList<Node *> nodes;
  Nodes(*this, nodes);

  for (pl = 1; pl <= players.Length(); pl++)  {
    for (int iset = 1; iset <= players[pl]->infosets.Length(); iset++)  {
      Infoset *s = players[pl]->infosets[iset];
      for (int i = 1, j = 1; i <= nodes.Length(); i++)  {
	if (nodes[i]->infoset == s)
	  s->members[j++] = nodes[i];
      }
    }
  }

}
  

//------------------------------------------------------------------------
//               BaseEfg: Title access and manipulation
//------------------------------------------------------------------------

void BaseEfg::SetTitle(const gString &s)
{ title = s; }

const gString &BaseEfg::GetTitle(void) const
{ return title; }

//------------------------------------------------------------------------
//                    BaseEfg: Writing data files
//------------------------------------------------------------------------

void BaseEfg::DisplayTree(gOutput &f, Node *n) const
{
  f << "{ " << n << ' ';
  for (int i = 1; i <= n->children.Length(); DisplayTree(f, n->children[i++]));
  f << "} ";
}

void BaseEfg::DisplayTree(gOutput &f) const
{
  DisplayTree(f, root);
}

void BaseEfg::WriteEfgFile(gOutput &f, Node *n) const
{
  if (n->children.Length() == 0)   {
    f << "t \"" << n->name << "\" ";
    if (n->outcome)  {
      f << n->outcome->number << " \"" << n->outcome->name << "\" ";
      n->outcome->PrintValues(f);
      f << '\n';
    }
    else
      f << "0\n";
  }
  
  else if (n->infoset->player->number)   {
    f << "p \"" << n->name << "\" " << n->infoset->player->number << ' ';
    f << n->infoset->number << " \"" << n->infoset->name << "\" ";
    n->infoset->PrintActions(f);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->number << " \"" << n->outcome->name << "\" ";
      n->outcome->PrintValues(f);
      f << '\n';
    }
    else
      f << "0\n";
  }
  
  else   {    // chance node
    f << "c \"" << n->name << "\" ";
    f << n->infoset->number << " \"" << n->infoset->name << "\" ";
    n->infoset->PrintActions(f);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->number << " \"" << n->outcome->name << "\" ";
      n->outcome->PrintValues(f);
      f << '\n';
    }
    else
      f << "0\n";
  }

  for (int i = 1; i <= n->children.Length(); i++)
    WriteEfgFile(f, n->children[i]);
}

void BaseEfg::WriteEfgFile(gOutput &f) const
{
  f << "EFG 2 " << ((Type() == DOUBLE) ? 'D' : 'R');
  f << " \"" << title << "\" { ";
  for (int i = 1; i <= players.Length(); i++)
    f << '"' << players[i]->name << "\" ";
  f << "}\n\n";

  WriteEfgFile(f, root);
}


//------------------------------------------------------------------------
//                    BaseEfg: General data access
//------------------------------------------------------------------------

int BaseEfg::NumPlayers(void) const
{ return players.Length(); }

int BaseEfg::NumOutcomes(void) const
{ return outcomes.Length(); }

void BaseEfg::DeleteOutcome(Outcome *outc)
{
  root->DeleteOutcome(outc);
  outcomes.Remove(outcomes.Find(outc));
  ScrapOutcome(outc);
  DeleteLexicon();
}

Node *BaseEfg::RootNode(void) const
{ return root; }

bool BaseEfg::IsSuccessor(const Node *n, const Node *from) const
{ return IsPredecessor(from, n); }

bool BaseEfg::IsPredecessor(const Node *n, const Node *of) const
{ 
  while (of && n != of)    of = of->parent;

  return (n == of);
}

//------------------------------------------------------------------------
//                    ExtForm<T>: Operations on players
//------------------------------------------------------------------------

EFPlayer *BaseEfg::GetChance(void) const
{
  return chance;
}

EFPlayer *BaseEfg::NewPlayer(void)
{
  EFPlayer *ret = new EFPlayer(this, players.Length() + 1);
  players.Append(ret);
  root->Resize(players.Length());
  for (int i = 1; i <= outcomes.Length(); i++)
    outcomes[i]->Resize(players.Length());
  DeleteLexicon();
  return ret;
}

Infoset *BaseEfg::AppendNode(Node *n, EFPlayer *p, int count)
{
  assert(n && p && count > 0);

  if (n->children.Length() == 0)   {
    n->infoset = CreateInfoset(p->infosets.Length() + 1, p, count);
    n->infoset->members.Append(n);
    while (count--)
      n->children.Append(CreateNode(n));
  }

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}  

Infoset *BaseEfg::AppendNode(Node *n, Infoset *s)
{
  assert(n && s);
  
  // Can't bridge subgames...
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;

  if (n->children.Length() == 0)   {
    n->infoset = s;
    s->members.Append(n);
    for (int i = 1; i <= s->actions.Length(); i++)
      n->children.Append(CreateNode(n));
  }

  DeleteLexicon();
  SortInfosets();
  return s;
}
  
Node *BaseEfg::DeleteNode(Node *n, Node *keep)
{
  assert(n && keep);

  if (keep->parent != n)   return n;

  if (n->gameroot == n)
    MarkSubgame(keep, keep);

  n->children.Remove(n->children.Find(keep));
  DeleteTree(n);
  keep->parent = n->parent;
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = keep;
  else
    root = keep;

  ScrapNode(n);
  DeleteLexicon();
  SortInfosets();

  return keep;
}

Infoset *BaseEfg::InsertNode(Node *n, EFPlayer *p, int count)
{
  assert(n && p && count > 0);

  Node *m = CreateNode(n->parent);
  m->infoset = CreateInfoset(p->infosets.Length() + 1, p, count);
  m->infoset->members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    root = m;
  m->children.Append(n);
  n->parent = m;
  while (--count)
    m->children.Append(CreateNode(m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

Infoset *BaseEfg::InsertNode(Node *n, Infoset *s)
{
  assert(n && s);

  // can't bridge subgames
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;
  
  Node *m = CreateNode(n->parent);
  m->infoset = s;
  s->members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    root = m;
  m->children.Append(n);
  n->parent = m;
  int count = s->actions.Length();
  while (--count)
    m->children.Append(CreateNode(m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

Infoset *BaseEfg::JoinInfoset(Infoset *s, Node *n)
{
  assert(n && s);

  // can't bridge subgames
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;
  
  if (!n->infoset)   return 0; 
  if (n->infoset == s)   return s;
  if (s->actions.Length() != n->children.Length())  return n->infoset;

  Infoset *t = n->infoset;
  EFPlayer *p = n->infoset->player;

  t->members.Remove(t->members.Find(n));
  if (t->members.Length() == 0)
    dead_infosets.Append(p->infosets.Remove(p->infosets.Find(t)));
  s->members.Append(n);

  n->infoset = s;

  DeleteLexicon();
  SortInfosets();
  return s;
}

Infoset *BaseEfg::LeaveInfoset(Node *n)
{
  assert(n);

  if (!n->infoset)   return 0;

  Infoset *s = n->infoset;
  if (s->members.Length() == 1)   return s;

  EFPlayer *p = s->player;
  s->members.Remove(s->members.Find(n));
  n->infoset = CreateInfoset(p->infosets.Length() + 1, p,
			     n->children.Length());
  n->infoset->name = s->name;
  n->infoset->members.Append(n);
  for (int i = 1; i <= s->actions.Length(); i++)
    n->infoset->actions[i]->name = s->actions[i]->name;

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}

Infoset *BaseEfg::MergeInfoset(Infoset *to, Infoset *from)
{
  assert(to && from);

  if (to == from ||
      to->actions.Length() != from->actions.Length())   return from;

  if (to->members[1]->gameroot != from->members[1]->gameroot) 
    return from;

  to->members += from->members;
  for (int i = 1; i <= from->members.Length(); i++)
    from->members[i]->infoset = to;
  dead_infosets.Append(from->player->infosets.Remove(from->player->infosets.Find(from)));

  DeleteLexicon();
  SortInfosets();
  return to;
}

Infoset *BaseEfg::SwitchPlayer(Infoset *s, EFPlayer *p)
{
  assert(s && p);
  
  if (s->player == p)   return s;

  s->player->infosets.Remove(s->player->infosets.Find(s));
  s->player = p;
  p->infosets.Append(s);

  DeleteLexicon();
  SortInfosets();
  return s;
}

void BaseEfg::CopySubtree(Node *src, Node *dest, Node *stop)
{
  if (src == stop) {
    dest->outcome = src->outcome;
    return;
  }

  if (src->children.Length())  {
    AppendNode(dest, src->infoset);
    for (int i = 1; i <= src->children.Length(); i++)
      CopySubtree(src->children[i], dest->children[i], stop);
  }

  dest->name = src->name;
  dest->outcome = src->outcome;
}

void BaseEfg::MarkSubtree(Node *n)
{
  n->mark=true;
  for(int i=1;i<=n->children.Length();i++)
    MarkSubtree(n->children[i]);
}

void BaseEfg::UnmarkSubtree(Node *n)
{
  n->mark=false;
  for(int i=1;i<=n->children.Length();i++)
    UnmarkSubtree(n->children[i]);
}

void BaseEfg::Reveal(Infoset *where, const gArray<EFPlayer *> &who)
{
  int i,j,k,m;
  bool flag;
  gBlock<Node*> OldMembers;
  Node *n;
  Infoset *newiset;

  if(where->actions.Length()<=1)return;
  UnmarkSubtree(root);  // start with a clean tree
  
  for(i=1;i<=where->actions.Length();i++) {
    for(j=1;j<=where->members.Length();j++) 
      MarkSubtree(where->members[j]->children[i]);
    for(j=who.First();j<=who.Last();j++)
      for(k=1;k<=who[j]->infosets.Length();k++) {
	    // make copy to iterate correctly 
	OldMembers = who[j]->infosets[k]->members;
	flag=false;
	for(m=1;m<=OldMembers.Length();m++) {
	  n = OldMembers[m];
	  if(n->mark) {
	   n->mark=false;
	   if(!flag) {
	     newiset = LeaveInfoset(n);
	     flag=true;
	   }
	   else 
	     JoinInfoset(newiset,n);
	 }	    
	}
      }
  }

  Reindex();
}

Node *BaseEfg::CopyTree(Node *src, Node *dest)
{
  assert(src && dest);
  if (src == dest || dest->children.Length())   return src;
  if (src->gameroot != dest->gameroot)  return src;

  if (src->children.Length())  {
    AppendNode(dest, src->infoset);
    for (int i = 1; i <= src->children.Length(); i++)
      CopySubtree(src->children[i], dest->children[i], dest);
  }

  DeleteLexicon();
  SortInfosets();
  return dest;
}

Node *BaseEfg::MoveTree(Node *src, Node *dest)
{
  assert(src && dest);
  if (src == dest || dest->children.Length() || IsPredecessor(src, dest))
    return src;
  if (src->gameroot != dest->gameroot)  return src;

  Node *parent = src->parent;    // cannot be null, saves us some problems

  parent->children[parent->children.Find(src)] = dest;
  dest->parent->children[dest->parent->children.Find(dest)] = src;

  src->parent = dest->parent;
  dest->parent = parent;

  dest->name = "";
  dest->outcome = 0;
  
  DeleteLexicon();
  SortInfosets();
  return dest;
}

Node *BaseEfg::DeleteTree(Node *n)
{
  assert(n);

  while (n->NumChildren() > 0)   {
    DeleteTree(n->children[1]);
    ScrapNode(n->children.Remove(1));
  }
  
  if (n->infoset)  {
    n->infoset->members.Remove(n->infoset->members.Find(n));
    if (n->infoset->members.Length() == 0)
      dead_infosets.Append(n->infoset->player->infosets.Remove(n->infoset->player->infosets.Find(n->infoset)));
    for (int j = 1; j <= n->infoset->player->infosets.Length(); j++)
      n->infoset->player->infosets[j]->number = j;
    n->infoset = 0;
  }

  n->outcome = 0;
  n->name = "";

  DeleteLexicon();
  SortInfosets();
  return n;
}

Infoset *BaseEfg::AppendAction(Infoset *s)
{
  assert(s);
  s->InsertAction(s->NumActions() + 1);
  for (int i = 1; i <= s->members.Length(); i++)
    s->members[i]->children.Append(CreateNode(s->members[i]));
  DeleteLexicon();
  return s;
}

Infoset *BaseEfg::InsertAction(Infoset *s, Action *a)
{
  assert(a && s);
  int where;
  for (where = 1; where <= s->actions.Length() && s->actions[where] != a;
       where++);
  if (where > s->actions.Length())   return s;
  s->InsertAction(where);
  for (int i = 1; i <= s->members.Length(); i++)
    s->members[i]->children.Insert(CreateNode(s->members[i]), where);

  DeleteLexicon();
  return s;
}

Infoset *BaseEfg::DeleteAction(Infoset *s, Action *a)
{
  assert(a && s);
  int where;
  for (where = 1; where <= s->actions.Length() && s->actions[where] != a;
       where++);
  if (where > s->actions.Length())   return s;
  s->RemoveAction(where);
  for (int i = 1; i <= s->members.Length(); i++)   {
    DeleteTree(s->members[i]->children[where]);
    delete s->members[i]->children.Remove(where);
  }
  DeleteLexicon();
  return s;
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

void BaseEfg::MarkTree(Node *n, Node *base)
{
  n->ptr = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkTree(n->GetChild(i), base);
}

bool BaseEfg::CheckTree(Node *n, Node *base)
{
  int i;

  if (n->NumChildren() == 0)   return true;

  for (i = 1; i <= n->NumChildren(); i++)
    if (!CheckTree(n->GetChild(i), base))  return false;

  if (n->GetPlayer()->IsChance())   return true;

  for (i = 1; i <= n->GetInfoset()->NumMembers(); i++)
    if (n->GetInfoset()->GetMember(i)->ptr != base)
      return false;

  return true;
}

bool BaseEfg::IsLegalSubgame(Node *n)
{
  if (n->NumChildren() == 0 || n->GetPlayer()->IsChance())  
    return false;

  MarkTree(n, n);
  return CheckTree(n, n);
}

bool BaseEfg::DefineSubgame(Node *n)
{
  if (n->gameroot != n && IsLegalSubgame(n))  {
    n->gameroot = 0;
    MarkSubgame(n, n);
    return true;
  }

  return false;
}

void BaseEfg::RemoveSubgame(Node *n)
{
  if (n->gameroot == n && n->parent)  {
    n->gameroot = 0;
    MarkSubgame(n, n->parent->gameroot);
  }
}
  

void BaseEfg::MarkSubgame(Node *n, Node *base)
{
  if (n->gameroot == n)  return;
  n->gameroot = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkSubgame(n->GetChild(i), base);
}

void BaseEfg::MarkSubgames(const gList<Node *> &list)
{
  for (int i = 1; i <= list.Length(); i++)  {
    if (IsLegalSubgame(list[i]))  {
      list[i]->gameroot = 0;
      MarkSubgame(list[i], list[i]);
    }
  }
}

void BaseEfg::UnmarkSubgames(Node *n)
{
  if (n->NumChildren() == 0)   return;

  for (int i = 1; i <= n->NumChildren(); i++)
    UnmarkSubgames(n->GetChild(i));
  
  if (n->gameroot == n && n->parent)  {
    n->gameroot = 0;
    MarkSubgame(n, n->parent->gameroot);
  }
}


//========================================================================

#include "behav.h"
//--------------------------------------------------------------------------
//                    BaseBehavProfile member functions
//--------------------------------------------------------------------------

BaseBehavProfile::BaseBehavProfile(const BaseEfg &EF, bool trunc)
  : E(&EF), truncated(trunc), behavsupport(EF)  { }

BaseBehavProfile::BaseBehavProfile(const BaseEfg &EF, bool trunc,
				   const EFSupport &s)
  : E(&EF), truncated(trunc), behavsupport(s) 
{ 
  behavsupport.SetupSupport();
}

BaseBehavProfile::BaseBehavProfile(const BaseBehavProfile &p)
  : E(p.E), truncated(p.truncated), behavsupport(p.behavsupport)   { }

BaseBehavProfile::~BaseBehavProfile()   { }

BaseBehavProfile &BaseBehavProfile::operator=(const BaseBehavProfile &p)
{
  E = p.E;
  truncated = p.truncated;
  behavsupport = p.behavsupport;
  return *this;
}

DataType BaseBehavProfile::Type(void) const
{
  return E->Type();
}

const gString &BaseBehavProfile::GetPlayerName(int p) const
{
  return E->PlayerList()[p]->GetName();
}

const gString &BaseBehavProfile::GetInfosetName(int p, int iset) const
{
  return E->PlayerList()[p]->InfosetList()[iset]->GetName();
}

const gString &BaseBehavProfile::GetActionName(int p, int iset, int act) const 
{
  return E->PlayerList()[p]->InfosetList()[iset]->GetActionName(act);
}

EFSupport &BaseBehavProfile::GetEFSupport(void) 
{
  return (behavsupport);
}

