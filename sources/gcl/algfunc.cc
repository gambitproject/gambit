//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Functions for computing Nash equilibria
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

#include "base/base.h"

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "math/rational.h"

#include "game/nfg.h"
#include "game/efg.h"

#include "nash/efgsubsolve.h"
#include "nash/nfgpure.h"
#include "nash/efgpure.h"
#include "nash/nfgmixed.h"
#include "nash/nfglcp.h"
#include "nash/efglcp.h"
#include "nash/nfgliap.h"
#include "nash/efgliap.h"
#include "nash/nfglp.h"
#include "nash/efglp.h"
#include "nash/nfgalleq.h"
#include "nash/efgpoly.h"
#include "nash/nfgqregrid.h"
#include "nash/nfglogit.h"
#include "nash/efglogit.h"
#include "nash/simpdiv.h"
#include "nash/yamamoto.h"

#include "numerical/vertenum.h"
#include "numerical/lpsolve.h"


template <class T> Portion *ArrayToList(const gArray<T> &);
extern gVector<double>* ListToVector_Float(ListPortion* list);
extern gVector<gRational>* ListToVector_Rational(ListPortion* list);
extern gMatrix<double>* ListToMatrix_Float(ListPortion* list);
extern gMatrix<gRational>* ListToMatrix_Rational(ListPortion* list);


//
// Useful utilities for creation of lists of profiles
//
class Mixed_ListPortion : public ListPortion   {
  public:
    Mixed_ListPortion(const gList<MixedSolution> &);
    virtual ~Mixed_ListPortion()   { }
};

Mixed_ListPortion::Mixed_ListPortion(const gList<MixedSolution> &list)
{
  rep->_DataType = porMIXED;
  for (int i = 1; i <= list.Length(); i++)
    Append(new MixedPortion(new MixedSolution(list[i])));
}

class Behav_ListPortion : public ListPortion   {
  public:
    Behav_ListPortion(const gList<BehavSolution> &);
    virtual ~Behav_ListPortion()   { }
};

Behav_ListPortion::Behav_ListPortion(const gList<BehavSolution> &list)
{
  rep->_DataType = porBEHAV;
  for (int i = 1; i <= list.Length(); i++)
    Append(new BehavPortion(new BehavSolution(list[i])));
}

class NfSupport_ListPortion : public ListPortion   {
  public:
    NfSupport_ListPortion(const gList<const gbtNfgSupport> &);
    NfSupport_ListPortion();
    virtual ~NfSupport_ListPortion()   { }

  void SetValue(const gList<const gbtNfgSupport> &);
};

NfSupport_ListPortion::NfSupport_ListPortion(const gList<const 
					                 gbtNfgSupport> &list)
{
  rep->_DataType = porNFSUPPORT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new NfSupportPortion(new gbtNfgSupport(list[i])));
}

NfSupport_ListPortion::NfSupport_ListPortion()
{
  rep->_DataType = porNFSUPPORT;
}

void NfSupport_ListPortion::SetValue(const gList<const gbtNfgSupport> &list)
{
  for (int i = 1; i <= list.Length(); i++)
    Append(new NfSupportPortion(new gbtNfgSupport(list[i])));
}


class EfSupport_ListPortion : public ListPortion   {
  public:
    EfSupport_ListPortion(const gList<const EFSupport> &);
    EfSupport_ListPortion();
    virtual ~EfSupport_ListPortion()   { }

  void SetValue(const gList<const EFSupport> &);
};

EfSupport_ListPortion::EfSupport_ListPortion(const gList<const 
					                 EFSupport> &list)
{
  rep->_DataType = porEFSUPPORT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new EfSupportPortion(new EFSupport(list[i])));
}

EfSupport_ListPortion::EfSupport_ListPortion()
{
  rep->_DataType = porEFSUPPORT;
}

void EfSupport_ListPortion::SetValue(const gList<const EFSupport> &list)
{
  for (int i = 1; i <= list.Length(); i++)
    Append(new EfSupportPortion(new EFSupport(list[i])));
}



//-------------
// AgentForm
//-------------

static Portion *GSM_AgentForm(GSM &, Portion **param)
{
  efgGame &E = *((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg *N = MakeAfg(E);
  
  ((NumberPortion *) param[1])->SetValue(watch.Elapsed());

  if (N)
    return new NfgPortion(N);
  else
    throw gclRuntimeError("Conversion to agent form failed");
}

//------------
// Behav
//------------

static Portion *GSM_Behav(GSM &, Portion **param)
{
  MixedSolution &mp = *((MixedPortion*) param[0])->Value();
  BehavProfile<gNumber> *bp = new BehavProfile<gNumber>(MixedProfile<gNumber>(*mp.Profile()));
  BehavSolution *bs = new BehavSolution(*bp);
  bs->SetCreator(mp.Creator());
  bs->SetEpsilon(mp.Epsilon());
    
  return new BehavPortion(bs);
}


//------------------
// EnumMixedSolve
//------------------

static gbtNfgNashAlgorithm *GSM_EnumMixed_Nfg_Double(int p_stopAfter,
						  bool p_cliques)
{
  gbtNfgNashEnumMixed<double> *algorithm = new gbtNfgNashEnumMixed<double>;
  algorithm->SetStopAfter(p_stopAfter);
  algorithm->SetCliques(p_cliques);
  return algorithm;
}

static gbtNfgNashAlgorithm *GSM_EnumMixed_Nfg_Rational(int p_stopAfter,
						    bool p_cliques)
{
  gbtNfgNashEnumMixed<gRational> *algorithm = new gbtNfgNashEnumMixed<gRational>;
  algorithm->SetStopAfter(p_stopAfter);
  algorithm->SetCliques(p_cliques);
  return algorithm;
}

static Portion *GSM_EnumMixed_Nfg(GSM &gsm, Portion **param)
{
  const gbtNfgSupport &support = AsNfgSupport(param[0]);
  gbtNfgNashAlgorithm *algorithm = 0;

  if (((PrecisionPortion *) param[2])->Value() == precDOUBLE) {
    algorithm = GSM_EnumMixed_Nfg_Double(AsNumber(param[1]), AsBool(param[7]));
  }
  else {
    algorithm = GSM_EnumMixed_Nfg_Rational(AsNumber(param[1]),
					   AsBool(param[7]));
  }

  gList<MixedSolution> solutions;

  gsm.StartAlgorithmMonitor("EnumMixedSolve Progress");
  try {
    solutions = algorithm->Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    delete algorithm;
    throw;
  }

  delete algorithm;
  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

static Portion *GSM_EnumMixed_Efg(GSM &gsm, Portion **param)
{
  const EFSupport &support = AsEfgSupport(param[0]);

  if (!AsBool(param[1])) {
    throw gclRuntimeError("algorithm not implemented for extensive forms");
  }

  gbtNfgNashAlgorithm *nfgAlgorithm = 0;

  if (((PrecisionPortion *) param[3])->Value() == precDOUBLE) {
    nfgAlgorithm = GSM_EnumMixed_Nfg_Double(AsNumber(param[2]),
					    AsBool(param[8]));
  }
  else {
    nfgAlgorithm = GSM_EnumMixed_Nfg_Rational(AsNumber(param[2]),
					      AsBool(param[8]));
  }

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with EnumMixed; results not guaranteed\n";
  }

  gbtEfgNashSubgames *algorithm = new gbtEfgNashSubgames;
  algorithm->SetAlgorithm(nfgAlgorithm);

  gsm.StartAlgorithmMonitor("EnumMixedSolve Progress");
  gList<BehavSolution> solutions;
  try {
    solutions = algorithm->Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    delete algorithm;
    throw;
  }
  delete algorithm;
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}


//-----------------
// EnumPureSolve
//-----------------

static Portion *GSM_EnumPure_Nfg(GSM &gsm, Portion **param)
{
  gbtNfgSupport *support = ((NfSupportPortion*) param[0])->Value();

  gWatch watch;
  gList<MixedSolution> solutions;

  gsm.StartAlgorithmMonitor("EnumPureSolve Progress");
  try {
    gbtNfgNashEnumPure solver;
    solver.SetStopAfter(((NumberPortion *) param[1])->Value());
    solutions = solver.Solve(*support, gsm.GetStatusMonitor());
    ((NumberPortion *) param[3])->SetValue(watch.Elapsed());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

static Portion *GSM_EnumPure_Efg(GSM &gsm, Portion **param)
{
  const EFSupport &support = AsEfgSupport(param[0]);

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with EnumPure; results not guaranteed\n";
  }

  gsm.StartAlgorithmMonitor("EnumPureSolve Progress");

  gList<BehavSolution> solutions;
  if (AsBool(param[1])) {
    try {
      gbtNfgNashEnumPure *nfgAlgorithm = new gbtNfgNashEnumPure;
      nfgAlgorithm->SetStopAfter(AsNumber(param[2]));
      gbtEfgNashSubgames algorithm;
      algorithm.SetAlgorithm(nfgAlgorithm);
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }
  }
  else  {
    try {
      gbtEfgNashEnumPure algorithm;
      algorithm.SetStopAfter(AsNumber(param[2]));
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }
  }
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}

//------------
// LcpSolve
//------------

static gbtNfgNashAlgorithm *GSM_Lcp_Nfg_Double(int p_stopAfter)
{
  gbtNfgNashLcp<double> *algorithm = new gbtNfgNashLcp<double>;
  algorithm->SetStopAfter(p_stopAfter);
  return algorithm;
}

static gbtNfgNashAlgorithm *GSM_Lcp_Nfg_Rational(int p_stopAfter)
{
  gbtNfgNashLcp<gRational> *algorithm = new gbtNfgNashLcp<gRational>;
  algorithm->SetStopAfter(p_stopAfter);
  return algorithm;
}

static Portion *GSM_Lcp_Nfg(GSM &gsm, Portion **param)
{
  const gbtNfgSupport &support = AsNfgSupport(param[0]);
  const Nfg &nfg = support.Game();

  if (nfg.NumPlayers() != 2) {
    throw gclRuntimeError("Only valid for two-person games");
  }

  gbtNfgNashAlgorithm *algorithm = 0;

  if (AsPrecision(param[2]) == precDOUBLE) {
    algorithm = GSM_Lcp_Nfg_Double(AsNumber(param[1]));
  }
  else {
    algorithm = GSM_Lcp_Nfg_Rational(AsNumber(param[1]));
  }

  gList<MixedSolution> solutions;

  gsm.StartAlgorithmMonitor("LcpSolve Progress");
  try {
    solutions = algorithm->Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    delete algorithm;
    throw;
  }

  delete algorithm;
  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

static gbtEfgNashAlgorithm *GSM_Lcp_Efg_Double(int p_stopAfter)
{
  gbtEfgNashLcp<double> *algorithm = new gbtEfgNashLcp<double>;
  algorithm->SetStopAfter(p_stopAfter);
  return algorithm;
}

static gbtEfgNashAlgorithm *GSM_Lcp_Efg_Rational(int p_stopAfter)
{
  gbtEfgNashLcp<gRational> *algorithm = new gbtEfgNashLcp<gRational>;
  algorithm->SetStopAfter(p_stopAfter);
  return algorithm;
}

static Portion *GSM_Lcp_Efg(GSM &gsm, Portion **param)
{
  const EFSupport &support = AsEfgSupport(param[0]);
  const efgGame &efg = support.GetGame();

  if (efg.NumPlayers() != 2) {
    throw gclRuntimeError("Only valid for two-person games");
  }

  gbtEfgNashSubgames algorithm;
  
  if (AsBool(param[1])) {
    if (((PrecisionPortion *) param[3])->Value() == precDOUBLE) {
      algorithm.SetAlgorithm(GSM_Lcp_Nfg_Double(AsNumber(param[2])));
    }
    else {
      algorithm.SetAlgorithm(GSM_Lcp_Nfg_Rational(AsNumber(param[2])));	
    }
  }
  else {
    if (((PrecisionPortion *) param[3])->Value() == precDOUBLE) {
      algorithm.SetAlgorithm(GSM_Lcp_Efg_Double(AsNumber(param[2])));
    }
    else {
      algorithm.SetAlgorithm(GSM_Lcp_Efg_Rational(AsNumber(param[2])));
    }
  }

  gsm.StartAlgorithmMonitor("LcpSolve Progress");
  gList<BehavSolution> solutions;
  try {
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}

#include "numerical/lemketab.h"

Portion* GSM_Lcp_ListNumber(GSM &, Portion** param)
{
  if (((PrecisionPortion *) param[2])->Value() == precDOUBLE) {
    gMatrix<double> *a = ListToMatrix_Float((ListPortion*) param[0]);
    gVector<double> *b = ListToVector_Float((ListPortion*) param[1]);

    gMatrix<double> aa(1,a->NumRows(),0,a->NumColumns());

    int i,j;
    for (i = 1;i<=a->NumRows();i++) {
      for (j = 1;j<=a->NumColumns();j++)
	aa(i,j) = (*a)(a->MinRow()-1+i,a->MinCol()-1+j);
      aa(i,0) = 1;
    }
    aa(a->NumRows(),0) = 0;

    LTableau<double> *tab = new LTableau<double>(aa, *b);
    tab->LemkePath(0);
    gVector<double> vector(*b);
    tab->BasisVector(vector);
    Portion *result = ArrayToList(vector);
    delete tab;
    delete a;
    delete b;
      
    return result;
  }
  else {  // precision == precRATIONAL
    gMatrix<gRational> *a = ListToMatrix_Rational((ListPortion*) param[0]);
    gVector<gRational> *b = ListToVector_Rational((ListPortion*) param[1]);
      
    LTableau<gRational> *tab = new LTableau<gRational>(*a, *b);
    tab->LemkePath(0);
    gVector<gRational> vector;
    tab->BasisVector(vector);
    Portion *result = ArrayToList(vector);
    delete tab;
    delete a;
    delete b;
      
    return result;
  }
}

//-------------
// LiapSolve
//-------------

static Portion *GSM_Liap_Behav(GSM &gsm, Portion **param)
{
  const BehavProfile<gNumber> &start = *AsBehav(param[0]).Profile();
  const efgGame &efg = start.GetGame();
  const EFSupport &support = start.Support();
  
  gsm.StartAlgorithmMonitor("LiapSolve Progress");

  gbtEfgNashSubgames algorithm;

  if (AsBool(param[1])) {
    gbtNfgNashLiap *nfgAlgorithm = new gbtNfgNashLiap;
    nfgAlgorithm->SetStopAfter(AsNumber(param[2]));
    nfgAlgorithm->SetNumTries(AsNumber(param[3]));
    algorithm.SetAlgorithm(nfgAlgorithm);
  }
  else {
    gbtEfgNashLiap *efgAlgorithm = new gbtEfgNashLiap;
    efgAlgorithm->SetStopAfter(AsNumber(param[2]));
    efgAlgorithm->SetNumTries(AsNumber(param[3]));
    algorithm.SetAlgorithm(efgAlgorithm);
  }

  gList<BehavSolution> solutions;

  try {
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}

static Portion *GSM_Liap_Mixed(GSM &gsm, Portion **param)
{
  const MixedProfile<gNumber> &start = *AsMixed(param[0]).Profile();
  const Nfg &nfg = start.Game();

  gbtNfgNashLiap algorithm;
  algorithm.SetStopAfter(AsNumber(param[1]));
  algorithm.SetNumTries(AsNumber(param[2]));

  gList<MixedSolution> solutions;
  gsm.StartAlgorithmMonitor("LiapSolve Progress");
  try {
    solutions = algorithm.Solve(start.Support(), gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

//------------
// LpSolve
//------------

static gbtNfgNashAlgorithm *GSM_Lp_Nfg_Double(void)
{
  gbtNfgNashLcp<double> *algorithm = new gbtNfgNashLcp<double>;
  algorithm->SetStopAfter(1);
  return algorithm;
}

static gbtNfgNashAlgorithm *GSM_Lp_Nfg_Rational(void)
{
  gbtNfgNashLcp<gRational> *algorithm = new gbtNfgNashLcp<gRational>;
  algorithm->SetStopAfter(1);
  return algorithm;
}

static Portion *GSM_Lp_Nfg(GSM &gsm, Portion **param)
{
  const gbtNfgSupport &support = AsNfgSupport(param[0]);
  const Nfg &nfg = support.Game();

  if (nfg.NumPlayers() != 2 || !IsConstSum(nfg)) {
    throw gclRuntimeError("Only valid for two-person zero-sum games");
  }

  gbtNfgNashAlgorithm *algorithm =
    (AsBool(param[1])) ? GSM_Lp_Nfg_Double() : GSM_Lp_Nfg_Rational();

  gList<MixedSolution> solutions;
  gsm.StartAlgorithmMonitor("LpSolve Progress");
  try {
    solutions = algorithm->Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}


Portion* GSM_Lp_List(GSM &gsm, Portion** param)
{
  if (((PrecisionPortion *) param[4])->Value() == precDOUBLE)  {
    gMatrix<double>* a = ListToMatrix_Float((ListPortion*) param[0]);
    gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
    gVector<double>* c = ListToVector_Float((ListPortion*) param[2]);

    int nequals = ((NumberPortion*) param[3])->Value();
    bool isFeasible;
    bool isBounded;
    double value;

    LPSolve<double>* s = new LPSolve<double>(*a, *b, *c, nequals,
					     gsm.GetStatusMonitor());
    Portion* result = ArrayToList(s->OptimumVector());
    isFeasible = s->IsFeasible();
    isBounded = s->IsBounded();
    value = s->OptimumCost();
    delete s;
    delete a;
    delete b;
    delete c;
  
    ((BoolPortion*) param[5])->SetValue((isFeasible) ? triTRUE : triFALSE);
    ((BoolPortion*) param[6])->SetValue((isBounded) ? triTRUE : triFALSE);
    ((NumberPortion*) param[7])->SetValue((value));
    return result;
  }
  else  {
    gMatrix<gRational>* a = ListToMatrix_Rational((ListPortion*) param[0]);
    gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
    gVector<gRational>* c = ListToVector_Rational((ListPortion*) param[2]);

    int nequals = ((NumberPortion*) param[3])->Value();
    bool isFeasible;
    bool isBounded;
    gRational value;
  
    LPSolve<gRational>* s = new LPSolve<gRational>(*a, *b, *c, nequals,
						   gsm.GetStatusMonitor());
    Portion* result = ArrayToList(s->OptimumVector());
    isFeasible = s->IsFeasible();
    isBounded = s->IsBounded();
    value = s->OptimumCost();
    delete s;
    delete a;
    delete b;
    delete c;
  
    ((BoolPortion *) param[5])->SetValue((isFeasible) ? triTRUE : triFALSE);
    ((BoolPortion *) param[6])->SetValue((isBounded) ? triTRUE : triFALSE);
    ((NumberPortion*) param[7])->SetValue((value));
    return result;
  }
}

static gbtEfgNashAlgorithm *GSM_Lp_Efg_Double(void)
{
  gbtEfgNashLp<double> *algorithm = new gbtEfgNashLp<double>;
  //  algorithm->SetStopAfter(1);
  return algorithm;
}

static gbtEfgNashAlgorithm *GSM_Lp_Efg_Rational(void)
{
  gbtEfgNashLp<gRational> *algorithm = new gbtEfgNashLp<gRational>;
  //  algorithm->SetStopAfter(1);
  return algorithm;
}


static Portion *GSM_Lp_Efg(GSM &gsm, Portion **param)
{
  const EFSupport &support = AsEfgSupport(param[0]);
  const efgGame &efg = support.GetGame();
  
  if (efg.NumPlayers() != 2 || !efg.IsConstSum()) {
    throw gclRuntimeError("Only valid for two-person zero-sum games");
  }

  if (!IsPerfectRecall(efg)) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Lp; results not guaranteed\n";
  }

  gbtEfgNashSubgames algorithm;
  
  if (AsBool(param[1])) {
    if (((PrecisionPortion *) param[2])->Value() == precDOUBLE) {
      algorithm.SetAlgorithm(GSM_Lp_Nfg_Double());
    }
    else {
      algorithm.SetAlgorithm(GSM_Lp_Nfg_Rational());
    }
  }
  else {
    if (((PrecisionPortion *) param[2])->Value() == precDOUBLE) {
      algorithm.SetAlgorithm(GSM_Lp_Efg_Double());
    }
    else {
      algorithm.SetAlgorithm(GSM_Lp_Efg_Rational());
    }
  }

  gsm.StartAlgorithmMonitor("LpSolve Progress");
  gList<BehavSolution> solutions;
  try {
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}


//------------------
//  PolEnumSolve
//------------------

static Portion *GSM_PolEnumSolve_Nfg(GSM &gsm, Portion **param)
{
  const gbtNfgSupport &support = AsNfgSupport(param[0]);
  
  nfgPolEnum algorithm;
  algorithm.SetStopAfter(AsNumber(param[1]));
  gList<MixedSolution> solutions;

  gsm.StartAlgorithmMonitor("PolEnumSolve Progress");
  try {
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }
  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

static Portion *GSM_PolEnumSolve_Efg(GSM &gsm, Portion **param)
{
  const EFSupport &support = AsEfgSupport(param[0]);

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with PolEnum; results not guaranteed\n";
  }

  gsm.StartAlgorithmMonitor("PolEnumSolve Progress");

  gList<BehavSolution> solutions;
  gbtEfgNashSubgames algorithm;
  if (AsBool(param[1])) {
    nfgPolEnum *nfgAlgorithm = new nfgPolEnum;
    nfgAlgorithm->SetStopAfter(AsNumber(param[2]));
    algorithm.SetAlgorithm(nfgAlgorithm);
  }
  else {
    gbtEfgNashEnumPoly *efgAlgorithm = new gbtEfgNashEnumPoly;
    efgAlgorithm->SetStopAfter(AsNumber(param[2]));
    algorithm.SetAlgorithm(efgAlgorithm);
  }

  try {
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}

//---------
// Nfg
//---------

static Portion *GSM_Nfg(GSM &, Portion **param)
{
  const efgGame &efg = AsEfg(param[0]);
  Nfg *nfg = MakeReducedNfg(EFSupport(efg));
  if (nfg) {
    return new NfgPortion(nfg);
  }
  else {
    throw gclRuntimeError("Conversion to reduced nfg failed");
  }
}



//----------
// Payoff
//----------

Portion* GSM_Payoff_Behav(GSM &, Portion** param)
{
  BehavSolution *bp = ((BehavPortion *) param[0])->Value();
  EFPlayer *player = ((EfPlayerPortion *) param[1])->Value();

  return new NumberPortion(bp->Payoff(player->GetNumber()));
}

Portion* GSM_Payoff_Mixed(GSM &, Portion** param)
{
  MixedProfile<gNumber> mp(*(*((MixedPortion*) param[0])->Value()).Profile());
  gbtNfgPlayer player = ((NfPlayerPortion *) param[1])->Value();

  return new NumberPortion(mp.Payoff(player.GetId()));
}

//------------------
// QreGridSolve
//------------------

static Portion *GSM_QreGrid_Support(GSM &gsm, Portion **param)
{
  gbtNfgSupport &support = *((NfSupportPortion*) param[0])->Value();

  gOutput *pxiFile = 0;
  if (((TextPortion *) param[1])->Value() != "") {
    pxiFile = new gFileOutput(((TextPortion *) param[1])->Value());
  }
  else {
    pxiFile = new gNullOutput;
  }

  QreNfgGrid qre;
  qre.SetMinLambda(((NumberPortion *) param[2])->Value());
  qre.SetMaxLambda(((NumberPortion *) param[3])->Value());
  qre.SetDelLambda(((NumberPortion *) param[4])->Value());
  qre.SetPowLambda(((NumberPortion *) param[5])->Value());
  qre.SetFullGraph(((BoolPortion *) param[6])->Value());
  qre.SetDelP1(((NumberPortion *) param[7])->Value());
  qre.SetTol1(((NumberPortion *) param[8])->Value());
  qre.SetDelP2(((NumberPortion *) param[9])->Value());
  qre.SetTol2(((NumberPortion *) param[10])->Value());

  gList<MixedSolution> solutions;
  gsm.StartAlgorithmMonitor("QreGridSolve Progress");

  try {
    qre.Solve(support, *pxiFile, 
	      gsm.GetStatusMonitor(), solutions);
  }
  catch (gSignalBreak &) { }
  catch (...) {
    delete pxiFile;
    gsm.EndAlgorithmMonitor();
    throw;
  }

  delete pxiFile;
  gsm.EndAlgorithmMonitor();

  return new Mixed_ListPortion(solutions);
}

//---------------
// QreSolve
//---------------

static Portion *GSM_Qre_Start(GSM &gsm, Portion **param)
{
  gOutput *pxiFile = 0;
  if (((TextPortion *) param[1])->Value() != "") {
    pxiFile = new gFileOutput(((TextPortion *) param[1])->Value());
  }
  else {
    pxiFile = new gNullOutput;
  }

  if (param[0]->Spec().Type == porMIXED)  {
    const MixedSolution &start = AsMixed(param[0]);
    gbtNfgNashLogit algorithm;
    algorithm.SetMaxLambda(AsNumber(param[3]));
    algorithm.SetFullGraph(AsBool(param[6]));

    gList<MixedSolution> solutions;
    gsm.StartAlgorithmMonitor("QreSolve Progress");
    try {
      solutions = algorithm.Solve(start.Support(), gsm.GetStatusMonitor());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      delete pxiFile;  
      gsm.EndAlgorithmMonitor();
      throw;
    }

    delete pxiFile;
    gsm.EndAlgorithmMonitor();
    return new Mixed_ListPortion(solutions);
  }
  else  {     // BEHAV
    const BehavSolution &start = AsBehav(param[0]);
    efgGame &efg = start.GetGame();
  
    if (!IsPerfectRecall(efg)) {
      gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Qre; results not guaranteed\n";
    }

    gbtEfgNashLogit algorithm;
    algorithm.SetMaxLambda(AsNumber(param[3]));
    algorithm.SetFullGraph(AsNumber(param[6]));
    
    gList<BehavSolution> solutions;
    gsm.StartAlgorithmMonitor("QreSolve Progress");
    try {
      solutions = algorithm.Solve(start.Support(), gsm.GetStatusMonitor());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      delete pxiFile;
      gsm.EndAlgorithmMonitor();
      throw;
    }

    delete pxiFile;
    gsm.EndAlgorithmMonitor();

    return new Behav_ListPortion(solutions);
  }
}


//----------------
// SimpDivSolve
//----------------

static gbtNfgNashAlgorithm *GSM_Simpdiv_Nfg_Double(int p_stopAfter,
						int p_numRestarts,
						int p_leashLength)
{
  nfgSimpdiv<double> *algorithm = new nfgSimpdiv<double>;
  //  algorithm->SetStopAfter(p_stopAfter);
  algorithm->SetNumRestarts(p_numRestarts);
  algorithm->SetLeashLength(p_leashLength);
  return algorithm;
}

static gbtNfgNashAlgorithm *GSM_Simpdiv_Nfg_Rational(int p_stopAfter,
						  int p_numRestarts,
						  int p_leashLength)
{
  nfgSimpdiv<gRational> *algorithm = new nfgSimpdiv<gRational>;
  //  algorithm->SetStopAfter(p_stopAfter);
  algorithm->SetNumRestarts(p_numRestarts);
  algorithm->SetLeashLength(p_leashLength);
  return algorithm;
}

static Portion *GSM_Simpdiv_Nfg(GSM &gsm, Portion **param)
{
  const gbtNfgSupport &support = AsNfgSupport(param[0]);
  gbtNfgNashAlgorithm *algorithm;
  if (AsPrecision(param[4]) == precDOUBLE) {
    algorithm = GSM_Simpdiv_Nfg_Double(AsNumber(param[1]),
				       AsNumber(param[2]),
				       AsNumber(param[3]));
  }
  else {
    algorithm = GSM_Simpdiv_Nfg_Rational(AsNumber(param[1]),
					 AsNumber(param[2]),
					 AsNumber(param[3]));
  }

  gsm.StartAlgorithmMonitor("SimpDivSolve Progress");
  gList<MixedSolution> solutions;

  try {
    solutions = algorithm->Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    delete algorithm;
    gsm.EndAlgorithmMonitor();
    throw;
  }

  delete algorithm;
  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

static Portion *GSM_Simpdiv_Efg(GSM &gsm, Portion **param)
{
  const EFSupport &support = AsEfgSupport(param[0]);

  if (!AsBool(param[1])) {
    throw gclRuntimeError("algorithm not implemented for extensive forms");
  }

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Simpdiv; results not guaranteed\n";
  }

  gbtNfgNashAlgorithm *nfgAlgorithm = 0;

  if (((PrecisionPortion *) param[5])->Value() == precDOUBLE) {
    nfgAlgorithm = GSM_Simpdiv_Nfg_Double(AsNumber(param[2]),
					  AsNumber(param[3]),
					  AsNumber(param[4]));
  }
  else {
    nfgAlgorithm = GSM_Simpdiv_Nfg_Rational(AsNumber(param[2]),
					    AsNumber(param[3]),
					    AsNumber(param[4]));
  }

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Simpdiv; results not guaranteed\n";
  }

  gbtEfgNashSubgames algorithm;
  algorithm.SetAlgorithm(nfgAlgorithm);

  gsm.StartAlgorithmMonitor("SimpDivSolve Progress");
  gList<BehavSolution> solutions;
  try {
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
     throw;
  }
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}

static Portion *GSM_VertEnum(GSM &gsm, Portion** param)
{
  if (((PrecisionPortion *) param[2])->Value() == precDOUBLE)  {
    gMatrix<double>* A = ListToMatrix_Float((ListPortion*) param[0]);
    gVector<double>* b = ListToVector_Float((ListPortion*) param[1]);
    
    gList< gVector< double > > verts;
    
    VertEnum<double> *vertenum = new VertEnum<double>(*A, *b,
						      gsm.GetStatusMonitor());
    vertenum->Vertices( verts );
    delete vertenum;

    delete A;
    delete b;
    
    ListPortion* list = new ListPortion();
    for (int i = 1; i <= verts.Length(); i++)  
      list->Append( ArrayToList( verts[i] ) );
    
    return list;
  }
  else {
    gMatrix<gRational>* A = ListToMatrix_Rational((ListPortion*) param[0]);
    gVector<gRational>* b = ListToVector_Rational((ListPortion*) param[1]);
    
    gList< gVector< gRational > > verts;
    
    VertEnum<gRational> *vertenum = new VertEnum<gRational>(*A, *b,
							    gsm.GetStatusMonitor());
    vertenum->Vertices( verts );
    delete vertenum;

    delete A;
    delete b;
    
    ListPortion* list = new ListPortion();
    for (int i = 1; i <= verts.Length(); i++)  
      list->Append( ArrayToList( verts[i] ) );
    
    return list;
  }
}

static Portion *GSM_Yamamoto_Nfg(GSM &gsm, Portion **param)
{
  const gbtNfgSupport &support = AsNfgSupport(param[0]);
  nfgYamamoto algorithm;
  gsm.StartAlgorithmMonitor("YamamotoSolve Progress");
  gList<MixedSolution> solutions;
  try {
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

void Init_algfunc(GSM *gsm)
{
  gclFunction *FuncObj;

  FuncObj = new gclFunction(*gsm, "AgentForm", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_AgentForm, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("efg", porEFG));
  FuncObj->SetParamInfo(0, 1, gclParameter("time", porNUMBER,
					    new NumberPortion(0), BYREF));
  gsm->AddFunction(FuncObj);



  FuncObj = new gclFunction(*gsm, "Behav", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Behav, porBEHAV, 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("mixed", porMIXED));
  gsm->AddFunction(FuncObj);



  FuncObj = new gclFunction(*gsm, "EnumMixedSolve", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_EnumMixed_Nfg, 
				       PortionSpec(porMIXED, 1), 8));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 2, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(0, 3, gclParameter("nPivots", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 4, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 7, gclParameter("cliques", porBOOLEAN,
					    new BoolPortion(false)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_EnumMixed_Efg, 
				       PortionSpec(porBEHAV, 1), 9));
  FuncObj->SetParamInfo(1, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, gclParameter("asNfg", porBOOLEAN,
					    new BoolPortion(true)));
  FuncObj->SetParamInfo(1, 2, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(1, 3, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(1, 4, gclParameter("nPivots", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 5, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 6, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(1, 7, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(1, 8, gclParameter("cliques", porBOOLEAN,
					    new BoolPortion(false)));

  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "EnumPureSolve", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_EnumPure_Nfg, 
				       PortionSpec(porMIXED, 1), 5));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 2, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(0, 4, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_EnumPure_Efg, 
				       PortionSpec(porBEHAV, 1), 7));
  FuncObj->SetParamInfo(1, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, gclParameter("asNfg", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(1, 3, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(1, 4, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 5, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(1, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "QreGridSolve", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_QreGrid_Support, 
				       PortionSpec(porMIXED, 1), 15));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("pxifile", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 2, gclParameter("minLam", porNUMBER,
					    new NumberPortion(0.001)));
  FuncObj->SetParamInfo(0, 3, gclParameter("maxLam", porNUMBER,
					    new NumberPortion(500.0)));
  FuncObj->SetParamInfo(0, 4, gclParameter("delLam", porNUMBER,
					    new NumberPortion(0.02)));
  FuncObj->SetParamInfo(0, 5, gclParameter("powLam", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(0, 6, gclParameter("fullGraph", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 7, gclParameter("delp1", porNUMBER,
					    new NumberPortion(.1)));
  FuncObj->SetParamInfo(0, 8, gclParameter("tol1", porNUMBER,
					    new NumberPortion(.1)));
  FuncObj->SetParamInfo(0, 9, gclParameter("delp2", porNUMBER,
					    new NumberPortion(.01)));
  FuncObj->SetParamInfo(0, 10, gclParameter("tol2", porNUMBER,
					    new NumberPortion(.01)));
  FuncObj->SetParamInfo(0, 11, gclParameter("nEvals", porNUMBER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 12, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 13, gclParameter("traceFile", porOUTPUT,
					     new OutputPortion(*new gNullOutput),
					     BYREF));
  FuncObj->SetParamInfo(0, 14, gclParameter("traceLevel", porNUMBER,
					     new NumberPortion(0)));

  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "QreSolve", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Qre_Start, 
				       PortionSpec(porMIXED | porBEHAV, 1), 14));
  FuncObj->SetParamInfo(0, 0, gclParameter("start",
					    porMIXED | porBEHAV));
  FuncObj->SetParamInfo(0, 1, gclParameter("pxifile", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 2, gclParameter("minLam", porNUMBER,
					    new NumberPortion(0.001)));
  FuncObj->SetParamInfo(0, 3, gclParameter("maxLam", porNUMBER,
					    new NumberPortion(500.0)));
  FuncObj->SetParamInfo(0, 4, gclParameter("delLam", porNUMBER,
					    new NumberPortion(0.02)));
  FuncObj->SetParamInfo(0, 5, gclParameter("powLam", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(0, 6, gclParameter("fullGraph", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 7, gclParameter("accuracy", porNUMBER,
					    new NumberPortion(1.0e-8)));
  FuncObj->SetParamInfo(0, 8, gclParameter("time", porNUMBER,
					     new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 9, gclParameter("nEvals", porINTEGER,
					     new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 10, gclParameter("nIters", porINTEGER,
					     new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 11, gclParameter("traceFile", porOUTPUT,
					     new OutputPortion(*new gNullOutput), 
					     BYREF));
  FuncObj->SetParamInfo(0, 12, gclParameter("traceLevel", porNUMBER,
					     new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 13, gclParameter("method", porINTEGER,
					    new NumberPortion(0)));

  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "LcpSolve", 3);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Lcp_Nfg, 
				       PortionSpec(porMIXED, 1), 7));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(0, 2, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(0, 3, gclParameter("nPivots", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 4, gclParameter("time", porNUMBER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput),
					    BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Lcp_Efg, 
				       PortionSpec(porBEHAV, 1), 8));
  FuncObj->SetParamInfo(1, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, gclParameter("asNfg", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(1, 3, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(1, 4, gclParameter("nPivots", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 5, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 6, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(1, 7, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  
  FuncObj->SetFuncInfo(2, gclSignature(GSM_Lcp_ListNumber, 
				       PortionSpec(porNUMBER, 1), 3));
  FuncObj->SetParamInfo(2, 0, gclParameter("a", PortionSpec(porNUMBER, 2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(2, 1, gclParameter("b", PortionSpec(porNUMBER, 1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(2, 2, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));

  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "LiapSolve", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Liap_Behav,
				       PortionSpec(porBEHAV, 1), 9));
  FuncObj->SetParamInfo(0, 0, gclParameter("start", porBEHAV));
  FuncObj->SetParamInfo(0, 1, gclParameter("asNfg", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(0, 2, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(0, 3, gclParameter("nTries", porINTEGER,
					    new NumberPortion(10)));
  FuncObj->SetParamInfo(0, 4, gclParameter("accuracy", porNUMBER,
					    new NumberPortion(1.0e-8)));
  FuncObj->SetParamInfo(0, 5, gclParameter("time", porNUMBER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("nEvals", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 7, gclParameter("traceFile", porOUTPUT,
					     new OutputPortion(*new gNullOutput), 
					     BYREF));
  FuncObj->SetParamInfo(0, 8, gclParameter("traceLevel", porNUMBER,
					     new NumberPortion(0)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Liap_Mixed,
				       PortionSpec(porMIXED, 1), 8));
  FuncObj->SetParamInfo(1, 0, gclParameter("start", porMIXED));
  FuncObj->SetParamInfo(1, 1, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(1, 2, gclParameter("nTries", porINTEGER,
					    new NumberPortion(10)));
  FuncObj->SetParamInfo(1, 3, gclParameter("accuracy", porNUMBER,
					    new NumberPortion(1.0e-8)));
  FuncObj->SetParamInfo(1, 4, gclParameter("time", porNUMBER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 5, gclParameter("nEvals", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 6, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(1, 7, gclParameter("traceLevel", porNUMBER,
					     new NumberPortion(0)));

  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "LpSolve", 3);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Lp_Nfg, 
				       PortionSpec(porMIXED, 1), 6));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(0, 2, gclParameter("nPivots", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput),
					    BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Lp_Efg, 
				       PortionSpec(porBEHAV, 1), 7));
  FuncObj->SetParamInfo(1, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, gclParameter("asNfg", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(1, 3, gclParameter("nPivots", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 4, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 5, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(1, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));

  FuncObj->SetFuncInfo(2, gclSignature(GSM_Lp_List, 
				       PortionSpec(porNUMBER, 1), 8));
  FuncObj->SetParamInfo(2, 0, gclParameter("a", PortionSpec(porNUMBER, 2)));
  FuncObj->SetParamInfo(2, 1, gclParameter("b", PortionSpec(porNUMBER, 1)));
  FuncObj->SetParamInfo(2, 2, gclParameter("c", PortionSpec(porNUMBER, 1)));
  FuncObj->SetParamInfo(2, 3, gclParameter("nEqualities", porNUMBER));
  FuncObj->SetParamInfo(2, 4, gclParameter("precision", porPRECISION,
					    new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(2, 5, gclParameter("isFeasible", porBOOLEAN,
					    new BoolPortion(false), BYREF));
  FuncObj->SetParamInfo(2, 6, gclParameter("isBounded", porBOOLEAN,
					    new BoolPortion(false), BYREF));
  FuncObj->SetParamInfo(2, 7, gclParameter("optimalCost", porNUMBER,
					    new NumberPortion(0), BYREF));

  gsm->AddFunction(FuncObj);


  FuncObj = new gclFunction(*gsm, "Nfg", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Nfg, porNFG, 2));
  FuncObj->SetParamInfo(0, 0, gclParameter("efg", porEFG));
  FuncObj->SetParamInfo(0, 1, gclParameter("time", porNUMBER,
					    new NumberPortion(0), BYREF));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "PolEnumSolve", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_PolEnumSolve_Nfg, 
				       PortionSpec(porMIXED, 1), 8));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 2, gclParameter("nEvals", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 3, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 4, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					    BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 6, gclParameter("singularSupps", 
					   PortionSpec(porNFSUPPORT,1),
					   new NfSupport_ListPortion(), 
					   BYREF));
  FuncObj->SetParamInfo(0, 7, gclParameter("recurse",porBOOLEAN, 
					   new BoolPortion(true)));
  FuncObj->SetFuncInfo(1, gclSignature(GSM_PolEnumSolve_Efg, 
				       PortionSpec(porBEHAV, 1), 9));
  FuncObj->SetParamInfo(1, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, gclParameter("asNfg", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(1, 3, gclParameter("nEvals", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 4, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 5, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput), 
					   BYREF));
  FuncObj->SetParamInfo(1, 6, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(1, 7, gclParameter("singularSupps", 
					   PortionSpec(porEFSUPPORT,1),
					   new EfSupport_ListPortion(), 
					   BYREF));
  FuncObj->SetParamInfo(1, 8, gclParameter("recurse",porBOOLEAN, 
					   new BoolPortion(true)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "Payoff", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Payoff_Behav, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(0, 0, gclParameter("profile", porBEHAV));
  FuncObj->SetParamInfo(0, 1, gclParameter("player", porEFPLAYER));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Payoff_Mixed, porNUMBER, 2,
				       0, funcLISTABLE | funcGAMEMATCH));
  FuncObj->SetParamInfo(1, 0, gclParameter("profile", porMIXED));
  FuncObj->SetParamInfo(1, 1, gclParameter("player", porNFPLAYER));
  gsm->AddFunction(FuncObj);



  FuncObj = new gclFunction(*gsm, "SimpDivSolve", 2);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Simpdiv_Nfg,
				       PortionSpec(porMIXED, 1), 9));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  FuncObj->SetParamInfo(0, 1, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(0, 2, gclParameter("nRestarts", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(0, 3, gclParameter("leashLength", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 4, gclParameter("precision", porPRECISION,
              new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(0, 5, gclParameter("nEvals", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 7, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput),
					    BYREF));
  FuncObj->SetParamInfo(0, 8, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));

  FuncObj->SetFuncInfo(1, gclSignature(GSM_Simpdiv_Efg,
				       PortionSpec(porBEHAV, 1), 10));
  FuncObj->SetParamInfo(1, 0, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(1, 1, gclParameter("asNfg", porBOOLEAN,
					    new BoolPortion(false)));
  FuncObj->SetParamInfo(1, 2, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(1, 3, gclParameter("nRestarts", porINTEGER,
					    new NumberPortion(1)));
  FuncObj->SetParamInfo(1, 4, gclParameter("leashLength", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(1, 5, gclParameter("precision", porPRECISION,
					   new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(1, 6, gclParameter("nEvals", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(1, 7, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(1, 8, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(*new gNullOutput),
					    BYREF));
  FuncObj->SetParamInfo(1, 9, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "VertEnum", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_VertEnum,
				       PortionSpec(porNUMBER, 2), 3));
  FuncObj->SetParamInfo(0, 0, gclParameter("A", PortionSpec(porNUMBER,2),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(0, 1, gclParameter("b", PortionSpec(porNUMBER,1),
					    REQUIRED, BYVAL));
  FuncObj->SetParamInfo(0, 2, gclParameter("precision", porPRECISION,
					    new PrecisionPortion(precDOUBLE)));
  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "YamamotoSolve", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Yamamoto_Nfg, 
				       PortionSpec(porMIXED, 1), 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  gsm->AddFunction(FuncObj);
}




