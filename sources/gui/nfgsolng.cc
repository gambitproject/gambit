//
// FILE: nfgsolng.cc -- definition of the class dealing with the GUI part of
//                      the normal form solutions.
//
// $Id$
//

#include "wx/wx.h"
#include "guishare/wxmisc.h"
#include "guishare/wxstatus.h"
#include "base/base.h"
#include "gnullstatus.h"
#include "gpoly.h"
#include "nfgsolng.h"
#include "nfgconst.h"
#include "gfunc.h"

//=========================================================================
//                     guiNfgSolution: Member functions
//=========================================================================

guiNfgSolution::guiNfgSolution(NfgShow *p_parent)
  : m_parent(p_parent), m_traceFile(&gnull), m_traceLevel(0)
{ }

#include "nfstrat.h"
#include "nfdom.h"

NFSupport guiNfgSolution::Eliminate(const NFSupport &p_support)
{
  if (!m_eliminate) {
    return p_support;
  }

  NFSupport ret(p_support);
  const Nfg &nfg = p_support.Game();

  gArray<int> players(nfg.NumPlayers());
  for (int i = 1; i <= nfg.NumPlayers(); i++) 
    players[i] = i;

  if (m_eliminateAll) {
    if (m_eliminateMixed) {
      NFSupport *oldSupport = new NFSupport(ret), *newSupport;
      gNullStatus status;
      while ((newSupport = oldSupport->MixedUndominated(!m_eliminateWeak,
							precRATIONAL,
							players, gnull, status)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      ret = *oldSupport;
      delete oldSupport;
    }
    else {
      NFSupport *oldSupport = new NFSupport(ret), *newSupport;
      gNullStatus status;
      while ((newSupport = oldSupport->Undominated(!m_eliminateWeak,
						   players, gnull, status)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      ret = *oldSupport;
      delete oldSupport;
    }
  }
  else {
    if (m_eliminateMixed) {
      NFSupport *newSupport;
      gNullStatus status;
      if ((newSupport = ret.MixedUndominated(!m_eliminateWeak,
					     precRATIONAL,
					     players, gnull, status)) != 0) {
	ret = *newSupport;
	delete newSupport;
      }
    }
    else {
      NFSupport *newSupport;
      gNullStatus status;
      if ((newSupport = ret.Undominated(!m_eliminateWeak,
					players, gnull, status)) != 0) {
	ret = *newSupport;
	delete newSupport;
      }
    }
  }

  return ret;
}

//=========================================================================
//                     Derived classes, by algorithm
//=========================================================================

//------------
// LcpSolve
//------------

#include "dllcp.h"
#include "lemke.h"

guinfgLcp::guinfgLcp(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

guinfgLcp::guinfgLcp(NfgShow *p_parent,
		     int p_stopAfter, gPrecision p_precision, 
		     bool p_eliminateWeak)
  : guiNfgSolution(p_parent),
    m_stopAfter(p_stopAfter), m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgLcp::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "LcpSolve Progress");

  LemkeParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  int npivots;
  double time;
  gList<MixedSolution> solutions;
  try {
    Lemke(p_support, params, solutions, status, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgLcp::SolveSetup(void)
{
  dialogLcp dialog(m_parent, false, true);

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//--------
// Liap
//--------

#include "dlliap.h"
#include "nliap.h"

guinfgLiap::guinfgLiap(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

guinfgLiap::guinfgLiap(NfgShow *p_parent,
		       int p_stopAfter, int p_nTries, bool p_eliminateWeak)
  : guiNfgSolution(p_parent),
    m_nTries(p_nTries), m_stopAfter(p_stopAfter)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;

  m_accuracy = 1.0e-4;
  m_startOption = 0;
}

gList<MixedSolution> guinfgLiap::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "Liap Algorithm");
  NFLiapParams params;
  params.stopAfter = m_stopAfter;
  params.SetAccuracy(m_accuracy);
  params.nTries = m_nTries;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  MixedProfile<gNumber> start(p_support);
  long nevals, nits;
  gList<MixedSolution> solutions;
  try {
    Liap(p_support.Game(), params, start, solutions, status, nevals, nits);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgLiap::SolveSetup(void)
{
  dialogLiap dialog(m_parent, false, true);

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    m_accuracy = pow(10, -dialog.Accuracy());
    m_nTries = dialog.NumTries();
    m_startOption = dialog.StartOption();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//----------
// Simpdiv
//----------

#include "simpdiv.h"
#include "dlsimpdiv.h"

guinfgSimpdiv::guinfgSimpdiv(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

guinfgSimpdiv::guinfgSimpdiv(NfgShow *p_parent,
			     int p_stopAfter, gPrecision p_precision, 
			     bool p_eliminateWeak)
  : guiNfgSolution(p_parent),
    m_stopAfter(p_stopAfter), m_nRestarts(36), m_leashLength(0),
    m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgSimpdiv::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "SimpdivSolve Progress");
  SimpdivParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.nRestarts = m_nRestarts;
  params.leashLength = m_leashLength;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  int nevals, niters;
  double time;
  gList<MixedSolution> solutions;

  try {
    Simpdiv(p_support, params, solutions, status, nevals, niters, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgSimpdiv::SolveSetup(void)
{
  dialogSimpdiv dialog(m_parent);

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();
    m_precision = dialog.Precision();
    m_nRestarts = dialog.NumRestarts();
    m_leashLength = dialog.LeashLength();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//----------
// PolEnum
//----------

#include "dlpolenum.h"
#include "nfgalleq.h"

guinfgPolEnum::guinfgPolEnum(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

guinfgPolEnum::guinfgPolEnum(NfgShow *p_parent,
			     int p_stopAfter, bool p_eliminateWeak)
  : guiNfgSolution(p_parent), m_stopAfter(p_stopAfter)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgPolEnum::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "PolEnumSolve Progress");
  PolEnumParams params;
  params.stopAfter = m_stopAfter;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  long nevals;
  double time;
  gList<MixedSolution> solutions;
  gList<const NFSupport> singular_supports;

  try {
    AllNashSolve(p_support, params, solutions, status,
		 nevals, time, singular_supports);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgPolEnum::SolveSetup(void)
{
  dialogPolEnum dialog(m_parent, false, true);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = dialog.StopAfter();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}

//----------
// Qre
//----------

#include "dlqre.h"
#include "nfgqre.h"

guinfgQre::guinfgQre(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

gList<MixedSolution> guinfgQre::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "QreSolve Progress");
  QreNfg qre;
  qre.SetMaxLambda(m_maxLam);
  Correspondence<double, MixedSolution> qreCorresp;

  try {
    qre.Solve(p_support.Game(), *m_pxiFile, status, qreCorresp);
  }
  catch (gSignalBreak &) { }

  gList<MixedSolution> solutions;
  for (int i = 1; i <= qreCorresp.NumPoints(1); i++) {
    solutions.Append(qreCorresp.GetPoint(1, i));
  }

  if (m_runPxi) {
    if (!wxExecute((char *) (m_pxiCommand + " " + m_pxiFilename))) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  return solutions;
}

bool guinfgQre::SolveSetup(void)
{
#ifdef UNUSED
  dialogQre dialog(m_parent, m_parent->Filename().c_str(), true);

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_startOption = dialog.StartOption();
    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_accuracy = pow(10, -dialog.Accuracy());
    m_powLam = (dialog.LinearPlot()) ? 0 : 1;

    m_pxiFile = dialog.PxiFile();
    m_pxiFilename = dialog.PxiFilename();
    m_runPxi = dialog.RunPxi();
    m_pxiCommand = dialog.PxiCommand();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
#endif  // UNUSED
  return false;
}

//-------------
// QreGrid
//-------------

#include "dlqregrid.h"
#include "nfgqregrid.h"

guinfgQreAll::guinfgQreAll(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

gList<MixedSolution> guinfgQreAll::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "QreGridSolve Progress");

  QreNfgGrid qre;
  qre.SetMinLambda(m_minLam);
  qre.SetMaxLambda(m_maxLam);
  qre.SetDelLambda(m_delLam);
  qre.SetTol1(m_tol1);
  qre.SetTol2(m_tol2);
  qre.SetDelP1(m_delp1);
  qre.SetDelP2(m_delp2);
  qre.SetPowLambda(m_powLam);

  gList<MixedSolution> solutions;
  try {
    qre.Solve(p_support, *m_pxiFile, status, solutions);
  }
  catch (gSignalBreak &) { }
  
  if (m_runPxi) {
    if (!wxExecute((char *) (m_pxiCommand + " " + m_pxiFilename))) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }
  return solutions;
}

bool guinfgQreAll::SolveSetup(void)
{
  dialogQreGrid dialog(m_parent, m_parent->Filename().c_str());

  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_minLam = dialog.MinLam();
    m_maxLam = dialog.MaxLam();
    m_delLam = dialog.DelLam();
    m_tol1 = dialog.Tol1();
    m_tol2 = dialog.Tol2();
    m_delp1 = dialog.DelP1();
    m_delp2 = dialog.DelP2();
    m_powLam = (dialog.LinearPlot()) ? 0 : 1;
    m_multiGrid = dialog.MultiGrid();

    m_pxiFile = dialog.PxiFile();
    m_pxiFilename = dialog.PxiFilename();
    m_runPxi = dialog.RunPxi();
    m_pxiCommand = dialog.PxiCommand();

    m_traceFile = dialog.TraceFile();
    m_traceLevel = dialog.TraceLevel();
    return true;
  }
  else
    return false;
}



