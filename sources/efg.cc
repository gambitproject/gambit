//
// FILE: efg.cc -- Implementation of extensive form data type
//
//  $Id$
//

#include "garray.h"
#include "rational.h"
#include "glist.h"

#ifdef __GNUG__
#pragma implementation "outcome.h"
#pragma implementation "infoset.h"
#pragma implementation "efplayer.h"
#pragma implementation "node.h"
#endif   // __GNUG__

#include "efg.h"
#include "efgutils.h"
#include "gsm.h"


//----------------------------------------------------------------------
//                 EFPlayer: Member function definitions
//----------------------------------------------------------------------

EFPlayer::~EFPlayer()
{
  while (infosets.Length())   delete infosets.Remove(1);
}


//----------------------------------------------------------------------
//                 Infoset: Member function definitions
//----------------------------------------------------------------------

Infoset::Infoset(Efg *e, int n, EFPlayer *p, int br)
  : E(e), number(n), player(p), actions(br), flag(0) 
{
  while (br)   {
    actions[br] = new Action(br, ToText(br), this);
    br--; 
  }
}

Infoset::~Infoset()  
{
  for (int i = 1; i <= actions.Length(); i++)  delete actions[i];
}

void Infoset::PrintActions(gOutput &f) const
{ 
  f << "{ ";
  for (int i = 1; i <= actions.Length(); i++)
    f << '"' << actions[i]->name << "\" ";
  f << "}";
}

Action *Infoset::InsertAction(int where)
{
  Action *action = new Action(where, "", this);
  actions.Insert(action, where);
  for (; where <= actions.Length(); where++)
    actions[where]->number = where;
  return action;
}

void Infoset::RemoveAction(int which)
{
  delete actions.Remove(which);
  for (; which <= actions.Length(); which++)
    actions[which]->number = which;
}

const gList<const Action *> Infoset::ListOfActions(void) const
{
  gList<const Action *> answer;
  for (int i = 1; i <= actions.Length(); i++) 
    answer += actions[i];
  return answer;
}

const gList<const Node *> Infoset::ListOfMembers(void) const
{
  gList<const Node *> answer;
  for (int i = 1; i <= members.Length(); i++) 
    answer += members[i];
  return answer;
}

//------------------------------------------------------------------------
//           ChanceInfoset: Member function definitions
//------------------------------------------------------------------------

class ChanceInfoset : public Infoset  {
  friend class Efg;

  private:
    gBlock<gNumber> probs;

    ChanceInfoset(Efg *E, int n, EFPlayer *p, int br);
    virtual ~ChanceInfoset()    { }

    void PrintActions(gOutput &f) const;

  public:
    Action *InsertAction(int where);
    void RemoveAction(int which);

    void SetActionProb(int i, const gNumber &value)  { probs[i] = value; }
    const gNumber &GetActionProb(int i) const   { return probs[i]; }
    const gArray<gNumber> &GetActionProbs(void) const  { return probs; }
};


ChanceInfoset::ChanceInfoset(Efg *E, int n, EFPlayer *p, int br)
  : Infoset(E, n, p, br), probs(br)
{
  for (int i = 1; i <= br; probs[i++] = gRational(1, br));
}

Action *ChanceInfoset::InsertAction(int where)
{ 
  Action *action = Infoset::InsertAction(where);
  probs.Insert((gNumber) 0, where);
  return action;
}

void ChanceInfoset::RemoveAction(int which)
{
  Infoset::RemoveAction(which);
  probs.Remove(which);
}

void ChanceInfoset::PrintActions(gOutput &f) const
{ 
  f << "{ ";
  for (int i = 1; i <= actions.Length(); i++)
    f << '"' << actions[i]->GetName() << "\" " << probs[i] << ' ';
  f << "}";
}

//----------------------------------------------------------------------
//                   Node: Member function definitions
//----------------------------------------------------------------------

Node::Node(Efg *e, Node *p)
  : mark(false), number(0), E(e), infoset(0), parent(p), outcome(0),
    gameroot((p) ? p->gameroot : this)
{ }

Node::~Node()
{
  for (int i = children.Length(); i; delete children[i--]);
}


Node *Node::NextSibling(void) const  
{
  if (!parent)   return 0;
  if (parent->children.Find((Node * const) this) == parent->children.Length())
    return 0;
  else
    return parent->children[parent->children.Find((Node * const)this) + 1];
}

Node *Node::PriorSibling(void) const
{ 
  if (!parent)   return 0;
  if (parent->children.Find((Node * const)this) == 1)
    return 0;
  else
    return parent->children[parent->children.Find((Node * const)this) - 1];

}

Action *Node::GetAction() const
{
  if (this == Game()->RootNode()) 
    throw Efg::Exception();
  
  gArray<Action *> actions = GetParent()->GetInfoset()->Actions();
  for (int i = 1; i <= actions.Length(); i++)
    if (this == GetParent()->GetChild(actions[i]))
      return actions[i];
  throw Efg::Exception();
}

void Node::DeleteOutcome(EFOutcome *outc)
{
  if (outc == outcome)   outcome = 0;
  for (int i = 1; i <= children.Length(); i++)
    children[i]->DeleteOutcome(outc);
}


//------------------------------------------------------------------------
//       Efg: Constructors, destructor, constructive operators
//------------------------------------------------------------------------

#ifdef MEMCHECK
int Efg::_NumObj = 0;
#endif // MEMCHECK

Efg::Efg(void)
  : sortisets(true), title("UNTITLED"), chance(new EFPlayer(this, 0)),
    afg(0), lexicon(0)
{
  root = new Node(this, 0);
#ifdef MEMCHECK
  _NumObj++;
  gout << "--- Efg Ctor: " << _NumObj << "\n";
#endif // MEMCHECK
}

Efg::Efg(const Efg &E, Node *n /* = 0 */)
  : sortisets(false), title(E.title), comment(E.comment),
    players(E.players.Length()), outcomes(E.outcomes.Length()),
    chance(new EFPlayer(this, 0)),
    afg(0), lexicon(0)
{
  int i = 0;
  for ( i = 1; i <= players.Length(); i++)  {
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

  for ( i = 1; i <= E.GetChance()->NumInfosets(); i++)   {
    ChanceInfoset *t = (ChanceInfoset *) E.GetChance()->Infosets()[i];
    ChanceInfoset *s = new ChanceInfoset(this, i, chance,
					       t->NumActions());
    s->name = t->GetName();
    for (int act = 1; act <= s->probs.Length(); act++) {
      s->probs[act] = t->probs[act];
      s->actions[act]->name = t->actions[act]->name;
    }
    chance->infosets.Append(s);
  }

  for (int outc = 1; outc <= E.NumOutcomes(); outc++)  {
    (outcomes[outc] = new EFOutcome(this, outc))->name = E.outcomes[outc]->name;
    outcomes[outc]->payoffs = E.outcomes[outc]->payoffs;
  }

  root = new Node(this, 0);
  CopySubtree(root, (n ? n : E.RootNode()));
  
  if (n)   {
    for (int pl = 1; pl <= players.Length(); pl++)  {
      for (int i = 1; i <= players[pl]->infosets.Length(); i++)  {
	if (players[pl]->infosets[i]->members.Length() == 0)
	  delete players[pl]->infosets.Remove(i--);
      }
    }
  }

  sortisets = true;
  SortInfosets();

#ifdef MEMCHECK
  _NumObj++;
  gout << "--- Efg Ctor: " << _NumObj << "\n";
#endif // MEMCHECK
}

#ifndef EFG_ONLY
#include "lexicon.h"
#endif   // EFG_ONLY

Efg::~Efg()
{
  delete root;
  delete chance;
  int i;

  for (i = 1; i <= players.Length(); delete players[i++]);
  for (i = 1; i <= outcomes.Length(); delete outcomes[i++]);

#ifndef EFG_ONLY
  if (lexicon)   delete lexicon;
  lexicon = 0;
#endif   // EFG_ONLY

#ifdef MEMCHECK
  _NumObj--;
  gout << "--- Efg Dtor: " << _NumObj << "\n";
#endif // MEMCHECK
}

//------------------------------------------------------------------------
//                  Efg: Private member functions
//------------------------------------------------------------------------

void Efg::DeleteLexicon(void) const
{
#ifndef EFG_ONLY
  if (lexicon)   delete lexicon;
  lexicon = 0;
#endif   // EFG_ONLY
}

Infoset *Efg::GetInfosetByIndex(EFPlayer *p, int index) const
{
  for (int i = 1; i <= p->infosets.Length(); i++)
    if (p->infosets[i]->number == index)   return p->infosets[i];
  return 0;
}

EFOutcome *Efg::GetOutcomeByIndex(int index) const
{
  for (int i = 1; i <= outcomes.Length(); i++)
    if (outcomes[i]->number == index)   return outcomes[i];
  return 0;
}

void Efg::Reindex(void)
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


void Efg::NumberNodes(Node *n, int &index)
{
  n->number = index++;
  for (int child = 1; child <= n->children.Length();
       NumberNodes(n->children[child++], index));
} 

void Efg::SortInfosets(void)
{
  if (!sortisets)  return;

  int pl;

  for (pl = 0; pl <= players.Length(); pl++)  {
    gList<Node *> nodes;

    Nodes(*this, nodes);

    EFPlayer *player = (pl) ? players[pl] : chance;

    int i, isets = 0;

    // First, move all empty infosets to the back of the list so
    // we don't "lose" them
    int foo = player->infosets.Length();
    i = 1;
    while (i < foo)   {
      if (player->infosets[i]->members.Length() == 0)  {
	Infoset *bar = player->infosets[i];
	player->infosets[i] = player->infosets[foo];
	player->infosets[foo--] = bar;
      }
      else
	i++;
    }

    // This will give empty infosets their proper number; the nonempty
    // ones will be renumbered by the next loop
    for (i = 1; i <= player->infosets.Length(); i++)
      if (player->infosets[i]->members.Length() == 0)
	player->infosets[i]->number = i;
      else
	player->infosets[i]->number = 0;
  
    for (i = 1; i <= nodes.Length(); i++)  {
      Node *n = nodes[i];
      if (n->GetPlayer() == player && n->GetInfoset()->number == 0)  {
	n->GetInfoset()->number = ++isets;
	player->infosets[isets] = n->GetInfoset();
      }
    }  
  }

  // Now, we sort the nodes within the infosets
  
  gList<Node *> nodes;
  Nodes(*this, nodes);

  for (pl = 0; pl <= players.Length(); pl++)  {
    EFPlayer *player = (pl) ? players[pl] : chance;

    for (int iset = 1; iset <= player->infosets.Length(); iset++)  {
      Infoset *s = player->infosets[iset];
      for (int i = 1, j = 1; i <= nodes.Length(); i++)  {
	if (nodes[i]->infoset == s)
	  s->members[j++] = nodes[i];
      }
    }
  }

  int nodeindex = 1;
  NumberNodes(root, nodeindex);
}
  
Infoset *Efg::CreateInfoset(int n, EFPlayer *p, int br)
{
  Infoset *s = (p->IsChance()) ? new ChanceInfoset(this, n, p, br) :
               new Infoset(this, n, p, br);
  p->infosets.Append(s);
  return s;
}

EFOutcome *Efg::CreateOutcomeByIndex(int index)
{
  NewOutcome(index);
  return outcomes[outcomes.Length()];
}

void Efg::CopySubtree(Node *n, Node *m)
{
  n->name = m->name;

  if (m->gameroot == m)
    n->gameroot = n;

  if (m->outcome)
    n->outcome = outcomes[m->outcome->GetNumber()];

  if (m->infoset)   {
    EFPlayer *p;
    if (m->infoset->player->number)
      p = players[m->infoset->player->number];
    else
      p = chance;

    Infoset *s = p->Infosets()[m->infoset->number];
    AppendNode(n, s);

    for (int i = 1; i <= n->children.Length(); i++)
      CopySubtree(n->children[i], m->children[i]);
  }
}

//------------------------------------------------------------------------
//               Efg: Title access and manipulation
//------------------------------------------------------------------------

void Efg::SetTitle(const gText &s)
{ title = s; }

const gText &Efg::GetTitle(void) const
{ return title; }

void Efg::SetComment(const gText &s)
{ comment = s; }

const gText &Efg::GetComment(void) const
{ return comment; }
  

//------------------------------------------------------------------------
//                    Efg: Writing data files
//------------------------------------------------------------------------

void Efg::WriteEfgFile(gOutput &f, Node *n) const
{
  if (n->children.Length() == 0)   {
    f << "t \"" << EscapeQuotes(n->name) << "\" ";
    if (n->outcome)  {
      f << n->outcome->GetNumber() << " \"" <<
	EscapeQuotes(n->outcome->GetName()) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->payoffs[pl];
	if (pl < NumPlayers())
	  f << ", ";
	else
	  f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  else if (n->infoset->player->number)   {
    f << "p \"" << EscapeQuotes(n->name) << "\" " <<
      n->infoset->player->number << ' ';
    f << n->infoset->number << " \"" <<
      EscapeQuotes(n->infoset->name) << "\" ";
    n->infoset->PrintActions(f);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->GetNumber() << " \"" <<
	EscapeQuotes(n->outcome->GetName()) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->payoffs[pl];
	if (pl < NumPlayers())
	  f << ", ";
	else
	  f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  else   {    // chance node
    f << "c \"" << n->name << "\" ";
    f << n->infoset->number << " \"" <<
      EscapeQuotes(n->infoset->name) << "\" ";
    n->infoset->PrintActions(f);
    f << " ";
    if (n->outcome)  {
      f << n->outcome->GetNumber() << " \"" <<
	EscapeQuotes(n->outcome->GetName()) << "\" ";
      f << "{ ";
      for (int pl = 1; pl <= NumPlayers(); pl++)  {
	f << n->outcome->payoffs[pl];
        if (pl < NumPlayers()) 
          f << ", ";
        else
          f << " }\n";
      }
    }
    else
      f << "0\n";
  }

  for (int i = 1; i <= n->children.Length(); i++)
    WriteEfgFile(f, n->children[i]);
}

void Efg::WriteEfgFile(gOutput &f) const
{
  f << "EFG 2 R";
  f << " \"" << EscapeQuotes(title) << "\" { ";
  for (int i = 1; i <= players.Length(); i++)
    f << '"' << EscapeQuotes(players[i]->name) << "\" ";
  f << "}\n";
  f << "\"" << EscapeQuotes(comment) << "\"\n\n";

  WriteEfgFile(f, root);
}


//------------------------------------------------------------------------
//                    Efg: General data access
//------------------------------------------------------------------------

int Efg::NumPlayers(void) const
{ return players.Length(); }

EFPlayer *Efg::NewPlayer(void)
{
  EFPlayer *ret = new EFPlayer(this, players.Length() + 1);
  players.Append(ret);

  for (int outc = 1; outc <= outcomes.Length();
       outcomes[outc++]->payoffs.Append(0));
  DeleteLexicon();
  return ret;
}

Infoset* Efg::GetInfosetByIndex(const int &pl, const int &iset) const
{
  return GetInfosetByIndex(Players()[pl],iset);
}

gBlock<Infoset *> Efg::Infosets() const
{
  gBlock<Infoset *> answer;

  gArray<EFPlayer *> p = Players();
  int i;
  for (i = 1; i <= p.Length(); i++) {
    gArray<Infoset *> infosets_for_player = p[i]->Infosets();
    int j;
    for (j = 1; j <= infosets_for_player.Length(); j++)
      answer += infosets_for_player[j];
  }

  return answer;
}

int Efg::NumOutcomes(void) const
{ return outcomes.Length(); }

EFOutcome *Efg::NewOutcome(void)
{
  NewOutcome(outcomes.Length() + 1);
  return outcomes[outcomes.Length()];
}

void Efg::DeleteOutcome(EFOutcome *outc)
{
  root->DeleteOutcome(outc);
  outcomes.Remove(outcomes.Find(outc));
  delete outc;
  DeleteLexicon();
}

void Efg::SetPayoff(EFOutcome *outc, int pl, const gNumber &value)
{
  outc->payoffs[pl] = value;
}

gNumber Efg::Payoff(EFOutcome *outc, int pl) const
{
  return outc->payoffs[pl];
}

gArray<gNumber> Efg::Payoff(EFOutcome *outc) const
{
  return outc->payoffs;
}

bool Efg::IsConstSum(void) const
{
  int pl, index;
  gNumber cvalue = (gNumber) 0;

  if (outcomes.Length() == 0)  return true;

  for (pl = 1; pl <= players.Length(); pl++)
    cvalue += outcomes[1]->payoffs[pl];

  for (index = 2; index <= outcomes.Length(); index++)  {
    gNumber thisvalue(0);

    for (pl = 1; pl <= players.Length(); pl++)
      thisvalue += outcomes[index]->payoffs[pl];

    if (thisvalue > cvalue || thisvalue < cvalue)
      return false;
  }

  return true;
}

gNumber Efg::MinPayoff(int pl) const
{
  int index, p, p1, p2;
  gNumber minpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=players.Length();}

  minpay = outcomes[1]->payoffs[p1];

  for (index = 1; index <= outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++)
      if (outcomes[index]->payoffs[p] < minpay)
	minpay = outcomes[index]->payoffs[p];
  }
  return minpay;
}

gNumber Efg::MaxPayoff(int pl) const
{
  int index, p, p1, p2;
  gNumber maxpay;

  if (NumOutcomes() == 0)  return 0;

  if(pl) { p1=p2=pl;}
  else {p1=1;p2=players.Length();}

  maxpay = outcomes[1]->payoffs[p1];

  for (index = 1; index <= outcomes.Length(); index++)  {
    for (p = p1; p <= p2; p++)
      if (outcomes[index]->payoffs[p] > maxpay)
	maxpay = outcomes[index]->payoffs[p];
  }
  return maxpay;
}

Node *Efg::RootNode(void) const
{ return root; }

bool Efg::IsSuccessor(const Node *n, const Node *from) const
{ return IsPredecessor(from, n); }

bool Efg::IsPredecessor(const Node *n, const Node *of) const
{
  while (of && n != of)    of = of->parent;

  return (n == of);
}

gList<Node*> Efg::Children(const Node& n) const
{
  gList<Node*> answer;

  int i;
  for (i = 1; i <= n.NumChildren(); i++)
    answer += n.GetChild(i);

  return answer;
}

void Efg::DescendantNodesRECURSION(const Node* n, 
				   const EFSupport& supp,
				   gList<const Node*>& current) const
{
  current += n;
  if (n->IsNonterminal()) {
    const gArray<Action *> actions = supp.Actions(n->GetInfoset());
    for (int i = 1; i <= actions.Length(); i++) {
      const Node* newn = n->GetChild(actions[i]);
      DescendantNodesRECURSION(newn,supp,current);
    }
  }
}

void Efg::NonterminalDescendantsRECURSION(const Node* n, 
					  const EFSupport& supp,
					  gList<const Node*>& current) const
{
  if (n->IsNonterminal()) {
    current += n;
    const gArray<Action *> actions = supp.Actions(n->GetInfoset());
    for (int i = 1; i <= actions.Length(); i++) {
      const Node* newn = n->GetChild(actions[i]);
      DescendantNodesRECURSION(newn,supp,current);
    }
  }
}

gList<const Node*> 
Efg::DescendantNodes(const Node& n, const EFSupport& supp) const
{
  gList<const Node*> answer;
  DescendantNodesRECURSION(&n,supp,answer);
  return answer;
}

gList<const Node*> 
Efg::NonterminalDescendants(const Node& n, const EFSupport& supp) const
{
  gList<const Node*> answer;
  NonterminalDescendantsRECURSION(&n,supp,answer);
  return answer;
}

gList<Infoset*> Efg::DescendantInfosets(const Node& n, 
					const EFSupport& supp) const
{
  gList<Infoset*> answer;
  gList<const Node*> nodelist = NonterminalDescendants(n,supp);
  int i;
  for (i = 1; i <= nodelist.Length(); i++) {
      Infoset* iset = nodelist[i]->GetInfoset();
      if (!answer.Contains(iset))
	answer += iset;
  }
  return answer;
}

EFOutcome *Efg::NewOutcome(int index)
{
  outcomes.Append(new EFOutcome(this, index));
  return outcomes[outcomes.Length()];
} 

//------------------------------------------------------------------------
//                     Efg: Operations on players
//------------------------------------------------------------------------

EFPlayer *Efg::GetChance(void) const
{
  return chance;
}

Infoset *Efg::AppendNode(Node *n, EFPlayer *p, int count)
{
  if (!n || !p || count == 0)
    throw Exception();

  if (n->children.Length() == 0)   {
    n->infoset = CreateInfoset(p->infosets.Length() + 1, p, count);
    n->infoset->members.Append(n);
    while (count--)
      n->children.Append(new Node(this, n));
  }

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}  

Infoset *Efg::AppendNode(Node *n, Infoset *s)
{
  if (!n || !s)   throw Exception();
  
  // Can't bridge subgames...
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;

  if (n->children.Length() == 0)   {
    n->infoset = s;
    s->members.Append(n);
    for (int i = 1; i <= s->actions.Length(); i++)
      n->children.Append(new Node(this, n));
  }

  DeleteLexicon();
  SortInfosets();
  return s;
}
  
Node *Efg::DeleteNode(Node *n, Node *keep)
{
  if (!n || !keep)   throw Exception();

  if (keep->parent != n)   return n;

  if (n->gameroot == n)
    MarkSubgame(keep, keep);

// turn infoset sorting off during tree deletion -- problems will occur
  sortisets = false;

  n->children.Remove(n->children.Find(keep));
  DeleteTree(n);
  keep->parent = n->parent;
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = keep;
  else
    root = keep;

  delete n;
  DeleteLexicon();

  sortisets = true;

  SortInfosets();

  return keep;
}

Infoset *Efg::InsertNode(Node *n, EFPlayer *p, int count)
{
  if (!n || !p || count <= 0)  throw Exception();

  Node *m = new Node(this, n->parent);
  m->infoset = CreateInfoset(p->infosets.Length() + 1, p, count);
  m->infoset->members.Append(m);
  if (n->parent)
    n->parent->children[n->parent->children.Find(n)] = m;
  else
    root = m;
  m->children.Append(n);
  n->parent = m;
  while (--count)
    m->children.Append(new Node(this, m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

Infoset *Efg::InsertNode(Node *n, Infoset *s)
{
  if (!n || !s)  throw Exception();

  // can't bridge subgames
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;
  
  Node *m = new Node(this, n->parent);
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
    m->children.Append(new Node(this, m));

  DeleteLexicon();
  SortInfosets();
  return m->infoset;
}

Infoset *Efg::CreateInfoset(EFPlayer *p, int br)
{
  if (!p || p->Game() != this)  throw Exception();
  return CreateInfoset(p->infosets.Length() + 1, p, br);
}

Infoset *Efg::JoinInfoset(Infoset *s, Node *n)
{
  if (!n || !s)  throw Exception();

  // can't bridge subgames
  if (s->members.Length() > 0 && n->gameroot != s->members[1]->gameroot)
    return 0;
  
  if (!n->infoset)   return 0; 
  if (n->infoset == s)   return s;
  if (s->actions.Length() != n->children.Length())  return n->infoset;

  Infoset *t = n->infoset;

  t->members.Remove(t->members.Find(n));
  s->members.Append(n);

  n->infoset = s;

  DeleteLexicon();
  SortInfosets();
  return s;
}

Infoset *Efg::LeaveInfoset(Node *n)
{
  if (!n)  throw Exception();

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

Infoset *Efg::SplitInfoset(Node *n)
{
  if (!n)  throw Exception();

  if (!n->infoset)   return 0;

  Infoset *s = n->infoset;
  if (s->members.Length() == 1)   return s;

  EFPlayer *p = s->player;
  Infoset *ns = CreateInfoset(p->infosets.Length() + 1, p,
			      n->children.Length());
  ns->name = s->name;
  int i;
  for (i = s->members.Length(); i > s->members.Find(n); i--)   {
    Node *nn = s->members.Remove(i);
    ns->members.Append(nn);
    nn->infoset = ns;
  }
  for (i = 1; i <= s->actions.Length(); i++)
    ns->actions[i]->name = s->actions[i]->name;

  DeleteLexicon();
  SortInfosets();
  return n->infoset;
}

Infoset *Efg::MergeInfoset(Infoset *to, Infoset *from)
{
  if (!to || !from)  throw Exception();

  if (to == from ||
      to->actions.Length() != from->actions.Length())   return from;

  if (to->members[1]->gameroot != from->members[1]->gameroot) 
    return from;

  to->members += from->members;
  for (int i = 1; i <= from->members.Length(); i++)
    from->members[i]->infoset = to;

  from->members.Flush();

  DeleteLexicon();
  SortInfosets();
  return to;
}

bool Efg::DeleteEmptyInfoset(Infoset *s)
{
  if (!s)  throw Exception();

  if (s->NumMembers() > 0)   return false;

  s->player->infosets.Remove(s->player->infosets.Find(s));
  delete s;

  return true;
}

Infoset *Efg::SwitchPlayer(Infoset *s, EFPlayer *p)
{
  if (!s || !p)  throw Exception();
  
  if (s->player == p)   return s;

  s->player->infosets.Remove(s->player->infosets.Find(s));
  s->player = p;
  p->infosets.Append(s);

  DeleteLexicon();
  SortInfosets();
  return s;
}

void Efg::CopySubtree(Node *src, Node *dest, Node *stop)
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

void Efg::MarkSubtree(Node *n)
{
  n->mark=true;
  for(int i=1;i<=n->children.Length();i++)
    MarkSubtree(n->children[i]);
}

void Efg::UnmarkSubtree(Node *n)
{
  n->mark=false;
  for(int i=1;i<=n->children.Length();i++)
    UnmarkSubtree(n->children[i]);
}

void Efg::Reveal(Infoset *where, const gArray<EFPlayer *> &who)
{
  int i,j,k,m;
  bool flag;
  gBlock<Node*> OldMembers;
  Node *n;
  Infoset *newiset = 0;

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

Node *Efg::CopyTree(Node *src, Node *dest)
{
  if (!src || !dest)  throw Exception();
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

Node *Efg::MoveTree(Node *src, Node *dest)
{
  if (!src || !dest)  throw Exception();
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

Node *Efg::DeleteTree(Node *n)
{
  if (!n)  throw Exception();

  while (n->NumChildren() > 0)   {
    DeleteTree(n->children[1]);
    delete n->children.Remove(1);
  }
  
  if (n->infoset)  {
    n->infoset->members.Remove(n->infoset->members.Find(n));
    n->infoset = 0;
  }
  n->outcome = 0;
  n->name = "";

  DeleteLexicon();
  SortInfosets();
  return n;
}

Action *Efg::InsertAction(Infoset *s)
{
  if (!s)  throw Exception();
  Action *action = s->InsertAction(s->NumActions() + 1);
  for (int i = 1; i <= s->members.Length(); i++)
    s->members[i]->children.Append(new Node(this, s->members[i]));
  DeleteLexicon();
  return action;
}

Action *Efg::InsertAction(Infoset *s, Action *a)
{
  if (!a || !s)  throw Exception();
  int where;
  for (where = 1; where <= s->actions.Length() && s->actions[where] != a;
       where++);
  if (where > s->actions.Length())   return 0;
  Action *action = s->InsertAction(where);
  for (int i = 1; i <= s->members.Length(); i++)
    s->members[i]->children.Insert(new Node(this, s->members[i]), where);

  DeleteLexicon();
  return action;
}

Infoset *Efg::DeleteAction(Infoset *s, Action *a)
{
  if (!a || !s)  throw Exception();
  int where;
  for (where = 1; where <= s->actions.Length() && s->actions[where] != a;
       where++);
  if (where > s->actions.Length() || s->actions.Length() == 1)   return s;
  s->RemoveAction(where);
  for (int i = 1; i <= s->members.Length(); i++)   {
    DeleteTree(s->members[i]->children[where]);
    delete s->members[i]->children.Remove(where);
  }
  DeleteLexicon();
  return s;
}

void Efg::SetChanceProb(Infoset *infoset, int act, const gNumber &value)
{
  if (infoset->IsChanceInfoset())
    ((ChanceInfoset *) infoset)->SetActionProb(act, value);
}

gNumber Efg::GetChanceProb(Infoset *infoset, int act) const
{
  if (infoset->IsChanceInfoset())
    return ((ChanceInfoset *) infoset)->GetActionProb(act);
  else
    return (gNumber) 0;
}

gNumber Efg::GetChanceProb(Action *a) const
{
  return GetChanceProbs(a->BelongsTo())[a->GetNumber()];
}

gArray<gNumber> Efg::GetChanceProbs(Infoset *infoset) const
{
  if (infoset->IsChanceInfoset())
    return ((ChanceInfoset *) infoset)->GetActionProbs();
  else
    return gArray<gNumber>(infoset->NumActions());
}

//---------------------------------------------------------------------
//                     Subgame-related functions
//---------------------------------------------------------------------

void Efg::MarkTree(Node *n, Node *base)
{
  n->ptr = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkTree(n->GetChild(i), base);
}

bool Efg::CheckTree(Node *n, Node *base)
{
  int i;

  if (n->NumChildren() == 0)   return true;

  for (i = 1; i <= n->NumChildren(); i++)
    if (!CheckTree(n->GetChild(i), base))  return false;

  if (n->GetPlayer()->IsChance())   return true;

  for (i = 1; i <= n->GetInfoset()->NumMembers(); i++)
    if (n->GetInfoset()->Members()[i]->ptr != base)
      return false;

  return true;
}

bool Efg::IsLegalSubgame(Node *n)
{
  if (n->NumChildren() == 0)  
    return false;

  MarkTree(n, n);
  return CheckTree(n, n);
}

bool Efg::DefineSubgame(Node *n)
{
  if (n->gameroot != n && IsLegalSubgame(n))  {
    n->gameroot = 0;
    MarkSubgame(n, n);
    return true;
  }

  return false;
}

void Efg::RemoveSubgame(Node *n)
{
  if (n->gameroot == n && n->parent)  {
    n->gameroot = 0;
    MarkSubgame(n, n->parent->gameroot);
  }
}
  

void Efg::MarkSubgame(Node *n, Node *base)
{
  if (n->gameroot == n)  return;
  n->gameroot = base;
  for (int i = 1; i <= n->NumChildren(); i++)
    MarkSubgame(n->GetChild(i), base);
}

void Efg::MarkSubgames(const gList<Node *> &list)
{
  for (int i = 1; i <= list.Length(); i++)  {
    if (IsLegalSubgame(list[i]))  {
      list[i]->gameroot = 0;
      MarkSubgame(list[i], list[i]);
    }
  }
}

void Efg::UnmarkSubgames(Node *n)
{
  if (n->NumChildren() == 0)   return;

  for (int i = 1; i <= n->NumChildren(); i++)
    UnmarkSubgames(n->GetChild(i));
  
  if (n->gameroot == n && n->parent)  {
    n->gameroot = 0;
    MarkSubgame(n, n->parent->gameroot);
  }
}


int Efg::ProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= players.Length(); i++)
    for (int j = 1; j <= players[i]->infosets.Length(); j++)
      sum += players[i]->infosets[j]->actions.Length();

  return sum;
}

gArray<int> Efg::NumInfosets(void) const
{
  gArray<int> foo(players.Length());
  
  for (int i = 1; i <= foo.Length(); i++)
    foo[i] = players[i]->infosets.Length();

  return foo;
}

int Efg::TotalNumInfosets(void) const
{
  int foo=0;
  
  for (int i = 1; i <= players.Length(); i++)
    foo += (NumInfosets())[i];

  return foo;
}

gPVector<int> Efg::NumActions(void) const
{
  gArray<int> foo(players.Length());
  int i;
  for (i = 1; i <= players.Length(); i++)
    foo[i] = players[i]->infosets.Length();

  gPVector<int> bar(foo);
  for (i = 1; i <= players.Length(); i++)
    for (int j = 1; j <= players[i]->infosets.Length(); j++)
      bar(i, j) = players[i]->infosets[j]->actions.Length();

  return bar;
}  

gPVector<int> Efg::NumMembers(void) const
{
  gArray<int> foo(players.Length());
  int i;
  for (i = 1; i <= players.Length(); i++)
    foo[i] = players[i]->infosets.Length();

  gPVector<int> bar(foo);
  for (i = 1; i <= players.Length(); i++)
    for (int j = 1; j <= players[i]->infosets.Length(); j++)
      bar(i, j) = players[i]->infosets[j]->members.Length();

  return bar;
}

//------------------------------------------------------------------------
//                   Efg<T>: Payoff computation
//------------------------------------------------------------------------

void Efg::Payoff(Node *n, gNumber prob, const gPVector<int> &profile,
		 gVector<gNumber> &payoff) const
{
  if (n->outcome)  {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->outcome->payoffs[i];
  }

  if (n->infoset && n->infoset->player->IsChance())
    for (int i = 1; i <= n->children.Length(); i++)
      Payoff(n->children[i],
	     prob * GetChanceProb(n->infoset, i),
	     profile, payoff);
  else if (n->infoset)
    Payoff(n->children[profile(n->infoset->player->number,n->infoset->number)],
	   prob, profile, payoff);
}

void Efg::InfosetProbs(Node *n, gNumber prob, const gPVector<int> &profile,
			  gPVector<gNumber> &probs) const
{
  if (n->infoset && n->infoset->player->IsChance())
    for (int i = 1; i <= n->children.Length(); i++)
      InfosetProbs(n->children[i],
		   prob * GetChanceProb(n->infoset, i),
		   profile, probs);
  else if (n->infoset)  {
    probs(n->infoset->player->number, n->infoset->number) += prob;
    InfosetProbs(n->children[profile(n->infoset->player->number,n->infoset->number)],
		 prob, profile, probs);
  }
}

void Efg::Payoff(const gPVector<int> &profile, gVector<gNumber> &payoff) const
{
  ((gVector<gNumber> &) payoff).operator=((gNumber) 0);
  Payoff(root, 1.0, profile, payoff);
}

void Efg::InfosetProbs(const gPVector<int> &profile,
			  gPVector<gNumber> &probs) const
{
  ((gVector<gNumber> &) probs).operator=((gNumber) 0);
  InfosetProbs(root, 1.0, profile, probs);
}

void Efg::Payoff(Node *n, gNumber prob, const gArray<gArray<int> *> &profile,
		    gArray<gNumber> &payoff) const
{
  if (n->outcome)   {
    for (int i = 1; i <= players.Length(); i++)
      payoff[i] += prob * n->outcome->payoffs[i];
  }
  
  if (n->infoset && n->infoset->player->IsChance())
    for (int i = 1; i <= n->children.Length(); i++)
      Payoff(n->children[i],
	     prob * GetChanceProb(n->infoset, i),
	     profile, payoff);
  else if (n->infoset)
    Payoff(n->children[(*profile[n->infoset->player->number])[n->infoset->number]],
	   prob, profile, payoff);
}

void Efg::Payoff(const gArray<gArray<int> *> &profile,
		 gArray<gNumber> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); i++)
    payoff[i] = 0;
  Payoff(root, 1.0, profile, payoff);
}

#include "efgiter.imp"
#include "efgciter.imp"

