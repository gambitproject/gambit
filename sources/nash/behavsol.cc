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
#include "game/lexicon.h"  // needed for ReducedNormalFormRegrets

// we probably want to break this out into another file (rdm)

#include "base/gnullstatus.h"
#include "nash/subsolve.h"

class SubgamePerfectChecker : public SubgameSolver  {
private:
  int subgame_number;
  gNumber eps;
  gTriState isSubgamePerfect;
  gPVector<int> infoset_subgames;
  BehavProfile<gNumber> start;
  gList<Node *> oldroots;
  
  void SolveSubgame(const efgGame &, const EFSupport &,
		    gList<BehavSolution> &, gStatus &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_USER; }    
  
public:
  SubgamePerfectChecker(const efgGame &, const BehavProfile<gNumber> &, const gNumber & epsilon);
  virtual ~SubgamePerfectChecker();
  gTriState IsSubgamePerfect(void) {return isSubgamePerfect;}
};

gText ToText(EfgAlgType p_algorithm)
{
  switch (p_algorithm) {
  case algorithmEfg_USER:
    return "User"; 
  case algorithmEfg_ENUMPURE_EFG:
    return "EnumPure[EFG]";
  case algorithmEfg_ENUMPURE_NFG:
    return "EnumPure[NFG]";
  case algorithmEfg_ENUMMIXED_NFG:
    return "EnumMixed[NFG]";
  case algorithmEfg_LCP_EFG:
    return "Lcp[EFG]";
  case algorithmEfg_LCP_NFG:
    return "Lcp[NFG]";
  case algorithmEfg_LP_EFG:
    return "Lp[EFG]";
  case algorithmEfg_LP_NFG:
    return "Lp[NFG]";
  case algorithmEfg_LIAP_EFG:
    return "Liap[EFG]";
  case algorithmEfg_LIAP_NFG:
    return "Liap[NFG]";
  case algorithmEfg_SIMPDIV_NFG:
    return "Simpdiv[NFG]";
  case algorithmEfg_POLENUM_EFG:
    return "PolEnum[EFG]";
  case algorithmEfg_POLENUM_NFG:
    return "PolEnum[NFG]";
  case algorithmEfg_QRE_EFG:
    return "Qre[EFG]";
  case algorithmEfg_QRE_NFG:
    return "Qre[NFG]";
  case algorithmEfg_QREALL_NFG:
    return "QreAll[NFG]";
  default:
    return "None";
  }
}

gOutput &operator<<(gOutput &p_file, EfgAlgType p_algorithm)
{
  p_file << ToText(p_algorithm);
  return p_file;
}

//----------------------------------------------------
// Constructors, Destructor, Constructive Operators
//----------------------------------------------------

BehavSolution::BehavSolution(const BehavProfile<double> &p_profile,
			     EfgAlgType p_creator)
  : m_profile(new BehavProfile<gNumber>(EFSupport(p_profile.GetGame()))),
    m_precision(precDOUBLE),
    m_support(p_profile.Support()), m_creator(p_creator),
    m_ANFNash(), m_Nash(), m_SubgamePerfect(), m_Sequential(), 
    m_epsilon(0.0), m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(), m_rnfRegret(), 
    m_revision(p_profile.GetGame().RevisionNumber())
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    EFPlayer *player = GetGame().Players()[pl];  
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Infoset *infoset = player->Infosets()[iset];
      for (int act = 1; act <= infoset->NumActions(); act++) {
	int index = p_profile.Support().Find(infoset->Actions()[act]);
	if (index > 0)
	  (*m_profile)(pl, iset, act) = p_profile(pl, iset, index);
	else
	  (*m_profile)(pl, iset, act) = gNumber(0.0);
      }
    }
  }
}


BehavSolution::BehavSolution(const BehavProfile<gRational> &p_profile,
			     EfgAlgType p_creator)
  : m_profile(new BehavProfile<gNumber>(EFSupport(p_profile.GetGame()))),
    m_precision(precRATIONAL), 
    m_support(p_profile.Support()), m_creator(p_creator),
    m_ANFNash(), m_Nash(), m_SubgamePerfect(), m_Sequential(), 
    m_qreLambda(-1), m_qreValue(-1), m_liapValue(), m_rnfRegret(), 
    m_revision(p_profile.GetGame().RevisionNumber())
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    EFPlayer *player = GetGame().Players()[pl];  
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Infoset *infoset = player->Infosets()[iset];
      for (int act = 1; act <= infoset->NumActions(); act++) {
	int index = p_profile.Support().Find(infoset->Actions()[act]);
	if (index > 0)
	  (*m_profile)(pl, iset, act) = p_profile(pl, iset, index);
	else
	  (*m_profile)(pl, iset, act) = gNumber(0);
      }
    }
  }
}

BehavSolution::BehavSolution(const BehavProfile<gNumber> &p_profile, 
			     EfgAlgType p_creator)
  : m_profile(new BehavProfile<gNumber>(EFSupport(p_profile.GetGame()))),
    m_precision(precRATIONAL),
    m_support(p_profile.Support()), m_creator(p_creator),
    m_ANFNash(), m_Nash(), m_SubgamePerfect(), m_Sequential(), 
    m_qreLambda(-1), m_qreValue(-1), m_liapValue(), m_rnfRegret(), 
    m_revision(p_profile.GetGame().RevisionNumber())
{
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    EFPlayer *player = GetGame().Players()[pl];  
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      Infoset *infoset = player->Infosets()[iset];
      for (int act = 1; act <= infoset->NumActions(); act++) {
	int index = p_profile.Support().Find(infoset->Actions()[act]);
	if (index > 0)
	  (*m_profile)(pl, iset, act) = p_profile(pl, iset, index);
	else
	  (*m_profile)(pl, iset, act) = gNumber(0);
      }
    }
  }
  LevelPrecision();

  m_epsilon = 0;
  if((*m_profile).Length()>0) 
    if ((*m_profile)[1].Precision() == precDOUBLE)
      m_epsilon = 0.0;

  gEpsilon(m_epsilon);
}

BehavSolution::BehavSolution(const BehavSolution &p_solution)
  : m_profile(new BehavProfile<gNumber>(*p_solution.m_profile)),
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
    m_name(p_solution.m_name),
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
    m_profile = new BehavProfile<gNumber>(*p_solution.m_profile);
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
    m_name = p_solution.m_name;
    m_revision = p_solution.m_revision;
  }

  return *this;
}

//-----------------------------
// Private member functions
//-----------------------------

gTriState BehavSolution::GetANFNash(void) const
{
  gNullStatus status;
  algExtendsToAgentNash algorithm;
  gTriState answer = ((algorithm.ExtendsToAgentNash(*this,
						    Support(), Support(),
						    status)) ?
		      triTRUE : triFALSE);
  if (answer == triFALSE) {
    m_Nash.Set(triFALSE);
    m_SubgamePerfect.Set(triFALSE);
    m_Sequential.Set(triFALSE);
  }
  return answer;
  return triUNKNOWN;
}

gTriState BehavSolution::GetNash(void) const
{
  gTriState answer = triUNKNOWN;
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
      answer = (MaxRNFRegret() <= m_epsilon) ? triTRUE:triFALSE;
    }
    else {
      //  else let Andy figure it out
      // Is perfect recall needed here, Andy?
      if (IsPerfectRecall(m_profile->GetGame())) { 
	// not sure MaxRegret does the right thing here
	gNullStatus status;
	algExtendsToNash algorithm;
	answer = (m_profile->MaxRegret() <= m_epsilon  &&
		  algorithm.ExtendsToNash(*this,Support(),Support(),status)) 
	  ? triTRUE:triFALSE;
      }
      answer = triUNKNOWN;
    }
  }
  // Done.  Now mark other obvious inferences 
  
  if (answer == triFALSE) {
    m_SubgamePerfect.Set(triFALSE);
    m_Sequential.Set(triFALSE);
  }
  if (answer == triTRUE) {
    m_ANFNash.Set(triTRUE);
    if(!decomposes) 
      m_SubgamePerfect.Set(triTRUE);
  }
  return answer;
}

gTriState BehavSolution::GetSubgamePerfect(void) const
{
  gTriState answer;
  // Note -- HasSubgames should be cached in Efg
  bool decomposes = HasSubgames(GetGame());
  if(!decomposes) GetNash();
  // if it was already resolved ...
  if(m_SubgamePerfect.Checked() == true) 
    answer = m_SubgamePerfect.Answer();
  else {
    // for complete profiles, use subgame perfect checker.  
    if(IsComplete()) {
      BehavProfile<gNumber> p(*m_profile);
      SubgamePerfectChecker checker(p.GetGame(),p, Epsilon());
      gNullStatus status;
      checker.Solve(p.Support(), status);
      answer = checker.IsSubgamePerfect();
    }
    // else, for now, we require complete profiles for subgame perfection.  
    // but we may want to turn over to Andy here. 
    else 
      answer = triFALSE;
  }
  // Done.  Now mark other obvious inferences 
  
  if(answer) {
    m_Nash.Set(triTRUE);
    m_ANFNash.Set(triTRUE);
  }
  if(!answer) {
    m_Sequential.Set(triFALSE);
    if(!decomposes) 
      m_Nash.Set(triFALSE);
  }
  return answer;
}

gTriState BehavSolution::GetSequential(void) const
{
  if(IsSubgamePerfect()==triTRUE) {
    // Liap and QRE should be returning Nash solutions that give positive 
    // probability to all actions, and hence will be approximations to 
    // sequential equilibria.  But we should add code to check up on these 
    // algorithms
    if(Creator() == algorithmEfg_LIAP_EFG || Creator() == algorithmEfg_QRE_EFG)
      return triTRUE;
    else {
      // check if game is perfect info
      // this should be in efg.h
      bool flag = true;
      gPVector<int> v((GetGame()).NumMembers());
      for(int i=v.First();flag == true && i<=v.Last();i++)
	if(v[i]>1) flag = false;
      if(flag==true) return triTRUE;
    }
    return triUNKNOWN;
  }
  else 
    return IsSubgamePerfect();
}

/*
gNumber BehavSolution::GetLiapValue(void)
{ 
  return m_profile->LiapValue();
}
*/

void BehavSolution::LevelPrecision(void)
{
  m_precision = precRATIONAL;
  for (int pl = 1; m_precision == precRATIONAL && pl <= GetGame().NumPlayers();
       pl++) {
    EFPlayer *player = GetGame().Players()[pl];  
    for (int iset = 1; (m_precision == precRATIONAL && 
			iset <= player->NumInfosets()); iset++) {
      Infoset *infoset = player->Infosets()[iset];
      for (int act = 1; (m_precision == precRATIONAL && 
			 act <= infoset->NumActions()); act++) {
	if ((*m_profile)(pl, iset, act).Precision() == precDOUBLE)
	  m_precision = precDOUBLE;
      }
    }
  }

  if (m_precision == precDOUBLE) {
    for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
      EFPlayer *player = GetGame().Players()[pl];  
      for (int iset = 1; iset <= player->NumInfosets(); iset++) {
	Infoset *infoset = player->Infosets()[iset];
	for (int act = 1; act <= infoset->NumActions(); act++) {
	  (*m_profile)(pl, iset, act) = (double) (*m_profile)(pl, iset, act);
	}
      }
    }
  }
}

//------------------------
// Operator overloading
//------------------------

bool BehavSolution::Equals(const BehavProfile<double> &p_profile) const
{  
  gNumber eps(m_epsilon);
  gEpsilon(eps, 4);  // this should be a function of m_epsilon

  int i = p_profile.First();
  while (i <= p_profile.Length()) {
    if (abs((*m_profile)[i] - (gNumber) p_profile[i]) > eps) 
      break;
    i++;
  }
  return (i > p_profile.Length());
}

bool BehavSolution::operator==(const BehavSolution &p_solution) const
{ return (*m_profile == *p_solution.m_profile); }

void BehavSolution::Set(Action *p_action, const gNumber &p_prob)
{
  Invalidate();

  Infoset *infoset = p_action->BelongsTo();
  EFPlayer *player = infoset->GetPlayer();
  (*m_profile)(player->GetNumber(), infoset->GetNumber(),
	       p_action->GetNumber()) = p_prob;
  if (m_precision != p_prob.Precision())
    LevelPrecision();
}

void BehavSolution::Set(int p_player, int p_infoset, int p_action,
			const gNumber &p_prob)
{
  Invalidate();

  (*m_profile)(p_player, p_infoset, p_action) = p_prob;
  if (m_precision != p_prob.Precision())
    LevelPrecision();
}

const gNumber &BehavSolution::operator()(Action *p_action) const
{
  Infoset *infoset = p_action->BelongsTo();
  EFPlayer *player = infoset->GetPlayer();
  return (*m_profile)(player->GetNumber(), infoset->GetNumber(),
		      p_action->GetNumber());
}

gNumber BehavSolution::operator[](Action *p_action) const
{
  return m_profile->GetActionProb(p_action);
}

gNumber &BehavSolution::operator[](Action *p_action)
{
  return (*m_profile)(p_action->BelongsTo()->GetPlayer()->GetNumber(),
		      p_action->BelongsTo()->GetNumber(),
		      p_action->GetNumber());
}

BehavSolution &BehavSolution::operator+=(const BehavSolution &p_solution)
{
  Invalidate(); 
  *m_profile += *p_solution.m_profile;
  if (m_precision == precRATIONAL && p_solution.m_precision == precDOUBLE)
    m_precision = precDOUBLE;
  return *this;
}

BehavSolution &BehavSolution::operator-=(const BehavSolution &p_solution)
{
  Invalidate();
  *m_profile -= *p_solution.m_profile;
  if (m_precision == precRATIONAL && p_solution.m_precision == precDOUBLE)
    m_precision = precDOUBLE;
  return *this;
}

BehavSolution &BehavSolution::operator*=(const gNumber &p_constant)
{ 
  Invalidate();
  *m_profile *= p_constant;
  if (m_precision == precRATIONAL && p_constant.Precision() == precDOUBLE)
    m_precision = precDOUBLE;
  return *this;
}

//-----------------------
// General data access
//-----------------------

bool BehavSolution::IsComplete(void) const
{ 
  gNumber sum;
  for (int pl = 1; pl <= GetGame().NumPlayers(); pl++) {
    EFPlayer *player = GetGame().Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++) { 
      Infoset *infoset = player->Infosets()[iset];
      sum = -1;
      for (int act = 1; act <= infoset->NumActions(); act++) 
	sum += (*m_profile)(pl, iset, act);
      if (sum > m_epsilon || sum < -m_epsilon) 
	return false;
    }
  }
  return true;
}

const gTriState &BehavSolution::IsNash(void) const
{
  CheckIsValid();
  if(!m_Nash.Checked())
    m_Nash.Set(GetNash());
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

const gTriState &BehavSolution::IsANFNash(void) const
{
  CheckIsValid();
  if(!m_ANFNash.Checked())
    m_ANFNash.Set(GetANFNash());
  return m_ANFNash.Answer();
}

const gTriState &BehavSolution::IsSubgamePerfect(void) const
{
  CheckIsValid();
  if(!m_SubgamePerfect.Checked())
    m_SubgamePerfect.Set(GetSubgamePerfect());
  return m_SubgamePerfect.Answer();
}

const gTriState &BehavSolution::IsSequential(void) const
{
  CheckIsValid();
  if(!m_Sequential.Checked())
    m_Sequential.Set(GetSequential());
  return m_Sequential.Answer();
}

const gNumber &BehavSolution::LiapValue(void) const
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
  m_support = EFSupport(m_profile->GetGame());
  m_creator = algorithmEfg_USER;
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


gPVector<gNumber> BehavSolution::GetRNFRegret(void) const 
{
  const efgGame &E = GetGame(); 
  Lexicon L(E);  // we use the lexicon without allocating normal form.  
  
  for (int i = 1; i <= E.NumPlayers(); i++)
    L.MakeReducedStrats(m_support, E.Players()[i], E.RootNode(), NULL);
  
  gArray<int> dim(E.NumPlayers());
  for (int i = 1; i <= E.NumPlayers(); i++)
    dim[i] = (L.strategies[i].Length()) ? L.strategies[i].Length() : 1;
  
  gPVector<gNumber> regret(dim); 
  
  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    gNumber pay = Payoff(pl);
    for (int st = 1; st <= (L.strategies[pl]).Length(); st++) {
      BehavProfile<gNumber> scratch(*m_profile);
      //	gout << "\ninstalled 1:  " << scratch.IsInstalled() << " scratch: " << scratch;
      const gArray<int> *const actions = L.strategies[pl][st];
      for(int j = 1;j<=(*actions).Length();j++) {
	int a = (*actions)[j];
	//	  for (int k = 1;k<=m_support.NumActions(pl,j);k++)
	for (int k = 1;k<=scratch.Support().NumActions(pl,j);k++)
	  scratch(pl,j,k) = (gNumber)0;
	if(a>0)scratch(pl,j,a) = (gNumber)1;
      }
      //	gout << "\ninstalled 2:  " << scratch.IsInstalled() << " scratch: " << scratch;
      gNumber pay2 = scratch.Payoff(pl);
      // use pay - pay instead of zero to get correct precision
      regret(pl,st) = (pay2 < pay) ? pay - pay : pay2 - pay ;
    }
  }
  return regret;
}


const gPVector<gNumber> &BehavSolution::ReducedNormalFormRegret(void) const
{
  CheckIsValid();
  if(!m_rnfRegret.Checked()) 
    m_rnfRegret.Set(GetRNFRegret());
  return m_rnfRegret.Answer();
}

const gNumber BehavSolution::MaxRegret(void) const
{
  return m_profile->MaxRegret();
}

const gNumber BehavSolution::MaxRNFRegret(void) const
{
  gNumber ret = 0;
  const gVector<gNumber> & regret = (gVector<gNumber>)ReducedNormalFormRegret();
  for(int i=regret.First();i<=regret.Last();i++)
    if(regret[i]>=ret)ret = regret[i];

  return ret;
}

//----------
// Output
//----------

void BehavSolution::Dump(gOutput &p_file) const
{
  p_file << *m_profile;
  DumpInfo(p_file);
}

void BehavSolution::DumpInfo(gOutput &p_file) const
{
  p_file << " Creator:" << Creator();
  p_file << " IsNash:" << IsNash();
  p_file << " IsSubgamePerfect:" << IsSubgamePerfect();
  p_file << " IsSequential:" << IsSequential();
  p_file << " LiapValue:" << LiapValue();
  if(Creator() == algorithmEfg_QRE_EFG || Creator() == algorithmEfg_QRE_NFG) {
    p_file << " QreLambda:" << m_qreLambda;
    p_file << " QreValue:" << m_qreValue;
  }
}

gOutput &operator<<(gOutput &p_file, const BehavSolution &p_solution)
{
  p_solution.Dump(p_file);
  return p_file;
}

SubgamePerfectChecker::SubgamePerfectChecker(const efgGame &E, const BehavProfile<gNumber> &s,
					     const gNumber & epsilon)
  : subgame_number(0), eps(epsilon),  
    isSubgamePerfect(triTRUE), infoset_subgames(E.NumInfosets()), start(s)
{
  MarkedSubgameRoots(E, oldroots);
  gList<Node *> subroots;
  LegalSubgameRoots(E,subroots);
  (start.GetGame()).MarkSubgames(subroots);
  
  for (int pl = 1; pl <= E.NumPlayers(); pl++)   {
    EFPlayer *player = E.Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      int index;
      
      Infoset *infoset = player->Infosets()[iset];
      Node *member = infoset->Members()[1];
      
      for (index = 1; index <= subroots.Length() &&
	     member->GetSubgameRoot() != subroots[index]; index++);
      
      infoset_subgames(pl, iset) = index;
    }
  }   
}

void SubgamePerfectChecker::SolveSubgame(const efgGame &E,
					 const EFSupport &sup,
					 gList<BehavSolution> &solns,
					 gStatus &p_status)
{
  BehavProfile<gNumber> bp(sup);
  
  subgame_number++;
  
  gArray<int> infosets(infoset_subgames.Lengths());
  
  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
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
  //  gTriState x = IsNashOnSubgame(E,bp,eps);
  // for now, we do the following, which may give wrong answer if player can deviate at
  // multiple isets simultaneously.  :
  gTriState x = triFALSE;
  BehavSolution bs(bp);
  
  if(bs.MaxRNFRegret() <= eps) x = triTRUE;
  
  if(isSubgamePerfect == triTRUE &&  x == triTRUE) 
    isSubgamePerfect = triTRUE;
  else if(isSubgamePerfect == triFALSE ||  x == triFALSE) 
    isSubgamePerfect = triFALSE;
  else 
    isSubgamePerfect = triUNKNOWN;
  
  int index = solns.Append(BehavSolution(bp,AlgorithmID()));
  solns[index].SetEpsilon(eps);
}

SubgamePerfectChecker::~SubgamePerfectChecker() { 
  (start.GetGame()).UnmarkSubgames((start.GetGame()).RootNode());
  (start.GetGame()).MarkSubgames(oldroots);
}

#include "base/glist.imp"
template class gList<BehavSolution>;
