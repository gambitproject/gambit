//
// FILE: egobit.cc -- Implementation of gobit on extensive form games
//
// $Id$
//

#include <math.h>

#include "egobit.h"

#include "gfunc.h"
#include "gmatrix.h"

EFGobitParams::EFGobitParams(gStatus &s)
  : trace(0), powLam(1), maxits1(100), maxitsN(20),
    minLam(0.01), maxLam(30.0), delLam(0.01), tol1(2.0e-10), tolN(1.0e-10),
    fullGraph(false), tracefile(&gnull), pxifile(&gnull),
    status(s)
{ }

EFGobitParams::EFGobitParams(gOutput &out, gOutput &pxi, gStatus &s)
  : trace(0), powLam(1), maxits1(100), maxitsN(20),
    minLam(0.01), maxLam(30.0), delLam(0.01), tol1(2.0e-10), tolN(1.0e-10),
    fullGraph(false), tracefile(&out), pxifile(&pxi),
    status(s)
{ }


class EFGobitFunc : public gFunction<double>   {
  private:
    long _nevals;
    const Efg<double> &_efg;
    double _Lambda;
    gPVector<double> _probs;
    BehavProfile<double> _p, _cpay;
    gVector<double> ***_scratch;

    double Value(const gVector<double> &);

  public:
    EFGobitFunc(const Efg<double> &, const BehavProfile<double> &);
    virtual ~EFGobitFunc();
    
    void SetLambda(double l)   { _Lambda = l; }
    long NumEvals(void) const   { return _nevals; }
};

EFGobitFunc::EFGobitFunc(const Efg<double> &E,
			 const BehavProfile<double> &start)
  : _nevals(0L), _efg(E), _probs(E.Dimensionality().Lengths()),
    _p(E, true), _cpay(E)
{
  int pl;

  _scratch = new gVector<double> **[_efg.NumPlayers()] - 1;
  for (pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    int nisets = (_efg.PlayerList()[pl])->NumInfosets();
    _scratch[pl] = new gVector<double> *[nisets + 1] - 1;
    for (int iset = 1; iset <= nisets; iset++)
      _scratch[pl][iset] = new gVector<double>(_p.GetEFSupport().NumActions(pl, iset));
  }
  
  for (pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    EFPlayer *p = _efg.PlayerList()[pl];
    for (int iset = 1; iset <= p->NumInfosets(); iset++)  {
      Infoset *s = p->InfosetList()[iset];
      for (int act = 1; act < s->NumActions(); act++)  {
	_p(pl, iset, act) = start(pl, iset, act);
      }
    }
  }
}

EFGobitFunc::~EFGobitFunc()
{
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    int nisets = (_efg.PlayerList()[pl])->NumInfosets();
    for (int iset = 1; iset <= nisets; iset++)
      delete _scratch[pl][iset];
    delete [] (_scratch[pl] + 1);
  }
  delete [] (_scratch + 1);
}

double EFGobitFunc::Value(const gVector<double> &v)
{
  static const double PENALTY = 10000.0;

  _nevals++;
  ((gVector<double> &) _p).operator=(v);
  double val = 0.0, prob, psum, z;

  _p.CondPayoff(_cpay, _probs);
  
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    EFPlayer *player = _efg.PlayerList()[pl];
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      prob = 0.0;
      psum = 0.0;

      Infoset *s = player->InfosetList()[iset];
      int act;
      
      for (act = 1; act <= s->NumActions(); act++)  {
	z = _Lambda * _cpay(pl, iset, act);
	z = exp(z);
	psum += z;
	_cpay(pl, iset, act) = z;
      }
      
      for (act = 1; act < s->NumActions(); act++)  {
	z = _p(pl, iset, act);
	prob += z;
	if (z < 0.0)
	  val += PENALTY * z * z;
	z -= _cpay(pl, iset, act) / psum;
	val += z * z;
      }

      z = 1.0 - prob;
      if (z < 0.0)
	val += PENALTY * z * z;
      z -= _cpay(pl, iset, act) / psum;
      val += z * z;
    }
  }

  return val;
}


static void WritePXIHeader(gOutput &pxifile, const Efg<double> &E,
			   const EFGobitParams &/*params*/)
{
  int pl, iset, nisets = 0;

  pxifile << "Dimensionality:\n";
  for (pl = 1; pl <= E.NumPlayers(); pl++)
    nisets += E.PlayerList()[pl]->NumInfosets();
  pxifile << nisets;
  for (pl = 1; pl <= E.NumPlayers(); pl++)
    for (iset = 1; iset <= E.PlayerList()[pl]->NumInfosets(); iset++)
      pxifile << " " << E.PlayerList()[pl]->InfosetList()[iset]->NumActions();
  pxifile << "\n";

  int numcols = E.ProfileLength() + 2;
  pxifile << "DataFormat:";
  pxifile << "\n" << numcols;
  for (int i = 1; i <= numcols; i++)
    pxifile << ' ' << i;
  pxifile << "\nData:\n";
}

static void AddSolution(gList<BehavSolution<double> > &solutions,
			const BehavProfile<double> &profile,
			double lambda,
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

  i = solutions.Append(BehavSolution<double>(bar, EfgAlg_GOBIT));
  solutions[i].SetGobit(lambda, value);
}

extern bool Powell(gVector<double> &p, gMatrix<double> &xi,
		   gFunction<double> &func, double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile);



void Gobit(const Efg<double> &E, EFGobitParams &params,
	   const BehavProfile<double> &start,
	   gList<BehavSolution<double> > &solutions,
	   long &nevals, long &nits)
{
  EFGobitFunc F(E, start);

  int iter = 0, nit;
  double Lambda, value = 0.0;

  if (params.pxifile)
    WritePXIHeader(*params.pxifile, E, params);

  Lambda = (params.delLam < 0.0) ? params.maxLam : params.minLam;

  int num_steps, step = 0;

  if (params.powLam == 0)
    num_steps = (int) ((params.maxLam - params.minLam) / params.delLam);
  else
    num_steps = (int) (log(params.maxLam / params.minLam) /
		       log(params.delLam + 1.0));

  BehavProfile<double> p(E, true);
  p.Centroid();

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

  for (nit = 1; !params.status.Get() &&
       Lambda <= params.maxLam && Lambda >= params.minLam &&
       value < 10.0; nit++)   {

    F.SetLambda(Lambda);
    Powell(p, xi, F, value, iter,
	   params.maxits1, params.tol1, params.maxitsN, params.tolN,
	   (params.tracefile) ? *params.tracefile : gnull);
    
    // tracefile stuff omitted for now

    if (params.pxifile)  {
      *params.pxifile << "\n" << Lambda << " " << value;
      *params.pxifile << " ";
      for (int pl = 1; pl <= E.NumPlayers(); pl++)
	for (int iset = 1; iset <= E.PlayerList()[pl]->NumInfosets();
	     iset++)  {
	  double prob = 0.0;
	  for (int act = 1; act < E.PlayerList()[pl]->InfosetList()[iset]->NumActions(); prob += p(pl, iset, act++))
	    *params.pxifile << p(pl, iset, act) << ' ';
	  *params.pxifile << (1.0 - prob) << ' ';
	}
    } 

    if (params.fullGraph)
      AddSolution(solutions, p, Lambda, value);

    Lambda += params.delLam * pow(Lambda, params.powLam);
    params.status.SetProgress((double) step / (double) num_steps);
    step++;
  }

  if (!params.fullGraph)
    AddSolution(solutions, p, Lambda, value);

  if (params.status.Get())    params.status.Reset();

  nevals = F.NumEvals();
  nits = 0;
}

