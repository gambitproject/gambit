//
// FILE: nfgsolng.cc -- definition of the class dealing with the GUI part of
//                      the normal form solutions.
//
// $Id$
//

#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "wxstatus.h"
#include "glist.h"
#include "gpoly.h"
#include "nfgsolng.h"
#include "nfgconst.h"
#include "gfunc.h"

//=========================================================================
//                     guiNfgSolution: Member functions
//=========================================================================

guiNfgSolution::guiNfgSolution(const NFSupport &p_support,
			       NfgShowInterface *p_parent)
  : m_nfg(p_support.Game()), m_support(p_support), m_parent(p_parent)
{ }

#include "nfstrat.h"

void guiNfgSolution::Eliminate(void)
{
  if (!m_eliminate)  return;

  gArray<int> players(m_nfg.NumPlayers());
  for (int i = 1; i <= m_nfg.NumPlayers(); i++) 
    players[i] = i;

  if (m_eliminateAll) {
    if (m_eliminateMixed) {
      NFSupport *oldSupport = new NFSupport(m_support), *newSupport;
      while ((newSupport = ComputeMixedDominated(oldSupport->Game(), 
						 *oldSupport, !m_eliminateWeak,
						 players, gnull, gstatus)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      m_support = *oldSupport;
      delete oldSupport;
    }
    else {
      NFSupport *oldSupport = new NFSupport(m_support), *newSupport;
      while ((newSupport = ComputeDominated(oldSupport->Game(), 
					    *oldSupport, !m_eliminateWeak,
					    players, gnull, gstatus)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      m_support = *oldSupport;
      delete oldSupport;
    }
  }
  else {
    if (m_eliminateMixed) {
      NFSupport *newSupport;
      if ((newSupport = ComputeMixedDominated(m_support.Game(), 
					      m_support, !m_eliminateWeak,
					      players, gnull, gstatus)) != 0) {
	m_support = *newSupport;
	delete newSupport;
      }
    }
    else {
      NFSupport *newSupport;
      if ((newSupport = ComputeDominated(m_support.Game(), 
					 m_support, !m_eliminateWeak,
					 players, gnull, gstatus)) != 0) {
	m_support = *newSupport;
	delete newSupport;
      }
    }
  }
}

//=========================================================================
//                     Derived classes, by algorithm
//=========================================================================

//-----------
// EnumPure
//-----------

#include "nfgpure.h"
#include "dlenumpure.h"

guinfgEnumPure::guinfgEnumPure(const NFSupport &p_support,
			       NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgEnumPure::Solve(void)
{
  wxStatus status(m_parent->Frame(), "EnumPureSolve Progress");

  gList<MixedSolution> solutions;

  try {
    FindPureNash(m_support, m_stopAfter, status, solutions);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgEnumPure::SolveSetup(void)
{
  dialogEnumPure dialog(m_parent->Frame());

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    return true;
  }
  else
    return false;
}

//------------
// EnumMixed
//------------

#include "dlenummixed.h"
#include "enum.h"

guinfgEnumMixed::guinfgEnumMixed(const NFSupport &p_support,
				 NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgEnumMixed::Solve(void)
{
  wxEnumStatus status(m_parent->Frame());

  EnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  long npivots;
  double time;
  gList<MixedSolution> solutions;
  try {
    Enum(m_support, params, solutions, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgEnumMixed::SolveSetup(void)
{
  dialogEnumMixed dialog(m_parent->Frame());

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    return true;
  }
  else
    return false;
}

//------------
// LcpSolve
//------------

#include "lemke.h"
#include "seqfprm.h"

guinfgLcp::guinfgLcp(const NFSupport &p_support,
		     NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgLcp::Solve(void)
{
  if (m_nfg.NumPlayers() != 2) {
    wxMessageBox("LCP algorithm only works on 2 player games.",
		 "Algorithm Error");
    return gList<MixedSolution>();
  }

  wxStatus status(m_parent->Frame(), "LcpSolve Progress");

  LemkeParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  int npivots;
  double time;
  gList<MixedSolution> solutions;
  try {
    Lemke(m_support, params, solutions, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgLcp::SolveSetup(void)
{
  LcpSolveDialog dialog(m_parent->Frame());

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();

    return true;
  }
  else
    return false;
}

//----------
// LpSolve
//----------

#include "dllp.h"
#include "nfgcsum.h"

guinfgLp::guinfgLp(const NFSupport &p_support, NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgLp::Solve(void)
{
  if (m_nfg.NumPlayers() > 2 || !IsConstSum(m_nfg)) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return gList<MixedSolution>();
  }

  wxStatus status(m_parent->Frame(), "LP Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  ZSumParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  int npivots;
  double time;
  gList<MixedSolution> solutions;
  try {
    ZSum(m_support, params, solutions, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgLp::SolveSetup(void)
{
  dialogLp dialog(m_parent->Frame());
  
  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    return true;
  }
  else
    return false;
}

//--------
// Liap
//--------

#include "nliap.h"
#include "liapprm.h"

guinfgLiap::guinfgLiap(const NFSupport &p_support, NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgLiap::Solve(void)
{
  wxStatus status(m_parent->Frame(), "Liap Algorithm");
  NFLiapParams params(status);
  params.tol1 = m_tol1D;
  params.tolN = m_tolND;
  params.maxits1 = m_maxits1D;
  params.maxitsN = m_maxitsND;
  params.nTries = m_nTries;

  MixedProfile<gNumber> start(m_parent->CreateStartProfile(m_startOption));
  long nevals, nits;
  gList<MixedSolution> solutions;
  try {
    Liap(m_nfg, params, start, solutions, nevals, nits);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgLiap::SolveSetup(void)
{
  LiapSolveParamsDialog dialog(m_parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_tol1D = dialog.Tol1D();
    m_tolND = dialog.TolND();
    m_maxits1D = dialog.Maxits1D();
    m_maxitsND = dialog.MaxitsND();
    m_nTries = dialog.NumTries();
    m_startOption = dialog.StartOption();

    return true;
  }
  else
    return false;
}

//----------
// Simpdiv
//----------

#include "simpdiv.h"
#include "simpprm.h"

guinfgSimpdiv::guinfgSimpdiv(const NFSupport &p_support,
			     NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgSimpdiv::Solve(void)
{
  wxStatus status(m_parent->Frame(), "SimpdivSolve Progress");
  SimpdivParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.nRestarts = m_nRestarts;
  params.leashLength = m_leashLength;

  int nevals, niters;
  double time;
  gList<MixedSolution> solutions;

  try {
    Simpdiv(m_support, params, solutions, nevals, niters, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgSimpdiv::SolveSetup(void)
{
  SimpdivSolveParamsDialog dialog(m_parent->Frame());

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    m_nRestarts = dialog.NumRestarts();
    m_leashLength = dialog.LeashLength();

    return true;
  }
  else
    return false;
}

//----------
// PolEnum
//----------

#include "peprm.h"
#include "polenum.h"

guinfgPolEnum::guinfgPolEnum(const NFSupport &p_support,
			     NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgPolEnum::Solve(void)
{
  wxStatus status(m_parent->Frame(), "PolEnumSolve Algorithm");
  PolEnumParams params(status);
  params.stopAfter = m_stopAfter;

  long nevals;
  double time;
  gList<MixedSolution> solutions;
  try {
    PolEnum(m_support, params, solutions, nevals, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgPolEnum::SolveSetup(void)
{
  guiPolEnumParamsDialog dialog(m_parent->Frame());
  
  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    return true;
  }
  else
    return false;
}

//----------
// Qre
//----------

#include "ngobit.h"
#include "gobitprm.h"

guinfgQre::guinfgQre(const NFSupport &p_support, NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgQre::Solve(void)
{
  wxStatus status(m_parent->Frame(), "QreSolve Progress");
  NFQreParams params(status);

  MixedProfile<gNumber> start(m_parent->CreateStartProfile(m_startOption));

  long nevals, nits;
  gList<MixedSolution> solutions;
  try {
    Qre(m_nfg, params, start, solutions, nevals, nits);
    //GSPD.RunPxi();
  }
  catch (gSignalBreak &) { }

  return solutions;
}

bool guinfgQre::SolveSetup(void)
{
  QreSolveParamsDialog dialog(m_parent->Frame(), m_parent->Filename());

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_startOption = dialog.StartOption();

    return true;
  }
  else
    return false;
}

//-------------
// QreGrid
//-------------

#include "grid.h"
#include "gridprm.h"

guinfgQreAll::guinfgQreAll(const NFSupport &p_support,
			   NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgQreAll::Solve(void)
{
  try {
    wxStatus status(m_parent->Frame(), "QreAllSolve Progress");
    GridParams params(status);
    gList<MixedSolution> solutions;
    GridSolve(m_support, params, solutions);
    //GSPD.RunPxi();
    return solutions;
  }
  catch (gSignalBreak &) {
    return gList<MixedSolution>();
  }
}

bool guinfgQreAll::SolveSetup(void)
{
  GridSolveParamsDialog dialog(m_parent->Frame(), m_parent->Filename());

  if (dialog.Completed() == wxOK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    return true;
  }
  else
    return false;
}



