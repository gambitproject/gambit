//
// FILE: behavsol.cc -- Behavioral strategy solution classes
//
// $Id$
//

#include "behavsol.h"
#include "lexicon.h"  // needed for ReducedNormalFormRegrets

// we probably want to break this out into another file (rdm)

#include "subsolve.h"

class SubgamePerfectChecker : public SubgameSolver  {
private:
  int subgame_number;
  gNumber eps;
  gTriState isSubgamePerfect;
  gPVector<int> infoset_subgames;
  BehavProfile<gNumber> start;
  gList<Node *> oldroots;
  
  void SolveSubgame(const Efg &, const EFSupport &, gList<BehavSolution> &);
  EfgAlgType AlgorithmID(void) const { return algorithmEfg_USER; }    
  
public:
  SubgamePerfectChecker(const Efg &, const BehavProfile<gNumber> &, const gNumber & epsilon);
  virtual ~SubgamePerfectChecker();
  gTriState IsSubgamePerfect(void) {return isSubgamePerfect;}
};

gText NameEfgAlgType(EfgAlgType p_algorithm)
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

void DisplayEfgAlgType(gOutput &p_file, EfgAlgType p_algorithm)
{
  p_file << NameEfgAlgType(p_algorithm);
}

//----------------------------------------------------
// Constructors, Destructor, Constructive Operators
//----------------------------------------------------

BehavSolution::BehavSolution(const BehavProfile<double> &p_profile,
			     EfgAlgType p_creator)
  : m_profile(new BehavProfile<gNumber>(EFSupport(p_profile.Game()))),
    m_precision(precDOUBLE),
    m_support(p_profile.Support()), m_creator(p_creator),
    m_isANFNash(triUNKNOWN), m_isNash(triUNKNOWN),
    m_isSubgamePerfect(triUNKNOWN), m_isSequential(triUNKNOWN), 
    m_checkedANFNash(false), m_checkedNash(false),
    m_checkedSubgamePerfect(false), m_checkedSequential(false), 
    m_epsilon(0.0),
    m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_beliefs(0), m_regret(0), m_rnf_regret(0), m_id(0)
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    EFPlayer *player = Game().Players()[pl];  
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
  : m_profile(new BehavProfile<gNumber>(EFSupport(p_profile.Game()))),
    m_precision(precRATIONAL), 
    m_support(p_profile.Support()), m_creator(p_creator),
    m_isANFNash(triUNKNOWN), m_isNash(triUNKNOWN), 
    m_isSubgamePerfect(triUNKNOWN), m_isSequential(triUNKNOWN), 
    m_checkedANFNash(false), m_checkedNash(false),
    m_checkedSubgamePerfect(false), m_checkedSequential(false), 
    m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_beliefs(0), m_regret(0), m_rnf_regret(0), m_id(0)
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    EFPlayer *player = Game().Players()[pl];  
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
  : m_profile(new BehavProfile<gNumber>(EFSupport(p_profile.Game()))),
    m_precision(precRATIONAL),
    m_support(p_profile.Support()), m_creator(p_creator),
    m_isANFNash(triUNKNOWN), m_isNash(triUNKNOWN),
    m_isSubgamePerfect(triUNKNOWN), m_isSequential(triUNKNOWN), 
    m_checkedANFNash(false), m_checkedNash(false),
    m_checkedSubgamePerfect(false), m_checkedSequential(false), 
    m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_beliefs(0), m_regret(0), m_rnf_regret(0), m_id(0)
{
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    EFPlayer *player = Game().Players()[pl];  
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
    m_isANFNash(p_solution.m_isANFNash),
    m_isNash(p_solution.m_isNash),
    m_isSubgamePerfect(p_solution.m_isSubgamePerfect),
    m_isSequential(p_solution.m_isSequential), 
    m_checkedANFNash(p_solution.m_checkedANFNash),
    m_checkedNash(p_solution.m_checkedNash),
    m_checkedSubgamePerfect(p_solution.m_checkedSubgamePerfect),
    m_checkedSequential(p_solution.m_checkedSequential), 
    m_epsilon(p_solution.m_epsilon),
    m_qreLambda(p_solution.m_qreLambda),
    m_qreValue(p_solution.m_qreValue),
    m_liapValue(p_solution.m_liapValue),
    m_beliefs(0), m_regret(0), m_rnf_regret(0), m_id(0)
{
  if (p_solution.m_beliefs) {
    m_beliefs = new gDPVector<gNumber>(*p_solution.m_beliefs);
  }
  if (p_solution.m_regret) {
    m_regret = new gDPVector<gNumber>(*p_solution.m_regret);   
  }
  if (p_solution.m_rnf_regret) {
    m_rnf_regret = new gPVector<gNumber>(*p_solution.m_regret);   
  }
}

BehavSolution::~BehavSolution() 
{ 
  delete m_profile;
  if (m_beliefs) delete m_beliefs;
  if (m_regret)  delete m_regret;
  if (m_rnf_regret)  delete m_rnf_regret;
}

BehavSolution& BehavSolution::operator=(const BehavSolution &p_solution)
{
  if (this != &p_solution)   {
    delete m_profile;
    m_profile = new BehavProfile<gNumber>(*p_solution.m_profile);
    m_precision = p_solution.m_precision;
    m_support = p_solution.m_support;
    m_creator = p_solution.m_creator;
    m_isANFNash = p_solution.m_isANFNash;
    m_isNash = p_solution.m_isNash;
    m_isSubgamePerfect = p_solution.m_isSubgamePerfect;
    m_isSequential = p_solution.m_isSequential;
    m_checkedANFNash = p_solution.m_checkedANFNash;
    m_checkedNash = p_solution.m_checkedNash;
    m_checkedSubgamePerfect = p_solution.m_checkedSubgamePerfect;
    m_checkedSequential = p_solution.m_checkedSequential;
    m_epsilon = p_solution.m_epsilon;
    m_qreLambda = p_solution.m_qreLambda;
    m_qreValue = p_solution.m_qreValue;
    m_liapValue = p_solution.m_liapValue;
    if (m_beliefs)   delete m_beliefs;
    if (p_solution.m_beliefs)
      m_beliefs = new gDPVector<gNumber>(*p_solution.m_beliefs);
    else
      m_beliefs = 0;
    if (m_regret)   delete m_regret;
    if (p_solution.m_regret) {
      m_regret = new gDPVector<gNumber>(*p_solution.m_regret);
    }
    else {
      m_regret = 0;
    }
    if (m_rnf_regret)   delete m_rnf_regret;
    if (p_solution.m_rnf_regret) {
      m_rnf_regret = new gPVector<gNumber>(*p_solution.m_rnf_regret);
    }
    else {
      m_rnf_regret = 0;
    }
  }

  return *this;
}

//-----------------------------
// Private member functions
//-----------------------------

void BehavSolution::CheckIsNash(void) const
{
  if (m_checkedNash == false) {
    bool decomposes = HasSubgames(Game());
    // check subgame perfection if game decomposes (its faster)
    if(decomposes) 
      CheckIsSubgamePerfect();
    // and if it wasn't settled in that call, ...
    if (m_checkedNash == false) { 
      // use reduced normal form regrets for complete profiles
      if(IsComplete()) 
	m_isNash = (MaxRNFRegret() <= m_epsilon) ? triTRUE:triFALSE;
      // else let Andy figure it out
      // Is perfect recall needed here, Andy?
      else if (IsPerfectRecall(m_profile->Game())) { 
	gStatus &m_status = gstatus;
	// not sure MaxGripe does the right thing here
	m_isNash = (m_profile->MaxGripe() <= m_epsilon  &&
		    ExtendsToNash(Support(),Support(),m_status)) ? triTRUE:triFALSE;
      }
    }
    m_checkedNash = true;

    // Done.  Now mark other obvious inferences 

    if (m_isNash == triFALSE) {
      m_isSubgamePerfect = triFALSE; m_checkedSubgamePerfect = true;
      m_isSequential = triFALSE; m_checkedSequential = true;
    }
    if (m_isNash == triTRUE) {
      m_isANFNash = triTRUE; m_checkedANFNash = true;
      if(!decomposes) { 
	m_isSubgamePerfect = triTRUE; m_checkedSubgamePerfect = true;
      }
    }
  }
}

void BehavSolution::CheckIsSubgamePerfect(void) const
{
  if(m_checkedSubgamePerfect == false) {
    // Note -- HasSubgames should be cached in Efg
    bool decomposes = HasSubgames(Game());
    if(!decomposes) CheckIsNash();
    // if it is not yet resolved, ... 
    if(m_checkedSubgamePerfect == false) {
      // for complete profiles, use subgame perfect checker.  
      if(IsComplete()) {
	BehavProfile<gNumber> p(*this);
	SubgamePerfectChecker checker(p.Game(),p, Epsilon());
	checker.Solve(p.Support());
	m_isSubgamePerfect = checker.IsSubgamePerfect();
      }
      // else, for now, we require complete profiles for subgame perfection.  
      // but we may want to turn over to Andy here. 
      else 
	m_isSubgamePerfect = triFALSE;
    }
    m_checkedSubgamePerfect = true;

    // Done.  Now mark other obvious inferences 

    if(m_isSubgamePerfect) {
      m_isNash = triTRUE; m_checkedNash = triTRUE;
      m_isANFNash = triTRUE; m_checkedANFNash = true;
    }
    if(!m_isSubgamePerfect) {
      m_isSequential = triFALSE; m_checkedSequential = triFALSE;
      if(!decomposes) { 
	m_isNash = triFALSE; m_checkedNash = true;
      }
    }
  }
}

void BehavSolution::CheckIsANFNash(void) const
{
  if (m_checkedANFNash == false) {
    gStatus &m_status = gstatus;
    m_isANFNash = (m_profile->ExtendsToANFNash(Support(),Support(),m_status)) ?
      triTRUE:triFALSE;
    if (m_isANFNash == triFALSE) {
      m_isNash = triFALSE; m_checkedNash = true;
      m_isSubgamePerfect = triFALSE; m_checkedSubgamePerfect = true;
      m_isSequential = triFALSE; m_checkedSequential = true;
    }
    m_checkedANFNash = true;
  }
}

void BehavSolution::LevelPrecision(void)
{
  m_precision = precRATIONAL;
  for (int pl = 1; m_precision == precRATIONAL && pl <= Game().NumPlayers();
       pl++) {
    EFPlayer *player = Game().Players()[pl];  
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
    for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
      EFPlayer *player = Game().Players()[pl];  
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
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    EFPlayer *player = Game().Players()[pl];
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

gTriState BehavSolution::IsNash(void) const
{
  CheckIsNash();
  return m_isNash;
}

gTriState BehavSolution::IsANFNash(void) const
{
  CheckIsANFNash();
  return m_isANFNash;
}

gTriState BehavSolution::IsSubgamePerfect(void) const
{
  CheckIsSubgamePerfect();
  return m_isSubgamePerfect;
}

gTriState BehavSolution::IsSequential(void) const
{
  CheckIsNash();
  if(m_checkedSequential == false) {
    if(IsSubgamePerfect()==triTRUE) {
      // Liap and QRE should be returning Nash solutions that give positive 
      // probability to all actions, and hence will be approximations to 
      // sequential equilibria.  But we should add code to check up on these 
      // algorithms
      if(Creator() == algorithmEfg_LIAP_EFG || Creator() == algorithmEfg_QRE_EFG)
	m_isSequential = triTRUE;
      else {
	// check if game is perfect info
	// this should be in efg.h
	bool flag = true;
	gPVector<int> v((Game()).NumMembers());
	for(int i=v.First();flag == true && i<=v.Last();i++)
	  if(v[i]>1) flag = false;
	if(flag==true) m_isSequential = triTRUE;
      }
    }
    else 
      m_isSequential = IsSubgamePerfect();
    m_checkedSequential = true;
  }
  return m_isSequential;
}

const gNumber &BehavSolution::LiapValue(void) const
{ 
  if (m_liapValue < (gNumber) 0)
    m_liapValue = m_profile->LiapValue();
  return m_liapValue; 
}

void BehavSolution::Invalidate(void) const
{
  m_support = EFSupport(m_profile->Game());
  m_creator = algorithmEfg_USER;
  m_isANFNash = triUNKNOWN;
  m_isSubgamePerfect = triUNKNOWN;
  m_isSequential = triUNKNOWN;
  m_checkedANFNash = false;
  m_checkedSubgamePerfect = false;
  m_checkedSequential = false;
  m_qreLambda = -1;
  m_qreValue = -1;
  m_liapValue = -1;

  if (m_beliefs) { 
    delete m_beliefs;
    m_beliefs = 0;
  }
  
  if (m_regret)  {
    delete m_regret;
    m_regret = 0;
  }
  if (m_rnf_regret)  {
    delete m_rnf_regret;
    m_rnf_regret = 0;
  }
}

//-----------------------------------------
// Computation of interesting quantities
//-----------------------------------------

const gDPVector<gNumber> &BehavSolution::Beliefs(void) const
{ 
  if (!m_beliefs) 
    m_beliefs = new gDPVector<gNumber>(m_profile->Beliefs());

  return *m_beliefs; 
}

const gDPVector<gNumber> &BehavSolution::Regret(void) const
{
  if (!m_regret)  {
    m_regret = new gDPVector<gNumber>(Game().NumActions());
    m_profile->Gripe(*m_regret);
  }

  return *m_regret;
}

const gPVector<gNumber> &BehavSolution::ReducedNormalFormRegret(void) const
{
  if (!m_rnf_regret)  {
    const Efg& E = Game(); 
    Lexicon L(E);  // we use the lexicon without allocating normal form.  
    
    for (int i = 1; i <= E.NumPlayers(); i++)
      L.MakeReducedStrats(m_support, E.Players()[i], E.RootNode(), NULL);
    
    gArray<int> dim(E.NumPlayers());
    for (int i = 1; i <= E.NumPlayers(); i++)
      dim[i] = (L.strategies[i].Length()) ? L.strategies[i].Length() : 1;
    
    m_rnf_regret = new gPVector<gNumber>(dim);
    
    for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
      gNumber pay = Payoff(pl);
      for (int st = 1; st <= (L.strategies[pl]).Length(); st++) {
	BehavProfile<gNumber> scratch(*this);
	const gArray<int> *const actions = L.strategies[pl][st];
	for(int j = 1;j<=(*actions).Length();j++) {
	  int a = (*actions)[j];
	  for (int k = 1;k<=m_support.NumActions(pl,j);k++)
	    scratch(pl,j,k) = (gNumber)0;
	  if(a>0)scratch(pl,j,a) = (gNumber)1;
	}
	gNumber pay2 = scratch.Payoff(pl);
	// use pay - pay instead of zero to get correct precision
	(*m_rnf_regret)(pl,st) = (pay2 < pay) ? pay - pay : pay2 - pay ;
      }
    }
  }
  return *m_rnf_regret;
}

const gNumber BehavSolution::MaxRegret(void) const
{
  Regret();
  gNumber ret = 0;
  for(int i=m_regret->First();i<=m_regret->Last();i++)
    if((*m_regret)[i]>=ret)ret = (*m_regret)[i];
  return ret;
}

const gNumber BehavSolution::MaxRNFRegret(void) const
{
  ReducedNormalFormRegret();
  gNumber ret = 0;
  for(int i=m_rnf_regret->First();i<=m_rnf_regret->Last();i++)
    if((*m_rnf_regret)[i]>=ret)ret = (*m_rnf_regret)[i];

  return ret;
}

//----------------------------------------
// Restriction to Support Extends to Nash
//----------------------------------------

gPolyList<gDouble> 
BehavSolution::ActionProbsSumToOneIneqs(const gSpace &BehavStratSpace, 
					const term_order &Lex,
					const EFSupport &big_supp,
					const gList<gList<int> > &var_index) 
  const
{
  gPolyList<gDouble> answer(&BehavStratSpace, &Lex);

  int pl;
  for (pl = 1; pl <= Game().NumPlayers(); pl++) 
    for (int i = 1; i <= Game().NumPlayersInfosets(pl); i++) {
      Infoset *current_infoset = Game().GetInfosetByIndex(pl,i);
      if ( !big_supp.HasActiveActionAt(current_infoset) ) {
	int index_base = var_index[pl][i];
	gPoly<gDouble> factor(&BehavStratSpace, (gDouble)1.0, &Lex);
	for (int k = 1; k < current_infoset->NumActions(); k++)
	  factor -= gPoly<gDouble>(&BehavStratSpace, index_base + k, 1, &Lex);
	answer += factor;
      }
    }
  return answer;
}

bool BehavSolution::ANFNodeProbabilityPoly(gPoly<gDouble> & node_prob,
					   const gSpace &BehavStratSpace, 
					   const term_order &Lex,
					  const EFSupport &big_supp,
					   const gList<gList<int> > &var_index,
					   const Node *tempnode,
					   const int &pl,
					   const int &i,
					   const int &j) const
{
  while (tempnode != Game().RootNode()) {

    const Action *last_action = tempnode->GetAction();
    Infoset *last_infoset = last_action->BelongsTo();
    
    if (last_infoset->IsChanceInfoset()) 
      node_prob *= (gDouble)Game().GetChanceProb(last_action);
    else 
      if (big_supp.HasActiveActionAt(last_infoset)) {
	if (last_infoset == Game().GetInfosetByIndex(pl,i)) {
	  if (j != last_action->GetNumber()) 
	    return false;
	}
	else
	  if (big_supp.ActionIsActive((Action *)last_action))
	    node_prob *= (gDouble)Profile()->GetValue(last_action);
	  else 
	    return false;
      }
      else {
	int initial_var_no = 
 var_index[last_infoset->GetPlayer()->GetNumber()][last_infoset->GetNumber()];
	if (last_action->GetNumber() < last_infoset->NumActions()){
	  int varno = initial_var_no + last_action->GetNumber();
	  node_prob *= gPoly<gDouble>(&BehavStratSpace, varno, 1, &Lex);
	}
	else {
	  gPoly<gDouble> factor(&BehavStratSpace, (gDouble)1.0, &Lex);
	  int k;
	  for (k = 1; k < last_infoset->NumActions(); k++)
	    factor -= gPoly<gDouble>(&BehavStratSpace,
				     initial_var_no + k, 1, &Lex);
	  node_prob *= factor;
	}
      } 
    tempnode = tempnode->GetParent();
  }
  return true;
}

gPolyList<gDouble> 
BehavSolution::ANFExpectedPayoffDiffPolys(const gSpace &BehavStratSpace, 
					  const term_order &Lex,
					  const EFSupport &little_supp,
					  const EFSupport &big_supp,
					  const gList<gList<int> > &var_index) 
  const
{
  gPolyList<gDouble> answer(&BehavStratSpace, &Lex);

  gList<const Node *> terminal_nodes = Game().TerminalNodes();

  for (int pl = 1; pl <= Game().NumPlayers(); pl++)
    for (int i = 1; i <= Game().NumPlayersInfosets(pl); i++) {
      Infoset *infoset = Game().GetInfosetByIndex(pl,i);
      if (little_supp.MayReach(infoset)) 
	for (int j = 1; j <= infoset->NumActions(); j++)
	  if (!little_supp.ActionIsActive(pl,i,j)) {
	
	    // This will be the utility difference between the
	    // payoff resulting from the profile and deviation to 
	    // action j
	    gPoly<gDouble> next_poly(&BehavStratSpace, &Lex);

	    for (int n = 1; n <= terminal_nodes.Length(); n++) {
	      gPoly<gDouble> node_prob(&BehavStratSpace, (gDouble)1.0, &Lex);
	      if (ANFNodeProbabilityPoly(node_prob,
					 BehavStratSpace,
					 Lex,
					 big_supp,
					 var_index,
					 terminal_nodes[n],
					 pl,i,j)) {
		node_prob *= 
		  (gDouble)Game().Payoff(terminal_nodes[n]->GetOutcome(),pl);
		next_poly += node_prob;
	      }
	    }
	    answer += -next_poly + (gDouble)Payoff(pl);
	  }
    }
  return answer;
}

gPolyList<gDouble> 
BehavSolution::ExtendsToANFNashIneqs(const gSpace &BehavStratSpace, 
				     const term_order &Lex,
				     const EFSupport &little_supp,
				     const EFSupport &big_supp,
				     const gList<gList<int> > &var_index) const
{
  gPolyList<gDouble> answer(&BehavStratSpace, &Lex);
  answer += ActionProbsSumToOneIneqs(BehavStratSpace, 
				     Lex, 
				     big_supp, 
				     var_index);
  answer += ANFExpectedPayoffDiffPolys(BehavStratSpace, 
				       Lex, 
				       little_supp,
				       big_supp,
				       var_index);
  return answer;
}

bool BehavSolution::ExtendsToNash(const EFSupport &little_supp,
				  const EFSupport &big_supp,
				        gStatus &m_status) const
{
  // This asks whether there is a Nash extension of the BehavSolution to 
  // all information sets at which the behavioral probabilities are not
  // specified.  The assumption is that the support has active actions
  // at infosets at which the behavioral probabilities are defined, and
  // no others.  Also, the BehavSol is assumed to be already a Nash
  // equilibrium for the truncated game obtained by eliminating stuff
  // outside little_supp.
  
  // First we compute the number of variables, and indexing information
  int num_vars(0);
  gList<gList<int> > var_index;
  int pl;
  for (pl = 1; pl <= Game().NumPlayers(); pl++) {

    gList<int> list_for_pl;

    for (int i = 1; i <= Game().NumPlayersInfosets(pl); i++) {
      list_for_pl += num_vars;
      if ( !big_supp.HasActiveActionAt(Game().GetInfosetByIndex(pl,i)) ) {
	num_vars += Game().NumActionsAtInfoset(pl,i) - 1;
      }
    }
    var_index += list_for_pl;
  }

  // We establish the space
  gSpace BehavStratSpace(num_vars);
  ORD_PTR ptr = &lex;
  term_order Lex(&BehavStratSpace, ptr);

  num_vars = BehavStratSpace.Dmnsn();

  gPolyList<gDouble> inequalities = ExtendsToNashIneqs(BehavStratSpace,
						       Lex,
						       little_supp,
						       big_supp,
						       var_index);
  // set up the rectangle of search
  gVector<gDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gDouble)0;
  tops = (gDouble)1;
  gRectangle<gDouble> Cube(bottoms, tops); 

  // Set up the test and do it
  IneqSolv<gDouble> extension_tester(inequalities,m_status);
  gVector<gDouble> sample(num_vars);
  bool answer = extension_tester.ASolutionExists(Cube,sample); 
  
  assert (answer == m_profile->ExtendsToNash(little_supp, big_supp, m_status));

  return answer;
}

bool BehavSolution::ExtendsToANFNash(const EFSupport &little_supp,
				     const EFSupport &big_supp,
				           gStatus &m_status) const
{
  // This asks whether there is an ANF Nash extension of the BehavSolution to 
  // all information sets at which the behavioral probabilities are not
  // specified.  The assumption is that the support has active actions
  // at infosets at which the behavioral probabilities are defined, and
  // no others.
  
  // First we compute the number of variables, and indexing information
  int num_vars(0);
  gList<gList<int> > var_index;
  int pl;
  for (pl = 1; pl <= Game().NumPlayers(); pl++) {

    gList<int> list_for_pl;

    for (int i = 1; i <= Game().NumPlayersInfosets(pl); i++) {
      list_for_pl += num_vars;
      if ( !big_supp.HasActiveActionAt(Game().GetInfosetByIndex(pl,i)) ) {
	num_vars += Game().NumActionsAtInfoset(pl,i) - 1;
      }
    }
    var_index += list_for_pl;
  }

  // We establish the space
  gSpace BehavStratSpace(num_vars);
  ORD_PTR ptr = &lex;
  term_order Lex(&BehavStratSpace, ptr);

  num_vars = BehavStratSpace.Dmnsn();
  gPolyList<gDouble> inequalities = ExtendsToANFNashIneqs(BehavStratSpace,
							  Lex,
							  little_supp,
							  big_supp,
							  var_index);

  // set up the rectangle of search
  gVector<gDouble> bottoms(num_vars), tops(num_vars);
  bottoms = (gDouble)0;
  tops = (gDouble)1;
  gRectangle<gDouble> Cube(bottoms, tops); 

  // Set up the test and do it
  IneqSolv<gDouble> extension_tester(inequalities,m_status);
  gVector<gDouble> sample(num_vars);

  // Temporarily, we check the old set up vs. the new
  bool ANFanswer = extension_tester.ASolutionExists(Cube,sample); 
  assert (ANFanswer == m_profile->ExtendsToANFNash(little_supp,
						   big_supp,
						   m_status));

  /* 
  bool NASHanswer = m_profile->ExtendsToNash(Support(),Support(),m_status);

  //DEBUG
  if (ANFanswer && !NASHanswer)
    gout << 
      "The following should be extendable to an ANF Nash, but not to a Nash:\n"
	 << *m_profile << "\n\n";
  if (NASHanswer && !ANFanswer)
    gout << 
      "ERROR: said to be extendable to a Nash, but not to an ANF Nash:\n"
	 << *m_profile << "\n\n";
	  */
  return ANFanswer;
}


//------------------------------------------------------------------------
//         BehavSolution: Used in test of extendability to Nash
//------------------------------------------------------------------------

void
BehavSolution::DeviationInfosetsRECURSION(    gList<const Infoset *> &answer,
					    const EFSupport & big_supp,
					    const EFPlayer *pl,
					    const Node* node,
					    const Action *act) const 
{
  Node *child  = node->GetChild(act);
  if ( child->IsNonterminal() ) {
    const Infoset *iset = child->GetInfoset();
    if ( iset->GetPlayer() == pl ) {
      int insert = 0;
      bool done = false;
      while (!done) {
	insert ++;
	if (insert > answer.Length() ||
	    iset->Precedes(answer[insert]->GetMember(1)))
	  done = true;
      }
      answer.Insert(iset,insert);
    }
    gList<Action *> action_list = iset->ListOfActions();
    for (int j = 1; j <= action_list.Length(); j++)
      DeviationInfosetsRECURSION(answer,big_supp,pl,child,action_list[j]);
  }
  return;
}

const gList<const Infoset *> 
BehavSolution::DeviationInfosets(const EFSupport & big_supp,
				   const EFPlayer *pl,
				   const Infoset *iset,
				   const Action *act) const 
{
  gList<const Infoset *> answer;
  
  gList<const Node *> node_list = iset->ListOfMembers();
  for (int i = 1; i <= node_list.Length(); i++) {
    DeviationInfosetsRECURSION(answer,big_supp,pl,node_list[i],act);
  }

  return answer;
}

const gList<const EFSupport> 
BehavSolution::DeviationSupports(const EFSupport & big_supp,
				   const gList<const Infoset *> & isetlist,
				   const EFPlayer */*pl*/,
				   const Infoset */*iset*/,
				   const Action */*act*/) const 
{
  gList<const EFSupport> answer;

  gArray<int> active_act_no(isetlist.Length());

  for (int k = 1; k <= active_act_no.Length(); k++)
    active_act_no[k] = 0;
 
  EFSupport new_supp(big_supp);

  for (int i = 1; i <= isetlist.Length(); i++) {
    for (int j = 1; j < isetlist[i]->NumActions(); j++)
      new_supp.RemoveAction(isetlist[i]->GetAction(j));
    new_supp.AddAction(isetlist[i]->GetAction(1));

    active_act_no[i] = 1;
    for (int k = 1; k < i; k++)
      if (isetlist[k]->Precedes(isetlist[i]->GetMember(1)))
	if (isetlist[k]->GetAction(1)->Precedes(isetlist[i]->GetMember(1))) {
	  new_supp.RemoveAction(isetlist[i]->GetAction(1));
	  active_act_no[i] = 0;
	}
  }
  answer += new_supp;

  int iset_cursor = isetlist.Length();
  while (iset_cursor > 0) {
    if ( active_act_no[iset_cursor] == 0 || 
	 active_act_no[iset_cursor] == isetlist[iset_cursor]->NumActions() )
      iset_cursor--;
    else {
      new_supp.RemoveAction(isetlist[iset_cursor]->
			    GetAction(active_act_no[iset_cursor]));
      active_act_no[iset_cursor]++;
      new_supp.AddAction(isetlist[iset_cursor]->
			 GetAction(active_act_no[iset_cursor]));
      for (int k = iset_cursor + 1; k <= isetlist.Length(); k++) {
	if (active_act_no[k] > 0)
	  new_supp.RemoveAction(isetlist[k]->GetAction(1));
	int h = 1;
	bool active = true;
	while (active && h < k) {
	  if (isetlist[h]->Precedes(isetlist[k]->GetMember(1)))
	    if (active_act_no[h] == 0 || 
		!isetlist[h]->GetAction(active_act_no[h])->
	              Precedes(isetlist[k]->GetMember(1))) {
	      active = false;
	      if (active_act_no[k] > 0) {
		new_supp.RemoveAction(isetlist[k]->
				      GetAction(active_act_no[k]));
		active_act_no[k] = 0;
	      }
	    }
	  h++;
	}
	if (active){
	  new_supp.AddAction(isetlist[k]->GetAction(1));
	  active_act_no[k] = 1;
	}
      }
      answer += new_supp;
    }
  }
  return answer;
}

bool 
BehavSolution::NashNodeProbabilityPoly(      gPoly<gDouble> & node_prob,
		          	         const gSpace &BehavStratSpace, 
				         const term_order &Lex,
				         const EFSupport &dsupp,
				         const gList<gList<int> > &var_index,
				         const Node *tempnode,
					 const EFPlayer */*pl*/,
				         const Infoset *iset,
				         const Action *act) const 
{
  while (tempnode != Game().RootNode()) {

    const Action *last_action = tempnode->GetAction();
    Infoset *last_infoset = last_action->BelongsTo();
    
    if (last_infoset->IsChanceInfoset()) 
      node_prob *= (gDouble)Game().GetChanceProb(last_action);
    else 
      if (dsupp.HasActiveActionAt(last_infoset)) {
	if (last_infoset == iset) {
	  if (act != last_action) {
	    return false;
	  }
	}
	else
	  if (dsupp.ActionIsActive((Action *)last_action)) {
	    if ( last_action->BelongsTo()->GetPlayer() !=
		         act->BelongsTo()->GetPlayer()     ||
		 !act->Precedes(tempnode) )
	    node_prob *= (gDouble)m_profile->GetValue(last_action);
	  }
	  else {
	    return false;
	  }
      }
      else {
	int initial_var_no = 
 var_index[last_infoset->GetPlayer()->GetNumber()][last_infoset->GetNumber()];
	if (last_action->GetNumber() < last_infoset->NumActions()){
	  int varno = initial_var_no + last_action->GetNumber();
	  node_prob *= gPoly<gDouble>(&BehavStratSpace, varno, 1, &Lex);
	}
	else {
	  gPoly<gDouble> factor(&BehavStratSpace, (gDouble)1.0, &Lex);
	  int k;
	  for (k = 1; k < last_infoset->NumActions(); k++)
	    factor -= gPoly<gDouble>(&BehavStratSpace,
				     initial_var_no + k, 1, &Lex);
	  node_prob *= factor;
	}
      } 
    tempnode = tempnode->GetParent();
  }
  return true;
}


gPolyList<gDouble> 
BehavSolution::NashExpectedPayoffDiffPolys(const gSpace &BehavStratSpace, 
				          const term_order &Lex,
					  const EFSupport &little_supp,
					  const EFSupport &big_supp,
				          const gList<gList<int> > &var_index) 
  const
{
  gPolyList<gDouble> answer(&BehavStratSpace, &Lex);

  gList<const Node *> terminal_nodes = Game().TerminalNodes();

  const gArray<EFPlayer *> players = Game().Players();
  for (int pl = 1; pl <= players.Length(); pl++) {
    const gArray<Infoset *> isets_for_pl = players[pl]->Infosets();
    for (int i = 1; i <= isets_for_pl.Length(); i++) {
      if (little_supp.MayReach(isets_for_pl[i])) {
	const gArray<Action *> acts_for_iset = isets_for_pl[i]->Actions();
	for (int j = 1; j <= acts_for_iset.Length(); j++)
	  if ( !little_supp.ActionIsActive(acts_for_iset[j]) ) {
	    gList<const Infoset *> isetlist = DeviationInfosets(big_supp, 
							     players[pl],
				         		     isets_for_pl[i],
							     acts_for_iset[j]);
	    gList<const EFSupport> dsupps = DeviationSupports(big_supp, 
							     isetlist, 
							     players[pl],
				         		     isets_for_pl[i],
							     acts_for_iset[j]);
	    for (int k = 1; k <= dsupps.Length(); k++) {

	    // This will be the utility difference between the
	    // payoff resulting from the profile and deviation to 
	    // the strategy for pl specified by dsupp[k]

	      gPoly<gDouble> next_poly(&BehavStratSpace, &Lex);

	      for (int n = 1; n <= terminal_nodes.Length(); n++) {
		gPoly<gDouble> node_prob(&BehavStratSpace, (gDouble)1.0, &Lex);
		if (NashNodeProbabilityPoly(node_prob,
					    BehavStratSpace,
					    Lex,
					    dsupps[k],
					    var_index,
					    terminal_nodes[n],
					    players[pl],
					    isets_for_pl[i],
					    acts_for_iset[j])) {
		  node_prob *= 
		    (gDouble)Game().Payoff(terminal_nodes[n]->GetOutcome(),pl);
		  next_poly += node_prob;
		}
	      }
	      answer += -next_poly + (gDouble)Payoff(pl);
	    }
	  }
      }
    }
  }
  return answer;
}

gPolyList<gDouble> 
BehavSolution::ExtendsToNashIneqs(const gSpace &BehavStratSpace, 
				    const term_order &Lex,
				    const EFSupport &little_supp,
				    const EFSupport &big_supp,
				    const gList<gList<int> > &var_index) const
{
  gPolyList<gDouble> answer(&BehavStratSpace, &Lex);
  answer += ActionProbsSumToOneIneqs(BehavStratSpace, 
				     Lex, 
				     big_supp, 
				     var_index);

  answer += NashExpectedPayoffDiffPolys(BehavStratSpace, 
				    Lex, 
				    little_supp,
				    big_supp,
				    var_index);
  return answer;
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
  p_file << " Creator:"; DisplayEfgAlgType(p_file, m_creator);
  p_file << " IsNash:" << IsNash();
  p_file << " IsSubgamePerfect:" << IsSubgamePerfect();
  p_file << " IsSequential:" << IsSequential();
  p_file << " LiapValue:" << LiapValue();
  if(m_creator == algorithmEfg_QRE_EFG || m_creator == algorithmEfg_QRE_NFG) {
    p_file << " QreLambda:" << m_qreLambda;
    p_file << " QreValue:" << m_qreValue;
  }
  if (m_beliefs)
    p_file << " Beliefs:" << *m_beliefs;
}

gOutput &operator<<(gOutput &p_file, const BehavSolution &p_solution)
{
  p_solution.Dump(p_file);
  return p_file;
}

SubgamePerfectChecker::SubgamePerfectChecker(const Efg &E, const BehavProfile<gNumber> &s,
					     const gNumber & epsilon)
  : SubgameSolver(1), subgame_number(0), eps(epsilon),  
    isSubgamePerfect(triTRUE), infoset_subgames(E.NumInfosets()), start(s)
{
  MarkedSubgameRoots(E, oldroots);
  gList<Node *> subroots;
  LegalSubgameRoots(E,subroots);
  (start.Game()).MarkSubgames(subroots);
  
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

void SubgamePerfectChecker::SolveSubgame(const Efg &E, const EFSupport &sup,
					 gList<BehavSolution> &solns)
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
  (start.Game()).UnmarkSubgames((start.Game()).RootNode());
  (start.Game()).MarkSubgames(oldroots);
}
