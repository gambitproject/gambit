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
#include "tree-behav-mixed.h"

#include "table-game.h"

//======================================================================
//              Implementation of class gbtTreeGameRep
//======================================================================

//----------------------------------------------------------------------
//         class gbtTreeGameRep: Constructor and destructor
//----------------------------------------------------------------------

gbtTreeGameRep::gbtTreeGameRep(void)
  : m_refCount(0), m_sortInfosets(true), 
    m_hasStrategies(false), m_label("UNTITLED"),
    m_chance(new gbtTreePlayerRep(this, 0))
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


//----------------------------------------------------------------------
//                class gbtTreeGameRep: Game tree
//----------------------------------------------------------------------
  
gbtGameNode gbtTreeGameRep::GetRoot(void) const
{ return m_root; }

//----------------------------------------------------------------------
//               class gbtTreeGameRep: Game table
//----------------------------------------------------------------------

gbtNfgContingency gbtTreeGameRep::NewContingency(void) const
{
  if (!m_hasStrategies)   BuildReducedNfg();
  return new gbtNfgContingencyTree(const_cast<gbtTreeGameRep *>(this));
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
  OnStrategiesChanged();
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
//! made to the game to invalidate the computed reduced normal form
//! strategies.
//!
void gbtTreeGameRep::OnStrategiesChanged(void) 
{
  for (int pl = 1; pl <= m_players.Length(); pl++) {
    for (int st = 1; st <= m_players[pl]->m_strategies.Length(); st++) {
      m_players[pl]->m_strategies[st]->Delete();
    }
    m_players[pl]->m_strategies.Flush();
  }
  m_hasStrategies = false;
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

void gbtTreeGameRep::BuildReducedNfg(void) const
{
  if (m_hasStrategies)  return;
  
  for (int pl = 1; pl <= NumPlayers(); pl++) {
    MakeReducedStrats(m_players[pl], m_root, NULL);
  }

  m_hasStrategies = true;
}

//!
//! Recursively numbers nodes.  Nodes are numbered in prefix traversal
//! order, starting at 1 for the root node.
//!
void gbtTreeGameRep::NumberNodes(gbtTreeNodeRep *n, int &index)
{
  n->m_id = index++;
  for (int child = 1; child <= n->m_children.Length();
       NumberNodes(n->m_children[child++], index));
} 

//!
//! Sort the information sets for players.  Information sets are sorted
//! in the order first encountered in a traversal of the tree.
//!
void gbtTreeGameRep::SortInfosets(void)
{
  if (!m_sortInfosets)  return;

  int pl;

  for (pl = 0; pl <= m_players.Length(); pl++)  {
    gbtList<gbtGameNode> nodes;

    Nodes(gbtGame(this), nodes);

    gbtTreePlayerRep *player = (pl) ? m_players[pl] : m_chance;

    int i, isets = 0;

    // First, move all empty infosets to the back of the list so
    // we don't "lose" them
    int foo = player->m_infosets.Length();
    i = 1;
    while (i < foo)   {
      if (player->m_infosets[i]->m_members.Length() == 0)  {
	gbtTreeInfosetRep *bar = player->m_infosets[i];
	player->m_infosets[i] = player->m_infosets[foo];
	player->m_infosets[foo--] = bar;
      }
      else
	i++;
    }

    // This will give empty infosets their proper number; the nonempty
    // ones will be renumbered by the next loop
    for (i = 1; i <= player->m_infosets.Length(); i++)
      if (player->m_infosets[i]->m_members.Length() == 0)
	player->m_infosets[i]->m_id = i;
      else
	player->m_infosets[i]->m_id = 0;
  
    for (i = 1; i <= nodes.Length(); i++)  {
      gbtTreeNodeRep *n = dynamic_cast<gbtTreeNodeRep *>(nodes[i].Get());
      if (n->GetPlayer().Get() == player && n->GetInfoset()->GetId() == 0)  {
	n->m_infoset->m_id = ++isets;
	player->m_infosets[isets] = n->m_infoset;
      }
    }  
  }

  // Now, we sort the nodes within the infosets
  gbtList<gbtGameNode> nodes;
  Nodes(gbtGame(this), nodes);

  for (pl = 0; pl <= m_players.Length(); pl++)  {
    gbtTreePlayerRep *player = (pl) ? m_players[pl] : m_chance;

    for (int iset = 1; iset <= player->m_infosets.Length(); iset++)  {
      gbtTreeInfosetRep *s = player->m_infosets[iset];
      for (int i = 1, j = 1; i <= nodes.Length(); i++)  {
	gbtTreeNodeRep *n = dynamic_cast<gbtTreeNodeRep *>(nodes[i].Get());
	if (n->m_infoset == s) {
	  s->m_members[j++] = n;
	}
      }
    }
  }

  int nodeindex = 1;
  NumberNodes(m_root, nodeindex);
}

void gbtTreeGameRep::Payoff(gbtTreeNodeRep *n, const gbtRational &prob,
			    const gbtPVector<int> &profile,
			    gbtVector<gbtRational> &payoff) const
{
  if (n->m_outcome)  {
    for (int i = 1; i <= m_players.Length(); i++) {
      payoff[i] += prob * n->m_outcome->m_payoffs[i];
    }
  }

  if (n->m_infoset && n->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      Payoff(n->m_children[i],
	     prob * n->m_infoset->m_chanceProbs[i],
	     profile, payoff);
    }
  }
  else if (n->m_infoset) {
    Payoff(n->m_children[profile(n->m_infoset->m_player->m_id,
				 n->m_infoset->m_id)],
	   prob, profile, payoff);
  }
}

void gbtTreeGameRep::InfosetProbs(gbtTreeNodeRep *n, const gbtRational &prob,
				  const gbtPVector<int> &profile,
				  gbtPVector<gbtRational> &probs) const
{
  if (n->m_infoset && n->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      InfosetProbs(n->m_children[i],
		   prob * n->m_infoset->m_chanceProbs[i],
		   profile, probs);
    }
  }
  else if (n->m_infoset)  {
    probs(n->m_infoset->m_player->m_id, n->m_infoset->m_id) += prob;
    InfosetProbs(n->m_children[profile(n->m_infoset->m_player->m_id, 
				       n->m_infoset->m_id)],
		 prob, profile, probs);
  }
}

void gbtTreeGameRep::Payoff(const gbtPVector<int> &profile, 
			    gbtVector<gbtRational> &payoff) const
{
  ((gbtVector<gbtRational> &) payoff).operator=((gbtRational) 0);
  Payoff(m_root, 1, profile, payoff);
}

void gbtTreeGameRep::InfosetProbs(const gbtPVector<int> &profile,
				  gbtPVector<gbtRational> &probs) const
{
  ((gbtVector<gbtRational> &) probs).operator=((gbtRational) 0);
  InfosetProbs(m_root, 1, profile, probs);
}

void gbtTreeGameRep::Payoff(gbtTreeNodeRep *n, const gbtRational &prob,
			    const gbtArray<gbtArray<int> *> &profile,
			    gbtArray<gbtRational> &payoff) const
{
  if (n->m_outcome)   {
    for (int i = 1; i <= m_players.Length(); i++)
      payoff[i] += prob * n->m_outcome->m_payoffs[i];
  }
  
  if (n->m_infoset && n->m_infoset->m_player->m_id == 0) {
    for (int i = 1; i <= n->m_children.Length(); i++) {
      Payoff(n->m_children[i],
	     prob * n->m_infoset->m_chanceProbs[i],
	     profile, payoff);
    }
  }
  else if (n->m_infoset) {
    Payoff(n->m_children[(*profile[n->m_infoset->m_player->m_id])[n->m_infoset->m_id]],
	   prob, profile, payoff);
  }
}

void gbtTreeGameRep::Payoff(const gbtArray<gbtArray<int> *> &profile,
			    gbtArray<gbtRational> &payoff) const
{
  for (int i = 1; i <= payoff.Length(); payoff[i++] = 0);
  Payoff(m_root, 1, profile, payoff);
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
