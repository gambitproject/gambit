//
// FILE: ngobit.cc -- Implementation of gobit on normal form games
//
// $Id$
//

#include <math.h>

#include "ngobit.h"

#include "gfunc.h"

NFGobitParams::NFGobitParams(gStatus &s)
  : trace(0), powLam(1), maxits1(100), maxitsN(20),
    minLam(0.01), maxLam(30.0), delLam(0.01), tol1(2.0e-10), tolN(1.0e-10),
    fullGraph(false), tracefile(&gnull), pxifile(&gnull),
    status(s)
{ }

NFGobitParams::NFGobitParams(gOutput &out, gOutput &pxi, gStatus &s)
  : trace(0), powLam(1), maxits1(100), maxitsN(20),
    minLam(0.01), maxLam(30.0), delLam(0.01), tol1(2.0e-10), tolN(1.0e-10),
    fullGraph(false), tracefile(&out), pxifile(&pxi),
    status(s)
{ }


class NFGobitFunc : public gC2Function<double>  {
  private:
    long _nevals;
    const Nfg<double> &_nfg;
    double _Lambda;
    gVector<double> **_scratch;
    MixedProfile<double> _p;

    double Value(const gVector<double> &);
    
    double GobitDerivValue(int, int, const MixedProfile<double> &);

    bool Deriv(const gVector<double> &, gVector<double> &);
    bool Hessian(const gVector<double> &, gMatrix<double> &);

  public:
    NFGobitFunc(const Nfg<double> &, const MixedProfile<double> &);
    virtual ~NFGobitFunc();
    
    void SetLambda(double l)   { _Lambda = l; }
    long NumEvals(void) const  { return _nevals; }
};


NFGobitFunc::NFGobitFunc(const Nfg<double> &N,
			 const MixedProfile<double> &start)
  : _nevals(0L), _nfg(N), _p(start)
{
  _scratch = new gVector<double> *[_nfg.NumPlayers()] - 1;
  for (int i = 1; i <= _nfg.NumPlayers(); i++)  
    _scratch[i] = new gVector<double>(_p.Support().NumStrats(i));
}

NFGobitFunc::~NFGobitFunc()
{
  for (int i = 1; i <= _nfg.NumPlayers(); i++) 
    delete _scratch[i];

  delete [] (_scratch + 1);
}

bool NFGobitFunc::Hessian(const gVector<double> &, gMatrix<double> &)
{
  return true;
}

double NFGobitFunc::GobitDerivValue(int i, int j,
				    const MixedProfile<double> &v)
{
  double x = 0.0, dv;
  
  for (int pl = 1; pl <= _nfg.NumPlayers(); pl++)  {
    gVector<double> &payoff = *_scratch[pl];
    v.Payoff(pl, pl, payoff);
    for (int st = 2; st <= _p.Support().NumStrats(pl); st++) {
      dv = (double) log(v(pl, 1)) - (double) log(v(pl, st)) -
	_Lambda * (payoff[1] - payoff[st]);
      if (pl == i)  {
	if (j == 1)          x += dv / v(pl, 1);
	else if (j == st)    x -= dv / v(pl, st);
      }
      else
	x -= dv * _Lambda * (v.Payoff(pl, pl, 1, i, j) -
			     v.Payoff(pl, pl, st, i, j));
    }
  }

  return 2.0 * x;
}


bool NFGobitFunc::Deriv(const gVector<double> &v, gVector<double> &d)
{
  ((gVector<double> &) _p).operator=(v);
  
  for (int pl = 1, index = 1; pl <= _nfg.NumPlayers(); pl++)  {
    int nstrats = _p.Support().NumStrats(pl);
    int st;

    for (st = 1; st <= nstrats;
	 d[index++] = GobitDerivValue(pl, st++, _p));
  }

  return true;
}
  
double NFGobitFunc::Value(const gVector<double> &v)
{
  _nevals++;
  ((gVector<double> &) _p).operator=(v);
  double val = 0.0, z;
  
  for (int pl = 1; pl <= _nfg.NumPlayers(); pl++)  {
    gVector<double> &payoff = *_scratch[pl];
    _p.Payoff(pl, pl, payoff);
    for (int st = 2; st <= _p.Support().NumStrats(pl); st++) {
      z = log(_p(pl, 1)) - log(_p(pl, st)) -
          _Lambda * (payoff[1] - payoff[st]);
      val += z * z;
    }
  }

  return val;
}



static void WritePXIHeader(gOutput &pxifile, const Nfg<double> &N,
			   const NFGobitParams &params)
{
  pxifile << "Dimensionality:\n";
  pxifile << N.NumPlayers() << " ";
  for (int pl = 1; pl <= N.NumPlayers(); pl++)
    pxifile << N.NumStrats(pl) << " ";
  pxifile << "\n";
  N.WriteNfgFile(pxifile);

  pxifile << "Settings:\n" << params.minLam;
  pxifile << "\n" << params.maxLam << "\n" << params.delLam;
  pxifile << "\n" << 0 << "\n" << 1 << "\n" << params.powLam << "\n";
  
  int numcols = N.GameForm().ProfileLength() + 2;

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


void Gobit(const Nfg<double> &N, NFGobitParams &params,
	   const MixedProfile<double> &start,
	   gList<MixedSolution<double> > &solutions,
	   long &nevals, long &nits)
{
  NFGobitFunc F(N, start);

  int i;
  int iter = 0, nit;
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

  MixedProfile<double> p(start);

  for (nit = 1; !params.status.Get() &&
       Lambda <= params.maxLam &&
       Lambda >= params.minLam && value < 10.0; nit++)  {

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
      i = solutions.Append(MixedSolution<double>(p, NfgAlg_GOBIT));      
      solutions[i].SetGobit(Lambda, value);
    }

    Lambda += params.delLam * pow(Lambda, params.powLam);
    params.status.SetProgress((double) step / (double) num_steps);
    step++;
  }

  if (!params.fullGraph)
  {
    i = solutions.Append(MixedSolution<double>(p, NfgAlg_GOBIT));
    solutions[i].SetGobit(Lambda, value);
  }

  if (params.status.Get())   params.status.Reset();

  nevals = F.NumEvals();
  nits = 0;
}




