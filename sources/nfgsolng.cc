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
  : m_nfg(p_support.Game()), m_support(p_support), m_parent(p_parent),
    m_traceFile(&gnull), m_traceLevel(0)
{ }

#include "nfstrat.h"
#include "nfdom.h"

void guiNfgSolution::Eliminate(void)
{
  if (!m_eliminate)  return;

  gArray<int> players(m_nfg.NumPlayers());
  for (int i = 1; i <= m_nfg.NumPlayers(); i++) 
    players[i] = i;

  if (m_eliminateAll) {
    if (m_eliminateMixed) {
      NFSupport *oldSupport = new NFSupport(m_support), *newSupport;
      while ((newSupport = oldSupport->MixedUndominated(!m_eliminateWeak,
							precRATIONAL,
							players, gnull, gstatus)) != 0) {
	delete oldSupport;
	oldSupport = newSupport;
      }
      
      m_support = *oldSupport;
      delete oldSupport;
    }
    else {
      NFSupport *oldSupport = new NFSupport(m_support), *newSupport;
      while ((newSupport = oldSupport->Undominated(!m_eliminateWeak,
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
      if ((newSupport = m_support.MixedUndominated(!m_eliminateWeak,
						   precRATIONAL,
						   players, gnull, gstatus)) != 0) {
	m_support = *newSupport;
	delete newSupport;
      }
    }
    else {
      NFSupport *newSupport;
      if ((newSupport = m_support.Undominated(!m_eliminateWeak,
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
  dialogEnumPure dialog(m_parent->Frame(), false, true);

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

guinfgEnumMixed::guinfgEnumMixed(const NFSupport &p_support,
				 NfgShowInterface *p_parent,
				 int p_stopAfter, gPrecision p_precision, 
				 bool p_eliminateWeak)
  : guiNfgSolution(p_support, p_parent),
    m_stopAfter(p_stopAfter), m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgEnumMixed::Solve(void)
{
  wxEnumStatus status(m_parent->Frame());

  EnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

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

guinfgLcp::guinfgLcp(const NFSupport &p_support,
		     NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

guinfgLcp::guinfgLcp(const NFSupport &p_support, NfgShowInterface *p_parent,
		     int p_stopAfter, gPrecision p_precision, 
		     bool p_eliminateWeak)
  : guiNfgSolution(p_support, p_parent),
    m_stopAfter(p_stopAfter), m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgLcp::Solve(void)
{
  wxStatus status(m_parent->Frame(), "LcpSolve Progress");

  LemkeParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

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
  dialogLcp dialog(m_parent->Frame(), false, true);

  if (dialog.Completed() == wxOK) {
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

guinfgLp::guinfgLp(const NFSupport &p_support, NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

guinfgLp::guinfgLp(const NFSupport &p_support, NfgShowInterface *p_parent,
		   int p_stopAfter, gPrecision p_precision, 
		   bool p_eliminateWeak)
  : guiNfgSolution(p_support, p_parent),
    m_stopAfter(p_stopAfter), m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgLp::Solve(void)
{
  wxStatus status(m_parent->Frame(), "LpSolve Progress");
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
    ZSum(m_support, params, solutions, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgLp::SolveSetup(void)
{
  dialogLp dialog(m_parent->Frame(), false, true);
  
  if (dialog.Completed() == wxOK) {
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

guinfgLiap::guinfgLiap(const NFSupport &p_support, NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

guinfgLiap::guinfgLiap(const NFSupport &p_support, NfgShowInterface *p_parent,
		       int p_stopAfter, int p_nTries, bool p_eliminateWeak)
  : guiNfgSolution(p_support, p_parent),
    m_nTries(p_nTries), m_stopAfter(p_stopAfter)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;

  m_accuracy = 1.0e-4;
  m_startOption = 0;
}

gList<MixedSolution> guinfgLiap::Solve(void)
{
  wxStatus status(m_parent->Frame(), "Liap Algorithm");
  NFLiapParams params(status);
  params.stopAfter = m_stopAfter;
  params.SetAccuracy(m_accuracy);
  params.nTries = m_nTries;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

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
  dialogLiap dialog(m_parent->Frame(), false, true);

  if (dialog.Completed() == wxOK) {
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

guinfgSimpdiv::guinfgSimpdiv(const NFSupport &p_support,
			     NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

guinfgSimpdiv::guinfgSimpdiv(const NFSupport &p_support,
			     NfgShowInterface *p_parent,
			     int p_stopAfter, gPrecision p_precision, 
			     bool p_eliminateWeak)
  : guiNfgSolution(p_support, p_parent),
    m_stopAfter(p_stopAfter), m_nRestarts(36), m_leashLength(0),
    m_precision(p_precision)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgSimpdiv::Solve(void)
{
  wxStatus status(m_parent->Frame(), "SimpdivSolve Progress");
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
    Simpdiv(m_support, params, solutions, nevals, niters, time);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgSimpdiv::SolveSetup(void)
{
  dialogSimpdiv dialog(m_parent->Frame());

  if (dialog.Completed() == wxOK) {
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

guinfgPolEnum::guinfgPolEnum(const NFSupport &p_support,
			     NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

guinfgPolEnum::guinfgPolEnum(const NFSupport &p_support,
			     NfgShowInterface *p_parent,
			     int p_stopAfter, bool p_eliminateWeak)
  : guiNfgSolution(p_support, p_parent), m_stopAfter(p_stopAfter)
{
  m_eliminate = true;
  m_eliminateAll = true;
  m_eliminateWeak = p_eliminateWeak;
  m_eliminateMixed = false;
}

gList<MixedSolution> guinfgPolEnum::Solve(void)
{
  wxPolEnumStatus status(m_parent->Frame());
  PolEnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  long nevals;
  double time;
  gList<MixedSolution> solutions;
  gList<const NFSupport> singular_supports;

  try {
    AllNashSolve(m_support, params, solutions, nevals, time, singular_supports);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgPolEnum::SolveSetup(void)
{
  dialogPolEnum dialog(m_parent->Frame(), false, true);
  
  if (dialog.Completed() == wxOK) {
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

guinfgQre::guinfgQre(const NFSupport &p_support, NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgQre::Solve(void)
{
  wxStatus status(m_parent->Frame(), "QreSolve Progress");
  NFQreParams params(status);
  params.minLam = m_minLam;
  params.maxLam = m_maxLam;
  params.delLam = m_delLam;
  params.SetAccuracy(m_accuracy);
  params.powLam = m_powLam;
  params.pxifile = m_pxiFile;
  params.trace = m_traceLevel;
  params.tracefile = m_traceFile;

  MixedProfile<gNumber> start(m_parent->CreateStartProfile(m_startOption));

  long nevals, nits;
  gList<MixedSolution> solutions;
  try {
    Qre(m_nfg, params, start, solutions, nevals, nits);
  }
  catch (gSignalBreak &) { }

  if (m_runPxi) {
    if (!wxExecute(m_pxiCommand + " " + m_pxiFilename)) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }

  return solutions;
}

bool guinfgQre::SolveSetup(void)
{
  dialogQre dialog(m_parent->Frame(), m_parent->Filename(), true);

  if (dialog.Completed() == wxOK) {
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

guinfgQreAll::guinfgQreAll(const NFSupport &p_support,
			   NfgShowInterface *p_parent)
  : guiNfgSolution(p_support, p_parent)
{ }

gList<MixedSolution> guinfgQreAll::Solve(void)
{
  wxStatus status(m_parent->Frame(), "QreGridSolve Progress");

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
    GridSolve(m_support, params, solutions);
  }
  catch (gSignalBreak &) { }
  
  if (m_runPxi) {
    if (!wxExecute(m_pxiCommand + " " + m_pxiFilename)) {
      wxMessageBox("Unable to launch PXI successfully");
    }
  }
  return solutions;
}

bool guinfgQreAll::SolveSetup(void)
{
  dialogQreGrid dialog(m_parent->Frame(), m_parent->Filename());

  if (dialog.Completed() == wxOK) {
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



