//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of game for explicit game trees
//
// This file is part of Gambit
// Copyright (c) 2002, The Gambit Project
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include "base/base.h"

#include "game.h"

// Declarations of internal structures
#include "tree-game.h"
#include "tree-contingency.h"
#include "tree-strategy-mixed.h"
#include "tree-behav-pure.h"
#include "tree-behav-mixed.h"

//======================================================================
//              Implementation of class gbtTreeGameRep
//======================================================================

//----------------------------------------------------------------------
//         class gbtTreeGameRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeGameRep::gbtTreeGameRep(void)
  : m_refCount(0),
    m_hasComputed(false), m_label("UNTITLED"),
    m_numNodes(1), m_chance(new gbtTreePlayerRep(this, 0))
{
  m_root = new gbtTreeNodeRep(this, 0);
  m_root->m_id = 1;
}

gbtTreeGameRep::~gbtTreeGameRep()
{
  delete m_root;
  delete m_chance;

  for (int i = 1; i <= m_players.Length(); delete m_players[i++]);
  for (int i = 1; i <= m_outcomes.Length(); delete m_outcomes[i++]);
}

//----------------------------------------------------------------------
//      class gbtTreeGameRep: Mechanism for reference counting
//----------------------------------------------------------------------

void gbtTreeGameRep::Reference(void)
{
  m_refCount++;
}

bool gbtTreeGameRep::Dereference(void)
{
  return (--m_refCount == 0);
}

//----------------------------------------------------------------------
//            class gbtTreeGameRep: Titles and comments
//----------------------------------------------------------------------

void gbtTreeGameRep::SetLabel(const std::string &p_label)
{ m_label = p_label; }

std::string gbtTreeGameRep::GetLabel(void) const
{ return m_label; }

void gbtTreeGameRep::SetComment(const std::string &s)
{ m_comment = s; }

std::string gbtTreeGameRep::GetComment(void) const
{ return m_comment; }

//----------------------------------------------------------------------
//            class gbtTreeGameRep: General information
//----------------------------------------------------------------------

bool gbtTreeGameRep::IsConstSum(void) const
{
  int pl, index;
  gbtRational cvalue = (gbtRational) 0;

  if (m_outcomes.Last() == 0)  return true;

  for (pl = 1; pl <= m_players.Length(); pl++)
    cvalue += m_outcomes[1]->m_payoffs[pl];

  for (index = 2; index <= m_outcomes.Last(); index++)  {
    gbtRational thisvalue(0);

    for (pl = 1; pl <= m_players.Length(); pl++)
      thisvalue += m_outcomes[index]->m_payoffs[pl];

    if (thisvalue > cvalue || thisvalue < cvalue)
      return false;
  }

  return true;
}

//!
//! This auxiliary function does the actual checking for perfect recall.
//! If the game is not perfect recall, parameters s1 and s2 contain
//! a pair of information sets that violate the definition upon return.
//!
bool 
gbtTreeGameRep::IsPerfectRecall(gbtGameInfoset &s1, gbtGameInfoset &s2) const
{
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    gbtGamePlayer player = GetPlayer(pl);

    for (int i = 1; i <= player->NumInfosets(); i++) {
      gbtGameInfoset iset1 = player->GetInfoset(i);
      for (int j = 1; j <= player->NumInfosets(); j++) {
        gbtGameInfoset iset2 = player->GetInfoset(j);

        bool precedes = false;
        int action = 0;

        for (int m = 1; m <= iset2->NumMembers(); m++) {
          int n;
          for (n = 1; n <= iset1->NumMembers(); n++) {
            if (iset1->GetMember(n)->IsPredecessorOf(iset2->GetMember(m))
                && iset1->GetMember(n) != iset2->GetMember(m)) {
              precedes = true;
              for (int act = 1; act <= iset1->NumActions(); act++) {
                if (iset1->GetMember(n)->GetChild(act)->
                    IsPredecessorOf(iset2->GetMember(m))) {
                  if (action != 0 && action != act) {
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

          if (i == j && precedes) {
            s1 = iset1;
            s2 = iset2;
            return false;
          }

          if (n > iset1->NumMembers() && precedes) {
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

bool gbtTreeGameRep::IsPerfectRecall(void) const
{
  gbtGameInfoset s1, s2;
  return IsPerfectRecall(s1, s2);
}

gbtRational gbtTreeGameRep::GetMinPayoff(void) const
{
  if (NumOutcomes() == 0 || NumPlayers() == 0)  return 0;

  gbtRational minpay = m_outcomes[1]->m_payoffs[1];

  for (int index = 1; index <= m_outcomes.Last(); index++)  {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      if (m_outcomes[index]->m_payoffs[pl] < minpay) {
	minpay = m_outcomes[index]->m_payoffs[pl];
      }
    }
  }
  return minpay;
}

gbtRational gbtTreeGameRep::GetMaxPayoff(void) const
{
  if (NumOutcomes() == 0 || NumPlayers() == 0)  return 0;

  gbtRational maxpay = m_outcomes[1]->m_payoffs[1];

  for (int index = 1; index <= m_outcomes.Last(); index++)  {
    for (int pl = 1; pl <= NumPlayers(); pl++) {
      if (m_outcomes[index]->m_payoffs[pl] > maxpay) {
	maxpay = m_outcomes[index]->m_payoffs[pl];
      }
    }
  }
  return maxpay;
}

//!
//! Recursively numbers nodes.  Nodes are numbered in prefix traversal
//! order, starting at 1 for the root node.
//!
void gbtTreeGameRep::NumberNodes(gbtTreeNodeRep *n) 
{
  n->m_id = ++m_numNodes;
  for (int child = 1; child <= n->m_children.Length();
       NumberNodes(n->m_children[child++]));
} 

//!
//! The canonical representation of the tree numbers nodes and information
//! sets in the order they are first encountered in a traversal of the tree.
//!
void gbtTreeGameRep::Canonicalize(void)
{
  m_numNodes = 0;
  NumberNodes(m_root);

  for (int pl = 0; pl <= m_players.Length(); pl++)  {
    gbtTreePlayerRep *player = (pl) ? m_players[pl] : m_chance;
    
    // Sort nodes within information sets according to ID.
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      gbtTreeInfosetRep *infoset = player->m_infosets[iset];
      for (int i = 1; i < infoset->m_members.Length(); i++) {
	for (int j = 1; j < infoset->m_members.Length() - i - 1; j++) {
	  if (infoset->m_members[j+1]->m_id < infoset->m_members[j]->m_id) {
	    gbtTreeNodeRep *tmp = infoset->m_members[j];
	    infoset->m_members[j] = infoset->m_members[j+1];
	    infoset->m_members[j+1] = tmp;
	  }
	}
      }
    }

    // Sort information sets by the smallest ID among their members
    // Coded using a bubble sort for simplicity; large games might
    // find a quicksort worthwhile.
    for (int i = 1; i < player->m_infosets.Length(); i++) {
      for (int j = 1; j < player->m_infosets.Length() - i - 1; j++) {
	int a = ((player->m_infosets[j+1]->m_members.Length()) ?
		 player->m_infosets[j+1]->m_members[1]->m_id : 0);
	int b = ((player->m_infosets[j]->m_members.Length()) ?
		 player->m_infosets[j]->m_members[1]->m_id : 0);

	if (a < b || b == 0) {
	  gbtTreeInfosetRep *tmp = player->m_infosets[j];
	  player->m_infosets[j] = player->m_infosets[j+1];
	  player->m_infosets[j+1] = tmp;
	}
      }
    }

    // Reassign information set IDs
    for (int iset = 1; iset <= player->m_infosets.Length(); iset++) {
      player->m_infosets[iset]->m_id = iset;
    }
  }
}

//----------------------------------------------------------------------
//                class gbtTreeGameRep: Game tree
//----------------------------------------------------------------------
  
gbtGameNode gbtTreeGameRep::GetRoot(void) const
{ return m_root; }

int gbtTreeGameRep::NumNodes(void) const
{ return m_numNodes; }

gbtGameBehavContingency
gbtTreeGameRep::NewBehavContingency(void) const
{ return new gbtTreeBehavContingencyRep(const_cast<gbtTreeGameRep *>(this)); }

gbtGameBehavProfileIterator
gbtTreeGameRep::NewBehavProfileIterator(void) const
{ return new gbtTreeBehavProfileIteratorRep(const_cast<gbtTreeGameRep *>(this)); }

//----------------------------------------------------------------------
//               class gbtTreeGameRep: Game table
//----------------------------------------------------------------------

gbtGameContingency gbtTreeGameRep::NewContingency(void) const
{
  if (!m_hasComputed)   BuildComputedElements();
  return new gbtTreeContingencyRep(const_cast<gbtTreeGameRep *>(this));
}

gbtGameContingencyIterator
gbtTreeGameRep::NewContingencyIterator(void) const
{ return new gbtTreeContingencyIteratorRep(const_cast<gbtTreeGameRep *>(this)); }

gbtGameContingencyIterator
gbtTreeGameRep::NewContingencyIterator(const gbtGameStrategy &p_strategy) const
{
  if (p_strategy.IsNull())  throw gbtGameNullException();
  gbtTreeStrategyRep *strategy = 
    dynamic_cast<gbtTreeStrategyRep *>(p_strategy.Get());
  if (!strategy || strategy->m_player->m_efg != this) {
    throw gbtGameMismatchException();
  }

  return new gbtTreeContingencyIteratorRep(const_cast<gbtTreeGameRep *>(this), strategy); 
}

//----------------------------------------------------------------------
//                class gbtTreeGameRep: Players
//----------------------------------------------------------------------

int gbtTreeGameRep::NumPlayers(void) const
{ return m_players.Length(); }

gbtGamePlayer gbtTreeGameRep::GetChance(void) const
{ return m_chance; }

gbtGamePlayer gbtTreeGameRep::NewPlayer(void)
{
  gbtTreePlayerRep *ret = new gbtTreePlayerRep(this, m_players.Length() + 1);
  m_players.Append(ret);

  for (int outc = 1; outc <= m_outcomes.Last();
       m_outcomes[outc++]->m_payoffs.Append(0));
  ClearComputedElements();
  return ret;
}

gbtGamePlayer gbtTreeGameRep::GetPlayer(int p_player) const
{
  if (p_player == 0) {
    return m_chance;
  }
  else {
    return m_players[p_player];
  }
}

//----------------------------------------------------------------------
//                class gbtTreeGameRep: Outcomes
//----------------------------------------------------------------------

int gbtTreeGameRep::NumOutcomes(void) const
{ return m_outcomes.Last(); }

gbtGameOutcome gbtTreeGameRep::GetOutcome(int p_index) const
{ return m_outcomes[p_index]; }

gbtGameOutcome gbtTreeGameRep::NewOutcome(void)
{
  m_outcomes.Append(new gbtTreeOutcomeRep(this, m_outcomes.Length() + 1));
  return m_outcomes[m_outcomes.Last()];
}

//----------------------------------------------------------------------
//               class gbtTreeGameRep: Dimensions
//----------------------------------------------------------------------

int gbtTreeGameRep::BehaviorProfileLength(void) const
{
  int sum = 0;

  for (int i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++) {
      sum += m_players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return sum;
}

gbtArray<int> gbtTreeGameRep::NumInfosets(void) const
{
  gbtArray<int> foo(m_players.Length());
  
  for (int i = 1; i <= foo.Length(); i++) {
    foo[i] = m_players[i]->m_infosets.Length();
  }

  return foo;
}

gbtPVector<int> gbtTreeGameRep::NumActions(void) const
{
  gbtArray<int> foo(m_players.Length());
  for (int i = 1; i <= m_players.Length(); i++) {
    foo[i] = m_players[i]->m_infosets.Length();
  }

  gbtPVector<int> bar(foo);
  for (int i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->m_actions.Length();
    }
  }

  return bar;
}  

gbtPVector<int> gbtTreeGameRep::NumMembers(void) const
{
  gbtArray<int> foo(m_players.Length());

  for (int i = 1; i <= m_players.Length(); i++) {
    foo[i] = m_players[i]->m_infosets.Length();
  }

  gbtPVector<int> bar(foo);
  for (int i = 1; i <= m_players.Length(); i++) {
    for (int j = 1; j <= m_players[i]->m_infosets.Length(); j++) {
      bar(i, j) = m_players[i]->m_infosets[j]->m_members.Length();
    }
  }

  return bar;
}

int gbtTreeGameRep::StrategyProfileLength(void) const
{
  int len = 0;

  for (int pl = 1; pl <= m_players.Length(); pl++) {
    len += m_players[pl]->NumStrategies();
  }

  return len;
}

gbtArray<int> gbtTreeGameRep::NumStrategies(void) const
{
  gbtArray<int> foo(m_players.Length());
  
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    foo[pl] = m_players[pl]->NumStrategies();
  }

  return foo;
}

//----------------------------------------------------------------------
//    class gbtTreeGameRep: Creating strategy profiles on the game
//----------------------------------------------------------------------

gbtMixedProfile<double> gbtTreeGameRep::NewMixedProfile(double) const
{ return new gbtTreeMixedProfileRep<double>(const_cast<gbtTreeGameRep *>(this)); }

gbtMixedProfile<gbtRational> 
gbtTreeGameRep::NewMixedProfile(const gbtRational &) const
{ return new gbtTreeMixedProfileRep<gbtRational>(const_cast<gbtTreeGameRep *>(this)); }

gbtBehavProfile<double> gbtTreeGameRep::NewBehavProfile(double) const
{ return new gbtTreeBehavProfileRep<double>(const_cast<gbtTreeGameRep *>(this)); }

gbtBehavProfile<gbtRational>
gbtTreeGameRep::NewBehavProfile(const gbtRational &) const
{ return new gbtTreeBehavProfileRep<gbtRational>(const_cast<gbtTreeGameRep *>(this)); }

//----------------------------------------------------------------------
//              class gbtTreeGameRep: Writing data files
//----------------------------------------------------------------------

//!
//! This helper function writes out the text entry corresponding to an 
//! outcome to the savefile.
//!
static void WriteEfgOutcome(std::ostream &f, const gbtGameOutcome &p_outcome)
{
  if (!p_outcome.IsNull())  {
    f << p_outcome->GetId() << " \"" <<
      EscapeQuotes(p_outcome->GetLabel()) << "\" ";
    f << "{ ";
    gbtArray<gbtRational> payoffs(p_outcome->GetPayoff());
    for (int pl = 1; pl <= payoffs.Length(); pl++)  {
      f << payoffs[pl];
      if (pl < payoffs.Length()) {
	f << ", ";
      }
      else {
	f << " }\n";
      }
    }
  }
  else {
    f << "0\n";
  }
}

//!
//! This helper function writes out the entry for the actions at an
//! information set to the savefile, including chance probabilities if
//! the information set belongs to the chance player.
//!  
static void WriteEfgActions(std::ostream &f, const gbtGameInfoset &p_infoset)
{ 
  f << "{ ";
  for (int i = 1; i <= p_infoset->NumActions(); i++) {
    f << '"' << EscapeQuotes(p_infoset->GetAction(i)->GetLabel()) << "\" ";
    if (p_infoset->GetPlayer()->IsChance()) {
      f << p_infoset->GetAction(i)->GetChanceProb() << ' ';
    }
  }
  f << "}";
}

//!
//! This helper function recursively writes out the nodes in the tree.
//! Savefiles store nodes in prefix-traversal order: at every node,
//! the entry for the node is written, then the function is called
//! recursively on the children.
//!
static void WriteEfg(std::ostream &f, const gbtGameNode &p_node)
{
  if (p_node->NumChildren() == 0)   {
    f << "t \"" << EscapeQuotes(p_node->GetLabel()) << "\" ";
    WriteEfgOutcome(f, p_node->GetOutcome());
  }

  else if (!p_node->GetPlayer()->IsChance()) {
    f << "p \"" << EscapeQuotes(p_node->GetLabel()) << "\" " <<
      p_node->GetPlayer()->GetId() << ' ';
    f << p_node->GetInfoset()->GetId() << " \"" <<
      EscapeQuotes(p_node->GetInfoset()->GetLabel()) << "\" ";
    WriteEfgActions(f, p_node->GetInfoset());
    f << " ";
    WriteEfgOutcome(f, p_node->GetOutcome());
  }

  else   {    // chance node
    f << "c \"" << EscapeQuotes(p_node->GetLabel()) << "\" ";
    f << p_node->GetInfoset()->GetId() << " \"" <<
      EscapeQuotes(p_node->GetInfoset()->GetLabel()) << "\" ";
    WriteEfgActions(f, p_node->GetInfoset());
    f << " ";
    WriteEfgOutcome(f, p_node->GetOutcome());
  }

  for (int i = 1; i <= p_node->NumChildren(); i++) {
    WriteEfg(f, p_node->GetChild(i));
  }
}

void gbtTreeGameRep::WriteEfg(std::ostream &p_file) const
{
  p_file << "EFG 2 R";
  p_file << " \"" << EscapeQuotes(m_label) << "\" { ";
  for (int i = 1; i <= m_players.Length(); i++) {
    p_file << '"' << EscapeQuotes(m_players[i]->m_label) << "\" ";
  }
  p_file << "}\n";
  p_file << "\"" << EscapeQuotes(m_comment) << "\"\n\n";

  ::WriteEfg(p_file, m_root);
}


//----------------------------------------------------------------------
//              class gbtTreeGameRep: Internal members
//----------------------------------------------------------------------

//!
//! This takes care of the internal bookkeeping when a change is
//! made to the game to invalidate the computed sequences and
//! reduced normal form strategies.
//!
void gbtTreeGameRep::ClearComputedElements(void) const
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); st++) {
      m_players[pl]->m_strategies[st]->Delete();
    }
    m_players[pl]->m_strategies.Flush();

    for (int seq = 1; seq <= m_players[pl]->m_sequences.Length(); seq++) {
      m_players[pl]->m_sequences[seq]->Delete();
    }
    m_players[pl]->m_sequences.Flush();
  }

  // Clear sequences of chance player as well
  for (int seq = 1; seq <= m_chance->m_sequences.Length(); seq++) {
    m_chance->m_sequences[seq]->Delete();
  }
  m_chance->m_sequences.Flush();

  m_hasComputed = false;
}

static void MakeStrategy(gbtTreePlayerRep *p_player)
{
  gbtTreeStrategyRep *strategy = 
    new gbtTreeStrategyRep(p_player, p_player->m_strategies.Length() + 1);
  strategy->m_behav = gbtArray<int>(p_player->NumInfosets());

  // FIXME: This is a rather lame labeling scheme.
  for (int iset = 1; iset <= p_player->NumInfosets(); iset++)  {
    if (p_player->m_infosets[iset]->m_flag) {
      strategy->m_behav[iset] = p_player->m_infosets[iset]->m_whichbranch;
      strategy->m_label += ToText(strategy->m_behav[iset]);
    }
    else {
      strategy->m_behav[iset] = 0;
      strategy->m_label += "*";
    }
  }

  p_player->m_strategies.Append(strategy);
}


static void 
MakeReducedStrats(gbtTreePlayerRep *p, gbtTreeNodeRep *n, gbtTreeNodeRep *nn)
{
  if (!n->m_parent)  n->m_ptr = 0;

  if (n->m_children.Length() > 0)  {
    if (n->m_infoset->m_player == p)  {
      if (!n->m_infoset->m_flag)  {
	// we haven't visited this infoset before
	n->m_infoset->m_flag = true;
	for (int i = 1; i <= n->m_children.Length(); i++)   {
	  gbtTreeNodeRep *m = n->m_children[i];
	  n->m_whichbranch = m;
	  n->m_infoset->m_whichbranch = i;
	  MakeReducedStrats(p, m, nn);
	}
	n->m_infoset->m_flag = false;
      }
      else  {
	// we have visited this infoset, take same action
	MakeReducedStrats(p, n->m_children[n->m_infoset->m_whichbranch], nn);
      }
    }
    else  {
      n->m_ptr = NULL;
      if (nn != NULL) {
	n->m_ptr = nn->m_parent;
      }
      n->m_whichbranch = n->m_children[1];
      if (n->m_infoset) { 
	n->m_infoset->m_whichbranch = 0;
      }
      MakeReducedStrats(p, n->m_children[1], n->m_children[1]);
    }
  }
  else if (nn)  {
    gbtTreeNodeRep *m;
    for (; ; nn = nn->m_parent->m_ptr->m_whichbranch)  {
      m = dynamic_cast<gbtTreeNodeRep *>(nn->GetNextSibling().Get());
      if (m || nn->m_parent->m_ptr == NULL)   break;
    }
    if (m)  {
      gbtTreeNodeRep *mm = m->m_parent->m_whichbranch;
      m->m_parent->m_whichbranch = m;
      MakeReducedStrats(p, m, m);
      m->m_parent->m_whichbranch = mm;
    }
    else {
      MakeStrategy(p);
    }
  }
  else {
    MakeStrategy(p);
  }
}

static void
MakeSequences(gbtTreePlayerRep *p, gbtTreeNodeRep *n, 
	      gbtArray<int> &visited, gbtTreeSequenceRep *parent)
{
  if (n->m_infoset && n->m_infoset->m_player == p &&
      !visited[n->m_infoset->m_id]) {
    // create new sequences on first visit to information set
    visited[n->m_infoset->m_id] = 1;
    for (int i = 1; i <= n->m_children.Length(); i++) {
      gbtTreeSequenceRep *seq =
	new gbtTreeSequenceRep(p, n->m_infoset->m_actions[i], parent, 
			       p->m_sequences.Length() + 1);
      MakeSequences(p, n->m_children[i], visited, seq);
    }
  }
  else {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      MakeSequences(p, n->m_children[i], visited, parent);
    }
  }
}

//!
//! This computes the sequences and reduced normal form strategies
//! generated by the game tree, if not already computed since the
//! last edit of the game.
//!
void gbtTreeGameRep::BuildComputedElements(void) const
{
  if (m_hasComputed)  return;
    
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    MakeReducedStrats(m_players[pl], m_root, NULL);
    
    gbtArray<int> visited(m_players[pl]->m_infosets.Length());
    for (int i = 1; i <= visited.Length(); visited[i++] = 0);
    gbtTreeSequenceRep *nullSeq = new gbtTreeSequenceRep(m_players[pl],
							 0, 0, 1);
    m_players[pl]->m_sequences.Append(nullSeq);
    MakeSequences(m_players[pl], m_root, visited, nullSeq);
  }

  // Build sequences for chance player as well
  gbtArray<int> visited(m_chance->m_infosets.Length());
  for (int i = 1; i <= visited.Length(); visited[i++] = 0);
  gbtTreeSequenceRep *nullSeq = new gbtTreeSequenceRep(m_chance,
						       0, 0, 1);
  m_chance->m_sequences.Append(nullSeq);
  MakeSequences(m_chance, m_root, visited, nullSeq);

  m_hasComputed = true;
}

gbtRational 
gbtTreeGameRep::GetPayoff(gbtTreePlayerRep *player,
			  gbtTreeNodeRep *n, 
			  const gbtArray<gbtTreeStrategyRep *> &profile) const
{
  gbtRational payoff(0);
  if (n->m_outcome) {
    payoff += n->m_outcome->m_payoffs[player->m_id];
  }
  
  if (n->m_infoset && n->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      payoff += (n->m_infoset->m_chanceProbs[i] * 
		 GetPayoff(player, n->m_children[i], profile));
    }
  }
  else if (n->m_infoset) {
    payoff = GetPayoff(player,
		       n->m_children[profile[n->m_infoset->m_player->m_id]->m_behav[n->m_infoset->m_id]],
		       profile);
  }
  return payoff;
}

gbtRational
gbtTreeGameRep::GetPayoff(gbtTreePlayerRep *player,
			  const gbtArray<gbtTreeStrategyRep *> &profile) const
{
  return GetPayoff(player, m_root, profile);
}

//-------------------------------------------------------------------------
//                           Global functions 
//-------------------------------------------------------------------------

//
// Returns a new trivial extensive form game
//
gbtGame NewEfg(void)
{
  return gbtGame(new gbtTreeGameRep);
}
