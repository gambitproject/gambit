//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Solution class for mixed strategies
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
#include "mixedsol.h"
#include "game/nfdom.h"
#include "nash/polenum.h"  

//----------------------------------------------------
// Constructors, Destructor, Constructive Operators
//----------------------------------------------------

MixedSolution::MixedSolution(const MixedProfile<double> &p_profile,
			     const gText &p_creator)
  : m_profile(gbtNfgSupport(p_profile.Game())), m_precision(precDOUBLE),
    m_support(p_profile.Support()), 
    m_creator(p_creator), m_Nash(), m_Perfect(), m_Proper(), 
    m_liapValue(), m_epsilon(0.0), m_qreLambda(-1), m_qreValue(-1),
    m_revision(p_profile.Game().RevisionNumber())
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    for (int st = 1; st <= Game().NumStrats(pl); st++) {
      int index = p_profile.Support().GetIndex(Game().Strategies(pl)[st]);
      if (index > 0)
	m_profile(pl, st) = p_profile(pl, index);
      else
	m_profile(pl, st) = gNumber(0.0);
    }
  }
}

MixedSolution::MixedSolution(const MixedProfile<gRational> &p_profile,
			     const gText &p_creator)
  : m_profile(gbtNfgSupport(p_profile.Game())), m_precision(precRATIONAL),
    m_support(p_profile.Support()),
    m_creator(p_creator), m_Nash(), m_Perfect(), m_Proper(), 
    m_liapValue(), m_qreLambda(-1), m_qreValue(-1),
    m_revision(p_profile.Game().RevisionNumber())
{
  gEpsilon(m_epsilon);
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    for (int st = 1; st <= Game().NumStrats(pl); st++) {
      int index = p_profile.Support().GetIndex(Game().Strategies(pl)[st]);
      if (index > 0)
	m_profile(pl, st) = p_profile(pl, index);
      else
	m_profile(pl, st) = gNumber(0);
    }
  }
}

MixedSolution::MixedSolution(const MixedProfile<gNumber> &p_profile,
			     const gText &p_creator)
  : m_profile(gbtNfgSupport(p_profile.Game())), m_precision(precRATIONAL),
    m_support(p_profile.Support()),
    m_creator(p_creator), m_Nash(), m_Perfect(), m_Proper(), 
    m_liapValue(), m_qreLambda(-1), m_qreValue(-1),
    m_revision(p_profile.Game().RevisionNumber())
{
  for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
    for (int st = 1; st <= Game().NumStrats(pl); st++) {
      int index = p_profile.Support().GetIndex(Game().Strategies(pl)[st]);
      if (index > 0)
	m_profile(pl, st) = p_profile(pl, index);
      else
	m_profile(pl, st) = gNumber(0);
    }
  }
  LevelPrecision();
  if (m_profile[1].Precision() == precDOUBLE)
    m_epsilon = 0.0;
  else
    m_epsilon = 0;
  gEpsilon(m_epsilon);
}

MixedSolution::MixedSolution(const MixedSolution &p_solution)
  : m_profile(p_solution.m_profile), m_precision(p_solution.m_precision),
    m_support(p_solution.m_support), m_creator(p_solution.m_creator), 
    m_Nash(p_solution.m_Nash), m_Perfect(p_solution.m_Perfect),
    m_Proper(p_solution.m_Proper), 
    m_liapValue(p_solution.m_liapValue), 
    m_epsilon(p_solution.m_epsilon),
    m_qreLambda(p_solution.m_qreLambda),
    m_qreValue(p_solution.m_qreValue), 
    m_name(p_solution.m_name), 
    m_revision(p_solution.m_revision)
{ }

MixedSolution::~MixedSolution()
{ }

MixedSolution &MixedSolution::operator=(const MixedSolution &p_solution)
{
  if (this != &p_solution)  {
    m_profile = p_solution.m_profile;
    m_precision = p_solution.m_precision;
    m_support = p_solution.m_support;
    m_creator = p_solution.m_creator;
    m_Nash = p_solution.m_Nash;
    m_Perfect = p_solution.m_Perfect;
    m_Proper = p_solution.m_Proper;
    m_liapValue = p_solution.m_liapValue;
    m_epsilon = p_solution.m_epsilon;
    m_qreLambda = p_solution.m_qreLambda;
    m_qreValue = p_solution.m_qreValue;
    m_name = p_solution.m_name;
    m_revision = p_solution.m_revision;
  }
  return *this;
}


//-----------------------------
// Private member functions
//-----------------------------

gTriState MixedSolution::GetNash(void) const
{
  gTriState answer;
  if(IsComplete())
    answer = (m_profile.MaxRegret() <= m_epsilon) ? triTRUE : triFALSE;
  else
    answer =  triFALSE;
  if (answer == triFALSE) {
    m_Perfect.Set(triFALSE);
    m_Proper.Set(triFALSE);
  }
  return answer;
}

gTriState MixedSolution::GetPerfect(void) const
{
  if (IsNash()) {
    gNullOutput gnull;
    if (IsMixedDominated(m_profile,false,m_precision,gnull)) {
      return triFALSE;
    }
    else if (Game().NumPlayers()==2) {
      return triTRUE;
    }
  }
  return triUNKNOWN;
}

gTriState MixedSolution::GetProper(void) const
{
  return triUNKNOWN;
}

void MixedSolution::LevelPrecision(void)
{
  m_precision = precRATIONAL;
  m_epsilon = 0;
  for (int pl = 1; m_precision == precRATIONAL && pl <= Game().NumPlayers();
       pl++) {
    gbtNfgPlayer player = Game().GetPlayer(pl);
    for (int st = 1; (m_precision == precRATIONAL && 
		      st <= player.NumStrategies()); st++) {
      if (m_profile(pl, st).Precision() == precDOUBLE) {
	m_precision = precDOUBLE;
	m_epsilon = 0.0;
	gEpsilon(m_epsilon);
      }
    }
  }

  if (m_precision == precDOUBLE) {
    for (int pl = 1; pl <= Game().NumPlayers(); pl++) {
      gbtNfgPlayer player = Game().GetPlayer(pl);
      for (int st = 1; st <= player.NumStrategies(); st++) 
	m_profile(pl, st) = (double) m_profile(pl, st);
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
  m_profile(p_strategy->GetPlayer().GetId(), p_strategy->Number()) = p_value;
  if (p_value.Precision() != m_precision)
    LevelPrecision();
}

const gNumber &MixedSolution::operator()(Strategy *p_strategy) const
{
  gbtNfgPlayer player = p_strategy->GetPlayer();
  return m_profile(player.GetId(), p_strategy->Number()); 
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

const gTriState &MixedSolution::IsNash(void) const
{
  CheckIsValid();
  if(!m_Nash.Checked())
    m_Nash.Set(GetNash());
  return m_Nash.Answer();
}

MixedSolution MixedSolution::PolishEq(void) const
{
  bool is_singular = false;
  return PolishEquilibrium(m_support,*this,is_singular);
}

const gTriState &MixedSolution::IsPerfect(void) const
{
  CheckIsValid();
  if(!m_Perfect.Checked())
    m_Perfect.Set(GetPerfect());
  return m_Perfect.Answer();
}

const gTriState &MixedSolution::IsProper(void) const
{
  CheckIsValid();
  if(!m_Proper.Checked())
    m_Proper.Set(GetProper());
  return m_Proper.Answer();
}

const gNumber &MixedSolution::LiapValue(void) const 
{ 
  CheckIsValid();
  if(!m_liapValue.Checked())
    m_liapValue.Set(m_profile.LiapValue());
  return m_liapValue.Answer();
}

void MixedSolution::Invalidate(void) const
{
  m_support = gbtNfgSupport(m_profile.Game());
  m_creator = "User";
  m_Nash.Invalidate();
  m_Perfect.Invalidate();
  m_Proper.Invalidate();
  m_liapValue.Invalidate();
  m_qreLambda = -1;
  m_qreValue = -1;
  m_revision = Game().RevisionNumber();
}

//---------------------
// Payoff computation
//---------------------

gNumber MixedSolution::Payoff(gbtNfgPlayer p_player, 
			      Strategy *p_strategy) const
{ return m_profile.Payoff(p_player.GetId(), p_strategy); }


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
  p_file << " Creator:" << Creator();
  p_file << " IsNash:" << IsNash();
  p_file << " IsPerfect:" << IsPerfect();
  p_file << " IsProper:" << IsProper();
  p_file << " LiapValue:" << LiapValue();
  if (m_creator == "Qre[NFG]" || m_creator == "QreGrid[NFG]") {
    p_file << " QreLambda:" << m_qreLambda;
    p_file << " QreValue:" << m_qreValue;
  }
}

gOutput &operator<<(gOutput &p_file, const MixedSolution &p_solution)
{ 
  p_solution.Dump(p_file);
  return p_file;
}

#include "base/glist.imp"
template class gList<MixedSolution>;
