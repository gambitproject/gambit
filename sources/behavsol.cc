//
// FILE: behavsol.cc -- Behavioral strategy solution classes
//
// $Id$
//

#include "behavsol.h"

gText NameEfgAlgType(EfgAlgType p_algorithm)
{
  switch (p_algorithm) {
  case EfgAlg_USER:
    return "User"; 
  case EfgAlg_GOBIT: // GobitSolve
    return "Gobit"; 
  case EfgAlg_LIAP: // LiapSolve
    return "Liap"; 
  case EfgAlg_PURENASH: // ?
    return "PureNash"; 
  case EfgAlg_SEQFORM: // ?
    return "SeqForm"; 
  case EfgAlg_LEMKESUB: // LcpSolve
    return "LCP(Lemke*)"; 
  case EfgAlg_SIMPDIVSUB: // SimpDivSolve
    return "SimpDiv*"; 
  case EfgAlg_ZSUMSUB: // LpSolve
    return "LP(ZSum*)"; 
  case EfgAlg_PURENASHSUB: // EnumPureSolve
    return "PureNash*"; 
  case EfgAlg_SEQFORMSUB: // LcpSolve
    return "LCP(Seq*)"; 
  case EfgAlg_ELIAPSUB:
    return "EFLiap*"; 
  case EfgAlg_NLIAPSUB:
    return "NFLiap*"; 
  case EfgAlg_ENUMSUB: // EnumSolve
    return "Enum*"; 
  case EfgAlg_CSSEQFORM:
    return "CSSeqForm";
  case EfgAlg_POLENSUB:  // PolEnumSolve
    return "PolEnum*"; 
  case EfgAlg_POLENUM:  // PolEnumSolve
    return "PolEnum"; 
  default:
    return "ERROR" ;
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
    m_creator(p_creator), m_isNash(triUNKNOWN), m_isSubgamePerfect(triUNKNOWN),
    m_isSequential(triUNKNOWN), m_epsilon(0.0),
    m_gobitLambda(-1), m_gobitValue(-1),
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
    m_creator(p_creator), m_isNash(triUNKNOWN), m_isSubgamePerfect(triUNKNOWN),
    m_isSequential(triUNKNOWN), m_gobitLambda(-1), m_gobitValue(-1),
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
    m_creator(p_creator), m_isNash(triUNKNOWN), m_isSubgamePerfect(triUNKNOWN),
    m_isSequential(triUNKNOWN), m_gobitLambda(-1), m_gobitValue(-1),
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

BehavSolution::BehavSolution(const BehavSolution &p_solution)
  : m_profile(new BehavProfile<gNumber>(*p_solution.m_profile)),
    m_creator(p_solution.m_creator), m_isNash(p_solution.m_isNash),
    m_isSubgamePerfect(p_solution.m_isSubgamePerfect),
    m_isSequential(p_solution.m_isSequential), m_epsilon(p_solution.m_epsilon),
    m_gobitLambda(p_solution.m_gobitLambda),
    m_gobitValue(p_solution.m_gobitValue),
    m_liapValue(p_solution.m_liapValue),
    m_beliefs(0), m_regret(0), m_id(0)
{
  if (p_solution.m_beliefs)
    m_beliefs = new gDPVector<gNumber>(*p_solution.m_beliefs);
  if (p_solution.m_regret)
    m_regret = new gDPVector<gNumber>(*p_solution.m_regret);   
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
    *m_profile = *p_solution.m_profile;
    m_creator = p_solution.m_creator;
    m_isNash = p_solution.m_isNash;
    m_isSubgamePerfect = p_solution.m_isSubgamePerfect;
    m_isSequential = p_solution.m_isSequential;
    m_gobitLambda = p_solution.m_gobitLambda;
    m_gobitValue = p_solution.m_gobitValue;
    m_liapValue = p_solution.m_liapValue;
    if (m_beliefs)   delete m_beliefs;
    if (p_solution.m_beliefs)
      m_beliefs = new gDPVector<gNumber>(*p_solution.m_beliefs);
    else
      m_beliefs = 0;
    if (m_regret)   delete m_regret;
    if (p_solution.m_regret)
      m_regret = new gDPVector<gNumber>(*p_solution.m_regret);
    else
      m_regret = 0;
  }

  return *this;
}

//-----------------------------
// Private member functions
//-----------------------------

void BehavSolution::EvalEquilibria(void) const
{
  if (IsComplete()) {
    if (m_isNash == triUNKNOWN)
      m_isNash = (m_profile->MaxGripe() <= m_epsilon) ? triTRUE : triFALSE;
  }  
  if (m_isNash == triFALSE) {
    m_isSubgamePerfect = triFALSE;
    m_isSequential = triFALSE;
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

gNumber &BehavSolution::operator()(int p_player, int p_infoset, int p_action)
{
  Invalidate();
  return (*m_profile)(p_player, p_infoset, p_action);
}

const gNumber &BehavSolution::operator()(int p_player, int p_infoset,
					 int p_action) const
{ return (*m_profile)(p_player, p_infoset, p_action); }

BehavSolution &BehavSolution::operator+=(const BehavSolution &p_solution)
{
  Invalidate(); 
  *m_profile += *p_solution.m_profile;
  return *this;
}

BehavSolution &BehavSolution::operator-=(const BehavSolution &p_solution)
{
  Invalidate();
  *m_profile -= *p_solution.m_profile;
  return *this;
}

BehavSolution &BehavSolution::operator*=(const gNumber &p_constant)
{ 
  Invalidate();
  *m_profile *= p_constant;
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
	sum += (*this)(pl, iset, act);
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
  m_creator = EfgAlg_USER;
  m_isNash = triUNKNOWN;
  m_isSubgamePerfect = triUNKNOWN;
  m_isSequential = triUNKNOWN;
  m_gobitLambda = -1;
  m_gobitValue = -1;
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
  p_file << " Creator:"; DisplayEfgAlgType(p_file, m_creator);
  p_file << " IsNash:" << m_isNash;
  p_file << " IsSubgamePerfect:" << m_isSubgamePerfect;
  p_file << " IsSequential:" << m_isSequential;
  p_file << " GobitLambda:" << m_gobitLambda;
  p_file << " GobitValue:" << m_gobitValue;
  p_file << " LiapValue:" << m_liapValue;
  if (m_beliefs)
    p_file << " Beliefs:" << *m_beliefs;
}

gOutput &operator<<(gOutput &p_file, const BehavSolution &p_solution)
{
  p_solution.Dump(p_file);
  return p_file;
}
