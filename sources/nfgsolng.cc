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

// sections in the defaults file(s)
#define     SOLN_SECT               "Soln-Defaults"

//=========================================================================
//                     NfgSolutionG: Member functions
//=========================================================================

NfgSolutionG::NfgSolutionG(const Nfg &p_nfg, const NFSupport &p_support,
			   NfgShowInterface *p_parent)
  : nf(p_nfg), sup(p_support), parent(p_parent)
{ }

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
  : NfgSolutionG(p_support.Game(), p_support, p_parent)
{ }

gList<MixedSolution> guinfgEnumPure::Solve(void) const
{
  wxStatus status(parent->Frame(), "EnumPureSolve Progress");

  gList<MixedSolution> solutions;

  try {
    FindPureNash(sup, m_stopAfter, status, solns);
  }
  catch (gSignalBreak &) { }
  return solutions;
}

bool guinfgEnumPure::SolveSetup(void)
{
  dialogEnumPure dialog(parent->Frame());

  if (dialog.Completed() == wxOK) {
    m_stopAfter = dialog.StopAfter();
    return true;
  }
  else
    return false;
}

//------------
// EnumMixed
//------------

#include "enum.h"
#include "enumprm.h"

NfgEnumG::NfgEnumG(const Nfg &p_nfg, const NFSupport &p_support,
		   NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> NfgEnumG::Solve(void) const
{
  wxEnumStatus status(parent->Frame());

  EnumParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  long npivots;
  double time;
  try {
    Enum(sup, params, solns, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solns;
}

bool NfgEnumG::SolveSetup(void)
{
  EnumSolveParamsDialog dialog(parent->Frame());

  if (dialog.Completed() == wxOK) {
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

guinfgLcp::guinfgLcp(const Nfg &p_nfg, const NFSupport &p_support,
		     NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> guinfgLcp::Solve(void) const
{
  if (nf.NumPlayers() != 2) {
    wxMessageBox("LCP algorithm only works on 2 player games.",
		 "Algorithm Error");
    return gList<MixedSolution>();
  }

  wxStatus status(parent->Frame(), "LcpSolve Progress");

  LemkeParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  int npivots;
  double time;
  try {
    Lemke(sup, params, solns, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solns;
}

bool guinfgLcp::SolveSetup(void)
{
  LcpSolveDialog dialog(parent->Frame());

  if (dialog.Completed() == wxOK) {
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

#include "nfgcsum.h"
#include "csumprm.h"

NfgZSumG::NfgZSumG(const Nfg &p_nfg, const NFSupport &p_support,
		   NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> NfgZSumG::Solve(void) const
{
  if (nf.NumPlayers() > 2 || !IsConstSum(nf)) {
    wxMessageBox("Only valid for two-person zero-sum games");
    return solns;
  }

  wxStatus status(parent->Frame(), "LP Algorithm");
  status << "Progress not implemented\n" << "Cancel button disabled\n";

  ZSumParams params;
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;

  int npivots;
  double time;
  try {
    ZSum(sup, params, solns, npivots, time);
  }
  catch (gSignalBreak &) { }
  return solns;
}

bool NfgZSumG::SolveSetup(void)
{
  LPSolveParamsDialog dialog(parent->Frame());
  
  if (dialog.Completed() == wxOK) {
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

guinfgLiap::guinfgLiap(const Nfg &p_nfg, const NFSupport &p_support,
		       NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> guinfgLiap::Solve(void) const
{
  wxStatus status(parent->Frame(), "Liap Algorithm");
  NFLiapParams params(status);
  params.tol1 = m_tol1D;
  params.tolN = m_tolND;
  params.maxits1 = m_maxits1D;
  params.maxitsN = m_maxitsND;
  params.nTries = m_nTries;

  MixedProfile<gNumber> start(parent->CreateStartProfile(m_startOption));
  long nevals, nits;
  try {
    Liap(nf, params, start, solns, nevals, nits);
  }
  catch (gSignalBreak &) { }
  return solns;
}

bool guinfgLiap::SolveSetup(void)
{
  LiapSolveParamsDialog dialog(parent->Frame(), true);

  if (dialog.Completed() == wxOK) {
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

NfgSimpdivG::NfgSimpdivG(const Nfg &p_nfg, const NFSupport &p_support,
			 NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> NfgSimpdivG::Solve(void) const
{
  wxStatus status(parent->Frame(), "SimpdivSolve Progress");
  SimpdivParams params(status);
  params.stopAfter = m_stopAfter;
  params.precision = m_precision;
  params.nRestarts = m_nRestarts;
  params.leashLength = m_leashLength;

  int nevals, niters;
  double time;
  try {
    Simpdiv(sup, params, solns, nevals, niters, time);
  }
  catch (gSignalBreak &) { }
  return solns;
}

bool NfgSimpdivG::SolveSetup(void)
{
  SimpdivSolveParamsDialog dialog(parent->Frame());

  if (dialog.Completed() == wxOK) {
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

guiPolEnumNfg::guiPolEnumNfg(const Nfg &p_nfg, const NFSupport &p_support,
			     NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> guiPolEnumNfg::Solve(void) const
{
  wxStatus status(parent->Frame(), "PolEnumSolve Algorithm");
  PolEnumParams params(status);
  params.stopAfter = m_stopAfter;

  long nevals;
  double time;
  try {
    PolEnum(sup, params, solns, nevals, time);
  }
  catch (gSignalBreak &) { }
  return solns;
}

bool guiPolEnumNfg::SolveSetup(void)
{
  guiPolEnumParamsDialog dialog(parent->Frame());
  
  if (dialog.Completed() == wxOK) {
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

NfgQreG::NfgQreG(const Nfg &p_nfg, const NFSupport &p_support,
		     NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> NfgQreG::Solve(void) const
{
  wxStatus status(parent->Frame(), "QreSolve Progress");
  NFQreParams params(status);

  MixedProfile<gNumber> start(parent->CreateStartProfile(m_startOption));

  long nevals, nits;
  try {
    Qre(nf, params, start, solns, nevals, nits);
    //GSPD.RunPxi();
  }
  catch (gSignalBreak &) { }

  return solns;
}

bool NfgQreG::SolveSetup(void)
{
  QreSolveParamsDialog dialog(parent->Frame(), parent->Filename());

  if (dialog.Completed() == wxOK) {
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

NfgQreAllG::NfgQreAllG(const Nfg &p_nfg, const NFSupport &p_support,
			   NfgShowInterface *p_parent)
  : NfgSolutionG(p_nfg, p_support, p_parent)
{ }

gList<MixedSolution> NfgQreAllG::Solve(void) const
{
  wxStatus *status = new wxStatus(parent->Frame(), "QreAllSolve Progress");
  GridParams params(*status);
  try {
    GridSolve(sup, params, solns);
  }
  catch (gSignalBreak &) { }
  delete status;
  //GSPD.RunPxi();
  return solns;
}

bool NfgQreAllG::SolveSetup(void)
{
  GridSolveParamsDialog dialog(parent->Frame(), parent->Filename());

  if (dialog.Completed() == wxOK) {
    return true;
  }
  else
    return false;
}



