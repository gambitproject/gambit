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
    const Efg<double> &_efg;
    BehavProfile<double> _p;
    gDPVector<double> _cpay;

    double Value(const gVector<double> &x);

  public:
    EFLiapFunc(const Efg<double> &, const BehavProfile<double> &);
    virtual ~EFLiapFunc();
    
    long NumEvals(void) const  { return _nevals; }
};


EFLiapFunc::EFLiapFunc(const Efg<double> &E,
		       const BehavProfile<double> &start)
  : _nevals(0L), _efg(E), _p(E, true), _cpay(E.Dimensionality())
{
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    EFPlayer *p = _efg.PlayerList()[pl];
    for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
      Infoset *s = p->InfosetList()[iset];
      for (int act = 1; act < s->NumActions(); act++)  {
	_p(pl, iset, act) = start(pl, iset, act);
      }
    }
  }
}

EFLiapFunc::~EFLiapFunc()
{ }


double EFLiapFunc::Value(const gVector<double> &v)
{
  static const double BIG1 = 10000.0;
//  static const double BIG2 = 100.0;

  _nevals++;


  ((gVector<double> &) _p).operator=(v);
  BehavProfile<double> tmp(_p);
  double x, result = 0.0, avg, sum;

  gPVector<double> probs(_efg.Dimensionality().Lengths());  
  tmp.CondPayoff(_cpay, probs);

  for (int i = 1; i <= _efg.NumPlayers(); i++) {
    EFPlayer *player = _efg.PlayerList()[i];
    for (int j = 1; j <= player->NumInfosets(); j++) {
      avg = sum = 0.0;
      Infoset *s = player->InfosetList()[j];
      int k;
      for (k = 1; k < s->NumActions(); k++) {
	x = _p(i, j, k); 
	avg += x * _cpay(i, j, k);
	sum += x;
	if (x > 0.0)  x = 0.0;
	result += BIG1 * x * x;         // add penalty for neg probabilities
      }

      // last component truncated off....
      x = 1.0 - sum;
      avg += x * _cpay(i, j, k);
      if (x > 0.0)   x = 0.0;
      result += BIG1 * x * x;

      for (k = 1; k <= s->NumActions(); k++) {
	x = _cpay(i, j, k) - avg;
	if (x < 0.0) x = 0.0;
	result += x * x;          // add penalty if not best response
      }

/*  with truncation enforced, this is always zero
      x = sum - 1.0;
      result += BIG2 * x * x;       // add penalty for sum not equal to 1
      */
    }
  }

  return result;
}


static void PickRandomProfile(BehavProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.BelongsTo()->NumPlayers(); pl++)  {
    for (int iset = 1; iset <= p.BelongsTo()->PlayerList()[pl]->NumInfosets();
	 iset++)  {
      sum = 0.0;
      int act;
    
      for (act = 1; act < p.GetEFSupport().NumActions(pl, iset); act++)  {
	do
	  tmp = Uniform();
	while (tmp + sum > 1.0);
	p(pl, iset, act) = tmp;
	sum += tmp;
      }
  
// with truncation, this is unnecessary
//    p(pl, iset, act) = 1.0 - sum;
    }
  }
}


static void AddSolution(gList<BehavSolution<double> > &solutions,
			const BehavProfile<double> &profile,
		        double value)
{
  int i;
  const Efg<double> &E = *profile.BelongsTo();

  BehavProfile<double> bar(E);
  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    for (int iset = 1; iset <= E.PlayerList()[pl]->NumInfosets(); iset++)  {
      double accum = 0.0;
      int act;
      for (act = 1; act < E.PlayerList()[pl]->InfosetList()[iset]->NumActions(); act++)  {
	bar(pl, iset, act) = profile(pl, iset, act);
	accum += profile(pl, iset, act);
      }
      bar(pl, iset, act) = 1.0 - accum;
    }
  }

  i = solutions.Append(BehavSolution<double>(bar, EfgAlg_LIAP));
  solutions[i].SetLiap(value);
}


extern bool Powell(gVector<double> &p,
		   gMatrix<double> &xi,
		   gFunction<double> &func,
		   double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile);


bool Liap(const Efg<double> &E, EFLiapParams &params,
	  const BehavProfile<double> &start,
	  gList<BehavSolution<double> > &solutions,
	  long &nevals, long &niters)
{
  EFLiapFunc F(E, start);

  BehavProfile<double> p(E, true);
  
  // convert from non-truncated to truncated vector for starting point
  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    EFPlayer *player = E.PlayerList()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      Infoset *s = player->InfosetList()[iset];
      for (int act = 1; act < s->NumActions(); act++)  {
	p(pl, iset, act) = start(pl, iset, act);
      }
    }
  }

  gMatrix<double> xi(p.Length(), p.Length());
  xi.MakeIdent();

  double value;
  int iter;
  bool found;

  for (int i = 1; !params.status.Get() && i <= params.nTries &&
       solutions.Length() < params.stopAfter; i++)   {
    if (i > 1)  PickRandomProfile(p);

    if (found = Powell(p, xi, F, value, iter,
		       params.maxits1, params.tol1, params.maxitsN, params.tolN,
		       *params.tracefile)) 
      AddSolution(solutions, p, value);
    
  }

  nevals = F.NumEvals();
  niters = 0L;

  return (solutions.Length() > 0);
}



//------------------------------------------
// Interfacing to solve-by-subgame code
//------------------------------------------



int EFLiapBySubgame::SolveSubgame(const Efg<double> &E,
				  gList<BehavSolution<double> > &solns)
{
  BehavProfile<double> bp(E);
  
  subgame_number++;

  gArray<int> infosets(infoset_subgames.Lengths());

  for (int pl = 1; pl <= E.NumPlayers(); pl++)  {
    int niset = 1;
    for (int iset = 1; iset <= infosets[pl]; iset++)  {
      if (infoset_subgames(pl, iset) == subgame_number)  {
	for (int act = 1; act <= bp.GetEFSupport().NumActions(pl, niset); act++)
	  bp(pl, niset, act) = start(pl, iset, act);
	niset++;
      }
    }
  }

  long this_nevals, this_niters;

  Liap(E, params, bp, solns, this_nevals, this_niters);

  nevals += this_nevals;
  return params.status.Get();
}

extern void MarkedSubgameRoots(const BaseEfg &, gList<Node *> &);

EFLiapBySubgame::EFLiapBySubgame(const Efg<double> &E, const EFLiapParams &p,
				 const BehavProfile<double> &s, int max)
  : SubgameSolver<double>(E, max), nevals(0), subgame_number(0),
    infoset_subgames(E.PureDimensionality()), params(p), start(s)
{
  gList<Node *> subroots;
  MarkedSubgameRoots(E, subroots);

  for (int pl = 1; pl <= E.NumPlayers(); pl++)   {
    EFPlayer *player = E.PlayerList()[pl];
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      int index;

      Infoset *infoset = player->InfosetList()[iset];
      Node *member = infoset->GetMember(1);

      for (index = 1; index <= subroots.Length() &&
	   member->GetSubgameRoot() != subroots[index]; index++);

      assert(index <= subroots.Length());

      infoset_subgames(pl, iset) = index;
    }
  }   

}

EFLiapBySubgame::~EFLiapBySubgame()   { }


