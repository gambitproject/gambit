//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of (agent) QRE computation for extensive forms
//

#include <math.h>

#include "efgqre.h"

#include "gfunc.h"
#include "math/math.h"
#include "math/gsmatrix.h"
#include "gnullstatus.h"

EFQreParams::EFQreParams(void)
  : m_homotopy(true), powLam(1), minLam(0.01), maxLam(30.0), delLam(0.01),
    m_stepSize(0.0001), fullGraph(false)
{ }

//=========================================================================
//                  QRE version of Liapunov function
//=========================================================================

class EFQreFunc : public gFunction<double>   {
  private:
    long _nevals;
    bool _domain_err;
    const Efg::Game &_efg;
    gVector<double> _Lambda;
    gPVector<double> _probs;
    BehavProfile<double> _p;
    gVector<double> ***_scratch;

    double Value(const gVector<double> &);

  public:
    EFQreFunc(const Efg::Game &, const BehavProfile<gNumber> &);
    virtual ~EFQreFunc();
    
    gVector<double> GetLambda()   { return _Lambda; }
    void SetLambda(double l)   { _Lambda = l; }
    void SetLambda(const gVector<double> &l)   { _Lambda = l; }
    long NumEvals(void) const   { return _nevals; }
    bool DomainErr(void) const { return _domain_err;}
};

EFQreFunc::EFQreFunc(const Efg::Game &E,
		     const BehavProfile<gNumber> &start)
  : _nevals(0L), _domain_err(false), _efg(E), 
    _Lambda(E.NumPlayers()),_probs(E.NumInfosets()),
    _p(start.Support())
{
  for (int i = 1; i <= _p.Length(); i++)
    _p[i] = start[i];

  _scratch = new gVector<double> **[_efg.NumPlayers()] - 1;
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    int nisets = (_efg.Players()[pl])->NumInfosets();
    _scratch[pl] = new gVector<double> *[nisets + 1] - 1;
    for (int iset = 1; iset <= nisets; iset++)
      _scratch[pl][iset] = new gVector<double>(_p.Support().NumActions(pl, iset));
  }
}

EFQreFunc::~EFQreFunc()
{
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    int nisets = (_efg.Players()[pl])->NumInfosets();
    for (int iset = 1; iset <= nisets; iset++)
      delete _scratch[pl][iset];
    delete [] (_scratch[pl] + 1);
  }
  delete [] (_scratch + 1);
}

double EFQreFunc::Value(const gVector<double> &v)
{
  _nevals++;
  _domain_err = false;
  ((gVector<double> &) _p).operator=(v);
  //  _p = v;
  return _p.QreValue(_Lambda,_domain_err);
}

static void WritePXIHeader(gOutput &pxifile, const Efg::Game &E,
			   const EFQreParams &params)
{
  int pl, iset, nisets = 0;

  pxifile << "Dimensionality:\n";
  for (pl = 1; pl <= E.NumPlayers(); pl++)
    nisets += E.Players()[pl]->NumInfosets();
  pxifile << nisets;
  for (pl = 1; pl <= E.NumPlayers(); pl++)
    for (iset = 1; iset <= E.Players()[pl]->NumInfosets(); iset++)
      pxifile << " " << E.Players()[pl]->Infosets()[iset]->NumActions();
  pxifile << "\n";

	pxifile << "Settings:\n" << params.minLam;
	pxifile << "\n" << params.maxLam << "\n" << params.delLam;
	pxifile << "\n" << 0 << "\n" << 1 << "\n" << params.powLam << "\n";

  int numcols = E.ProfileLength() + 2;
  pxifile << "DataFormat:";
  pxifile << "\n" << numcols;
  for (int i = 1; i <= numcols; i++)
    pxifile << ' ' << i;
  pxifile << "\nData:\n";
}

static void AddSolution(gList<BehavSolution> &solutions,
			const BehavProfile<double> &profile,
			double lambda,
			double value, double epsilon)
{
  int i = solutions.Append(BehavSolution(profile, algorithmEfg_QRE_EFG));
  solutions[i].SetQre(lambda, value);
  solutions[i].SetEpsilon(epsilon);
}

extern void Project(gVector<double> &, const gArray<int> &);

static void InitMatrix(gMatrix<double> &xi, const gArray<int> &dim)
{
  xi.MakeIdent();

  gVector<double> foo(xi.NumColumns());
  for (int i = 1; i <= xi.NumRows(); i++)   {
    xi.GetRow(i, foo);
    Project(foo, dim);
    xi.SetRow(i, foo);
  }
}

//=========================================================================
//            QRE Correspondence Computation via Optimization
//=========================================================================

extern bool Powell(gPVector<double> &p, gMatrix<double> &xi,
		   gFunction<double> &func, double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile, int tracelevel,  bool interior,
		   gStatus &status);



void QreOptimization(const Efg::Game &E, EFQreParams &params, 
		     gOutput &p_pxifile,
		     const BehavProfile<gNumber> &start,
		     gList<BehavSolution> &solutions, gStatus &p_status,
		     long &nevals, long &nits)
{
  static const double ALPHA = .00000001;

  EFQreFunc F(E, start);

  int iter = 0;
  double Lambda, LambdaOld, LambdaSave, LambdaStart, value = 0.0;

  WritePXIHeader(p_pxifile, E, params);

  LambdaStart = (params.delLam < 0.0) ? params.maxLam : params.minLam;
  LambdaOld = LambdaSave = Lambda = LambdaStart;

  double max_prog, prog;

  if (params.powLam == 0)
    max_prog = params.maxLam - params.minLam;
  else
    max_prog = log(params.maxLam / params.minLam);

  BehavProfile<double> p(start.Support());
  for (int i = 1; i <= p.Length(); i++)
    p[i] = start[i];

  // if starting vector not interior, perturb it towards centroid
  int kk;
  for(kk=1;kk <= p.Length() && p[kk]>ALPHA;kk++);
  if(kk<=p.Length()) {
    BehavProfile<double> c(start.Support());
    for(int k=1;k<=p.Length();k++)
      p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
  }

  BehavProfile<double> pold(p);
  BehavProfile<double> psave(p);
  BehavProfile<double> pdiff(p);
  pdiff-= pold;
  gMatrix<double> xi(p.Length(), p.Length());

  InitMatrix(xi, p.Lengths());

  bool FoundSolution = true;
  double delta, mindelta;
  delta = params.delLam;
  mindelta = delta/10000.0;

  try {
    while (delta>mindelta &&
		      Lambda <= params.maxLam && Lambda >= params.minLam)   {
      p_status.Get();
      F.SetLambda(Lambda);
      
      gPVector<double> pvect(p.GetPVector());
      FoundSolution = Powell(pvect, xi, F, value, iter,
		      params.maxits1, params.tol1, params.maxitsN, params.tolN,
		      *params.tracefile, params.trace-1,true, p_status);
      p = pvect;
      bool derr = F.DomainErr();      
      // dist = dist to last good point
      // dsave = dist to last saved point
      double dist = 0.0;
      assert(LambdaSave>=1.0e-200);
      double  dsave = params.powLam ? abs(Lambda/LambdaSave - 1.0) : abs(Lambda-LambdaSave);
      for(int jj=p.First();jj<=p.Last();jj++) {
	double xx = abs(p[jj]-pold[jj]);
	if(xx>dist)dist=xx;
	xx = abs(p[jj]-psave[jj]);
	if(xx>dsave)dsave=xx;
      }

      if(FoundSolution && !derr && (Lambda == LambdaStart || dist < params.delLam)) {

	if (params.trace>0)  {
	  *params.tracefile << "\nLam: " << Lambda << " val: ";
	  (*params.tracefile).SetExpMode() << value;
	  (*params.tracefile).SetFloatMode() << " p: " << p;
	} 
	
	if(dsave > params.delLam/4.0) {

	  p_pxifile << "\n" << Lambda << " " << value << " ";
	  for (int pl = 1; pl <= E.NumPlayers(); pl++)
	    for (int iset = 1; iset <= E.Players()[pl]->NumInfosets();
		 iset++)  {
	      double prob = 0.0;
	      for (int act = 1; act <= E.Players()[pl]->Infosets()[iset]->NumActions(); 
		   prob += p(pl, iset, act++))
		p_pxifile << p(pl, iset, act) << ' ';
	    } 
	
	  if (params.fullGraph)
	    AddSolution(solutions, p, Lambda, value, params.Accuracy());

	  psave=p;
	}

	pdiff = p; pdiff-= pold;
	pold=p;                              // pold is last good solution
	if(delta < params.delLam && dist<params.delLam/2.0) {
	  delta*=2.0;
	  pdiff*=2.0;
	}
      }

      else {
	Lambda = LambdaOld;
	InitMatrix(xi, p.Lengths());
	if(delta>mindelta) { 
	  delta/=2.0;
	  pdiff*=0.5;
	}
	p = pold;
      }

      for(int jj = p.First();jj<=p.Last();jj++)
	assert (p[jj] > 0.0);
      
      bool flag = false;
      int jj = p.First();
      while(jj<=p.Last() && !flag) {
	if(p[jj]+pdiff[jj]<0.0)flag = true;
	jj++;
      }
      if(flag) {
	for(jj = pdiff.First();jj<=pdiff.Last();jj++)
	  pdiff[jj] = 0.0;
      }
      p+=pdiff;

      if (params.powLam == 0)
	prog = abs(Lambda - LambdaStart);
      else
	prog = abs(log(Lambda/LambdaStart));
      p_status.SetProgress(prog/max_prog);

      LambdaOld = Lambda;
      Lambda += delta * pow(Lambda, (long)params.powLam);
    }

    if (!params.fullGraph)
      AddSolution(solutions, pold, Lambda, value, params.Accuracy());
    
    nevals = F.NumEvals();
    nits = 0;
  }
  catch (gSignalBreak &E) {
    if (!params.fullGraph) 
      AddSolution(solutions, pold, Lambda, value, params.Accuracy());

    nevals = F.NumEvals();
    nits = 0;
    throw;
  }
  catch (gFuncMinError &E) {
    if (!params.fullGraph)
      AddSolution(solutions, pold, Lambda, value, params.Accuracy());

    nevals = F.NumEvals();
    nits = 0;
    // This should be re-thrown, but wait til we have better exception handling downstream
    //    throw;  
  }
}

//=========================================================================
//             QRE Correspondence Computation via Homotopy
//=========================================================================

void QreJacobian(const Efg::Game &p_efg,
		 const BehavProfile<double> &p_profile,
		 const double &p_lambda, gMatrix<double> &p_matrix)
{
  p_matrix = (double) 0;

  int rowno = 0;  // indexes the row number in the Jacobian matrix
  for (int pl1 = 1; pl1 <= p_efg.NumPlayers(); pl1++) {
    EFPlayer *player1 = p_efg.Players()[pl1];

    for (int iset1 = 1; iset1 <= player1->NumInfosets(); iset1++) {
      Infoset *infoset1 = player1->Infosets()[iset1];
      for (int act1 = 1; act1 <= infoset1->NumActions(); act1++) {
	rowno++;

	int colno = 0;
	for (int pl2 = 1; pl2 <= p_efg.NumPlayers(); pl2++) {
	  EFPlayer *player2 = p_efg.Players()[pl2];
	  for (int iset2 = 1; iset2 <= player2->NumInfosets(); iset2++) {
	    Infoset *infoset2 = player2->Infosets()[iset2];

	    for (int act2 = 1; act2 <= infoset2->NumActions(); act2++) {
	      colno++;
	      if (infoset1 == infoset2) {
		if (act1 == act2) {
		  p_matrix(rowno, colno) = 1.0;
		}
		else {
		  p_matrix(rowno, colno) = 0.0;
		}
	      }
	      else {   // infoset1 != infoset2
		for (int k = 1; k <= infoset1->NumActions(); k++) {
		  p_matrix(rowno, colno) += (p_profile.DiffActionValue(infoset1->Actions()[k], infoset2->Actions()[act2]) - p_profile.DiffActionValue(infoset1->Actions()[act1], infoset2->Actions()[act2])) * p_profile.GetActionProb(infoset1->Actions()[k]);
		}
		p_matrix(rowno, colno) *= p_lambda * p_profile.GetActionProb(infoset1->Actions()[act1]);
	      }
	    }
	  }
	}

	// Now for the column wrt lambda
	for (int k = 1; k <= infoset1->NumActions(); k++) {
	  p_matrix(rowno, p_matrix.NumColumns()) += (p_profile.GetActionValue(infoset1->Actions()[k]) - p_profile.GetActionValue(infoset1->Actions()[act1])) * p_profile.GetActionProb(infoset1->Actions()[k]);
	}
	p_matrix(rowno, p_matrix.NumColumns()) *= p_profile.GetActionProb(infoset1->Actions()[act1]);
      }
    }
  }
  /*
  gout << "lambda= " << p_lambda << '\n';
  gout << "profile= " << p_profile << '\n';
  gout << p_matrix << '\n';
  */
}

void QreComputeStep(const Efg::Game &p_efg, 
		    const BehavProfile<double> &p_profile,
		    const gMatrix<double> &p_matrix,
		    gDPVector<double> &p_delta, double &p_lambdainc,
		    double p_initialsign, double p_stepsize)
{
  double sign = p_initialsign;
  int rowno = 0; 

  gSquareMatrix<double> M(p_matrix.NumRows());

  for (int row = 1; row <= M.NumRows(); row++) {
    for (int col = 1; col <= M.NumColumns(); col++) {
      M(row, col) = p_matrix(row, col + 1);
    }
  }

  for (int i = 1; i <= p_delta.Length(); i++) {
    rowno++;
    p_delta[i] = sign * M.Determinant();   
    sign *= -1.0;

    for (int row = 1; row <= M.NumRows(); row++) {
      M(row, rowno) = p_matrix(row, rowno);
      if (rowno < M.NumColumns()) {
	M(row, rowno + 1) = p_matrix(row, rowno + 2);
      }
    }
  }   

  p_lambdainc = sign * M.Determinant();

  double norm = 0.0;
  for (int i = 1; i <= p_delta.Length(); i++) {
    norm += p_delta[i] * p_delta[i];
  }
  norm += p_lambdainc * p_lambdainc; 

  for (int i = 1; i <= p_delta.Length(); i++) {
    p_delta[i] /= sqrt(norm / p_stepsize);
  }

  p_lambdainc /= sqrt(norm / p_stepsize);
}

void QreHomotopy(const Efg::Game &p_efg, EFQreParams &params, 
		 gOutput &p_pxifile,
		 const BehavProfile<gNumber> &p_start,
		 gList<BehavSolution> &solutions, gStatus &p_status,
		 long &nevals, long &nits)
{
  gMatrix<double> H(p_efg.ProfileLength(), p_efg.ProfileLength() + 1);
  BehavProfile<double> profile(p_start);
  double lambda = 0.0;
  double stepsize = params.m_stepSize;
  int numSteps = 0;

  WritePXIHeader(p_pxifile, p_efg, params);

  // Pick the direction to follow the path so that lambda starts out
  // increasing
  double initialsign = (p_efg.ProfileLength() % 2 == 0) ? 1.0 : -1.0;

  try {
    while (lambda <= params.maxLam) {
      // Use a first-order Runge-Kutta style method
      gDPVector<double> delta1(profile.GetDPVector());
      gDPVector<double> delta2(profile.GetDPVector());
      double lambdainc1, lambdainc2;

      QreJacobian(p_efg, profile, lambda, H);
      QreComputeStep(p_efg, profile, H,
		     delta1, lambdainc1, initialsign, stepsize);

      BehavProfile<double> profile2(profile);
      delta1 *= 0.5;
      profile2 += delta1;
      QreJacobian(p_efg, profile2, lambda + lambdainc1 * 0.5, H);
      QreComputeStep(p_efg, profile, H,
		     delta2, lambdainc2, initialsign, stepsize);

      profile += delta1;
      delta2 *= 0.5;
      profile += delta2; 
      lambda += 0.5 * (lambdainc1 + lambdainc2);

      // Write out the QreValue as 0 in the PXI file; not generally
      // going to be the case, but QreValue is suspect for large lambda 
      p_pxifile << "\n" << lambda << " " << 0.0 << " ";
      for (int i = 1; i <= profile.Length(); i++) {
	p_pxifile << profile[i] << " ";
      }
 
      if (params.fullGraph) { 
	solutions.Append(BehavSolution(profile, algorithmEfg_QRE_EFG));
	solutions[solutions.Length()].SetQre(lambda, 0);
	solutions[solutions.Length()].SetEpsilon(params.Accuracy());
      }

      if (numSteps++ % 50 == 0) {
	p_status.Get();
	p_status.SetProgress(lambda / params.maxLam,
			     gText("Current lambda: ") + ToText(lambda));
      }
    }
  }
  catch (...) {
    return;
  }

  if (!params.fullGraph) {
    solutions.Append(BehavSolution(profile, algorithmEfg_QRE_EFG));
    solutions[solutions.Length()].SetQre(lambda, 0);
    // This doesn't really apply, at least currently...
    solutions[solutions.Length()].SetEpsilon(params.Accuracy());
  }
}

class EFKQreFunc : public gFunction<double>   {
private:
  long _nevals;
  bool _domain_err;
  const Efg::Game &_efg;
  double _K;
  gPVector<double> _probs;
  BehavProfile<double> _p, _cpay;
  gVector<double> ***_scratch;
  EFQreFunc F;
  const EFQreParams & params;
  
public:
  EFKQreFunc(const Efg::Game &, const BehavProfile<gNumber> &, 
	     const EFQreParams & params);
  virtual ~EFKQreFunc();
  
  double Value(const gVector<double> &);
  
  void SetK(double k)   { _K = k; }
  void Get_p(BehavProfile<double> &p) const {p = _p;}
  long NumEvals(void) const   { return _nevals; }
  bool DomainErr(void) const { return _domain_err;}
};


EFKQreFunc::EFKQreFunc(const Efg::Game &E,
		       const BehavProfile<gNumber> &start, 
		       const EFQreParams & p)
  :_nevals(0L), _domain_err(false), _efg(E), _K(1.0),
   _probs(E.NumInfosets()),
   _p(start.Support()), _cpay(start.Support()),
    F(E,start), params(p)
{
  for (int i = 1; i <= _p.Length(); i++)
    _p[i] = start[i];
    
  _scratch = new gVector<double> **[_efg.NumPlayers()] - 1;
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    int nisets = (_efg.Players()[pl])->NumInfosets();
    _scratch[pl] = new gVector<double> *[nisets + 1] - 1;
    for (int iset = 1; iset <= nisets; iset++)
      _scratch[pl][iset] = new gVector<double>(_p.Support().NumActions(pl, iset));
  }
}

EFKQreFunc::~EFKQreFunc()
{
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    int nisets = (_efg.Players()[pl])->NumInfosets();
    for (int iset = 1; iset <= nisets; iset++)
      delete _scratch[pl][iset];
    delete [] (_scratch[pl] + 1);
  }
  delete [] (_scratch + 1);
}

double EFKQreFunc::Value(const gVector<double> &lambda)
{
  int iter = 0;
  double value = 0.0;
  
  F.SetLambda(lambda);
  
  if(params.trace > 3) {
    *params.tracefile << "\n   EFKGobFunc start: " << _p << " Lambda = " << F.GetLambda();
  }
  
  // first find Qre solution of p for given lambda vector
  
  
  gMatrix<double> xi(_p.Length(), _p.Length());
  InitMatrix(xi, _p.Lengths());
  
  gNullStatus status;
  gPVector<double> pvect(_p.GetPVector());
  Powell(pvect, xi, F, value, iter,
	 params.maxits1, params.tol1, params.maxitsN, params.tolN,
	 *params.tracefile, params.trace-4,true,status);
  _p = pvect;

  _nevals = F.NumEvals();
  
  // now compute objective function for KQre 
  
  value = 0.0;
  
  //  _p.CondPayoff(_cpay, _probs);
  const gArray<EFPlayer *> players = _efg.Players();
  for (int pl = 1; pl <= players.Length(); pl++)  {
    const gArray<Infoset *> infosets = players[pl]->Infosets();    
    for (int iset = 1; iset <= infosets.Length(); iset++)  {
      double vij = 0.0;
      const gArray<Action *> &acts = _p.Support().Actions(infosets[iset]);
      for( int j = 1;j<=acts.Length();j++)
	for(int k = 1;k<=acts.Length();k++)
	  vij+=_p.GetActionProb(acts[j])*_p.GetActionProb(acts[k])*
	    _p.GetActionValue(acts[j])*(_p.GetActionValue(acts[j])-_p.GetActionValue(acts[k]));
      value += pow(vij -lambda[pl]*_K,2.0);
    }
  }

  /*
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    EFPlayer *player = _efg.Players()[pl];
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      double vij = 0.0;
      for( int j = 1;j<=(_p.Support().NumActions(pl,iset));j++)
	for(int k = 1;k<=(_p.Support().NumActions(pl,iset));k++)
	  vij+=_p(pl,iset,j)*_p(pl,iset,k)*_cpay(pl,iset,j)*(_cpay(pl,iset,j)-_cpay(pl,iset,k));
      value += pow(vij -lambda[pl]*_K,2.0);
    }
  }
  */

  if(params.trace > 3) {
    (*params.tracefile).SetExpMode().SetPrec(4) << "\n   EFKGobFunc val: " << value;
    *params.tracefile << " K = " << _K;
    *params.tracefile << " lambda = " << lambda;
    (*params.tracefile).SetFloatMode().SetPrec(6) << " p = " << _p;
  }
  return value;
}

/*
double EFKQreFunc::Value(const gVector<double> &lambda)
{
  int iter = 0;
  double value = 0.0;
  
  F.SetLambda(lambda);
  
  if(params.trace > 3) {
    *params.tracefile << "\n   EFKGobFunc start: " << _p << " Lambda = " << F.GetLambda();
  }
  
  // first find Qre solution of p for given lambda vector
  
  
  gMatrix<double> xi(_p.Length(), _p.Length());
  InitMatrix(xi, _p.Lengths());
  
  Powell(_p, xi, F, value, iter,
	 params.maxits1, params.tol1, params.maxitsN, params.tolN,
	 *params.tracefile, params.trace-4,true);
  
  _nevals = F.NumEvals();
  
  // now compute objective function for KQre 
  
  value = 0.0;
  
  _p.CondPayoff(_cpay, _probs);
  
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    EFPlayer *player = _efg.Players()[pl];
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      double vij = 0.0;
      for( int j = 1;j<=(_p.Support().NumActions(pl,iset));j++)
	for(int k = 1;k<=(_p.Support().NumActions(pl,iset));k++)
	  vij+=_p(pl,iset,j)*_p(pl,iset,k)*_cpay(pl,iset,j)*(_cpay(pl,iset,j)-_cpay(pl,iset,k));
      value += pow(vij -lambda[pl]*_K,2.0);
    }
  }
  if(params.trace > 3) {
    (params.tracefile->SetExpMode()).SetPrec(4);
    *params.tracefile << "\n   EFKGobFunc val: " << value;
    *params.tracefile << " K = " << _K;
    *params.tracefile << " lambda = " << lambda;
    params.tracefile->SetFloatMode().SetPrec(6);
    *params.tracefile << " p = " << _p;
  }
  return value;
}
*/

extern bool OldPowell(gVector<double> &p, gMatrix<double> &xi,
		      gFunction<double> &func, double &fret, int &iter,
		      int maxits1, double tol1, int maxitsN, double tolN,
		      gOutput &tracefile, int tracelevel, gStatus &status);

void KQre(const Efg::Game &E, EFQreParams &params, gOutput &p_pxifile,
	  const BehavProfile<gNumber> &start, gList<BehavSolution> &solutions,
	  gStatus &p_status, long &nevals, long &nits)
{
  EFKQreFunc F(E, start, params);
  int i;
  int iter = 0, nit;
  double K, K_old = 0.0, value = 0.0;
  gVector<double> lambda(E.NumPlayers());
  lambda = (double).0001;
  gVector<double> lam_old(lambda);
   
  WritePXIHeader(p_pxifile, E, params);
  
  K = (params.delLam < 0.0) ? params.maxLam : params.minLam;
  int num_steps, step = 0;
  if (params.powLam == 0)
    num_steps = (int) ((params.maxLam - params.minLam) / params.delLam);
  else
    num_steps = (int) (log(params.maxLam / params.minLam) /
		       log(params.delLam + 1.0));
  
  BehavProfile<double> p(start.Support());
  BehavProfile<double> p_old(p);
  
  gMatrix<double> xi(lambda.Length(), lambda.Length());
  xi.MakeIdent();
  
  if (params.trace> 0 )  {
    *params.tracefile << "\nin EFKQre";
    *params.tracefile << " traceLevel: " << params.trace;
    *params.tracefile << "\np: " << p << "\nxi: " << xi;
  }
  
  bool FoundSolution = true;
  for (nit = 1; FoundSolution && !F.DomainErr() &&
       K <= params.maxLam && K >= params.minLam &&
       value < 10.0; nit++)   {
    p_status.Get();
    
    F.SetK(K);
    
    FoundSolution =  OldPowell(lambda, xi, F, value, iter,
			params.maxits1, params.tol1, params.maxitsN, params.tolN,
			*params.tracefile, params.trace-1, p_status);
    
    F.Get_p(p);
/*
  if (params.trace>0)  {
    *params.tracefile << "\nKQre iter: " << nit << " val = ";
    params.tracefile->SetExpMode();
    *params.tracefile << value;
    params.tracefile->SetFloatMode();
    *params.tracefile << " K: " << K << " lambda: " << lambda << " val: ";
    *params.tracefile << " p: " << p;
  }
    */
    if(FoundSolution && !F.DomainErr()) {
      if (params.trace>0)  {
	*params.tracefile << "\nKQre iter: " << nit << " val = ";
	(*params.tracefile).SetExpMode() << value;
	(*params.tracefile).SetFloatMode() << " K: " << K << " lambda: " << lambda << " val: ";
	*params.tracefile << " p: " << p;
      }
      
      p_pxifile << "\n" << K << " " << value;
      p_pxifile << " ";
    
	
      for (int pl = 1; pl <= E.NumPlayers(); pl++) {
	EFPlayer *player = E.Players()[pl];
	for (int iset = 1; iset <= player->NumInfosets();iset++)
	  for( int act = 1;act<=(p.Support().NumActions(pl,iset));act++)
	    p_pxifile << p(pl, iset,act) << " ";
      }
      
      if (params.fullGraph) {
	i = solutions.Append(BehavSolution(p, algorithmEfg_QRE_EFG));      
	solutions[i].SetQre(K, value);
      }
      K_old=K;                              // keep last good solution
      lam_old=lambda;                            
      p_old=p;                             
    }
    K += params.delLam * pow(K, (long)params.powLam);
    p_status.SetProgress((double) step / (double) num_steps);
    step++;
  }
  
  if (!params.fullGraph) {
    i = solutions.Append(BehavSolution(p, algorithmEfg_QRE_EFG));
    solutions[i].SetQre(K_old, value);
  }
  
  nevals = F.NumEvals();
  nits = 0;
}


void Qre(const Efg::Game &E, EFQreParams &params, 
	 gOutput &p_pxifile,
	 const BehavProfile<gNumber> &start,
	 gList<BehavSolution> &solutions, gStatus &p_status,
	 long &nevals, long &nits)
{
  if (params.m_homotopy) {
    QreHomotopy(E, params, p_pxifile, start, solutions, p_status,
		nevals, nits);
  }
  else {
    QreOptimization(E, params, p_pxifile, start, solutions, p_status,
    		    nevals, nits);
  }
}

