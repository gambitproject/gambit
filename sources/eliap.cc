//
// FILE: eliap.cc -- Extensive Form Liapunov module
//
// $Id$
//

#include "eliap.h"

#include "gfunc.h"
#include "gmatrix.h"

EFLiapParams::EFLiapParams(gStatus &s)
  : trace(0), nTries(10), stopAfter(1), maxits1(100), maxitsN(20),
    tol1(2.0e-10), tolN(1.0e-10), tracefile(&gnull), status(s)
{ }



class EFLiapFunc : public gFunction<double>  {
  private:
    long _nevals;
    const Efg &_efg;
    BehavProfile<double> _p;
    gDPVector<double> _cpay;

    double Value(const gVector<double> &x);

  public:
    EFLiapFunc(const Efg &, const BehavProfile<double> &);
    virtual ~EFLiapFunc();
    
    long NumEvals(void) const  { return _nevals; }
};


EFLiapFunc::EFLiapFunc(const Efg &E,
		       const BehavProfile<double> &start)
  : _nevals(0L), _efg(E), _p(start), _cpay(E.NumActions())
{ }

EFLiapFunc::~EFLiapFunc()
{ }


double EFLiapFunc::Value(const gVector<double> &v)
{
  static const double BIG1 = 10000.0;
  static const double BIG2 = 100.0;

  _nevals++;


  ((gVector<double> &) _p).operator=(v);
  BehavProfile<double> tmp(_p);
  double x, result = 0.0, avg, sum;

  gPVector<double> probs(_efg.NumInfosets());  
  tmp.CondPayoff(_cpay, probs);

  for (int i = 1; i <= _efg.NumPlayers(); i++) {
    EFPlayer *player = _efg.Players()[i];
    for (int j = 1; j <= player->NumInfosets(); j++) {
      avg = sum = 0.0;
      int k;
      for (k = 1; k <= _p.Support().NumActions(i, j); k++) {
	x = _p(i, j, k); 
	avg += x * _cpay(i, j, k);
	sum += x;
	if (x > 0.0)  x = 0.0;
	result += BIG1 * x * x;         // add penalty for neg probabilities
      }

      for (k = 1; k <= _p.Support().NumActions(i, j); k++) {
	x = _cpay(i, j, k) - avg;
	if (x < 0.0) x = 0.0;
	result += x * x;          // add penalty if not best response
      }

      x = sum - 1.0;
      result += BIG2 * x * x;       // add penalty for sum not equal to 1
    }
  }

  return result;
}


static void PickRandomProfile(BehavProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.Game().NumPlayers(); pl++)  {
    for (int iset = 1; iset <= p.Game().Players()[pl]->NumInfosets();
	 iset++)  {
      sum = 0.0;
      int act;
    
      for (act = 1; act < p.Support().NumActions(pl, iset); act++)  {
	do
	  tmp = Uniform();
	while (tmp + sum > 1.0);
	p(pl, iset, act) = tmp;
	sum += tmp;
      }
  
// with truncation, this is unnecessary
      p(pl, iset, act) = 1.0 - sum;
    }
  }
}


static void AddSolution(gList<BehavSolution> &solutions,
			const BehavProfile<double> &profile,
		        double value, double epsilon)
{
  int i = solutions.Append(BehavSolution(profile, EfgAlg_LIAP));
  solutions[i].SetLiap(value);
  solutions[i].SetEpsilon(epsilon);
  solutions[i].SetIsNash(triTRUE);
  solutions[i].SetIsSubgamePerfect(triTRUE);
  solutions[i].SetIsSequential(triTRUE);
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

extern bool Powell(gPVector<double> &p,
		   gMatrix<double> &xi,
		   gFunction<double> &func,
		   double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile, int tracelevel, bool interior = false,
		   gStatus &status = gstatus);


bool Liap(const Efg &E, EFLiapParams &params,
	  const BehavProfile<double> &start,
	  gList<BehavSolution> &solutions,
	  long &nevals, long &niters)
{
  EFLiapFunc F(E, start);

  BehavProfile<double> p(start);

  gMatrix<double> xi(p.Length(), p.Length());

  double value;
  int iter;

  for (int i = 1; (params.nTries == 0 || i <= params.nTries) &&
       (params.stopAfter==0 || solutions.Length() < params.stopAfter); 
       i++)   {
    params.status.Get();
    if (i > 1)  PickRandomProfile(p);

    InitMatrix(xi, p.Lengths());
    
    if(params.trace>0)
      *params.tracefile << "\nTry #: " << i << " p: ";
    
    if (Powell(p, xi, F, value, iter,
		       params.maxits1, params.tol1, params.maxitsN, 
		       params.tolN,*params.tracefile, params.trace-1, true, 
		       params.status)) {
      
      bool add = true;
      int ii=1;
      while(ii<=solutions.Length() && add == true) {
	if(solutions[ii].Equals(p)) 
	  add = false;
	ii++;
      }

      if (add)  {
	if(params.trace>0)
	  *params.tracefile << p;
	
	AddSolution(solutions, p, value, pow(params.tolN,.5));
      }
    }
  }

  nevals = F.NumEvals();
  niters = 0L;

  return (solutions.Length() > 0);
}



//------------------------------------------
// Interfacing to solve-by-subgame code
//------------------------------------------

void efgLiapSolve::SolveSubgame(const Efg &E, const EFSupport &sup,
				gList<BehavSolution> &solns)
{
  BehavProfile<double> bp(sup);
  
  subgame_number++;

  gArray<int> infosets(infoset_subgames.Lengths());

  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    int niset = 1;
    for (int iset = 1; iset <= infosets[pl]; iset++)  {
      if (infoset_subgames(pl, iset) == subgame_number)  {
	for (int act = 1; act <= bp.Support().NumActions(pl, niset); act++)
	  bp(pl, niset, act) = start(pl, iset, act);
	niset++;
      }
    }
  }

  long this_nevals, this_niters;

  Liap(E, params, bp, solns, this_nevals, this_niters);

  nevals += this_nevals;
}

extern void MarkedSubgameRoots(const Efg &, gList<Node *> &);

efgLiapSolve::efgLiapSolve(const Efg &E, const EFLiapParams &p,
			   const BehavProfile<gNumber> &s, int max)
  : SubgameSolver(max), nevals(0), subgame_number(0),
    infoset_subgames(E.NumInfosets()), params(p), start(s)
{
  gList<Node *> subroots;
  MarkedSubgameRoots(E, subroots);

  for (int pl = 1; pl <= E.NumPlayers(); pl++)   {
    EFPlayer *player = E.Players()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      int index;

      Infoset *infoset = player->Infosets()[iset];
      Node *member = infoset->Members()[1];

      for (index = 1; index <= subroots.Length() &&
	   member->GetSubgameRoot() != subroots[index]; index++);

      infoset_subgames(pl, iset) = index;
    }
  }   

}

efgLiapSolve::~efgLiapSolve()   { }


