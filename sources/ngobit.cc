//
// FILE: ngobit.cc -- Implementation of gobit on normal form games
//
//  $Id$
//

#include <math.h>

#include "ngobit.h"

#include "gfunc.h"
#include "gmatrix.h"

NFQreParams::NFQreParams(gStatus &s)
  : trace(0), powLam(1), maxits1(100), maxitsN(20),
    minLam(0.01), maxLam(30.0), delLam(0.01), tol1(2.0e-10), tolN(1.0e-10),
    fullGraph(false), tracefile(&gnull), pxifile(&gnull),
    status(s)
{ }

NFQreParams::NFQreParams(gOutput &out, gOutput &pxi, gStatus &s)
  : trace(0), powLam(1), maxits1(100), maxitsN(20),
    minLam(0.01), maxLam(30.0), delLam(0.01), tol1(2.0e-10), tolN(1.0e-10),
    fullGraph(false), tracefile(&out), pxifile(&pxi),
    status(s)
{ }


class NFQreFunc : public gC2Function<double>  {
  private:
    long _nevals;
    const Nfg &_nfg;
    gVector<double> _Lambda;
    gVector<double> **_scratch;
    MixedProfile<double> _p;

    double Value(const gVector<double> &);
    
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
};


NFQreFunc::NFQreFunc(const Nfg &N,
			 const MixedProfile<gNumber> &start)
  : _nevals(0L), _nfg(N), _Lambda(N.NumPlayers()), _p(start.Support())
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

extern bool DFP(gPVector<double> &p, gC2Function<double> &func,
		double &fret, int &iter,
	        int maxits1, double tol1, int maxitsN, double tolN,
		gOutput &tracefile, int tracelevel, bool interior = false,
		gStatus &status = gstatus);


void Qre(const Nfg &N, NFQreParams &params,
	   const MixedProfile<gNumber> &start,
	   gList<MixedSolution> &solutions,
	   long &nevals, long &nits)
{
  NFQreFunc F(N, start);

  int i;
  int iter = 0;
  double Lambda, value = 0.0;
  
  if (params.pxifile) 
    WritePXIHeader(*params.pxifile, N, params);

  Lambda = (params.delLam < 0.0) ? params.maxLam : params.minLam;
  int num_steps, step = 0;
  if (params.powLam == 0)
    num_steps = (int) ((params.maxLam - params.minLam) / params.delLam);
  else
    num_steps = (int) (log(params.maxLam / params.minLam) /
		       log(params.delLam + 1.0));

  MixedProfile<double> p(start.Support());
  for (int j = 1; j <= p.Length(); j++)
    p[j] = start[j];

  for (/*nit = 1*/;
       Lambda <= params.maxLam &&
       Lambda >= params.minLam && value < 10.0; /*nit++*/)  {
    params.status.Get();
    F.SetLambda(Lambda);
    DFP(p, F, value, iter,
        params.maxits1, params.tol1, params.maxitsN, params.tolN,
	*params.tracefile,params.trace-1,true);

    if (params.trace>0)  {
      *params.tracefile << "\nLam: " << Lambda << " val: " << value << " p: " << p;
    } 

    if (params.pxifile)   {
      *params.pxifile << "\n" << Lambda << " " << value;
      *params.pxifile << " ";
      for (int pl = 1; pl <= N.NumPlayers(); pl++)
	for (int strat = 1;
	     strat <= p.Support().NumStrats(pl);
	     strat++)
	  *params.pxifile << p(pl, strat) << " ";
    }
    
    if (params.fullGraph) 
    {
      i = solutions.Append(MixedSolution(p, NfgAlg_QRE));      
      solutions[i].SetQre(Lambda, value);
    }

    Lambda += params.delLam * pow(Lambda, (long)params.powLam);
    params.status.SetProgress((double) step / (double) num_steps);
    step++;
  }

  if (!params.fullGraph)
  {
    i = solutions.Append(MixedSolution(p, NfgAlg_QRE));
    solutions[i].SetQre(Lambda, value);
  }

  nevals = F.NumEvals();
  nits = 0;
}

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

  DFP(_p, F, value, iter,
      params.maxits1, params.tol1, params.maxitsN, params.tolN,
      *params.tracefile,params.trace-4,true);

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
    (params.tracefile->SetExpMode()).SetPrec(4);
    *params.tracefile << "\n   NFKGobFunc val: " << value;
    *params.tracefile << " K = " << _K;
    *params.tracefile << " lambda = " << lambda;
    params.tracefile->SetFloatMode().SetPrec(6);
    *params.tracefile << " p = " << _p;
  }
  return value;
}

extern bool OldPowell(gVector<double> &p, gMatrix<double> &xi,
		   gFunction<double> &func, double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile, int tracelevel,  gStatus &status = gstatus);

void KQre(const Nfg &N, NFQreParams &params,
	   const MixedProfile<gNumber> &start,
	   gList<MixedSolution> &solutions, 
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
    params.status.Get();

    F.SetK(K);
    
   powell =  OldPowell(lambda, xi, F, value, iter,
		     params.maxits1, params.tol1, params.maxitsN, params.tolN,
		     *params.tracefile, params.trace-1);

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
    if(powell && !F.DomainErr()) {
      if (params.trace>0)  {
	*params.tracefile << "\nKQre iter: " << nit << " val = ";
	params.tracefile->SetExpMode();
	*params.tracefile << value;
	params.tracefile->SetFloatMode();
	*params.tracefile << " K: " << K << " lambda: " << lambda << " val: ";
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
	i = solutions.Append(MixedSolution(p, NfgAlg_QRE));      
	solutions[i].SetQre(K, value);
      }
      K_old=K;                              // keep last good solution
      lam_old=lambda;                            
      p_old=p;                             
    }
    K += params.delLam * pow(K, (long)params.powLam);
    params.status.SetProgress((double) step / (double) num_steps);
    step++;
  }

  if (!params.fullGraph)
  {
    i = solutions.Append(MixedSolution(p, NfgAlg_QRE));
    solutions[i].SetQre(K_old, value);
  }

  nevals = F.NumEvals();
  nits = 0;
}



