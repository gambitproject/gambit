//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Computation of quantal response equilibrium correspondence for
// normal form games.
//

#include <math.h>

#include "nfgqre.h"

#include "gfunc.h"
#include "math/math.h"
#include "math/gmatrix.h"
#include "math/gsmatrix.h"
#include "gnullstatus.h"

static void WritePXIHeader(gOutput &pxifile, const Nfg &N)
{
  pxifile << "Dimensionality:\n";
  pxifile << N.NumPlayers() << " ";
  for (int pl = 1; pl <= N.NumPlayers(); pl++)
    pxifile << N.NumStrats(pl) << " ";
  pxifile << "\n";
  N.WriteNfgFile(pxifile, 6);

  pxifile << "Settings:\n" << 0.0;
  pxifile << "\n" << 1000.0 << "\n" << 1.05;
  pxifile << "\n" << 0 << "\n" << 1 << "\n" << 1 << "\n";
  
  int numcols = N.ProfileLength() + 2;

  pxifile << "DataFormat:\n" << numcols;
  
  for (int i = 1; i <= numcols; i++)
    pxifile << " " << i;
 
  pxifile << "\nData:\n";
}

//=========================================================================
//             QRE Correspondence Computation via Homotopy
//=========================================================================

void QreJacobian(const Nfg &p_nfg,
		 const MixedProfile<double> &p_profile,
		 const double &p_nu, gMatrix<double> &p_matrix)
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
	      p_matrix(rowno, colno) = (1.0 - p_nu) * (1.0 - p_nu);
	    }
	    else {
	      p_matrix(rowno, colno) = 0.0;
	    }
	  } 
	  else {  // pl1 != pl2
	    for (int k = 1; k <= p_profile.Support().NumStrats(pl1); k++) {
	      p_matrix(rowno, colno) += (p_profile.Payoff(pl1, pl1, k, pl2, st2) - p_profile.Payoff(pl1, pl1, st1, pl2, st2)) * p_profile(pl1, k);
	    }
	    p_matrix(rowno, colno) *= p_nu * (1.0-p_nu) * p_profile(pl1, st1);
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

static void QreComputeStep(const Nfg &p_nfg,
			   const MixedProfile<double> &p_profile,
			   const gMatrix<double> &p_matrix,
			   gPVector<double> &p_delta, double &p_nuinc,
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

  p_nuinc = sign * M.Determinant();

  double norm = 0.0;
  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.Support().NumStrats(pl); st++) {
      norm += p_delta(pl, st) * p_delta(pl, st);
    }
  }
  norm += p_nuinc * p_nuinc; 
  
  for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
    for (int st = 1; st <= p_profile.Support().NumStrats(pl); st++) {
      p_delta(pl, st) /= sqrt(norm / p_stepsize);
    }
  }

  p_nuinc /= sqrt(norm / p_stepsize);
}

QreNfg::QreNfg(void)
  : m_maxLam(30.0), m_stepSize(0.0001), m_fullGraph(false)
{ }

void QreNfg::SolveStep(MixedProfile<double> &p_profile, double &p_nu,
		       double p_initialSign, double p_stepsize) const
{
  // This is a (primitive) first-order Runge-Kutta style method
  gMatrix<double> H(p_profile.Length(), p_profile.Length() + 1);
  gPVector<double> delta1(p_profile), delta2(p_profile);
  double nuinc1, nuinc2;
      
  QreJacobian(p_profile.Game(), p_profile, p_nu, H);
  QreComputeStep(p_profile.Game(), p_profile, H,
		 delta1, nuinc1, p_initialSign, p_stepsize);
    
  MixedProfile<double> profile2(p_profile);
  profile2 += delta1 * 0.5; 
  QreJacobian(p_profile.Game(), profile2, p_nu + nuinc1 * 0.5, H);
  QreComputeStep(p_profile.Game(), p_profile, H,
		 delta2, nuinc2, p_initialSign, p_stepsize);

  p_profile += delta1 * 0.5;
  p_profile += delta2 * 0.5; 
  p_nu += 0.5 * (nuinc1 + nuinc2);
}

void QreNfg::Solve(const Nfg &p_nfg, gOutput &p_pxiFile,
		   gStatus &p_status,
		   Correspondence<double, MixedSolution> &p_corresp)
{
  MixedProfile<double> profile(p_nfg);
  double nu = 0.0;
  double stepsize = 0.0001;

  WritePXIHeader(p_pxiFile, p_nfg);

  // Pick the direction to follow the path so that nu starts out
  // increasing
  double initialsign = (p_nfg.ProfileLength() % 2 == 0) ? 1.0 : -1.0;

  try {
    while (nu / (1.0-nu) <= m_maxLam) {
      SolveStep(profile, nu, initialsign, stepsize);

      if (nu < 0.0 || nu > 1.0) {
	// negative nu is probably numerical instability;
	// nu > 1.0 runs past valid region...
	return;
      }

      // Write out the QreValue as 0 in the PXI file; not generally
      // going to be the case, but QreValue is suspect for large lambda 
      p_pxiFile << "\n" << (nu / (1.0-nu)) << " " << 0.0 << " ";
      for (int pl = 1; pl <= p_nfg.NumPlayers(); pl++) {
	for (int st = 1; st <= profile.Support().NumStrats(pl); st++) {
	  p_pxiFile << profile(pl, st) << " ";
	}
      }

      if (m_fullGraph) { 
	p_corresp.Append(1, nu / (1.0-nu),
			 MixedSolution(profile, algorithmNfg_QRE));
      }

      p_status.Get();
      p_status.SetProgress(nu * (1.0 + m_maxLam) / m_maxLam,
			   gText("Current lambda: ") + ToText(nu / (1.0-nu)));
    }
  }
  catch (...) {
    p_corresp.Append(1, nu / (1.0-nu), MixedSolution(profile, algorithmNfg_QRE));
    throw;
  }
  
  if (!m_fullGraph) { 
    p_corresp.Append(1, nu / (1.0-nu), MixedSolution(profile, algorithmNfg_QRE));
  }
}

void QreNfg::Solve(const MixedProfile<double> &p_startProfile,
		   double p_startLambda,
		   gOutput &p_pxiFile, gStatus &p_status,
		   Correspondence<double, MixedSolution> &p_corresp)
{
  double stepsize = 0.00001;

  WritePXIHeader(p_pxiFile, p_startProfile.Game());

  // Pick the direction to follow the path so that nu starts out
  // increasing
  double initialsign = (p_startProfile.Length() % 2 == 0) ? 1.0 : -1.0;

  for (int dir = 1; dir <= 2; dir++) {
    MixedProfile<double> profile(p_startProfile), lastProfile(p_startProfile);
    double nu = p_startLambda / (p_startLambda + 1.0), lastNu = nu;

    try {
      while (nu / (1.0-nu) <= m_maxLam) {
	SolveStep(profile, nu, initialsign, stepsize);

	if (nu < 0.0 || nu > 1.0) {
	  break;
	}

	p_pxiFile << "\n" << (nu / (1.0-nu)) << " " << 0.0 << " ";
	for (int pl = 1; pl <= profile.Game().NumPlayers(); pl++) {
	  for (int st = 1; st <= profile.Support().NumStrats(pl); st++) {
	    p_pxiFile << profile(pl, st) << " ";
	  }
	}
 
	if (m_fullGraph) { 
	  p_corresp.Append(1, nu / (1.0-nu),
			   MixedSolution(profile, algorithmNfg_QRE));
	}

	p_status.Get();
	p_status.SetProgress(nu * (1.0 + m_maxLam) / m_maxLam,
			     gText("Current lambda: ") + ToText(nu / (1.0-nu)));
	lastProfile = profile;
	lastNu = nu;
      }
      if (!m_fullGraph) {
	p_corresp.Append(1, lastNu / (1.0-lastNu),
			 MixedSolution(lastProfile, algorithmNfg_QRE));
      }
    }
    catch (gSignalBreak &) {
      if (!m_fullGraph) {
	p_corresp.Append(1, lastNu / (1.0-lastNu),
			 MixedSolution(lastProfile, algorithmNfg_QRE));
      }
      throw;
    }

    initialsign *= -1.0;
  }
}


#ifdef WITH_KQRE

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

void KQre(const Nfg &N, NFQreParams &params, gOutput &p_pxiFile,
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

  WritePXIHeader(p_pxiFile, N, params);

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
      
      p_pxiFile << "\n" << K << " " << value;
      p_pxiFile << " ";
      for (int pl = 1; pl <= N.NumPlayers(); pl++)
	for (int strat = 1;
	     strat <= p.Support().NumStrats(pl);
	     strat++)
	  p_pxiFile << p(pl, strat) << " ";
      
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

#endif  // WITH_KQRE
