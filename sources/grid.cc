//#
//# FILE: grid.cc -- Grid-search solution module
//#
//# $Id$ 
//#

#include "gmisc.h"
#include "rational.h"
#include "gmatrix.h"
#include "normal.h"
#include "normiter.h"
#include "probvect.h"
#include "gwatch.h"
#include "grid.h"


template <class T>
GridParams<T>::GridParams(void) :
	plev(0),outfile(0),errfile(0),pxifile(0),
	minLam(.01), maxLam(30), delLam(.01), tol(.01), delp(.01),
	status(gstatus)
{ }
template <class T>
GridParams<T>::GridParams(const GridParams<T> &p) :
	plev(p.plev),outfile(p.outfile),errfile(p.errfile),
	pxifile(p.pxifile),
	minLam(p.minLam),maxLam(p.maxLam),delLam(p.delLam),
	delp(p.delp),tol(p.tol),type(p.type),status(p.status)

{ }
template <class T>
GridParams<T>::GridParams(gStatus &st):
	plev(0),outfile(0),errfile(0),pxifile(0),
	minLam(.01), maxLam(30), delLam(.01), tol(.01), delp(.01),
	status(st)
{ }

template <class T>
int GridParams<T>::Ok(void) const
{
if (!pxifile) return 0;
return 1;
}

template <class T>
GridSolveModule<T>::GridSolveModule(const NormalForm<T> &r,const GridParams<T> &param)
	: nf(r), params(param), x(r.NumStrats(1)), p(r.NumStrats(1)),
		y(r.NumStrats(2)), q_calc(r.NumStrats(2)),
		matrix(r.NumStrats(1),r.NumStrats(2))
{}

template <class T>
GridSolveModule<T>::~GridSolveModule(void)
{
}

// Output header
template <class T>
void GridSolveModule<T>::OutputHeader(gOutput &out)
{
int st1=nf.NumStrats(1),st2=nf.NumStrats(2);
out<<"Dimensionality:\n";
out<<2<<' '<<nf.NumStrats(1)<<' '<<nf.NumStrats(2)<<'\n';

out<<"Game:\n";
out<<"3 2 1\n";
for (int i=1;i<=st1;i++)
{
	for (int j=1;j<=st2;j++)
		out<<matrix(i,j).row<<' '<<matrix(i,j).col<<' ';
	out<<'\n';
}

out<<"Settings:\n";
out<<params.minLam<<'\n'<<params.maxLam<<'\n'<<params.delLam<<'\n';
out<<0<<'\n'<<1<<'\n'<<params.type<<'\n';

out<<"Extra:\n";
out<<1<<'\n'<<params.tol<<'\n'<<params.delp<<'\n';

out<<"DataFormat:\n";
int num_columns=st1+st2+2;
out<<num_columns<<' '<<(num_columns-1)<<' '<<num_columns<<' ';
for (i=1;i<=st1;i++) out<<i<<' ';
for (i=1;i<=st2;i++) out<<(st1+i)<<' ';
out<<'\n';

out<<"Data:\n";
}

template <class T>
void GridSolveModule<T>::OutputResult(gOutput &out,T l, T dist,gVector<T> &q,gVector<T> &p)
{
	int i;
	for (i = p.First(); i <= p.Last(); i++)  out << p[i] << ' ';
	for (i = q.First(); i <= q.Last(); i++)  out << q[i] << ' ';
	out << l << ' ' << dist << '\n';
}

template <class T>
int GridSolveModule<T>::CheckEqu(gVector<T> &q, T l)
{
T denom;					// denominator of function
bool ok;         	// was a solution found?
T dist;   				// metric function--distance from the equ
int i, j;
int st1=nf.NumStrats(1),st2=nf.NumStrats(2);
/*---------------------make X's---------------------*/
x=(T)0;			// zero out the entire x-vector
for (i=1;i<=st1;i++)
	for (j=1;j<=st2;j++) x[i] += matrix(i,j).row*q[j];
/*--------------------make P's----------------------*/
denom=(T)0;
for (i=1;i<=st1;i++) denom += exp(l*x[i]);
for (i=1;i<=st1;i++) p[i] = ((T) exp(l*x[i])) / denom;
/*--------------------make Y's----------------------*/
y=(T)0;			// zero out the entire y-vector
for (i=1;i<=st2;i++)
	for (j=1;j<=st1;j++) y[i] += matrix(j,i).col*p[j]; // (i,j) or (j,i) ?!
/*--------------------make Q_CALC's-----------------*/
denom=(T)0;
for (i=1;i<=st2;i++) denom+=exp(l*y[i]);
for (i=1;i<=st2;i++) q_calc[i]=((T)exp(l*y[i])) / denom;

/*--------------------check for equilibrium---------*/
			 /* Note: this uses the very crude method for finding*
			 * if two points are 'close enough.'  The correspon-*
			 * ding coordinates of each point are subtracted and*
			 * abs or result is compared to MERROR              *
			 * A more precise way would be to use the Distance  *
			 * function provided, but that would be very slow   *
			 * if Distance is not used, value of ok is either   *
			 * 1.0 or 0.0                                       */

ok=true;dist=(T)0;
for (i = 1; i <= st1; i++)
{
	dist += abs(q[i]-q_calc[i]);
	if (abs(q[i]-q_calc[i])>=params.tol) ok=false;
}

if (ok)
{
	OutputResult(*params.pxifile,l,dist,q,p);
	if (params.plev>1)	OutputResult(*params.outfile,l,dist,q,p);
}

return (ok);
}

// GridSolve--call to start
template <class T> int GridSolveModule<T>::GridSolve(void)
{
int i,j;
if (!params.Ok()) {if (params.errfile) *params.errfile<<"Param Error\n";return 0;}

params.status<<"Grid Solve algorithm\n";
NormalIter<T> iter(nf);
int	st1=nf.NumStrats(1),st2=nf.NumStrats(2);
// Build a game matrix--this speeds things up enormously

for (i=1;i<=st1;i++)
	for (j=1;j<=st2;j++)
	{
		iter.Set(1,i);iter.Set(2,j);
		matrix(i,j).row=iter.Payoff(1);
		matrix(i,j).col=iter.Payoff(2);
	}

// Initialize the output file
gWatch timer;timer.Start();
OutputHeader(*params.pxifile);
if (params.plev>0) OutputHeader(*params.outfile);
// Create the ProbVector to give us all sets of probability values
ProbVect<T> *pv=new ProbVect<T>(st1,(int)((T)1.0/params.delp+(T)0.5));
int num_steps;
if (params.type==0)
	num_steps=(int)((params.maxLam-params.minLam)/params.delLam);
else
	num_steps=(int)(log(params.maxLam/params.minLam)/log(params.delLam+(T)1));
T l=params.minLam;
for (int step=1;step<num_steps && !params.status.Get();step++)
{
	if (params.type==0)  l=l+params.delLam; else l=l*(params.delLam+(T)1);
	while (!pv->Done()) if (pv->Inc())	CheckEqu(pv->GetP(),l);
	pv->Reset();
	params.status.SetProgress((double)step/(double)num_steps);
}
// Record the time taken and close the output file
*params.pxifile<<"Simulation took "<<timer.ElapsedStr()<<'\n';
params.status<<"Simulation took "<<timer.ElapsedStr()<<'\n';
delete pv;
return !params.status.Get();
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
class gRectArray<double>;
class gRectArray<gRational>;

#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class ProbVect<double>;
TEMPLATE class ProbVect<gRational>;

TEMPLATE class GridSolveModule<double>;
TEMPLATE class GridSolveModule<gRational>;

TEMPLATE class GridParams<double>;
TEMPLATE class GridParams<gRational>;

#include "grarray.imp"

TEMPLATE class PayoffClass<double>;
TEMPLATE class PayoffClass<gRational>;

TEMPLATE class gRectArray<PayoffClass<double> >;
TEMPLATE class gRectArray<PayoffClass<gRational> >;

gOutput &operator<<(gOutput &o, const PayoffClass<double> &p) {return o;}
gOutput &operator<<(gOutput &o, const PayoffClass<gRational> &p) {return o;}

