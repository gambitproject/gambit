//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of behavior strategy solution class
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

#include "math/gmath.h"
#include "behavsol.h"
#include "behavextend.h"

// we probably want to break this out into another file (rdm)

#include "base/gnullstatus.h"
#include "nash/efgsubsolve.h"

class SubgamePerfectChecker : public gbtEfgNashSubgames  {
private:
  int subgame_number;
  gbtNumber eps;
  gbtTriState isSubgamePerfect;
  gbtPVector<int> infoset_subgames;
  gbtBehavProfile<gbtNumber> start;
  gbtList<gbtEfgNode> oldroots;
  
  void SolveSubgame(const gbtEfgGame &, const gbtEfgSupport &,
		    gbtList<BehavSolution> &, gbtStatus &);
  
public:
  SubgamePerfectChecker(const gbtEfgGame &,
			const gbtBehavProfile<gbtNumber> &,
			const gbtNumber & epsilon);
  virtual ~SubgamePerfectChecker();
  gbtTriState IsSubgamePerfect(void) {return isSubgamePerfect;}
};

//----------------------------------------------------
// Constructors, Destructor, Constructive Operators
//----------------------------------------------------

BehavSolution::BehavSolution(const gbtBehavProfile<double> &p_profile,
			     const gbtText &p_creator)
  : m_profile(new gbtBehavProfile<gbtNumber>(gbtEfgSupport(p_profile.GetGame()))),
    m_precision(GBT_PREC_DOUBLE),
    m_support(p_profile.Support()), m_creator(p_creator),
    m_ANFNash(), m_Nash(), m_SubgamePerfect(), m_Sequential(), 
    m_epsilon(0.0), m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(), m_rnfRegret(), 
    m_revision(p_profile.GetGame().RevisionNumber())
{
  gEpsilon(m_epsilon);

  for (gbtEfgPlayerIterator player(GetGame()); !player.End(); player++) {
    for (gbtEfgInfosetIterator infoset(*player); !infoset.End(); infoset++) {
      for (int act = 1; act <= (*infoset).NumActions(); act++) {
	int index = p_profile.Support().GetIndex((*infoset).GetAction(act));
	if (index > 0)
	  (*m_profile)((*player).GetId(), (*infoset).GetId(), act) =
	    p_profile((*player).GetId(), (*infoset).GetId(), index);
	else
	  (*m_profile)((*player).GetId(), (*infoset).GetId(), act) =
	    gbtNumber(0.0);
      }
    }
  }
}


BehavSolution::BehavSolution(const gbtBehavProfile<gbtRational> &p_profile,
			     const gbtText &p_creator)
  : m_profile(new gbtBehavProfile<gbtNumber>(gbtEfgSupport(p_profile.GetGame()))),
    m_precision(GBT_PREC_RATIONAL), 
    m_support(p_profile.Support()), m_creator(p_creator),
    m_ANFNash(), m_Nash(), m_SubgamePerfect(), m_Sequential(), 
    m_qreLambda(-1), m_qreValue(-1), m_liapValue(), m_rnfRegret(), 
    m_revision(p_profile.GetGame().RevisionNumber())
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = GetGame().GetPlayer(pl);
    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      for (int act = 1; act <= infoset.NumActions(); act++) {
	int index = p_profile.Support().GetIndex(infoset.GetAction(act));
	if (index > 0)
	  (*m_profile)(pl, iset, act) = p_profile(pl, iset, index);
	else
	  (*m_profile)(pl, iset, act) = gbtNumber(0);
      }
    }
  }
}

BehavSolution::BehavSolution(const gbtBehavProfile<gbtNumber> &p_profile, 
			     const gbtText &p_creator)
  : m_profile(new gbtBehavProfile<gbtNumber>(gbtEfgSupport(p_profile.GetGame()))),
    m_precision(GBT_PREC_RATIONAL),
    m_support(p_profile.Support()), m_creator(p_creator),
    m_ANFNash(), m_Nash(), m_SubgamePerfect(), m_Sequential(), 
    m_qreLambda(-1), m_qreValue(-1), m_liapValue(), m_rnfRegret(), 
    m_revision(p_profile.GetGame().RevisionNumber())
{
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = GetGame().GetPlayer(pl);
    for (int iset = 1; iset <= player.NumInfosets(); iset++) {
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      for (int act = 1; act <= infoset.NumActions(); act++) {
	int index = p_profile.Support().GetIndex(infoset.GetAction(act));
	if (index > 0)
	  (*m_profile)(pl, iset, act) = p_profile(pl, iset, index);
	else
	  (*m_profile)(pl, iset, act) = gbtNumber(0);
      }
    }
  }
  LevelPrecision();

  m_epsilon = 0;
  if((*m_profile).Length()>0) 
    if ((*m_profile)[1].Precision() == GBT_PREC_DOUBLE)
      m_epsilon = 0.0;

  gEpsilon(m_epsilon);
}

BehavSolution::BehavSolution(const BehavSolution &p_solution)
  : m_profile(new gbtBehavProfile<gbtNumber>(*p_solution.m_profile)),
    m_precision(p_solution.m_precision), 
    m_support(p_solution.m_support), m_creator(p_solution.m_creator),
    m_ANFNash(p_solution.m_ANFNash),
    m_Nash(p_solution.m_Nash),
    m_SubgamePerfect(p_solution.m_SubgamePerfect),
    m_Sequential(p_solution.m_Sequential),
    m_epsilon(p_solution.m_epsilon),
    m_qreLambda(p_solution.m_qreLambda),
    m_qreValue(p_solution.m_qreValue),
    m_liapValue(p_solution.m_liapValue),
    m_rnfRegret(p_solution.m_rnfRegret), 
    m_label(p_solution.m_label),
    m_revision(p_solution.m_revision)
{ }

BehavSolution::~BehavSolution() 
{ 
  delete m_profile;
}

BehavSolution& BehavSolution::operator=(const BehavSolution &p_solution)
{
  if (this != &p_solution)   {
    delete m_profile;
    m_profile = new gbtBehavProfile<gbtNumber>(*p_solution.m_profile);
    m_precision = p_solution.m_precision;
    m_support = p_solution.m_support;
    m_creator = p_solution.m_creator;
    m_ANFNash = p_solution.m_ANFNash;
    m_Nash = p_solution.m_Nash;
    m_SubgamePerfect = p_solution.m_SubgamePerfect;
    m_Sequential = p_solution.m_Sequential;
    m_epsilon = p_solution.m_epsilon;
    m_qreLambda = p_solution.m_qreLambda;
    m_qreValue = p_solution.m_qreValue;
    m_liapValue = p_solution.m_liapValue;
    m_rnfRegret = p_solution.m_rnfRegret;
    m_label = p_solution.m_label;
    m_revision = p_solution.m_revision;
  }

  return *this;
}

//-----------------------------
// Private member functions
//-----------------------------

gbtTriState BehavSolution::GetANFNash(void) const
{
  gbtNullStatus status;
  algExtendsToAgentNash algorithm;
  gbtTriState answer = ((algorithm.ExtendsToAgentNash(*this,
						    Support(), Support(),
						    status)) ?
		      GBT_TRISTATE_TRUE : GBT_TRISTATE_FALSE);
  if (answer == GBT_TRISTATE_FALSE) {
    m_Nash.Set(GBT_TRISTATE_FALSE);
    m_SubgamePerfect.Set(GBT_TRISTATE_FALSE);
    m_Sequential.Set(GBT_TRISTATE_FALSE);
  }
  return answer;
  return GBT_TRISTATE_UNKNOWN;
}

gbtTriState BehavSolution::GetNash(void) const
{
  gbtTriState answer = GBT_TRISTATE_UNKNOWN;
  bool decomposes = HasSubgames(GetGame());
  // check subgame perfection if game decomposes (its faster)
  if(decomposes) 
    GetSubgamePerfect();
  // if it was settled in that call, ...
  if (m_Nash.Checked() == true) 
    answer = m_Nash.Answer();
  else {
    // use reduced normal form regrets for complete profiles
    if (IsComplete()) {
      answer = (MaxRNFRegret() <= m_epsilon) ? GBT_TRISTATE_TRUE:GBT_TRISTATE_FALSE;
    }
    else {
      //  else let Andy figure it out
      // Is perfect recall needed here, Andy?
      if (m_profile->GetGame().IsPerfectRecall()) { 
	// not sure MaxRegret does the right thing here
	gbtNullStatus status;
	algExtendsToNash algorithm;
	answer = (m_profile->MaxRegret() <= m_epsilon  &&
		  algorithm.ExtendsToNash(*this,Support(),Support(),status)) 
	  ? GBT_TRISTATE_TRUE:GBT_TRISTATE_FALSE;
      }
      else {
	answer = GBT_TRISTATE_UNKNOWN;
      }
    }
  }
  // Done.  Now mark other obvious inferences 
  
  if (answer == GBT_TRISTATE_FALSE) {
    m_SubgamePerfect.Set(GBT_TRISTATE_FALSE);
    m_Sequential.Set(GBT_TRISTATE_FALSE);
  }
  if (answer == GBT_TRISTATE_TRUE) {
    m_ANFNash.Set(GBT_TRISTATE_TRUE);
    if(!decomposes) 
      m_SubgamePerfect.Set(GBT_TRISTATE_TRUE);
  }
  return answer;
}

gbtTriState BehavSolution::GetSubgamePerfect(void) const
{
  gbtTriState answer;
  // Note -- HasSubgames should be cached in Efg
  bool decomposes = HasSubgames(GetGame());
  if(!decomposes) GetNash();
  // if it was already resolved ...
  if(m_SubgamePerfect.Checked() == true) 
    answer = m_SubgamePerfect.Answer();
  else {
    // for complete profiles, use subgame perfect checker.  
    if (IsComplete()) {
      gbtBehavProfile<gbtNumber> p(*m_profile);
      SubgamePerfectChecker checker(p.GetGame(),p, Epsilon());
      gbtNullStatus status;
      checker.Solve(p.Support(), status);
      answer = checker.IsSubgamePerfect();
    }
    // else, for now, we require complete profiles for subgame perfection.  
    // but we may want to turn over to Andy here. 
    else {
      answer = GBT_TRISTATE_UNKNOWN;
    }
  }
  // Done.  Now mark other obvious inferences 
  
  if (answer == GBT_TRISTATE_TRUE) {
    m_Nash.Set(GBT_TRISTATE_TRUE);
    m_ANFNash.Set(GBT_TRISTATE_TRUE);
  }
  else if (answer == GBT_TRISTATE_FALSE) {
    m_Sequential.Set(GBT_TRISTATE_FALSE);
    if (!decomposes) {
      m_Nash.Set(GBT_TRISTATE_FALSE);
    }
  }
  return answer;
}

gbtTriState BehavSolution::GetSequential(void) const
{
  if(IsSubgamePerfect()==GBT_TRISTATE_TRUE) {
    // Liap and QRE should be returning Nash solutions that give positive 
    // probability to all actions, and hence will be approximations to 
    // sequential equilibria.  But we should add code to check up on these 
    // algorithms
    if (GetCreator() == "Liap[EFG]" || GetCreator() == "Qre[EFG]")
      return GBT_TRISTATE_TRUE;
    else {
      // check if game is perfect info
      // this should be in efg.h
      bool flag = true;
      gbtPVector<int> v((GetGame()).NumMembers());
      for(int i=v.First();flag == true && i<=v.Last();i++)
	if(v[i]>1) flag = false;
      if(flag==true) return GBT_TRISTATE_TRUE;
    }
    return GBT_TRISTATE_UNKNOWN;
  }
  else 
    return IsSubgamePerfect();
}

//
// This function no longer inlined to satisfy a Borland C++ warning
//
gbtNumber BehavSolution::GetNodeValue(const gbtEfgNode &p_node,
				    const gbtEfgPlayer &p_player) const
{ 
  return m_profile->GetNodeValue(p_node)[p_player.GetId()];
}

void BehavSolution::LevelPrecision(void)
{
  m_precision = GBT_PREC_RATIONAL;
  for (int pl = 1; m_precision == GBT_PREC_RATIONAL && pl <= GetGame().NumPlayers();
       pl++) {
    gbtEfgPlayer player = GetGame().GetPlayer(pl);
    for (int iset = 1; (m_precision == GBT_PREC_RATIONAL && 
			iset <= player.NumInfosets()); iset++) {
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      for (int act = 1; (m_precision == GBT_PREC_RATIONAL && 
			 act <= infoset.NumActions()); act++) {
	if ((*m_profile)(pl, iset, act).Precision() == GBT_PREC_DOUBLE)
	  m_precision = GBT_PREC_DOUBLE;
      }
    }
  }

  if (m_precision == GBT_PREC_DOUBLE) {
    for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
      gbtEfgPlayer player = GetGame().GetPlayer(pl);
      for (int iset = 1; iset <= player.NumInfosets(); iset++) {
	gbtEfgInfoset infoset = player.GetInfoset(iset);
	for (int act = 1; act <= infoset.NumActions(); act++) {
	  (*m_profile)(pl, iset, act) = (double) (*m_profile)(pl, iset, act);
	}
      }
    }
  }
}

//------------------------
// Operator overloading
//------------------------

bool BehavSolution::Equals(const gbtBehavProfile<double> &p_profile) const
{  
  gbtNumber eps(m_epsilon);
  gEpsilon(eps, 4);  // this should be a function of m_epsilon

  int i = p_profile.First();
  while (i <= p_profile.Length()) {
    if (abs((*m_profile)[i] - (gbtNumber) p_profile[i]) > eps) 
      break;
    i++;
  }
  return (i > p_profile.Length());
}

bool BehavSolution::operator==(const BehavSolution &p_solution) const
{ return (*m_profile == *p_solution.m_profile); }

void BehavSolution::SetActionProb(const gbtEfgAction &p_action,
				  const gbtNumber &p_prob)
{
  Invalidate();

  gbtEfgInfoset infoset = p_action.GetInfoset();
  gbtEfgPlayer player = infoset.GetPlayer();
  (*m_profile)(player.GetId(), infoset.GetId(), p_action.GetId()) = p_prob;
  if (m_precision != p_prob.Precision())
    LevelPrecision();
}

void BehavSolution::Set(int p_player, int p_infoset, int p_action,
			const gbtNumber &p_prob)
{
  Invalidate();

  (*m_profile)(p_player, p_infoset, p_action) = p_prob;
  if (m_precision != p_prob.Precision())
    LevelPrecision();
}

const gbtNumber &BehavSolution::operator()(const gbtEfgAction &p_action) const
{
  gbtEfgInfoset infoset = p_action.GetInfoset();
  gbtEfgPlayer player = infoset.GetPlayer();
  return (*m_profile)(player.GetId(), infoset.GetId(),
		      p_action.GetId());
}

gbtNumber BehavSolution::operator[](const gbtEfgAction &p_action) const
{
  return m_profile->GetActionProb(p_action);
}

gbtNumber &BehavSolution::operator[](const gbtEfgAction &p_action)
{
  return (*m_profile)(p_action.GetInfoset().GetPlayer().GetId(),
		      p_action.GetInfoset().GetId(),
		      p_action.GetId());
}

BehavSolution &BehavSolution::operator+=(const BehavSolution &p_solution)
{
  Invalidate(); 
  *m_profile += *p_solution.m_profile;
  if (m_precision == GBT_PREC_RATIONAL && p_solution.m_precision == GBT_PREC_DOUBLE)
    m_precision = GBT_PREC_DOUBLE;
  return *this;
}

BehavSolution &BehavSolution::operator-=(const BehavSolution &p_solution)
{
  Invalidate();
  *m_profile -= *p_solution.m_profile;
  if (m_precision == GBT_PREC_RATIONAL && p_solution.m_precision == GBT_PREC_DOUBLE)
    m_precision = GBT_PREC_DOUBLE;
  return *this;
}

BehavSolution &BehavSolution::operator*=(const gbtNumber &p_constant)
{ 
  Invalidate();
  *m_profile *= p_constant;
  if (m_precision == GBT_PREC_RATIONAL && p_constant.Precision() == GBT_PREC_DOUBLE)
    m_precision = GBT_PREC_DOUBLE;
  return *this;
}

//-----------------------
// General data access
//-----------------------

bool BehavSolution::IsComplete(void) const
{ 
  gbtNumber sum;
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    gbtEfgPlayer player = GetGame().GetPlayer(pl);
    for (int iset = 1; iset <= player.NumInfosets(); iset++) { 
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      sum = -1;
      for (int act = 1; act <= infoset.NumActions(); act++) 
	sum += (*m_profile)(pl, iset, act);
      if (sum > m_epsilon || sum < -m_epsilon) 
	return false;
    }
  }
  return true;
}

const gbtTriState &BehavSolution::IsNash(void) const
{
  CheckIsValid();
  if (!m_Nash.Checked()) {
    m_Nash.Set(GetNash());
  }
  return m_Nash.Answer();
}

BehavSolution BehavSolution::PolishEq(void) const
{
#ifdef UNUSED
  bool is_singular = false;
  return PolishEquilibrium(m_support,*this,is_singular);
#endif // UNUSED
  return *this;
}

const gbtTriState &BehavSolution::IsANFNash(void) const
{
  CheckIsValid();
  if(!m_ANFNash.Checked())
    m_ANFNash.Set(GetANFNash());
  return m_ANFNash.Answer();
}

const gbtTriState &BehavSolution::IsSubgamePerfect(void) const
{
  CheckIsValid();
  if(!m_SubgamePerfect.Checked())
    m_SubgamePerfect.Set(GetSubgamePerfect());
  return m_SubgamePerfect.Answer();
}

const gbtTriState &BehavSolution::IsSequential(void) const
{
  CheckIsValid();
  if(!m_Sequential.Checked())
    m_Sequential.Set(GetSequential());
  return m_Sequential.Answer();
}

const gbtNumber &BehavSolution::GetLiapValue(void) const
{ 
  CheckIsValid();
  if(!m_liapValue.Checked())
    m_liapValue.Set(m_profile->LiapValue());
  return m_liapValue.Answer();
}

void BehavSolution::Invalidate(void) const
{
  // we depend on GCL or GUI to deallocate if there are structural 
  // changes in m_efg.  This only deals with changes in action probs of 
  // m_profile, and changes to outcome payoffs or chance probs of m_efg
  m_profile->Invalidate();
  m_support = gbtEfgSupport(m_profile->GetGame());
  m_creator = "User";
  m_ANFNash.Invalidate();
  m_Nash.Invalidate();
  m_SubgamePerfect.Invalidate();
  m_Sequential.Invalidate();
  m_qreLambda = -1;
  m_qreValue = -1;
  m_liapValue.Invalidate();
  m_rnfRegret.Invalidate();
  m_revision = GetGame().RevisionNumber();
}

//-----------------------------------------
// Computation of interesting quantities
//-----------------------------------------


gbtPVector<gbtNumber> BehavSolution::GetRNFRegret(void) const 
{
  gbtEfgGame efg = GetGame();
  gbtNfgGame nfg = efg.GetReducedNfg();
  
  gbtPVector<gbtNumber> regret(nfg.NumStrats());
  
  for (int pl = 1; pl <= efg.NumPlayers(); pl++)  {
    gbtNumber pay = Payoff(pl);
    gbtNfgPlayer player = nfg.GetPlayer(pl);
    for (int st = 1; st <= player.NumStrategies(); st++) {
      gbtBehavProfile<gbtNumber> scratch(*m_profile);
      const gbtArray<int> *const actions = player.GetStrategy(st).GetBehavior();
      for (int j = 1; j <= actions->Length(); j++) {
	int a = (*actions)[j];
	for (int k = 1; k <= scratch.Support().NumActions(pl,j); k++) {
	  scratch(pl, j, k) = (gbtNumber) 0;
	}
	if (a > 0) {
	  scratch(pl, j, a) = (gbtNumber) 1;
	}
      }
      gbtNumber pay2 = scratch.Payoff(pl);
      // use pay - pay instead of zero to get correct precision
      regret(pl,st) = (pay2 < pay) ? pay - pay : pay2 - pay ;
    }
  }
  return regret;
}


const gbtPVector<gbtNumber> &BehavSolution::ReducedNormalFormRegret(void) const
{
  CheckIsValid();
  if(!m_rnfRegret.Checked()) 
    m_rnfRegret.Set(GetRNFRegret());
  return m_rnfRegret.Answer();
}

const gbtNumber BehavSolution::MaxRegret(void) const
{
  return m_profile->MaxRegret();
}

const gbtNumber BehavSolution::MaxRNFRegret(void) const
{
  gbtNumber ret = 0;
  const gbtVector<gbtNumber> & regret = (gbtVector<gbtNumber>)ReducedNormalFormRegret();
  for(int i=regret.First();i<=regret.Last();i++)
    if(regret[i]>=ret)ret = regret[i];

  return ret;
}

//----------
// Output
//----------

void BehavSolution::Dump(gbtOutput &p_file) const
{
  p_file << *m_profile;
  DumpInfo(p_file);
}

void BehavSolution::DumpInfo(gbtOutput &p_file) const
{
  p_file << " Creator:" << GetCreator();
  p_file << " IsNash:" << IsNash();
  p_file << " IsSubgamePerfect:" << IsSubgamePerfect();
  p_file << " IsSequential:" << IsSequential();
  p_file << " LiapValue:" << GetLiapValue();
  if (GetCreator() == "Qre[EFG]" || GetCreator() == "Qre[NFG]") {
    p_file << " QreLambda:" << m_qreLambda;
    p_file << " QreValue:" << m_qreValue;
  }
}

gbtOutput &operator<<(gbtOutput &p_file, const BehavSolution &p_solution)
{
  p_solution.Dump(p_file);
  return p_file;
}

SubgamePerfectChecker::SubgamePerfectChecker(const gbtEfgGame &p_efg,
					     const gbtBehavProfile<gbtNumber> &s,
					     const gbtNumber & epsilon)
  : subgame_number(0), eps(epsilon),  
    isSubgamePerfect(GBT_TRISTATE_TRUE), infoset_subgames(p_efg.NumInfosets()), start(s)
{
  MarkedSubgameRoots(p_efg, oldroots);
  gbtList<gbtEfgNode> subroots;
  LegalSubgameRoots(p_efg,subroots);
  for (int i = 1; i <= subroots.Length(); i++) {
    (start.GetGame()).MarkSubgame(subroots[i]);
  }
  
  for (int pl = 1; pl <= p_efg.NumPlayers(); pl++)   {
    gbtEfgPlayer player = p_efg.GetPlayer(pl);
    for (int iset = 1; iset <= player.NumInfosets(); iset++)  {
      int index;
      
      gbtEfgInfoset infoset = player.GetInfoset(iset);
      gbtEfgNode member = infoset.GetMember(1);
      
      for (index = 1; (index <= subroots.Length() &&
		       member.GetSubgameRoot() != subroots[index]); index++);
      
      infoset_subgames(pl, iset) = index;
    }
  }   
}

void SubgamePerfectChecker::SolveSubgame(const gbtEfgGame &p_efg,
					 const gbtEfgSupport &sup,
					 gbtList<BehavSolution> &solns,
					 gbtStatus &p_status)
{
  gbtBehavProfile<gbtNumber> bp(sup);
  
  subgame_number++;
  
  gbtArray<int> infosets(infoset_subgames.Lengths());
  
  for (int pl = 1; pl <= p_efg.NumPlayers(); pl++)  {
    int niset = 1;
    for (int iset = 1; iset <= infosets[pl]; iset++)  {
      if (infoset_subgames(pl, iset) == subgame_number)  {
	for (int act = 1; act <= bp.Support().NumActions(pl, niset); act++)
	  bp(pl, niset, act) = start(pl, iset, act);
	niset++;
      }
    }
  }
  //  We need to add function to check if bp is Nash on Subgame.  
  //  gbtTriState x = IsNashOnSubgame(E,bp,eps);
  // for now, we do the following, which may give wrong answer if player can deviate at
  // multiple isets simultaneously.  :
  gbtTriState x = GBT_TRISTATE_FALSE;
  BehavSolution bs(bp);
  
  if(bs.MaxRNFRegret() <= eps) x = GBT_TRISTATE_TRUE;
  
  if(isSubgamePerfect == GBT_TRISTATE_TRUE &&  x == GBT_TRISTATE_TRUE) 
    isSubgamePerfect = GBT_TRISTATE_TRUE;
  else if(isSubgamePerfect == GBT_TRISTATE_FALSE ||  x == GBT_TRISTATE_FALSE) 
    isSubgamePerfect = GBT_TRISTATE_FALSE;
  else 
    isSubgamePerfect = GBT_TRISTATE_UNKNOWN;
  
  int index = solns.Append(BehavSolution(bp, "User"));
  solns[index].SetEpsilon(eps);
}

SubgamePerfectChecker::~SubgamePerfectChecker() { 
  (start.GetGame()).UnmarkSubgames(start.GetGame().GetRoot());
  for (int i = 1; i <= oldroots.Length(); i++) {
    start.GetGame().MarkSubgame(oldroots[i]);
  }
}

#include "base/glist.imp"
template class gbtList<BehavSolution>;
