//
// FILE: mixedsol.cc -- Mixed strategy solution classe
//
// $Id$
//


#include "mixedsol.h"

gText NameNfgAlgType(NfgAlgType p_algorithm)
{
  switch (p_algorithm) {
  case algorithmNfg_USER:
    return "User"; 
  case algorithmNfg_ENUMPURE:
    return "EnumPure";
  case algorithmNfg_ENUMMIXED:
    return "EnumMixed";
  case algorithmNfg_LCP:
    return "Lcp";
  case algorithmNfg_LP:
    return "Lp";
  case algorithmNfg_LIAP:
    return "Liap";
  case algorithmNfg_SIMPDIV:
    return "Simpdiv";
  case algorithmNfg_POLENUM:
    return "PolEnum";
  case algorithmNfg_QRE:
    return "Qre";
  case algorithmNfg_QREALL:
    return "QreAll";
  }
}

void DisplayNfgAlgType(gOutput &p_file, NfgAlgType p_algorithm)
{
  p_file << NameNfgAlgType(p_algorithm);
}


//----------------------------------------------------
// Constructors, Destructor, Constructive Operators
//----------------------------------------------------

MixedSolution::MixedSolution(const MixedProfile<double> &p_profile,
			     NfgAlgType p_creator)
  : m_profile(NFSupport(p_profile.Game())), m_precision(precDOUBLE),
    m_creator(p_creator), m_isNash(triUNKNOWN), m_isPerfect(triUNKNOWN),
    m_isProper(triUNKNOWN), m_epsilon(0.0),
    m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_id(0)
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    for (int st = 1; st <= Game().NumStrats(pl); st++) {
      int index = p_profile.Support().Find(Game().Strategies(pl)[st]);
      if (index > 0)
	m_profile(pl, st) = p_profile(pl, index);
      else
	m_profile(pl, st) = gNumber(0.0);
    }
  }
}

MixedSolution::MixedSolution(const MixedProfile<gRational> &p_profile,
			     NfgAlgType p_creator)
  : m_profile(NFSupport(p_profile.Game())), m_precision(precRATIONAL),
    m_creator(p_creator), m_isNash(triUNKNOWN), m_isPerfect(triUNKNOWN),
    m_isProper(triUNKNOWN), m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_id(0)
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    for (int st = 1; st <= Game().NumStrats(pl); st++) {
      int index = p_profile.Support().Find(Game().Strategies(pl)[st]);
      if (index > 0)
	m_profile(pl, st) = p_profile(pl, index);
      else
	m_profile(pl, st) = gNumber(0);
    }
  }
}

MixedSolution::MixedSolution(const MixedProfile<gNumber> &p_profile,
			     NfgAlgType p_creator)
  : m_profile(NFSupport(p_profile.Game())), m_precision(precRATIONAL),
    m_creator(p_creator), m_isNash(triUNKNOWN), m_isPerfect(triUNKNOWN),
    m_isProper(triUNKNOWN), m_qreLambda(-1), m_qreValue(-1),
    m_liapValue(-1), m_id(0)
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    for (int st = 1; st <= Game().NumStrats(pl); st++) {
      int index = p_profile.Support().Find(Game().Strategies(pl)[st]);
      if (index > 0)
	m_profile(pl, st) = p_profile(pl, index);
      else
	m_profile(pl, st) = gNumber(0);
    }
  }
  LevelPrecision();
}

MixedSolution::MixedSolution(const MixedSolution &p_solution)
  : m_profile(p_solution.m_profile),
    m_precision(p_solution.m_precision), m_creator(p_solution.m_creator), 
    m_isNash(p_solution.m_isNash), m_isPerfect(p_solution.m_isPerfect),
    m_isProper(p_solution.m_isProper), m_epsilon(p_solution.m_epsilon),
    m_qreLambda(p_solution.m_qreLambda),
    m_qreValue(p_solution.m_qreValue), 
    m_liapValue(p_solution.m_liapValue), m_id(p_solution.m_id)
{ }

MixedSolution::~MixedSolution()
{ }

MixedSolution &MixedSolution::operator=(const MixedSolution &p_solution)
{
  if (this != &p_solution)  {
    m_profile = p_solution.m_profile;
    m_precision = p_solution.m_precision;
    m_creator = p_solution.m_creator;
    m_isNash = p_solution.m_isNash;
    m_isPerfect = p_solution.m_isPerfect;
    m_isProper = p_solution.m_isProper;
    m_epsilon = p_solution.m_epsilon;
    m_qreLambda = p_solution.m_qreLambda;
    m_qreValue = p_solution.m_qreValue;
    m_liapValue = p_solution.m_liapValue;
    m_id = p_solution.m_id;
  }
  return *this;
}


//-----------------------------
// Private member functions
//-----------------------------

void MixedSolution::EvalEquilibria(void) const
{
  if (IsComplete())   {
    if (m_isNash == triUNKNOWN)  
      m_isNash = (m_profile.MaxRegret() <= m_epsilon) ? triTRUE : triFALSE;
  }
  if (m_isNash == triFALSE) {
    m_isPerfect = triFALSE;
    m_isProper = triFALSE;
  }
}

void MixedSolution::LevelPrecision(void)
{
  m_precision = precRATIONAL;
  for (int pl = 1; m_precision == precRATIONAL && pl <= Game().NumPlayers();
       pl++) {
    NFPlayer *player = Game().Players()[pl];  
    for (int st = 1; (m_precision == precRATIONAL && 
		      st <= player->NumStrats()); st++) {
      if (m_profile(pl, st).Precision() == precDOUBLE)
	m_precision = precDOUBLE;
    }
  }

  if (m_precision == precDOUBLE) {
    for (int pl = 1; m_precision == precRATIONAL && pl <= Game().NumPlayers();
	 pl++) {
      NFPlayer *player = Game().Players()[pl];  
      for (int st = 1; (m_precision == precRATIONAL && 
			st <= player->NumStrats()); st++) {
	m_profile(pl, st) = (double) m_profile(pl, st);
      }
    }
  }
}

//------------------------
// Operator overloading
//------------------------

bool MixedSolution::Equals(const MixedProfile<double> &p_profile) const
{ 
  gNumber eps(m_epsilon);
  gEpsilon(eps, 4);   // this should be a function of m_epsilon

  int i = p_profile.First();
  while (i <= p_profile.Length()) {
    if (abs(m_profile[i] - (gNumber) p_profile[i]) > eps) 
      break;
    i++;
  }
  return (i > p_profile.Length());
}

bool MixedSolution::operator==(const MixedSolution &p_solution) const
{ return (m_profile == p_solution.m_profile); }

void MixedSolution::Set(Strategy *p_strategy, const gNumber &p_value)
{ 
  Invalidate();
  m_profile(p_strategy->Player()->GetNumber(), p_strategy->Number()) = p_value;
  if (p_value.Precision() != m_precision)
    LevelPrecision();
}

const gNumber &MixedSolution::operator()(Strategy *p_strategy) const
{
  NFPlayer *player = p_strategy->Player();
  return m_profile(player->GetNumber(), p_strategy->Number()); 
}

MixedSolution &MixedSolution::operator+=(const MixedSolution &p_solution)
{
  Invalidate();
  m_profile += p_solution.m_profile;
  if (m_precision == precRATIONAL && p_solution.m_precision == precDOUBLE)
    m_precision = precDOUBLE;
  return *this;
}

MixedSolution &MixedSolution::operator-=(const MixedSolution &p_solution)
{
  Invalidate();
  m_profile -= p_solution.m_profile; 
  if (m_precision == precRATIONAL && p_solution.m_precision == precDOUBLE)
    m_precision = precDOUBLE;
  return *this;
}

MixedSolution &MixedSolution::operator*=(const gNumber &p_constant)
{ 
  Invalidate(); 
  m_profile *= p_constant;
  if (m_precision == precRATIONAL && p_constant.Precision() == precDOUBLE)
    m_precision = precDOUBLE;
  return *this; 
}


//-----------------------
// General data access
//-----------------------

bool MixedSolution::IsComplete(void) const
{ 
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    gNumber sum = -1;
    for (int st = 1; st <= Game().NumStrats(pl); st++) 
      sum += m_profile(pl, st);
    if (sum > m_epsilon || sum < -m_epsilon) 
      return false;
  }
  return true;
}

gTriState MixedSolution::IsNash(void) const
{
  EvalEquilibria();
  return m_isNash;
}

gTriState MixedSolution::IsPerfect(void) const
{
  EvalEquilibria();
  return m_isPerfect;
}

gTriState MixedSolution::IsProper(void) const
{
  EvalEquilibria();
  return m_isProper;
}

const gNumber &MixedSolution::LiapValue(void) const 
{ 
  if (m_liapValue < (gNumber) 0)
    m_liapValue = m_profile.LiapValue();
  return m_liapValue; 
}

void MixedSolution::Invalidate(void) const
{
  m_creator = algorithmNfg_USER;
  m_isNash = triUNKNOWN;
  m_isProper = triUNKNOWN;
  m_qreLambda = -1;
  m_qreValue = -1;
  m_liapValue = -1;
}

//---------------------
// Payoff computation
//---------------------

gNumber MixedSolution::Payoff(NFPlayer *p_player, Strategy *p_strategy) const
{ return m_profile.Payoff(p_player->GetNumber(), p_strategy); }


//----------
// Output
//----------

void MixedSolution::Dump(gOutput &p_file) const
{
  m_profile.Dump(p_file);
  DumpInfo(p_file);
}

void MixedSolution::DumpInfo(gOutput &p_file) const
{
  p_file << " Creator:"; DisplayNfgAlgType(p_file, m_creator);
  p_file << " IsNash:" << m_isNash;
  p_file << " IsPerfect:" << m_isPerfect;
  p_file << " IsProper:" << m_isProper;
  p_file << " QreLambda:" << m_qreLambda;
  p_file << " QreValue:" << m_qreValue;
  p_file << " LiapValue:" << m_liapValue;
}

gOutput &operator<<(gOutput &p_file, const MixedSolution &p_solution)
{ 
  p_solution.Dump(p_file);
  return p_file;
}
