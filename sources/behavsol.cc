//
// FILE: behavsol.cc -- Behavioral strategy solution classes
//
// $Id$
//

#include "behavsol.h"

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
    m_isNash(triUNKNOWN), m_isSubgamePerfect(triUNKNOWN),
    m_isSequential(triUNKNOWN), m_epsilon(0.0),
    m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_beliefs(0), m_regret(0), m_id(0)
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
    m_isNash(triUNKNOWN), m_isSubgamePerfect(triUNKNOWN),
    m_isSequential(triUNKNOWN), m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_beliefs(0), m_regret(0), m_id(0)
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
    m_isNash(triUNKNOWN), m_isSubgamePerfect(triUNKNOWN),
    m_isSequential(triUNKNOWN), m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_beliefs(0), m_regret(0), m_id(0)
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

  if ((*m_profile)[1].Precision() == precDOUBLE)
    m_epsilon = 0.0;
  else
    m_epsilon = 0;
  gEpsilon(m_epsilon);
}

BehavSolution::BehavSolution(const BehavSolution &p_solution)
  : m_profile(new BehavProfile<gNumber>(*p_solution.m_profile)),
    m_precision(p_solution.m_precision), 
    m_support(p_solution.m_support), m_creator(p_solution.m_creator),
    m_isNash(p_solution.m_isNash),
    m_isSubgamePerfect(p_solution.m_isSubgamePerfect),
    m_isSequential(p_solution.m_isSequential), m_epsilon(p_solution.m_epsilon),
    m_qreLambda(p_solution.m_qreLambda),
    m_qreValue(p_solution.m_qreValue),
    m_liapValue(p_solution.m_liapValue),
    m_beliefs(0), m_regret(0), m_id(0)
{
  if (p_solution.m_beliefs) {
    m_beliefs = new gDPVector<gNumber>(*p_solution.m_beliefs);
  }
  if (p_solution.m_regret) {
    m_regret = new gDPVector<gNumber>(*p_solution.m_regret);   
  }
}

BehavSolution::~BehavSolution() 
{ 
  delete m_profile;
  if (m_beliefs) delete m_beliefs;
  if (m_regret)  delete m_regret;
}

BehavSolution& BehavSolution::operator=(const BehavSolution &p_solution)
{
  if (this != &p_solution)   {
    delete m_profile;
    m_profile = new BehavProfile<gNumber>(*p_solution.m_profile);
    m_precision = p_solution.m_precision;
    m_support = p_solution.m_support;
    m_creator = p_solution.m_creator;
    m_isNash = p_solution.m_isNash;
    m_isSubgamePerfect = p_solution.m_isSubgamePerfect;
    m_isSequential = p_solution.m_isSequential;
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
  }

  return *this;
}

//-----------------------------
// Private member functions
//-----------------------------

void BehavSolution::EvalEquilibria(void) const
{
  if (IsComplete() && IsPerfectRecall(m_profile->Game())) {
    if (m_isNash == triUNKNOWN) {
      m_isNash = (m_profile->MaxGripe() <= m_epsilon) ? triTRUE : triFALSE;
    }
  }  
  if (m_isNash == triFALSE) {
    m_isSubgamePerfect = triFALSE;
    m_isSequential = triFALSE;
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
  EvalEquilibria();
  return m_isNash;
}

gTriState BehavSolution::IsSubgamePerfect(void) const
{
  EvalEquilibria();
  return m_isSubgamePerfect;
}

gTriState BehavSolution::IsSequential(void) const
{
  EvalEquilibria();
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
  m_isNash = triUNKNOWN;
  m_isSubgamePerfect = triUNKNOWN;
  m_isSequential = triUNKNOWN;
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
  p_file << " IsNash:" << m_isNash;
  p_file << " IsSubgamePerfect:" << m_isSubgamePerfect;
  p_file << " IsSequential:" << m_isSequential;
  p_file << " QreLambda:" << m_qreLambda;
  p_file << " QreValue:" << m_qreValue;
  p_file << " LiapValue:" << m_liapValue;
  if (m_beliefs)
    p_file << " Beliefs:" << *m_beliefs;
}

gOutput &operator<<(gOutput &p_file, const BehavSolution &p_solution)
{
  p_solution.Dump(p_file);
  return p_file;
}
