//
// FILE: nfgqre.cc -- Computation of QRE correspondence for normal forms
//
// $Id$
//

#include <math.h>

#include "nfgqre.h"

#include "gfunc.h"
#include "math/gmatrix.h"
#include "math/gsmatrix.h"
#include "gnullstatus.h"

NFQreParams::NFQreParams(void)
  : m_method(qreHOMOTOPY), 
    powLam(1), minLam(0.01), maxLam(30.0), delLam(0.01), 
    fullGraph(false), pxifile(&gnull)
{ }

NFQreParams::NFQreParams(gOutput &, gOutput &pxi)
  : powLam(1), minLam(0.01), maxLam(30.0), delLam(0.01), 
    fullGraph(false), pxifile(&pxi)
{ }


static void WritePXIHeader(gOutput &pxifile, const Nfg &N,
			   const NFQreParams &params)
{
  pxifile << "Dimensionality:\n";
  pxifile << N.NumPlayers() << " ";
  for (int pl = 1; pl <= N.NumPlayers(); pl++)
    pxifile << N.NumStrats(pl) << " ";
  pxifile << "\n";
  N.WriteNfgFile(pxifile, 6);

  pxifile << "Settings:\n" << params.minLam;
  pxifile << "\n" << params.maxLam << "\n" << params.delLam;
  pxifile << "\n" << 0 << "\n" << 1 << "\n" << params.powLam << "\n";
  
  int numcols = N.ProfileLength() + 2;

  pxifile << "DataFormat:\n" << numcols;
  
  for (int i = 1; i <= numcols; i++)
    pxifile << " " << i;
 
  pxifile << "\nData:\n";
}

//=========================================================================
//                  QRE version of Liapunov function
//=========================================================================

// This modification of the Liapunov function is a non-negative-valued
// C2 function that is zero only on the QRE correspondence.  
// This is minimized in the optimization approach, and used as a 
// diagnostic for the homotopy approach.

class NFQreFunc : public gC2Function<double>  {
  private:
    long _nevals;
    bool _domain_err;
    const Nfg &_nfg;
    gVector<double> _Lambda;
    gVector<double> **_scratch;
    MixedProfile<double> _p;

    double QreDerivValue(int, int, const MixedProfile<double> &);

    bool Deriv(const gVector<double> &, gVector<double> &);
    bool Hessian(const gVector<double> &, gMatrix<double> &);

  public:
    NFQreFunc(const Nfg &, const MixedProfile<gNumber> &);
    virtual ~NFQreFunc();
    
    gVector<double> GetLambda()   { return _Lambda; }
    void SetLambda(double l)   { _Lambda = l; }
    void SetLambda(const gVector<double> &l)   { _Lambda = l; }
    long NumEvals(void) const  { return _nevals; }
    bool DomainErr(void) const { return _domain_err;}

    double Value(const gVector<double> &);
};


NFQreFunc::NFQreFunc(const Nfg &N,
			 const MixedProfile<gNumber> &start)
  : _nevals(0L), _domain_err(false), _nfg(N), 
    _Lambda(N.NumPlayers()), _p(start.Support())
{
  for (int i = 1; i <= _p.Length(); i++)
    _p[i] = start[i];

  _scratch = new gVector<double> *[_nfg.NumPlayers()] - 1;
  for (int i = 1; i <= _nfg.NumPlayers(); i++)
    _scratch[i] = new gVector<double>(_p.Support().NumStrats(i));
}

NFQreFunc::~NFQreFunc()
{
  for (int i = 1; i <= _nfg.NumPlayers(); i++) 
    delete _scratch[i];

  delete [] (_scratch + 1);
}

bool NFQreFunc::Hessian(const gVector<double> &, gMatrix<double> &)
{
  return true;
}

double NFQreFunc::QreDerivValue(int i, int j,
				    const MixedProfile<double> &v)
{
  double x = 0.0, dv;
  
  for (int pl = 1; pl <= _nfg.NumPlayers(); pl++)  {
    gVector<double> &payoff = *_scratch[pl];
    v.Payoff(pl, pl, payoff);
    for (int st = 2; st <= _p.Support().NumStrats(pl); st++) {
      dv = (double) log(v(pl, 1)) - (double) log(v(pl, st)) -
	_Lambda[pl] * (payoff[1] - payoff[st]);
      if (pl == i)  {
	if (j == 1)          x += dv / v(pl, 1);
	else if (j == st)    x -= dv / v(pl, st);
      }
      else
	x -= dv * _Lambda[pl] * (v.Payoff(pl, pl, 1, i, j) -
			     v.Payoff(pl, pl, st, i, j));
    }
  }

  return 2.0 * x;
}


bool NFQreFunc::Deriv(const gVector<double> &v, gVector<double> &d)
{
  ((gVector<double> &) _p).operator=(v);
  
  for (int pl = 1, index = 1; pl <= _nfg.NumPlayers(); pl++)  {
    int nstrats = _p.Support().NumStrats(pl);
    int st;

    for (st = 1; st <= nstrats;
	 d[index++] = QreDerivValue(pl, st++, _p));
  }

  return true;
}
  
double NFQreFunc::Value(const gVector<double> &v)
{
  _nevals++;
  _domain_err = false;
  ((gVector<double> &) _p).operator=(v);
  double val = 0.0, z;
  
  for (int pl = 1; pl <= _nfg.NumPlayers(); pl++)  {
    gVector<double> &payoff = *_scratch[pl];
    _p.Payoff(pl, pl, payoff);
    for (int st = 2; st <= _p.Support().NumStrats(pl); st++) {
      z = log(_p(pl, 1)) - log(_p(pl, st)) -
          _Lambda[pl] * (payoff[1] - payoff[st]);
      val += z * z;
    }
  }

  return val;
}

//=========================================================================
//             QRE Correspondence Computation via Homotopy
//=========================================================================

void QreJacobian(const Nfg &p_nfg,
		 const MixedProfile<double> &p_profile,
		 const double &p_lambda, gMatrix<long double> &p_matrix)
{
  p_matrix = (double) 0;

  int rowno = 0;   // indexes the row number in the Jacobian matrix
  for (int pl1 = 1; pl1 <= p_nfg.NumPlayers(); pl1++) {
    for (int st1 = 1; st1 <= p_profile.Support().NumStrats(pl1); st1++) {
      rowno++;

      int colno = 0;
      for (int pl2 = 1; pl2 <= p_nfg.NumPlayers(); pl2++) {
	for (int st2 = 1; st2 <= p_profile.Support().NumStrats(pl2); st2++) {
	  colno++;

	  if (pl1 == pl2) {
	    if (st1 == st2) {
	      p_matrix(rowno, colno) = 1.0;
	    }
	    else {
	      p_matrix(rowno, colno) = 0.0;
	    }
	  } 
	  else {  // pl1 != pl2
	    for (int k = 1; k <= p_profile.Support().NumStrats(pl1); k++) {
	      p_matrix(rowno, colno) += (p_profile.Payoff(pl1, pl1, k, pl2, st2) - p_profile.Payoff(pl1, pl1, st1, pl2, st2)) * p_profile(pl1, k);
	    }
	    p_matrix(rowno, colno) *= p_lambda * p_profile(pl1, st1);
	  }
	}
      }

      // Now for the column wrt lambda
      for (int k = 1; k <= p_profile.Support().NumStrats(pl1); k++) {
	p_matrix(rowno, p_matrix.NumColumns()) += (p_profile.Payoff(pl1, pl1, k) - p_profile.Payoff(pl1, pl1, st1)) * p_profile(pl1, k);
      } 
      p_matrix(rowno, p_matrix.NumColumns()) *= p_profile(pl1, st1);
    }
  }
}

void QreComputeStep(const Nfg &p_nfg, const MixedProfile<double> &p_profile,
		    const gMatrix<long double> &p_matrix,
		    gPVector<double> &p_delta, double &p_lambdainc,
		    double p_initialsign, double p_stepsize)
{
  double sign = p_initialsign;
  int rowno = 0; 

  gSquareMatrix<long double> M(p_matrix.NumRows());

  for (int row = 1; row <= M.NumRows(); row++) {
    for (int col = 1; col <= M.NumColumns(); col++) {
      M(row, col) = p_matrix(row, col + 1);
    }
  }

  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.Support().NumStrats(pl); st++) {
      rowno++;
      p_delta(pl, st) = sign * M.Determinant();   
      sign *= -1.0;

      for (int row = 1; row <= M.NumRows(); row++) {
	M(row, rowno) = p_matrix(row, rowno);
	if (rowno < M.NumColumns()) {
	  M(row, rowno + 1) = p_matrix(row, rowno + 2);
	}
      } 
    }
  }   

  p_lambdainc = sign * M.Determinant();

  double norm = 0.0;
  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.Support().NumStrats(pl); st++) {
      norm += p_delta(pl, st) * p_delta(pl, st);
    }
  }
  norm += p_lambdainc * p_lambdainc; 
  
  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.Support().NumStrats(pl); st++) {
      p_delta(pl, st) /= sqrt(norm / p_stepsize);
    }
  }

  p_lambdainc /= sqrt(norm / p_stepsize);
}

void QreHomotopy(const Nfg &p_nfg, NFQreParams &params,
		 const MixedProfile<gNumber> &p_start,
		 gList<MixedSolution> &p_solutions, gStatus &p_status,
		 long &nevals, long &nits)
{
  gMatrix<long double> H(p_nfg.ProfileLength(), p_nfg.ProfileLength() + 1);
  MixedProfile<double> profile(p_start);
  double lambda = params.minLam;
  double stepsize = 0.0001;
  // Pick the direction to follow the path so that lambda starts out
  // increasing
  double initialsign = (p_nfg.ProfileLength() % 2 == 0) ? 1.0 : -1.0;

  p_solutions.Flush();

  try {
    while (lambda <= params.maxLam) {
      // Use a first-order Runge-Kutta style method
      gPVector<double> delta1(profile), delta2(profile);
      double lambdainc1, lambdainc2;
      
      QreJacobian(p_nfg, profile, lambda, H);
      QreComputeStep(p_nfg, profile, H,
		     delta1, lambdainc1, initialsign, stepsize);
    
      MixedProfile<double> profile2(profile);
      profile2 += delta1 * 0.5; 
      QreJacobian(p_nfg, profile2, lambda + lambdainc1 * 0.5, H);
      QreComputeStep(p_nfg, profile, H,
		     delta2, lambdainc2, initialsign, stepsize);
    
      profile += delta1 * 0.5;
      profile += delta2 * 0.5; 
      lambda += 0.5 * (lambdainc1 + lambdainc2);

      if (params.fullGraph) { 
	p_solutions.Append(MixedSolution(profile, algorithmNfg_QRE));
	NFQreFunc qreValue(p_nfg, MixedProfile<gNumber>(profile));
	qreValue.SetLambda(lambda);
	p_solutions[p_solutions.Length()].SetQre(lambda, qreValue.Value(profile));
      }

      p_status.Get();
      p_status.SetProgress((lambda - params.minLam) /
			   (params.maxLam - params.minLam),
			   gText("Current lambda: ") + ToText(lambda));
    }
  }
  catch (...) {
    p_solutions.Append(MixedSolution(profile, algorithmNfg_QRE));
    NFQreFunc qreValue(p_nfg, MixedProfile<gNumber>(profile));
    qreValue.SetLambda(lambda);
    p_solutions[p_solutions.Length()].SetQre(lambda, qreValue.Value(profile));
    throw;
  }
  
  if (!params.fullGraph) { 
    p_solutions.Append(MixedSolution(profile, algorithmNfg_QRE));
    NFQreFunc qreValue(p_nfg, MixedProfile<gNumber>(profile));
    qreValue.SetLambda(lambda);
    p_solutions[p_solutions.Length()].SetQre(lambda, qreValue.Value(profile));
  }
}

//=========================================================================
//            QRE Correspondence Computation via Optimization
//=========================================================================

// DFP routine from Numerical Recipies (with modifications)
// p = starting vector
// func = a gC2Function representing the QRE function for which we need 
//        to find the zeros
// fret = function return value (should be close to zero after return)
// iter = number of iterations to complete
// maxits1 = maximum number of iterations in line search
// tol1    = tolerance in line search
// maxitsN = maximum number of iterations in DFP
// tolN    = tolerance in DFP
// tracefile  = output stream for debugging output
// tracelevel = level of debugging output
// interior   = true restricts from hitting boundary


extern bool DFP(gPVector<double> &p, gC2Function<double> &func,
		double &fret, int &iter,
	        int maxits1, double tol1, int maxitsN, double tolN,
		gOutput &tracefile, int tracelevel, bool interior,
		gStatus &status);


// This is the function with the main computational loop for tracing the QRE 
// correspondence.  It proceeds by starting at a point (p, lambda) 
// on the correspondence.  Then it increments lambda, and uses p 
// as a starting point to find a new point on the correspondence.  The Davidon 
// fletcher Powell method is used for minimization.  

static void QreOptimization(const Nfg &N, NFQreParams &params,
			    const MixedProfile<gNumber> &start,
			    gList<MixedSolution> &solutions, gStatus &p_status,
			    long &nevals, long &nits)
{
  static const double ALPHA = .00000001;

  NFQreFunc F(N, start);

  int iter = 0;
  double Lambda, LambdaOld, LambdaSave, LambdaStart, value = 0.0;
  
  if (params.pxifile) 
    WritePXIHeader(*params.pxifile, N, params);

  LambdaStart = (params.delLam < 0.0) ? params.maxLam : params.minLam;
  LambdaOld = LambdaSave = Lambda = LambdaStart;

  double max_prog, prog;

  if (params.powLam == 0)
    max_prog = params.maxLam - params.minLam;
  else
    max_prog = log(params.maxLam / params.minLam);

  MixedProfile<double> p(start.Support());
  for (int j = 1; j <= p.Length(); j++)
    p[j] = start[j];

  // if starting vector not interior, perturb it towards centroid
  int kk;
  for(kk=1;kk <= p.Length() && p[kk]>ALPHA;kk++);
  if(kk<=p.Length()) {
    MixedProfile<double> c(start.Support());
    for(int k=1;k<=p.Length();k++)
      p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
  }

  MixedProfile<double> pold(p);
  MixedProfile<double> psave(p);
  MixedProfile<double> pdiff(p);
  pdiff-= pold;

  bool FoundSolution = true;
  double delta, mindelta;
  delta = params.delLam;
  mindelta = delta/1000000.0;
  
  try {
    while (delta>mindelta && Lambda <= params.maxLam &&
	   Lambda >= params.minLam)  {
      p_status.Get();
      F.SetLambda(Lambda);

      // enter Davidon Fletcher Powell routine.  

      FoundSolution = DFP(p, F, value, iter, 
			  params.maxits1, params.tol1, params.maxitsN, params.tolN,
			  *params.tracefile,params.trace-1,true,p_status);
      
      bool derr = F.DomainErr();
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

	  if (params.pxifile)   {
	    *params.pxifile << "\n" << Lambda << " " << value << " ";
	    for (int pl = 1; pl <= N.NumPlayers(); pl++)
	      for (int strat = 1;
		   strat <= p.Support().NumStrats(pl);
		   strat++)
		*params.pxifile << p(pl, strat) << " ";
	  }
	  
	  if (params.fullGraph) {
	    int index = solutions.Append(MixedSolution(p, algorithmNfg_QRE));      
	    solutions[index].SetQre(Lambda, value);
	    solutions[index].SetEpsilon(params.Accuracy());
	  }

	  psave = p;
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

    if (!params.fullGraph) {
      int index = solutions.Append(MixedSolution(pold, algorithmNfg_QRE));
      solutions[index].SetQre(Lambda, value);
      solutions[index].SetEpsilon(params.Accuracy());
    }

    nevals = F.NumEvals();
    nits = 0;
  }
  catch (gSignalBreak &E) {
    if (!params.fullGraph) {
      int index = solutions.Append(MixedSolution(p, algorithmNfg_QRE));
      solutions[index].SetQre(Lambda, value);
      solutions[index].SetEpsilon(params.Accuracy());
    }
    throw;
  }
  catch (gFuncMinError &E) {
    if (!params.fullGraph) {
      int index = solutions.Append(MixedSolution(p, algorithmNfg_QRE));
      solutions[index].SetQre(Lambda, value);
      solutions[index].SetEpsilon(params.Accuracy());
    }
    // This should be re-thrown, but wait til we have better exception handling downstream
    //    throw;
  }
}

// all of the below is for KQRE computations

class NFKQreFunc : public gFunction<double>   {
private:
  long _nevals;
  bool _domain_err;
  const Nfg &_nfg;
  double _K;
  gVector<double> **_scratch;
  MixedProfile<double> _p;
  NFQreFunc F;
  const NFQreParams & params;
  
public:
  NFKQreFunc(const Nfg &, const MixedProfile<gNumber> &, 
		  const NFQreParams & params);
  virtual ~NFKQreFunc();
  
  double Value(const gVector<double> &);
  
  void SetK(double k)   { _K = k; }
  void Get_p(MixedProfile<double> &p) const {p = _p;}
  long NumEvals(void) const   { return _nevals; }
  bool DomainErr(void) const { return _domain_err;}
};


NFKQreFunc::NFKQreFunc(const Nfg &N,
				 const MixedProfile<gNumber> &start, 
				 const NFQreParams & p)
  :_nevals(0L), _domain_err(false), _nfg(N), _K(1.0),
   _p(start.Support()), F(N,start), params(p)
{
  for (int i = 1; i <= _p.Length(); i++)
    _p[i] = start[i];

  _scratch = new gVector<double> *[_nfg.NumPlayers()] - 1;
  for (int i = 1; i <= _nfg.NumPlayers(); i++)
    _scratch[i] = new gVector<double>(_p.Support().NumStrats(i));
}

NFKQreFunc::~NFKQreFunc()
{
  for (int i = 1; i <= _nfg.NumPlayers(); i++) 
    delete _scratch[i];

  delete [] (_scratch + 1);
}

double NFKQreFunc::Value(const gVector<double> &lambda)
{
  int iter = 0;
  double value = 0.0;

  F.SetLambda(lambda);

  if(params.trace > 3) {
//    _p(1,1)= .5709;_p(1,2)= 1.0-_p(1,1);
//    _p(2,1)= .1227;_p(2,2)= 1.0-_p(2,1);
    *params.tracefile << "\n   NFKGobFunc start: " << _p << " Lambda = " << F.GetLambda();
  }
  
  // first find Qre solution of p for given lambda vector
  
  gNullStatus status;
  DFP(_p, F, value, iter,
      params.maxits1, params.tol1, params.maxitsN, params.tolN,
      *params.tracefile,params.trace-4,true,status);

  _nevals = F.NumEvals();

 // now compute objective function for KQre 

  value = 0.0;
  for (int pl = 1; pl <= _nfg.NumPlayers(); pl++)  {
    gVector<double> &payoff = *_scratch[pl];
    _p.Payoff(pl, pl, payoff);
    double vij = 0.0;
    for( int j = 1;j<=(_p.Support().NumStrats(pl));j++)
      for(int k = 1;k<=(_p.Support().NumStrats(pl));k++)
	vij+=_p(pl,j)*_p(pl,k)*payoff[j]*(payoff[j]-payoff[k]);
    value += pow(vij -lambda[pl]*_K,2.0);
  }
  if(params.trace > 3) {
    (*params.tracefile).SetExpMode().SetPrec(4) << "\n   NFKGobFunc val: " << value;
    *params.tracefile << " K = " << _K;
    *params.tracefile << " lambda = " << lambda;
    (*params.tracefile).SetFloatMode().SetPrec(6) << " p = " << _p;
  }
  return value;
}

extern bool OldPowell(gVector<double> &p, gMatrix<double> &xi,
		   gFunction<double> &func, double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile, int tracelevel,  gStatus &status);

// This is for computation of the KQRE correspondence.  

void KQre(const Nfg &N, NFQreParams &params,
	   const MixedProfile<gNumber> &start,
	   gList<MixedSolution> &solutions, gStatus &p_status,
	   long &nevals, long &nits)
{
  NFKQreFunc F(N, start, params);
  int i;
  int iter = 0, nit;
  double K, K_old = 0.0, value = 0.0;
  gVector<double> lambda(N.NumPlayers());
  lambda = (double).0001;
  gVector<double> lam_old(lambda);

  if (params.pxifile) 
    WritePXIHeader(*params.pxifile, N, params);

  K = (params.delLam < 0.0) ? params.maxLam : params.minLam;
  int num_steps, step = 0;
  if (params.powLam == 0)
    num_steps = (int) ((params.maxLam - params.minLam) / params.delLam);
  else
    num_steps = (int) (log(params.maxLam / params.minLam) /
		       log(params.delLam + 1.0));

  MixedProfile<double> p(start.Support());
  MixedProfile<double> p_old(p);

  gMatrix<double> xi(lambda.Length(), lambda.Length());
  xi.MakeIdent();

  if (params.trace> 0 )  {
    *params.tracefile << "\nin NFKQre";
    *params.tracefile << " traceLevel: " << params.trace;
    *params.tracefile << "\np: " << p << "\nxi: " << xi;
  }

  bool powell = true;
  for (nit = 1; powell && !F.DomainErr() &&
       K <= params.maxLam && K >= params.minLam &&
       value < 10.0; nit++)   {
    p_status.Get();

    F.SetK(K);
    
   powell =  OldPowell(lambda, xi, F, value, iter,
		     params.maxits1, params.tol1, params.maxitsN, params.tolN,
		     *params.tracefile, params.trace-1,p_status);

    F.Get_p(p);

    if(powell && !F.DomainErr()) {
      if (params.trace>0)  {
	*params.tracefile << "\nKQre iter: " << nit << " val = ";
	(*params.tracefile).SetExpMode() << value;
	(*params.tracefile).SetFloatMode() << " K: " << K << " lambda: " << lambda << " val: ";
	*params.tracefile << " p: " << p;
      }
      
      if (params.pxifile)   {
	*params.pxifile << "\n" << K << " " << value;
	*params.pxifile << " ";
	for (int pl = 1; pl <= N.NumPlayers(); pl++)
	  for (int strat = 1;
	       strat <= p.Support().NumStrats(pl);
	       strat++)
	    *params.pxifile << p(pl, strat) << " ";
      }
      
      if (params.fullGraph) {
	i = solutions.Append(MixedSolution(p, algorithmNfg_QRE));      
	solutions[i].SetQre(K, value);
	solutions[i].SetEpsilon(params.Accuracy());
      }
      K_old=K;                              // keep last good solution
      lam_old=lambda;                            
      p_old=p;                             
    }
    K += params.delLam * pow(K, (long)params.powLam);
    p_status.SetProgress((double) step / (double) num_steps);
    step++;
  }

  if (!params.fullGraph)
  {
    i = solutions.Append(MixedSolution(p, algorithmNfg_QRE));
    solutions[i].SetQre(K_old, value);
    solutions[i].SetEpsilon(params.Accuracy());
  }

  nevals = F.NumEvals();
  nits = 0;
}

void Qre(const Nfg &p_nfg, NFQreParams &params,
	 const MixedProfile<gNumber> &start,
	 gList<MixedSolution> &solutions, gStatus &p_status,
	 long &nevals, long &nits)
{
  if (params.m_method == qreOPTIMIZE) {
    QreOptimization(p_nfg, params, start, solutions, p_status, nevals, nits);
  }
  else {
    QreHomotopy(p_nfg, params, start, solutions, p_status, nevals, nits);
  }
}

