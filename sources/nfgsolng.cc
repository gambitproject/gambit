//
// FILE: nfgsolng.cc -- definition of the class dealing with the GUI part of
//                      the normal form solutions.
//
// $Id$
//

#include "wx/wx.h"
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
      while ((newSupport = oldSupport->MixedUndominated(!m_eliminateWeak,
							precRATIONAL,
							players, gnull, gstatus)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      ret = *oldSupport;
      delete oldSupport;
    }
    else {
      NFSupport *oldSupport = new NFSupport(ret), *newSupport;
      while ((newSupport = oldSupport->Undominated(!m_eliminateWeak,
						   players, gnull, gstatus)) != 0) {
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
      if ((newSupport = ret.MixedUndominated(!m_eliminateWeak,
					     precRATIONAL,
					     players, gnull, gstatus)) != 0) {
	ret = *newSupport;
	delete newSupport;
      }
    }
    else {
      NFSupport *newSupport;
      if ((newSupport = ret.Undominated(!m_eliminateWeak,
					players, gnull, gstatus)) != 0) {
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

//-----------
// EnumPure
//-----------

#include "nfgpure.h"
#include "dlenumpure.h"

guinfgEnumPure::guinfgEnumPure(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

gList<MixedSolution> guinfgEnumPure::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "EnumPureSolve Progress");

  gList<MixedSolution> solutions;

  try {
    nfgEnumPure solver;
    solver.SetStopAfter(m_stopAfter);
    solver.Solve(p_support, status, solutions);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgEnumPure::SolveSetup(void)
{
  dialogEnumPure dialog(m_parent, false, true);

  if (dialog.ShowModal() == wxID_OK) {
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

guinfgEnumMixed::guinfgEnumMixed(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

guinfgEnumMixed::guinfgEnumMixed(NfgShow *p_parent,
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

gList<MixedSolution> guinfgEnumMixed::Solve(const NFSupport &p_support)
{
  wxEnumStatus status(m_parent);

  EnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  long npivots;
  double time;
  gList<MixedSolution> solutions;
  try {
    Enum(p_support, params, solutions, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgEnumMixed::SolveSetup(void)
{
  dialogEnumMixed dialog(m_parent);

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

  LemkeParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  int npivots;
  double time;
  gList<MixedSolution> solutions;
  try {
    Lemke(p_support, params, solutions, npivots, time);
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

//----------
// LpSolve
//----------

#include "dllp.h"
#include "nfgcsum.h"

guinfgLp::guinfgLp(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

guinfgLp::guinfgLp(NfgShow *p_parent,
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

gList<MixedSolution> guinfgLp::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "LpSolve Progress");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  ZSumParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  int npivots;
  double time;
  gList<MixedSolution> solutions;
  try {
    ZSum(p_support, params, solutions, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgLp::SolveSetup(void)
{
  dialogLp dialog(m_parent, false, true);
  
  if (dialog.ShowModal() == wxID_OK) {
    m_eliminate = dialog.Eliminate();
    m_eliminateAll = dialog.EliminateAll();
    m_eliminateWeak = dialog.EliminateWeak();
    m_eliminateMixed = dialog.EliminateMixed();

    m_stopAfter = 1;
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
  NFLiapParams params(status);
  params.stopAfter = m_stopAfter;
  params.SetAccuracy(m_accuracy);
  params.nTries = m_nTries;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  MixedProfile<gNumber> start(p_support);
  long nevals, nits;
  gList<MixedSolution> solutions;
  try {
    Liap(p_support.Game(), params, start, solutions, nevals, nits);
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
  SimpdivParams params(status);
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
    Simpdiv(p_support, params, solutions, nevals, niters, time);
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
  wxPolEnumStatus status(m_parent);
  PolEnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  long nevals;
  double time;
  gList<MixedSolution> solutions;
  gList<const NFSupport> singular_supports;

  try {
    AllNashSolve(p_support, params, solutions, nevals, time, singular_supports);
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
#include "ngobit.h"

guinfgQre::guinfgQre(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

gList<MixedSolution> guinfgQre::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "QreSolve Progress");
  NFQreParams params(status);
  params.minLam = m_minLam;
  params.maxLam = m_maxLam;
  params.delLam = m_delLam;
  params.SetAccuracy(m_accuracy);
  params.powLam = m_powLam;
  params.pxifile = m_pxiFile;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  MixedProfile<gNumber> start(p_support);

  long nevals, nits;
  gList<MixedSolution> solutions;
  try {
    Qre(p_support.Game(), params, start, solutions, nevals, nits);
  }
  catch (gSignalBreak &) { }

  if (m_runPxi) {
    if (!wxExecute((char *) (m_pxiCommand + " " + m_pxiFilename))) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  return solutions;
}

bool guinfgQre::SolveSetup(void)
{
  dialogQre dialog(m_parent, m_parent->Filename(), true);

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
}

//-------------
// QreGrid
//-------------

#include "dlqregrid.h"
#include "grid.h"

guinfgQreAll::guinfgQreAll(NfgShow *p_parent)
  : guiNfgSolution(p_parent)
{ }

gList<MixedSolution> guinfgQreAll::Solve(const NFSupport &p_support)
{
  wxStatus status(m_parent, "QreGridSolve Progress");

  GridParams params(status);
  params.minLam = m_minLam;
  params.maxLam = m_maxLam;
  params.delLam = m_delLam;
  params.tol1 = m_tol1;
  params.tol2 = m_tol2;
  params.delp1 = m_delp1;
  params.delp2 = m_delp2;
  params.powLam = m_powLam;
  params.multi_grid = m_multiGrid;
  params.pxifile = m_pxiFile;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  gList<MixedSolution> solutions;
  try {
    GridSolve(p_support, params, solutions);
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
  dialogQreGrid dialog(m_parent, m_parent->Filename());

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



