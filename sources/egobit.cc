//
// FILE: egobit.cc -- Implementation of gobit on extensive form games
//
// $Id$
//

#include <math.h>

#include "egobit.h"

#include "gfunc.h"
#include "gmatrix.h"

EFQreParams::EFQreParams(gStatus &s)
  : FuncMinParams(s), powLam(1), minLam(0.01), maxLam(30.0), delLam(0.01),
    fullGraph(false), pxifile(&gnull)
{ }

EFQreParams::EFQreParams(gOutput &, gOutput &pxi,gStatus &s)
  : FuncMinParams(s), powLam(1), minLam(0.01), maxLam(30.0), delLam(0.01), 
    fullGraph(false), pxifile(&pxi)
{ }


class EFQreFunc : public gFunction<double>   {
  private:
    long _nevals;
    bool _domain_err;
    const Efg &_efg;
    gVector<double> _Lambda;
    gPVector<double> _probs;
    BehavProfile<double> _p, _cpay;
    gVector<double> ***_scratch;

    double Value(const gVector<double> &);

  public:
    EFQreFunc(const Efg &, const BehavProfile<gNumber> &);
    virtual ~EFQreFunc();
    
    gVector<double> GetLambda()   { return _Lambda; }
    void SetLambda(double l)   { _Lambda = l; }
    void SetLambda(const gVector<double> &l)   { _Lambda = l; }
    long NumEvals(void) const   { return _nevals; }
    bool DomainErr(void) const { return _domain_err;}
};

EFQreFunc::EFQreFunc(const Efg &E,
			 const BehavProfile<gNumber> &start)
  : _nevals(0L), _domain_err(false), _efg(E), 
    _Lambda(E.NumPlayers()),_probs(E.NumInfosets()),
    _p(start.Support()), _cpay(start.Support())
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
  static const double PENALTY = 10000.0;

  _nevals++;
  _domain_err = false;
 ((gVector<double> &) _p).operator=(v);
  double val = 0.0, prob, psum, z,factor;
 
  _p.CondPayoff(_cpay, _probs);
  
  for (int pl = 1; pl <= _efg.NumPlayers(); pl++)  {
    EFPlayer *player = _efg.Players()[pl];
    
    for (int iset = 1; iset <= player->NumInfosets(); iset++)  {
      prob = 0.0;
      psum = 0.0;

      int act;
      
      for (act = 1; act <= _p.Support().NumActions(pl, iset); act++)  {
	z = _Lambda[pl] * _cpay(pl, iset, act);
	factor=1.0;
	if(z>500.0) {factor+=z-500.0;z=500.0;_domain_err=true;}
	if(z<-500.0) {factor+=z+500.0;z=-500.0;_domain_err=true;}
	z = exp(z)*factor;
	psum += z;
	_cpay(pl, iset, act) = z;
      }
      
      for (act = 1; act <= _p.Support().NumActions(pl, iset); act++)  {
	z = _p(pl, iset, act);
	prob += z;
	if (z < 0.0)
	  val += PENALTY * z * z;
	z -= _cpay(pl, iset, act) / psum;
	val += z * z;
      }

      z = 1.0 - prob;
      val += 100.0 * z * z;
//      z -= _cpay(pl, iset, act) / psum;
//      val += z * z;
    }
  }

  return val;
}


static void WritePXIHeader(gOutput &pxifile, const Efg &E,
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

extern bool Powell(gPVector<double> &p, gMatrix<double> &xi,
		   gFunction<double> &func, double &fret, int &iter,
		   int maxits1, double tol1, int maxitsN, double tolN,
		   gOutput &tracefile, int tracelevel,  bool interior = false,
		   gStatus &status = gstatus);



void Qre(const Efg &E, EFQreParams &params,
	   const BehavProfile<gNumber> &start,
	   gList<BehavSolution> &solutions,
	   long &nevals, long &nits)
{
  EFQreFunc F(E, start);

  int iter = 0;
  double Lambda, LambdaOld, LambdaStart, value = 0.0;

  if (params.pxifile)
    WritePXIHeader(*params.pxifile, E, params);

  LambdaStart = (params.delLam < 0.0) ? params.maxLam : params.minLam;
  LambdaOld = Lambda = LambdaStart;

  double max_prog, prog;

  if (params.powLam == 0)
    max_prog = params.maxLam - params.minLam;
  else
    max_prog = log(params.maxLam / params.minLam);

  BehavProfile<double> p(start.Support());
  for (int i = 1; i <= p.Length(); i++)
    p[i] = start[i];
  BehavProfile<double> pold(p);
  gMatrix<double> xi(p.Length(), p.Length());

  InitMatrix(xi, p.Lengths());

  bool FoundSolution = true;
  double delta, mindelta;
  delta = params.delLam;
  mindelta = delta/100.0;

  try {
    while (delta>mindelta &&
		      Lambda <= params.maxLam && Lambda >= params.minLam)   {
      params.status.Get();
      F.SetLambda(Lambda);
      FoundSolution = Powell(p, xi, F, value, iter,
		      params.maxits1, params.tol1, params.maxitsN, params.tolN,
		      *params.tracefile, params.trace-1,true);
      bool derr = F.DomainErr();      
      double dist = 0.0;
      for(int jj=p.First();jj<=p.Last();jj++) {
	double xx = abs(p[jj]-pold[jj]);
	if(xx>dist)dist=xx;
      }

      if(FoundSolution && !derr && dist<params.delLam) {
	if (params.trace>0)  {
	  *params.tracefile << "\nLam: " << Lambda << " val: ";
	  params.tracefile->SetExpMode();
	  *params.tracefile << value;
	  params.tracefile->SetFloatMode();
	  *params.tracefile << " p: " << p;
	} 
	
	if (params.pxifile)  {
	  *params.pxifile << "\n" << Lambda << " " << value << " ";
	  for (int pl = 1; pl <= E.NumPlayers(); pl++)
	    for (int iset = 1; iset <= E.Players()[pl]->NumInfosets();
		 iset++)  {
	      double prob = 0.0;
	      for (int act = 1; act <= E.Players()[pl]->Infosets()[iset]->NumActions(); 
		   prob += p(pl, iset, act++))
		*params.pxifile << p(pl, iset, act) << ' ';
	      //	  *params.pxifile << (1.0 - prob) << ' ';
	    }
	} 
	
	if (params.fullGraph)
	  AddSolution(solutions, p, Lambda, value, params.Accuracy());
	pold=p;                              // pold is last good solution
	if(delta < params.delLam && dist<params.delLam/2.0) delta*=2.0;
      }

      else {
	Lambda = LambdaOld;
	p = pold;
	InitMatrix(xi, p.Lengths());
	if(delta>mindelta) delta/=2.0;
      }

      if (params.powLam == 0)
	prog = abs(Lambda - LambdaStart);
      else
	prog = abs(log(Lambda/LambdaStart));
      params.status.SetProgress(prog/max_prog);

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
}

class EFKQreFunc : public gFunction<double>   {
private:
  long _nevals;
  bool _domain_err;
  const Efg &_efg;
  double _K;
  gPVector<double> _probs;
  BehavProfile<double> _p, _cpay;
  gVector<double> ***_scratch;
  EFQreFunc F;
  const EFQreParams & params;
  
public:
  EFKQreFunc(const Efg &, const BehavProfile<gNumber> &, 
		  const EFQreParams & params);
  virtual ~EFKQreFunc();
  
  double Value(const gVector<double> &);
  
  void SetK(double k)   { _K = k; }
  void Get_p(BehavProfile<double> &p) const {p = _p;}
  long NumEvals(void) const   { return _nevals; }
  bool DomainErr(void) const { return _domain_err;}
};


EFKQreFunc::EFKQreFunc(const Efg &E,
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

extern bool OldPowell(gVector<double> &p, gMatrix<double> &xi,
		      gFunction<double> &func, double &fret, int &iter,
		      int maxits1, double tol1, int maxitsN, double tolN,
		      gOutput &tracefile, int tracelevel,  gStatus &status = gstatus);

void KQre(const Efg &E, EFQreParams &params, const BehavProfile<gNumber> &start,
	    gList<BehavSolution> &solutions, long &nevals, long &nits)
{
  EFKQreFunc F(E, start, params);
  int i;
  int iter = 0, nit;
  double K, K_old = 0.0, value = 0.0;
  gVector<double> lambda(E.NumPlayers());
  lambda = (double).0001;
  gVector<double> lam_old(lambda);
  
  if (params.pxifile) 
    WritePXIHeader(*params.pxifile, E, params);
  
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
    params.status.Get();
    
    F.SetK(K);
    
    FoundSolution =  OldPowell(lambda, xi, F, value, iter,
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
    if(FoundSolution && !F.DomainErr()) {
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
    
	
	for (int pl = 1; pl <= E.NumPlayers(); pl++) {
	  EFPlayer *player = E.Players()[pl];
	  for (int iset = 1; iset <= player->NumInfosets();iset++)
	    for( int act = 1;act<=(p.Support().NumActions(pl,iset));act++)
	      *params.pxifile << p(pl, iset,act) << " ";
	}
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
    params.status.SetProgress((double) step / (double) num_steps);
    step++;
  }
  
  if (!params.fullGraph) {
    i = solutions.Append(BehavSolution(p, algorithmEfg_QRE_EFG));
    solutions[i].SetQre(K_old, value);
  }
  
  nevals = F.NumEvals();
  nits = 0;
}





