//
// FILE: nliap.cc -- Implementation of Liapunov algorithm for normal forms
//
// $Id$
//

#include "nliap.h"

#include "gfunc.h"

NFLiapParams::NFLiapParams(gStatus &s)
  : trace(0), nTries(10), stopAfter(1), maxits1(100), maxitsN(20),
    tol1(2.0e-10), tolN(1.0e-10), tracefile(&gnull), status(s)
{ }

class NFLiapFunc : public gC2Function<double>   {
  private:
    long _nevals;
    const Nfg<double> &_nfg;
    MixedProfile<double> _p;

    double Value(const gVector<double> &);

    double LiapDerivValue(int, int, const MixedProfile<double> &);
    
    bool Deriv(const gVector<double> &, gVector<double> &);
    bool Hessian(const gVector<double> &, gMatrix<double> &);

  public:
    NFLiapFunc(const Nfg<double> &, const MixedProfile<double> &);
    virtual ~NFLiapFunc();
    
    long NumEvals(void) const  { return _nevals; }
};

NFLiapFunc::NFLiapFunc(const Nfg<double> &N,
		       const MixedProfile<double> &start)
  : _nevals(0L), _nfg(N), _p(start)
{ }

NFLiapFunc::~NFLiapFunc()
{ }

bool NFLiapFunc::Hessian(const gVector<double> &, gMatrix<double> &)
{
  return true;
} 

double NFLiapFunc::LiapDerivValue(int i1, int j1,
				  const MixedProfile<double> &p)
{
  int i, j;
  double x, x1, psum;
  
  x = 0.0;
  for (i = 1; i <= _nfg.NumPlayers(); i++)  {
    psum = 0.0;
    for (j = 1; j <= p.GetNFSupport().NumStrats(i); j++)  {
      psum += p(i,j);
      x1 = p.Payoff(i, i, j) - p.Payoff(i);
      if (i1 == i) {
	if (x1 > 0.0)
	  x -= x1 * p.Payoff(i, i1, j1);
      }
      else {
	if (x1> 0.0)
	  x += x1 * (p.Payoff(i, i, j, i1, j1) - p.Payoff(i, i1, j1));
      }
    }
    if (i == i1)  x += psum - 1.0;
  }
  if (p(i1, j1) < 0.0)   x += p(i1, j1);
  return 2.0 * x;
}

bool NFLiapFunc::Deriv(const gVector<double> &v, gVector<double> &d)
{
  ((gVector<double> &) _p).operator=(v);
  int i1, j1, ii;
  
  for (i1 = 1, ii = 1; i1 <= _nfg.NumPlayers(); i1++) {//
    for (j1 = 1; j1 <= _p.GetNFSupport().NumStrats(i1); j1++) {
      d[ii++] = LiapDerivValue(i1, j1, _p);
    }
  }
  return true;
}
  
double NFLiapFunc::Value(const gVector<double> &v)
{
  static const double BIG1 = 100.0;
  static const double BIG2 = 100.0;

  _nevals++;

  ((gVector<double> &) _p).operator=(v);
  
  MixedProfile<double> tmp(_p);
  gPVector<double> payoff(_p);

  double x, result = 0.0, avg, sum;
  payoff = 0.0;
  
  for (int i = 1; i <= _nfg.NumPlayers(); i++)  {
    tmp.CopyRow(i, payoff);
    avg = sum = 0.0;

    // then for each strategy for player i, consider the value of
    // deviating to that strategy

    int j;
    for (j = 1; j <= _p.GetNFSupport().NumStrats(i); j++)  {
      tmp(i, j) = 1.0;
      x = _p(i, j);
      payoff(i, j) = tmp.Payoff(i);
      avg += x * payoff(i, j);
      sum += x;
      if (x > 0.0)  x = 0.0;
      result += BIG1 * x * x;   // penalty for neg probabilities
      tmp(i, j) = 0.0;
    }

    tmp.CopyRow(i, _p);
    for (j = 1; j <= _p.GetNFSupport().NumStrats(i); j++)  {
      x = payoff(i, j) - avg;
      if (x < 0.0)  x = 0.0;
      result += x * x;        // penalty for not best response
    }
    
    x = sum - 1.0;
    result += BIG2 * x * x;   // penalty for not summing to 1
  }
  return result;
}

static void PickRandomProfile(MixedProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.BelongsTo()->NumPlayers(); pl++)  {
    sum = 0.0;
    int st;
    
    for (st = 1; st < p.GetNFSupport().NumStrats(pl); st++)  {
      do
	tmp = Uniform();
      while (tmp + sum > 1.0);
      p(pl, st) = tmp;
      sum += tmp;
    }
    p(pl, st) = 1.0 - sum;
  }
}

extern bool DFP(gPVector<double> &p,
		gC2Function<double> &func,
		double &fret, int &iter,
		int maxits1, double tol1, int maxitsN, double tolN,
		gOutput &tracefile, int tracelevel,
		gStatus &status = gstatus);


bool Liap(const Nfg<double> &N, NFLiapParams &params,
	  const MixedProfile<double> &start,
	  gList<MixedSolution<double> > &solutions,
	  long &nevals, long &niters)
{
  NFLiapFunc F(N, start);

  MixedProfile<double> p(start);

  double value;
  int iter;
  bool found;

  solutions.Flush();

  for (int i = 1; !params.status.Get() &&
       i <= params.nTries && solutions.Length() < params.stopAfter; i++)   {
    if (i > 1)   PickRandomProfile(p);

    if (found = DFP(p, F, value, iter, params.maxits1, params.tol1,
		    params.maxitsN, params.tolN, *params.tracefile,
		    params.trace, params.status))  {
      bool add = false;
      if ((!params.status.Get()) 
//	  || (params.status.Get() && p.IsNash())
	)
	add = true;
      if (add)  {
	int index = solutions.Append(MixedSolution<double>(p, NfgAlg_LIAP));
	solutions[index].SetLiap(value);
	if (!params.status.Get()) {
	  solutions[index].SetEpsilon(params.tolN);
	  solutions[index].IsNash();
	}
      }   
    }
    if(params.status.Get()) params.status.Reset(); 
  }

  nevals = F.NumEvals();
  niters = 0L;

  return found;
}


  
