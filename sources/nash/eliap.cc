//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Compute Nash equilibria by minimizing Liapunov function
//

#include "eliap.h"
#include "math/gmatrix.h"
#include "numerical/gfunc.h"

class EFLiapFunc : public gFunction<double>  {
  private:
    long _nevals;
    const efgGame &_efg;
    BehavProfile<double> _p;

    double Value(const gVector<double> &x);

  public:
    EFLiapFunc(const efgGame &, const BehavProfile<double> &);
    virtual ~EFLiapFunc();
    
    long NumEvals(void) const  { return _nevals; }
};


EFLiapFunc::EFLiapFunc(const efgGame &E,
		       const BehavProfile<double> &start)
  : _nevals(0L), _efg(E), _p(start)
{ }

EFLiapFunc::~EFLiapFunc()
{ }


double EFLiapFunc::Value(const gVector<double> &v)
{
  _nevals++;
  ((gVector<double> &) _p).operator=(v);
    //_p = v;
  return _p.LiapValue();
}

static void PickRandomProfile(BehavProfile<double> &p)
{
  double sum, tmp;

  for (int pl = 1; pl <= p.GetGame().NumPlayers(); pl++)  {
    for (int iset = 1; iset <= p.GetGame().Players()[pl]->NumInfosets();
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
		        double /*value*/, double epsilon)
{
  int index = solutions.Append(BehavSolution(profile, algorithmEfg_LIAP_EFG));
  solutions[index].SetEpsilon(epsilon);
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
		   gOutput &tracefile, int tracelevel, bool interior,
		   gStatus &status);


efgLiap::efgLiap(void)
  : m_stopAfter(1), m_numTries(10), m_maxits1(100), m_maxitsN(20),
    m_tol1(2.0e-10), m_tolN(1.0e-10)
{ }

gList<BehavSolution> efgLiap::Solve(const EFSupport &p_support,
				    gStatus &p_status)
{
  static const double ALPHA = .00000001;

  BehavProfile<double> p(p_support);
  EFLiapFunc F(p_support.GetGame(), p);

  // if starting vector not interior, perturb it towards centroid
  int kk;
  for(kk=1;kk <= p.Length() && p[kk]>ALPHA;kk++);
  if(kk<=p.Length()) {
    BehavProfile<double> c(p_support);
    for(int k=1;k<=p.Length();k++)
      p[k] = c[k]*ALPHA + p[k]*(1.0-ALPHA);
  }

  gMatrix<double> xi(p.Length(), p.Length());

  double value;
  int iter;

  gList<BehavSolution> solutions;
  
  for (int i = 1; (m_numTries == 0 || i <= m_numTries) &&
       (m_stopAfter == 0 || solutions.Length() < m_stopAfter); 
       i++)   {
    p_status.Get();
    if (i > 1)  PickRandomProfile(p);

    InitMatrix(xi, p.Lengths());
    
    gPVector<double> pvect(p.GetPVector());
    gNullOutput gnull;
    if (Powell(pvect, xi, F, value, iter,
	       m_maxits1, m_tol1, m_maxitsN, 
	       m_tolN, gnull, 0, true, 
	       p_status)) {
      p = pvect;
      bool add = true;
      int ii=1;
      while(ii<=solutions.Length() && add == true) {
	if(solutions[ii].Equals(p)) 
	  add = false;
	ii++;
      }

      if (add)  {
	AddSolution(solutions, p, value, pow(m_tolN,.5));
      }
    }
  }

  return solutions;
}

