//
// FILE: nfgsolng.cc -- definition of the class dealing with the GUI part of the
// normal form solutions.
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

/************************** Nfg SOLUTION G ***************************/
NfgSolutionG::NfgSolutionG(const Nfg &N, const NFSupport &S, NfgShowInterface *parent_)
    : nf(N), sup(S), parent(parent_)
{ }

//***************************** NORMAL FORM SOLUTIONS **************
// SolveLemke

#include "lemke.h"
#include "lemkeprm.h"

NfgLemkeG::NfgLemkeG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent)
    : NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgLemkeG::Solve(void) const
{
    if (nf.NumPlayers() != 2)
    {
        wxMessageBox("LCP algorithm only works on 2 player games.", "Algorithm Error");
    }

    wxStatus status(parent->Frame(), "LCP Algorithm");
    LemkeParamsSettings LPS;
    LemkeParams P(status);
    LPS.GetParams(P);
    int npivots;
    double time;
    Lemke(sup, P, (gList<MixedSolution> &)solns, npivots, time);
    return solns;
}


void NfgLemkeG::SolveSetup(void) const
{
    LemkeSolveParamsDialog(parent->Frame());
}

// Purenash

#include "nfgpure.h"
#include "purenprm.h"

NfgEnumPureG::NfgEnumPureG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent)
    : NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgEnumPureG::Solve(void) const
{
  wxStatus status(parent->Frame(), "EnumPure Algorithm");
  try {
    FindPureNash(sup, 0, status, (gList<MixedSolution> &)solns);
  }
  catch (gSignalBreak &) { }
  return solns;
}


void NfgEnumPureG::SolveSetup(void) const
{
    PureNashSolveParamsDialog PNPD(parent->Frame());
}


#include "grid.h"
#include "gridprm.h"

NfgGobitAllG::NfgGobitAllG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent):
    NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgGobitAllG::Solve(void) const
{
    GridParamsSettings GSPD(parent->Filename());
    wxStatus *status = new wxStatus(parent->Frame(), "GobitAll Solve");
    GridParams P(*status);
    GSPD.GetParams(P);
    GridSolve(sup, P, (gList<MixedSolution> &)solns);
    delete status;
    GSPD.RunPxi();
    return solns;
}


void NfgGobitAllG::SolveSetup(void) const
{
    GridSolveParamsDialog GSPD(parent->Frame(), parent->Filename());
}


#include "ngobit.h"
#include "gobitprm.h"

NfgGobitG::NfgGobitG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent)
    : NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgGobitG::Solve(void) const
{
    GobitParamsSettings GSPD(parent->Filename());
    wxStatus *status = new wxStatus(parent->Frame(), "Gobit Algorithm");
    NFGobitParams P(*status);
    GSPD.GetParams(&P);

    MixedProfile<gNumber> start(parent->CreateStartProfile(GSPD.StartOption()));

    long nevals, nits;
    Gobit(nf, P, start, (gList<MixedSolution> &)solns, nevals, nits);

    delete status;
    GSPD.RunPxi();
    return solns;
}


void NfgGobitG::SolveSetup(void) const
{
    GobitSolveParamsDialog GSPD(parent->Frame(), parent->Filename());
}


#include "nliap.h"
#include "liapprm.h"

NfgLiapG::NfgLiapG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent):
    NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgLiapG::Solve(void) const
{
    wxStatus status(parent->Frame(), "Liap Algorithm");
    LiapParamsSettings LPS;
    NFLiapParams P(status);
    LPS.GetParams(&P);
    MixedProfile<gNumber> start(parent->CreateStartProfile(LPS.StartOption()));
    gList<MixedSolution> temp_solns;
    long nevals, nits;
    Liap(nf, P, start, (gList<MixedSolution> &)solns, nevals, nits);
    return solns;
}


void NfgLiapG::SolveSetup(void) const
{
    LiapSolveParamsDialog LSPD(parent->Frame());
}


#include "simpdiv.h"
#include "simpprm.h"

NfgSimpdivG::NfgSimpdivG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent)
    : NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgSimpdivG::Solve(void) const
{
    SimpdivParamsSettings SPS;
    wxStatus status(parent->Frame(), "Simpdiv Algorithm");
    SimpdivParams P(status);
    SPS.GetParams(P);
    int nevals, niters;
    double time;
    Simpdiv(sup, P, (gList<MixedSolution> &)solns, nevals, niters, time);
    return solns;
}


void NfgSimpdivG::SolveSetup(void) const
{
    SimpdivSolveParamsDialog SDPD(parent->Frame());
}


#include "enum.h"
#include "enumprm.h"

NfgEnumG::NfgEnumG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent)
    : NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgEnumG::Solve(void) const
{
    EnumParamsSettings EPS;
    wxEnumStatus status(parent->Frame());
    EnumParams P(status);
    EPS.GetParams(P);
    long npivots;
    double time;
    Enum(sup, P, (gList<MixedSolution> &)solns, npivots, time);
    return solns;
}


void NfgEnumG::SolveSetup(void) const
{
    EnumSolveParamsDialog ESPD(parent->Frame());
}


#include "nfgcsum.h"
#include "csumprm.h"

NfgZSumG::NfgZSumG(const Nfg &N, const NFSupport &sup, NfgShowInterface *parent)
    : NfgSolutionG(N, sup, parent)
{ }


gList<MixedSolution> NfgZSumG::Solve(void) const
{
    if (nf.NumPlayers() > 2 || !IsConstSum(nf))
    {
        wxMessageBox("Only valid for two-person zero-sum games");
        return solns;
    }

    wxStatus status(parent->Frame(), "LP Algorithm");
    status << "Progress not implemented\n" << "Cancel button disabled\n";
    LPParamsSettings LPPS;
    ZSumParams P;
    LPPS.GetParams(&P);
    int npivots;
    double time;
    ZSum(sup, P, (gList<MixedSolution> &)solns, npivots, time);
    return solns;
}


void NfgZSumG::SolveSetup(void) const
{
    LPSolveParamsDialog LPD(parent->Frame());
}

