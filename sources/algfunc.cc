//
// FILE: algfunc.cc -- Solution algorithm functions for GCL
//
// $Id$
//

#include "base/base.h"

#include "gsm.h"
#include "portion.h"
#include "gsmfunc.h"

#include "math/rational.h"

#include "mixedsol.h"
#include "behavsol.h"
#include "nfg.h"
#include "nfplayer.h"
#include "efg.h"

#include "vertenum.h"

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
    NfSupport_ListPortion(const gList<const NFSupport> &);
    NfSupport_ListPortion();
    virtual ~NfSupport_ListPortion()   { }

  void SetValue(const gList<const NFSupport> &);
};

NfSupport_ListPortion::NfSupport_ListPortion(const gList<const 
					                 NFSupport> &list)
{
  rep->_DataType = porNFSUPPORT;
  for (int i = 1; i <= list.Length(); i++)
    Append(new NfSupportPortion(new NFSupport(list[i])));
}

NfSupport_ListPortion::NfSupport_ListPortion()
{
  rep->_DataType = porNFSUPPORT;
}

void NfSupport_ListPortion::SetValue(const gList<const NFSupport> &list)
{
  for (int i = 1; i <= list.Length(); i++)
    Append(new NfSupportPortion(new NFSupport(list[i])));
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
  Efg::Game &E = *((EfgPortion*) param[0])->Value();
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

EfgAlgType NfgAlgType2EfgAlgType(NfgAlgType algtype)
{
  switch (algtype) {
  case algorithmNfg_ENUMPURE:
    return algorithmEfg_ENUMPURE_NFG;
  case algorithmNfg_ENUMMIXED:
    return algorithmEfg_ENUMMIXED_NFG;
  case algorithmNfg_LCP:
    return algorithmEfg_LCP_NFG;
  case algorithmNfg_LP:
    return algorithmEfg_LP_NFG;
  case algorithmNfg_LIAP:
    return algorithmEfg_LIAP_NFG;
  case algorithmNfg_SIMPDIV:
    return algorithmEfg_SIMPDIV_NFG;
  case algorithmNfg_POLENUM:
    return algorithmEfg_POLENUM_NFG;
  case algorithmNfg_QRE:
    return algorithmEfg_QRE_NFG;
  case algorithmNfg_QREALL:
    return algorithmEfg_QREALL_NFG;
  default:
    return algorithmEfg_USER;
  }
}

static Portion *GSM_Behav(GSM &, Portion **param)
{
  MixedSolution &mp = *((MixedPortion*) param[0])->Value();
  BehavProfile<gNumber> *bp = new BehavProfile<gNumber>(MixedProfile<gNumber>(mp));
  BehavSolution *bs = new BehavSolution(*bp);
  bs->SetCreator(NfgAlgType2EfgAlgType(mp.Creator()));
  bs->SetEpsilon(mp.Epsilon());
    
  return new BehavPortion(bs);
}


//------------------
// EnumMixedSolve
//------------------

#include "enum.h"

static Portion *GSM_EnumMixed_Nfg(GSM &gsm, Portion **param)
{
  NFSupport* S = ((NfSupportPortion*) param[0])->Value();

  EnumParams params;
  params.stopAfter = ((NumberPortion *) param[1])->Value();
  params.precision = ((PrecisionPortion *) param[2])->Value();
  params.tracefile = &((OutputPortion *) param[5])->Value();
  params.trace = ((NumberPortion *) param[6])->Value();
  params.cliques = ((BoolPortion *) param[7])->Value();
  
  gList<MixedSolution> solutions;

  gsm.StartAlgorithmMonitor("EnumMixedSolve Progress");
  try {
    double time;
    long npivots;
    Enum(*S, params, solutions, gsm.OutputStream(),
	 gsm.GetStatusMonitor(), npivots, time);
    ((NumberPortion *) param[3])->SetValue(npivots);
    ((NumberPortion *) param[4])->SetValue(time);
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

#include "enumsub.h"

static Portion *GSM_EnumMixed_Efg(GSM &gsm, Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();

  if (!((BoolPortion *) param[1])->Value()) {
    throw gclRuntimeError("algorithm not implemented for extensive forms");
  }

  EnumParams params;
  params.stopAfter = ((NumberPortion *) param[2])->Value();
  params.precision = ((PrecisionPortion *) param[3])->Value();
  params.tracefile = &((OutputPortion *) param[6])->Value();
  params.trace = ((NumberPortion *) param[7])->Value();
  params.cliques = ((BoolPortion *) param[8])->Value();

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with EnumMixed; results not guaranteed\n";
  }

  gsm.StartAlgorithmMonitor("EnumMixedSolve Progress");
  gList<BehavSolution> solutions;
  try {
    double time;
    long npivots;
    Enum(support, params, solutions, gsm.GetStatusMonitor(), npivots, time);
    ((NumberPortion *) param[4])->SetValue(npivots);
    ((NumberPortion *) param[5])->SetValue(time);
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}


//-----------------
// EnumPureSolve
//-----------------

#include "nfgpure.h"

static Portion *GSM_EnumPure_Nfg(GSM &gsm, Portion **param)
{
  NFSupport *support = ((NfSupportPortion*) param[0])->Value();

  gWatch watch;
  gList<MixedSolution> solutions;

  gsm.StartAlgorithmMonitor("EnumPureSolve Progress");
  try {
    nfgEnumPure solver;
    solver.SetStopAfter(((NumberPortion *) param[1])->Value());
    solver.Solve(*support, gsm.GetStatusMonitor(), solutions);
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

#include "efgpure.h"
#include "psnesub.h"

static Portion *GSM_EnumPure_Efg(GSM &gsm, Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with EnumPure; results not guaranteed\n";
  }

  gsm.StartAlgorithmMonitor("EnumPureSolve Progress");
  gList<BehavSolution> solutions;

  if (((BoolPortion *) param[1])->Value())   {
    try {
      efgEnumPureNfgSolve algorithm(support,
				    ((NumberPortion *) param[2])->Value());
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
      ((NumberPortion *) param[4])->SetValue(algorithm.Time());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }
  }
  else  {
    try {
      gWatch watch;
      efgEnumPure algorithm(((NumberPortion *) param[2])->Value()); 
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
      ((NumberPortion *) param[4])->SetValue(watch.Elapsed());
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

//------------------
// QreGridSolve
//------------------

#include "nfgqregrid.h"

static Portion *GSM_QreGrid_Support(GSM &gsm, Portion **param)
{
  NFSupport &support = *((NfSupportPortion*) param[0])->Value();

  gOutput *pxiFile = 0;
  if (((TextPortion *) param[1])->Value() != "") {
    pxiFile = new gFileOutput(((TextPortion *) param[1])->Value());
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
    qre.Solve(support, (pxiFile) ? *pxiFile : gnull,
	      gsm.GetStatusMonitor(), solutions);
  }
  catch (gSignalBreak &) { }
  catch (...) {
    if (pxiFile) delete pxiFile;
    gsm.EndAlgorithmMonitor();
    throw;
  }

  if (pxiFile) delete pxiFile;
  gsm.EndAlgorithmMonitor();

  return new Mixed_ListPortion(solutions);
}

static Portion *GSM_Qre_Dynamics(GSM &p_gsm, Portion **p_param)
{
  MixedSolution &start = *((MixedPortion *) p_param[0])->Value();
  double lambda = ((NumberPortion *) p_param[1])->Value();
  double tmax = ((NumberPortion *) p_param[2])->Value();
  gOutput *file = 0;
  if (((TextPortion *) p_param[3])->Value() != "") {
    file = new gFileOutput(((TextPortion *) p_param[3])->Value());
  }

  MixedProfile<double> output = LogitDynamics(MixedProfile<double>(start), 
					      lambda, tmax,
					      (file) ? *file : gnull);

  if (file) {
    delete file;
  }

  return new MixedPortion(new MixedSolution(output));
}

//---------------
// QreSolve
//---------------

#include "nfgqre.h"
#include "efgqre.h"

static Portion *GSM_Qre_Start(GSM &gsm, Portion **param)
{
  gOutput *pxiFile = 0;
  if (((TextPortion *) param[1])->Value() != "") {
    pxiFile = new gFileOutput(((TextPortion *) param[1])->Value());
  }

  if (param[0]->Spec().Type == porMIXED)  {
    MixedSolution &start = *((MixedPortion *) param[0])->Value();

    QreNfg qre;
    qre.SetMaxLambda(((NumberPortion *) param[3])->Value());
    qre.SetFullGraph(((BoolPortion *) param[6])->Value());

    Correspondence<double, MixedSolution> qreCorresp;
    gsm.StartAlgorithmMonitor("QreSolve Progress");
    try {
      gWatch watch;
      qre.Solve(MixedProfile<double>(start), 
		((NumberPortion *) param[2])->Value(),
		(pxiFile) ? *pxiFile : gnull,
		gsm.GetStatusMonitor(), qreCorresp);

      ((NumberPortion *) param[8])->SetValue(watch.Elapsed());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      if (pxiFile) delete pxiFile;  
      gsm.EndAlgorithmMonitor();
      throw;
    }

    if (pxiFile) delete pxiFile;
    gsm.EndAlgorithmMonitor();

    gList<MixedSolution> solutions;
    for (int i = 1; i <= qreCorresp.NumPoints(1); i++) {
      solutions.Append(qreCorresp.GetPoint(1, i));
      solutions[solutions.Length()].SetQre(qreCorresp.GetParameter(1, i), 0);
    }
    return new Mixed_ListPortion(solutions);
  }
  else  {     // BEHAV
    BehavSolution &start = *((BehavPortion *) param[0])->Value();
    Efg::Game &E = start.GetGame();
  
    if (!IsPerfectRecall(E)) {
      gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Qre; results not guaranteed\n";
    }

    QreEfg qre;
    qre.SetMaxLambda(((NumberPortion *) param[3])->Value());
    qre.SetFullGraph(((NumberPortion *) param[6])->Value());
    
    gList<BehavSolution> solutions;
    gsm.StartAlgorithmMonitor("QreSolve Progress");
    try {
      long nevals, niters;
      gWatch watch;
    
      qre.Solve(E, (pxiFile) ? *pxiFile : gnull, 
		gsm.GetStatusMonitor(), solutions);

      ((NumberPortion *) param[8])->SetValue(watch.Elapsed());
      ((NumberPortion *) param[9])->SetValue(nevals);
      ((NumberPortion *) param[10])->SetValue(niters);
    }
    catch (gSignalBreak &) { }
    catch (...) {
      if (pxiFile) delete pxiFile;
      gsm.EndAlgorithmMonitor();
      throw;
    }

    if (pxiFile) delete pxiFile;
    gsm.EndAlgorithmMonitor();

    return new Behav_ListPortion(solutions);
  }
}

#ifdef INTERNAL_VERSION

//---------------
// KQreSolve
//---------------

static Portion *GSM_KQre_Start(GSM &gsm, Portion **param)
{
  if (param[0]->Spec().Type == porMIXED)  {
    MixedSolution &start = *((MixedPortion *) param[0])->Value();
    Nfg &N = start.Game();

    NFQreParams NP;
    if (((TextPortion *) param[1])->Value() != "")
      NP.pxifile = new gFileOutput(((TextPortion *) param[1])->Value());
    else
      NP.pxifile = &gnull;
    NP.minLam = ((NumberPortion *) param[2])->Value();
    NP.maxLam = ((NumberPortion *) param[3])->Value();
    NP.delLam = ((NumberPortion *) param[4])->Value();
    NP.powLam = ((NumberPortion *) param[5])->Value();
    NP.fullGraph = ((BoolPortion *) param[6])->Value();

    NP.SetAccuracy( ((NumberPortion *) param[7])->Value());

    NP.tracefile = &((OutputPortion *) param[11])->Value();
    NP.trace = ((NumberPortion *) param[12])->Value();

    gList<MixedSolution> solutions;
    try {
      long nevals, niters;
      gWatch watch;
      KQre(N, NP, MixedProfile<gNumber>(start), solutions, nevals, niters);

      ((NumberPortion *) param[8])->SetValue(watch.Elapsed());
      ((NumberPortion *) param[9])->SetValue(nevals);
      ((NumberPortion *) param[10])->SetValue(niters);
    }
    catch (gSignalBreak &) {
      NP.status.Reset();
    }
    catch (...) {
      if (NP.pxifile != &gnull)  delete NP.pxifile;
      throw;
    }

    if (NP.pxifile != &gnull)  delete NP.pxifile;
    return new Mixed_ListPortion(solutions);
  }
  else  {     // BEHAV
    BehavSolution &start = *((BehavPortion *) param[0])->Value();
    Efg &E = start.Game();
  
    if (!IsPerfectRecall(E)) {
      gsm.OutputStream() << "WARNING: Solving game of imperfect recall with KQre; results not guaranteed\n";
    }

    EFQreParams EP;
    if(((TextPortion*) param[1])->Value() != "")
      EP.pxifile = new gFileOutput(((TextPortion*) param[1])->Value());
    else
      EP.pxifile = &gnull;
    EP.minLam = ((NumberPortion *) param[2])->Value();
    EP.maxLam = ((NumberPortion *) param[3])->Value();
    EP.delLam = ((NumberPortion *) param[4])->Value();
    EP.powLam = ((NumberPortion *) param[5])->Value();
    EP.fullGraph = ((BoolPortion *) param[6])->Value();
    
    EP.SetAccuracy( ((NumberPortion *) param[7])->Value());

    EP.tracefile = &((OutputPortion *) param[11])->Value();
    EP.trace = ((NumberPortion *) param[12])->Value();
    
    gList<BehavSolution> solutions;
    try {
      long nevals, niters;
      gWatch watch;
    
      KQre(E, EP, BehavProfile<gNumber>(start), solutions, nevals, niters);

      ((NumberPortion *) param[8])->SetValue(watch.Elapsed());
      ((NumberPortion *) param[9])->SetValue(nevals);
      ((NumberPortion *) param[10])->SetValue(niters);
    }
    catch (gSignalBreak &) {
      EP.status.Reset();
    }
    catch (...) {
      if (EP.pxifile != &gnull)  delete EP.pxifile;
      throw;
    }

    if (EP.pxifile != &gnull)   delete EP.pxifile;
    return new Behav_ListPortion(solutions);
  }
}

#endif // INTERNAL_VERSION

//------------
// LcpSolve
//------------

#include "lemke.h"

static Portion *GSM_Lcp_Nfg(GSM &gsm, Portion **param)
{
  NFSupport& S = * ((NfSupportPortion*) param[0])->Value();
  const Nfg *N = &S.Game();

  if (N->NumPlayers() > 2)
    throw gclRuntimeError("Only valid for two-person games");

  LemkeParams params;
  params.stopAfter = ((NumberPortion *) param[1])->Value();
  params.precision = ((PrecisionPortion *) param[2])->Value();
  params.tracefile = &((OutputPortion *) param[5])->Value();
  params.trace = ((NumberPortion *) param[6])->Value();

  gList<MixedSolution> solutions;
  gsm.StartAlgorithmMonitor("LcpSolve Progress");
  try {
    double time;
    int npivots;
    Lemke(S, params, solutions, gsm.GetStatusMonitor(), npivots, time);
    ((NumberPortion *) param[3])->SetValue(npivots);
    ((NumberPortion *) param[4])->SetValue(time);
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

#include "seqform.h"
#include "lemkesub.h"

static Portion *GSM_Lcp_Efg(GSM &gsm, Portion **param)
{
  EFSupport &support = *((EfSupportPortion*) param[0])->Value();
  const Efg::Game *E = &support.GetGame();

  if (E->NumPlayers() > 2)
    throw gclRuntimeError("Only valid for two-person games");

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Lcp; results not guaranteed\n";
  }

  gsm.StartAlgorithmMonitor("LcpSolve Progress");
  if (((BoolPortion *) param[1])->Value())   {
    LemkeParams params;
    
    params.stopAfter = ((NumberPortion *) param[2])->Value();
    params.precision = ((PrecisionPortion *) param[3])->Value();
    params.tracefile = &((OutputPortion *) param[6])->Value();
    params.trace = ((NumberPortion *) param[7])->Value();

    gList<BehavSolution> solutions;
    try {
      efgLcpNfgSolve algorithm(support, params);
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
      ((NumberPortion *) param[4])->SetValue(algorithm.NumPivots());
      ((NumberPortion *) param[5])->SetValue(algorithm.Time());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Behav_ListPortion(solutions);
  }
  else  {
    SeqFormParams params;

    params.stopAfter = ((NumberPortion *) param[2])->Value();
    params.precision = ((PrecisionPortion *) param[3])->Value();
    params.tracefile = &((OutputPortion *) param[6])->Value();
    params.trace = ((NumberPortion *) param[7])->Value();

    gList<BehavSolution> solutions;
    try {
      efgLcpSolve algorithm(support, params);
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
      ((NumberPortion *) param[4])->SetValue(algorithm.NumPivots());
      ((NumberPortion *) param[5])->SetValue(algorithm.Time());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Behav_ListPortion(solutions);
  }
}

#include "lemketab.h"

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

#include "liapsub.h"
#include "eliap.h"

static Portion *GSM_Liap_Behav(GSM &gsm, Portion **param)
{
  BehavProfile<gNumber> start(*((BehavPortion *) param[0])->Value());
  Efg::Game &E = start.GetGame();
  const EFSupport &supp = (*((BehavPortion *) param[0])->Value()).Support();
  
  gsm.StartAlgorithmMonitor("LiapSolve Progress");
  if (((BoolPortion *) param[1])->Value())   {
    NFLiapParams LP;

    LP.stopAfter = ((NumberPortion *) param[2])->Value();
    LP.nTries = ((NumberPortion *) param[3])->Value();

    LP.SetAccuracy( ((NumberPortion *) param[4])->Value());

    LP.tracefile = &((OutputPortion *) param[7])->Value();
    LP.trace = ((NumberPortion *) param[8])->Value();

    gWatch watch;

    gList<BehavSolution> solutions;

    try {
      efgLiapNfgSolve algorithm(E, LP, start);
      solutions = algorithm.Solve(supp, gsm.GetStatusMonitor());
      ((NumberPortion *) param[5])->SetValue(watch.Elapsed());
      ((NumberPortion *) param[6])->SetValue(algorithm.NumEvals());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Behav_ListPortion(solutions);
  }
  else  {
    EFLiapParams LP;

    if (!IsPerfectRecall(E)) {
      gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Liap; results not guaranteed\n";
    }

    LP.stopAfter = ((NumberPortion *) param[2])->Value();
    LP.nTries = ((NumberPortion *) param[3])->Value();

    LP.SetAccuracy( ((NumberPortion *) param[4])->Value());

    LP.tracefile = &((OutputPortion *) param[7])->Value();
    LP.trace = ((NumberPortion *) param[8])->Value();

    gWatch watch;

    gList<BehavSolution> solutions;

    try {
      efgLiapSolve algorithm(E, LP, start);
      solutions = algorithm.Solve(supp, gsm.GetStatusMonitor());
      ((NumberPortion *) param[5])->SetValue(watch.Elapsed());
      ((NumberPortion *) param[6])->SetValue(algorithm.NumEvals());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Behav_ListPortion(solutions);
  }
}

#include "nliap.h"

static Portion *GSM_Liap_Mixed(GSM &gsm, Portion **param)
{
  MixedProfile<gNumber> start(*((MixedPortion *) param[0])->Value());
  Nfg &N = start.Game();

  NFLiapParams params;
  
  params.stopAfter = ((NumberPortion *) param[1])->Value();
  params.nTries = ((NumberPortion *) param[2])->Value();

  params.SetAccuracy( ((NumberPortion *) param[3])->Value());

  params.tracefile = &((OutputPortion *) param[6])->Value();
  params.trace = ((NumberPortion *) param[7])->Value();

  gList<MixedSolution> solutions;
  gsm.StartAlgorithmMonitor("LiapSolve Progress");
  try {
    long nevals, niters;
    gWatch watch;
  
    Liap(N, params, start, solutions, gsm.GetStatusMonitor(), nevals, niters);

    ((NumberPortion *) param[4])->SetValue(watch.Elapsed());
    ((NumberPortion *) param[5])->SetValue(nevals);
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

#include "nfgcsum.h"

static Portion *GSM_Lp_Nfg(GSM &gsm, Portion **param)
{
  NFSupport& S = * ((NfSupportPortion*) param[0])->Value();
  const Nfg *N = &S.Game();

  if (N->NumPlayers() > 2 || !IsConstSum(*N))
    throw gclRuntimeError("Only valid for two-person zero-sum games");

  ZSumParams params;
  params.stopAfter = 1;
  params.precision = ((PrecisionPortion *) param[1])->Value();
  params.tracefile = &((OutputPortion *) param[4])->Value();
  params.trace = ((NumberPortion *) param[5])->Value();

  gList<MixedSolution> solutions;
  gsm.StartAlgorithmMonitor("LpSolve Progress");
  try {
    double time;
    int npivots;
    ZSum(S, params, solutions, gsm.GetStatusMonitor(), npivots, time);
    ((NumberPortion *) param[2])->SetValue(npivots);
    ((NumberPortion *) param[3])->SetValue(time);
  }
  catch (gSignalBreak &) { }
  catch (...) {
    gsm.EndAlgorithmMonitor();
    throw;
  }

  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}


#include "lpsolve.h"

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


#include "csumsub.h"
#include "efgcsum.h"

static Portion *GSM_Lp_Efg(GSM &gsm, Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();
  const Efg::Game &E = support.GetGame();
  
  if (E.NumPlayers() > 2 || !E.IsConstSum())
    throw gclRuntimeError("Only valid for two-person zero-sum games");

  if (!IsPerfectRecall(E)) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Lp; results not guaranteed\n";
  }

  gsm.StartAlgorithmMonitor("LpSolve Progress");
  if (((BoolPortion *) param[1])->Value())   {
    ZSumParams params;
    params.stopAfter = 1;
    params.precision = ((PrecisionPortion *) param[2])->Value();
    params.tracefile = &((OutputPortion *) param[5])->Value();
    params.trace = ((NumberPortion *) param[6])->Value();

    gList<BehavSolution> solutions;
    try {
      efgLpNfgSolve algorithm(support, params);
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
      ((NumberPortion *) param[3])->SetValue(algorithm.NumPivots());
      ((NumberPortion *) param[4])->SetValue(algorithm.Time());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Behav_ListPortion(solutions);
  }
  else  {
    CSSeqFormParams params;
    params.stopAfter = 1;
    params.precision = ((PrecisionPortion *) param[2])->Value();
    params.tracefile = &((OutputPortion *) param[5])->Value();
    params.trace = ((NumberPortion *) param[6])->Value();

    gList<BehavSolution> solutions;

    try {
      efgLpSolve algorithm(support, params);
      solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
      ((NumberPortion *) param[3])->SetValue(algorithm.NumPivots());
      ((NumberPortion *) param[4])->SetValue(algorithm.Time());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Behav_ListPortion(solutions);
  }
}


//------------------
//  PolEnumSolve (was AllNashSolve)
//------------------

#include "nfgalleq.h"

static Portion *GSM_PolEnumSolve_Nfg(GSM &gsm, Portion **param)
{
  const NFSupport &S = *((NfSupportPortion*) param[0])->Value();
  PolEnumParams params;
  params.stopAfter = ((NumberPortion *) param[1])->Value();
  params.tracefile = &((OutputPortion *) param[4])->Value();
  params.trace = ((NumberPortion *) param[5])->Value();
  bool recurse = ((BoolPortion *) param[7])->Value();
  gList<MixedSolution> solutions;
  gList<const NFSupport> singular_supports;

  gsm.StartAlgorithmMonitor("PolEnumSolve Progress");
  if (recurse) {
    try {
      long nevals = 0;
      double time = 0.0;
      AllNashSolve(S, params, solutions, gsm.GetStatusMonitor(),
		   nevals, time, singular_supports);
      
      ((NumberPortion *) param[2])->SetValue(nevals);
      ((NumberPortion *) param[3])->SetValue(time);
      ((NfSupport_ListPortion *) param[6])->SetValue(singular_supports);
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }
  }
  else {
    bool is_singular;
    try {
      long nevals;
      double time;
      PolEnum(S, params, solutions, gsm.GetStatusMonitor(),
	      nevals, time, is_singular);
      ((NumberPortion *) param[2])->SetValue(nevals);
      ((NumberPortion *) param[3])->SetValue(time);
      if (is_singular) {
	singular_supports.Append(S);
      }
      ((NfSupport_ListPortion *) param[6])->SetValue(singular_supports);
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }
  }
  gsm.EndAlgorithmMonitor();
  return new Mixed_ListPortion(solutions);
}

#include "efgalleq.h"
#include "polensub.h"

static Portion *GSM_PolEnumSolve_Efg(GSM &gsm, Portion **param)
{
  const EFSupport &S = *((EfSupportPortion*) param[0])->Value();
  bool recurse = ((BoolPortion *) param[8])->Value();
  gList<BehavSolution> solutions;
  gList<const EFSupport> singular_supports;

  if (!IsPerfectRecall(S.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with AllNash; results not guaranteed\n";
  }
  // If asNfg->True (salvaged from old PolEnum_Efg)  

  gsm.StartAlgorithmMonitor("PolEnumSolve Progress");
  if (((BoolPortion *) param[1])->Value()) {
    // need to add recurse capability here
    if (recurse) { 
      gsm.EndAlgorithmMonitor();
      throw gclRuntimeError("Recursion not implemented for asNfg->True");
    }
    PolEnumParams params;
    params.stopAfter = ((NumberPortion *) param[2])->Value();
    params.tracefile = &((OutputPortion *) param[5])->Value();
    params.trace = ((NumberPortion *) param[6])->Value();
    try {
      efgPolEnumNfgSolve algorithm(S, params);
      solutions = algorithm.Solve(S, gsm.GetStatusMonitor());
      ((NumberPortion *) param[3])->SetValue(algorithm.NumEvals());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }
  }
  else {
    // end salvage
    EfgPolEnumParams params;
    params.stopAfter = ((NumberPortion *) param[2])->Value();
    params.tracefile = &((OutputPortion *) param[5])->Value();
    params.trace = ((NumberPortion *) param[6])->Value();
    if (recurse) {
      try {
	long nevals = 0;
	double time = 0.0;
	AllEFNashSolve(S, params, solutions, gsm.GetStatusMonitor(), 
		       nevals, time, singular_supports);
	
	((NumberPortion *) param[3])->SetValue(nevals);
	((NumberPortion *) param[4])->SetValue(time);
	((EfSupport_ListPortion *) param[7])->SetValue(singular_supports);
      }
      catch (gSignalBreak &) { }
      catch (...) {
	gsm.EndAlgorithmMonitor();
	throw;
      }
    }
    else {
      bool is_singular = false;
      try {
	long nevals;
	double time;
	EfgPolEnum(S, params, solutions, gsm.GetStatusMonitor(),
		   nevals, time, is_singular);
	((NumberPortion *) param[3])->SetValue(nevals);
	((NumberPortion *) param[4])->SetValue(time);
	if (is_singular) {
	  singular_supports.Append(S);
	}
	((EfSupport_ListPortion *) param[7])->SetValue(singular_supports);
      }
      catch (gSignalBreak &) { }
      catch (...) {
	gsm.EndAlgorithmMonitor();
	throw;
      }
    }
  }
  gsm.EndAlgorithmMonitor();
  return new Behav_ListPortion(solutions);
}

#ifdef INTERNAL_VERSION

#include "seqeq.h"

static Portion *GSM_SequentialEquilib(GSM &gsm, Portion **param)
{
  EFBasis &basis = *((EfBasisPortion *) param[0])->Value();
  EFSupport &support = *((EfSupportPortion *) param[1])->Value();
  
  if (!IsPerfectRecall(support.Game())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with SequentialEquilib; results not guaranteed\n";
  }

  double time;
  gList<BehavSolution> solutions;
  
  SeqEquilibParams params;
  params.stopAfter = ((NumberPortion *) param[2])->Value();
  params.precision = ((PrecisionPortion *) param[3])->Value();
  params.tracefile = &((OutputPortion *) param[6])->Value();
  params.trace = ((NumberPortion *) param[7])->Value();

  try {
    long nevals;
    SequentialEquilib(basis, support, params, solutions, nevals, time);

    ((NumberPortion *) param[4])->SetValue(nevals);
    ((NumberPortion *) param[5])->SetValue(time);
  }
  catch (gSignalBreak &) {
    params.status.Reset();
  }

  return new Behav_ListPortion(solutions);
}
#endif // INTERNAL_VERSION

//---------
// Nfg
//---------

static Portion *GSM_Nfg(GSM &, Portion **param)
{
  Efg::Game &E = * ((EfgPortion*) param[0])->Value();
  gWatch watch;

  Nfg *N = MakeReducedNfg(EFSupport(E));

  ((NumberPortion *) param[1])->SetValue(watch.Elapsed());

  if (N)
    return new NfgPortion(N);
  else
    throw gclRuntimeError("Conversion to reduced nfg failed");
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
  MixedProfile<gNumber> mp(*((MixedPortion*) param[0])->Value());
  NFPlayer *player = ((NfPlayerPortion *) param[1])->Value();

  return new NumberPortion(mp.Payoff(player->GetNumber()));
}

//----------------
// SimpDivSolve
//----------------

#include "simpdiv.h"

static Portion *GSM_Simpdiv_Nfg(GSM &gsm, Portion **param)
{
  NFSupport& S = * ((NfSupportPortion*) param[0])->Value();
  SimpdivParams SP;
  SP.stopAfter = ((NumberPortion *) param[1])->Value();
  SP.nRestarts = ((NumberPortion *) param[2])->Value();
  SP.leashLength = ((NumberPortion *) param[3])->Value();

  SP.tracefile = &((OutputPortion *) param[7])->Value();
  SP.trace = ((NumberPortion *) param[8])->Value();

  gsm.StartAlgorithmMonitor("SimpDivSolve Progress");
  if (((PrecisionPortion *) param[4])->Value() == precDOUBLE)  {
    gList<MixedSolution> solutions;

    try {
      SimpdivModule<double> SM(S, SP);
      SM.Simpdiv(gsm.GetStatusMonitor());
      solutions = SM.GetSolutions();
      ((NumberPortion *) param[5])->SetValue(SM.NumEvals());
      ((NumberPortion *) param[6])->SetValue(SM.Time());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Mixed_ListPortion(solutions);
  }
  else  {
    gList<MixedSolution> solutions;

    try {
      SimpdivModule<gRational> SM(S, SP);
      SM.Simpdiv(gsm.GetStatusMonitor());
      solutions = SM.GetSolutions();

      ((NumberPortion *) param[5])->SetValue(SM.NumEvals());
      ((NumberPortion *) param[6])->SetValue(SM.Time());
    }
    catch (gSignalBreak &) { }
    catch (...) {
      gsm.EndAlgorithmMonitor();
      throw;
    }

    gsm.EndAlgorithmMonitor();
    return new Mixed_ListPortion(solutions);
  }
}

#include "simpsub.h"

static Portion *GSM_Simpdiv_Efg(GSM &gsm, Portion **param)
{
  EFSupport &support = *((EfSupportPortion *) param[0])->Value();

  if (!((BoolPortion *) param[1])->Value())
    throw gclRuntimeError("algorithm not implemented for extensive forms");

  if (!IsPerfectRecall(support.GetGame())) {
    gsm.OutputStream() << "WARNING: Solving game of imperfect recall with Simpdiv; results not guaranteed\n";
  }

  SimpdivParams params;
  params.stopAfter = ((NumberPortion *) param[2])->Value();
  params.nRestarts = ((NumberPortion *) param[3])->Value();
  params.leashLength = ((NumberPortion *) param[4])->Value();
  params.precision = ((PrecisionPortion *) param[5])->Value();
  params.tracefile = &((OutputPortion *) param[8])->Value();
  params.trace = ((NumberPortion *) param[9])->Value();

  gList<BehavSolution> solutions;
  gsm.StartAlgorithmMonitor("SimpDivSolve Progress");
  try {
    efgSimpDivNfgSolve algorithm(support, params);
    solutions = algorithm.Solve(support, gsm.GetStatusMonitor());
    ((NumberPortion *) param[6])->SetValue(algorithm.NumEvals());
    ((NumberPortion *) param[7])->SetValue(algorithm.Time());
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

//------------------
// YamamotoSolve
//------------------

#include "yamamoto.h"

static Portion *GSM_YamamotoSolve(GSM &gsm, Portion **param)
{
  NFSupport *support = ((NfSupportPortion*) param[0])->Value();
  gList<MixedSolution> solutions;

  try {
    gsm.StartAlgorithmMonitor("YamamotoSolve");
    Yamamoto(*support, gsm.GetStatusMonitor(), solutions);
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
					    new OutputPortion(gnull), 
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
					    new OutputPortion(gnull), 
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
					    new OutputPortion(gnull), 
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
					    new OutputPortion(gnull), 
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
					     new OutputPortion(gnull),
					     BYREF));
  FuncObj->SetParamInfo(0, 14, gclParameter("traceLevel", porNUMBER,
					     new NumberPortion(0)));

  gsm->AddFunction(FuncObj);

  FuncObj = new gclFunction(*gsm, "LogitDyn", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_Qre_Dynamics, porMIXED, 4));
  FuncObj->SetParamInfo(0, 0, gclParameter("start", porMIXED));
  FuncObj->SetParamInfo(0, 1, gclParameter("lambda", porNUMBER));
  FuncObj->SetParamInfo(0, 2, gclParameter("tmax", porNUMBER));
  FuncObj->SetParamInfo(0, 3, gclParameter("output", porTEXT));
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
					     new OutputPortion(gnull), 
					     BYREF));
  FuncObj->SetParamInfo(0, 12, gclParameter("traceLevel", porNUMBER,
					     new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 13, gclParameter("method", porINTEGER,
					    new NumberPortion(0)));

  gsm->AddFunction(FuncObj);

#ifdef INTERNAL_VERSION
  FuncObj = new gclFunction(*gsm, "KQreSolve", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_KQre_Start, 
				       PortionSpec(porMIXED | porBEHAV , 1), 13));
  FuncObj->SetParamInfo(0, 0, gclParameter("start",
					    porMIXED | porBEHAV));
  FuncObj->SetParamInfo(0, 1, gclParameter("pxifile", porTEXT,
					    new TextPortion("")));
  FuncObj->SetParamInfo(0, 2, gclParameter("minK", porNUMBER,
					    new NumberPortion(0.001)));
  FuncObj->SetParamInfo(0, 3, gclParameter("maxK", porNUMBER,
					    new NumberPortion(500.0)));
  FuncObj->SetParamInfo(0, 4, gclParameter("delK", porNUMBER,
					    new NumberPortion(-0.1)));
  FuncObj->SetParamInfo(0, 5, gclParameter("powK", porINTEGER,
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
					     new OutputPortion(gnull), 
					     BYREF));
  FuncObj->SetParamInfo(0, 12, gclParameter("traceLevel", porNUMBER,
					     new NumberPortion(0)));

  gsm->AddFunction(FuncObj);
#endif // INTERNAL_VERSION

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
					    new OutputPortion(gnull),
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
					    new OutputPortion(gnull), 
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
					     new OutputPortion(gnull), 
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
					    new OutputPortion(gnull), 
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
					    new OutputPortion(gnull),
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
					    new OutputPortion(gnull), 
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
					    new OutputPortion(gnull), 
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
					    new OutputPortion(gnull), 
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

#ifdef INTERNAL_VERSION
  FuncObj = new gclFunction(*gsm, "SeqEquilibSolve", 1);
  FuncObj->SetFuncInfo(0, gclSignature(GSM_SequentialEquilib, 
				       PortionSpec(porBEHAV, 1), 8));
  FuncObj->SetParamInfo(0, 0, gclParameter("basis", porEFBASIS));
  FuncObj->SetParamInfo(0, 1, gclParameter("support", porEFSUPPORT));
  FuncObj->SetParamInfo(0, 2, gclParameter("stopAfter", porINTEGER,
					    new NumberPortion(0)));
  FuncObj->SetParamInfo(0, 3, gclParameter("precision", porPRECISION,
					   new PrecisionPortion(precDOUBLE)));
  FuncObj->SetParamInfo(0, 4, gclParameter("nEvals", porINTEGER,
					    new NumberPortion(0), BYREF));
  FuncObj->SetParamInfo(0, 5, gclParameter("time", porNUMBER,
					    new NumberPortion(0.0), BYREF));
  FuncObj->SetParamInfo(0, 6, gclParameter("traceFile", porOUTPUT,
					    new OutputPortion(gnull), 
					    BYREF));
  FuncObj->SetParamInfo(0, 7, gclParameter("traceLevel", porNUMBER,
					    new NumberPortion(0)));
  gsm->AddFunction(FuncObj);

#endif // INTERNAL_VERSION

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
					    new OutputPortion(gnull),
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
					    new OutputPortion(gnull),
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
  FuncObj->SetFuncInfo(0, gclSignature(GSM_YamamotoSolve,
				       PortionSpec(porMIXED, 1), 1));
  FuncObj->SetParamInfo(0, 0, gclParameter("support", porNFSUPPORT));
  gsm->AddFunction(FuncObj);
}
