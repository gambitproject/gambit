//
// FILE: nliap.cc -- Implementation of Liapunov algorithm for normal forms
//
//  $Id$
//

#include "nliap.h"
#include "numerical/gfunc.h"

NFLiapParams::NFLiapParams(void)
  : nTries(10)
{ }

class NFLiapFunc : public gC2Function<double>   {
  private:
    long _nevals;
    const Nfg &_nfg;
    MixedProfile<double> _p;

    double Value(const gVector<double> &);

    double LiapDerivValue(int, int, const MixedProfile<double> &);
    
    bool Deriv(const gVector<double> &, gVector<double> &);
    bool Hessian(const gVector<double> &, gMatrix<double> &);

  public:
    NFLiapFunc(const Nfg &, const MixedProfile<double> &);
    virtual ~NFLiapFunc();
    
    long NumEvals(void) const  { return _nevals; }
};

NFLiapFunc::NFLiapFunc(const Nfg &N,
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
    for (j = 1; j <= p.Support().NumStrats(i); j++)  {
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
    for (j1 = 1; j1 <= _p.Support().NumStrats(i1); j1++) {
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
    for (j = 1; j <= _p.Support().NumStrats(i); j++)  {
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
    for (j = 1; j <= _p.Support().NumStrats(i); j++)  {
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

  for (int pl = 1; pl <= p.Game().NumPlayers(); pl++)  {
    sum = 0.0;
    int st;
    
    for (st = 1; st < p.Support().NumStrats(pl); st++)  {
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
		gOutput &tracefile, int tracelevel, bool interior,
		gStatus &status);


bool Liap(const Nfg &N, NFLiapParams &params,
	  const MixedProfile<gNumber> &start,
	  gList<MixedSolution> &solutions, gStatus &p_status,
	  long &nevals, long &niters)
{
  static const double ALPHA = .00000001;
  MixedProfile<double> p(start.Support());
  for (int i = 1; i <= p.Length(); i++)
    p[i] = start[i];

  NFLiapFunc F(N, p);

  // if starting vector not interior, perturb it towards centroid
  int kk;
  for(kk=1;kk <= p.Length() && p[kk]>ALPHA;kk++);
  if(kk<=p.Length()) {
    MixedProfile<double> c(start.Support());
    for(int k=1;k<=p.Length();k++)
      p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
  }

  double value;
  int iter;
  bool found = false;

  solutions.Flush();

  for (int i = 1;  
       (params.nTries == 0 || i <= params.nTries) &&
	 (params.stopAfter==0 || solutions.Length() < params.stopAfter);
       i++) { 
    p_status.Get();
    if (i > 1) PickRandomProfile(p);
    
    if (params.trace>0)
      *params.tracefile << "\nTry #: " << i << " p: ";
    
    if ((found = DFP(p, F, value, iter, params.maxits1, params.tol1,
		     params.maxitsN, params.tolN, *params.tracefile,
		     params.trace-1, false, p_status)) == true)  {
      bool add = true;
      int ii = 1;
      while (ii <= solutions.Length() && add == true) {
	if (solutions[ii].Equals(p)) 
	  add = false;
	  ii++;
	}

      if (add)  {
	if(params.trace>0)
	  *params.tracefile << p;
      
	int index = solutions.Append(MixedSolution(p, algorithmNfg_LIAP));
	solutions[index].SetEpsilon( params.Accuracy() );
      }
    }
  }

  nevals = F.NumEvals();
  niters = 0L;

  return found;
}
