//#
//# FILE: grid.cc -- Grid-search solution module
//#
//# @(#)grid.cc	1.8 2/7/95
//#

#ifdef __GNUG__
#pragma implementation "grid.h"
#endif   // __GNUG__

#include "basic.h"
#include "rational.h"
#include "gmatrix.h"
#include "normal.h"
#include "normiter.h"
#include "probvect.h"
#include "gwatch.h"
#include "grid.h"


template <class T>
GridParams<T>::GridParams(void) :
	plev(0),outfile(0),errfile(0),pxifile(0),update_func(0)
{ }
template <class T>
GridParams<T>::GridParams(const GridParams<T> &p) :
	plev(p.plev),outfile(p.outfile),errfile(p.errfile),
	pxifile(p.pxifile),update_func(p.update_func),
	minLam(p.minLam),maxLam(p.maxLam),delLam(p.delLam),
	delp(p.delp),tol(p.tol),type(p.type)

{ }

template <class T>
GridParams<T>::~GridParams(void)
{}

template <class T>
int GridParams<T>::Ok(void) const
{
if (!pxifile) return 0;
if (type==0 && delLam<=(T)1.0) return 0;
return 1;
}
/*
template <class T>
GridSolveModule<T>::GridSolveModule(const NormalForm<T> &r, gInput &param)
	: nf(r), x(r.NumStrats(1)), p(r.NumStrats(1)),y(r.NumStrats(2)),
		q_calc(r.NumStrats(2)),matrix(r.NumStrats(1),r.NumStrats(2))
{assert(0);	// this does not work
}
*/
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
void GridSolveModule<T>::OutputHeader(void)
{
int st1=nf.NumStrats(1),st2=nf.NumStrats(2);
*params.pxifile<<2<<' '<<nf.NumStrats(1)<<' '<<nf.NumStrats(2)<<'\n';
*params.pxifile<<"3 2 1\n";
for (int i=1;i<=st1;i++)
{
	for (int j=1;j<=st2;j++)
		*params.pxifile<<matrix(i,j).row<<' '<<matrix(i,j).col<<' ';
	*params.pxifile<<'\n';
}
*params.pxifile<<"Settings:\n";
*params.pxifile<<params.tol<<'\n'<<params.delp<<'\n'<<params.delLam<<'\n';
*params.pxifile<<params.minLam<<'\n'<<params.maxLam<<'\n'<<params.type<<'\n';
int num_columns=st1+st2+2;
*params.pxifile<<num_columns<<' '<<(num_columns-1)<<' '<<num_columns<<' ';
for (i=1;i<=st1;i++) *params.pxifile<<i<<' ';
for (i=1;i<=st2;i++) *params.pxifile<<(st1+i)<<' ';
*params.pxifile<<'\n';
*params.pxifile<<"Data:\n";
}

template <class T>
void GridSolveModule<T>::OutputResult(T l, T dist,gVector<T> &q,gVector<T> &p)
{
	int i;
	for (i = p.First(); i <= p.Last(); i++)  *params.pxifile << p[i] << ' ';
	for (i = q.First(); i <= q.Last(); i++)  *params.pxifile << q[i] << ' ';
	*params.pxifile << l << ' ' << dist << '\n';
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
	for (j=1;j<=st1;j++) y[i] += matrix(i,j).col*p[j];
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
	dist += fabs((double)(q[i]-q_calc[i]));
	if ((T)fabs(q[i]-q_calc[i])>=params.tol) ok=false;
}

if (ok) OutputResult(l,dist,q,p);

return (ok);
}

// GridSolve--call to start
template <class T> int GridSolveModule<T>::GridSolve(void)
{
int i,j;
if (!params.Ok()) {if (params.errfile) *params.errfile<<"Param Error\n";return 0;}

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
OutputHeader();
// Create the ProbVector to give us all sets of probability values
ProbVect<T> *pv=new ProbVect<T>(st1,(int)((T)1.0/params.delp+(T)0.5));
int num_steps;
if (params.type)
	num_steps=(int)((params.maxLam-params.minLam)/params.delLam);
else
	num_steps=(int)(log(params.maxLam/params.minLam)/log(params.delLam));
T l=params.minLam;
for (int step=1;step<num_steps;step++)
{
	if (params.type) l=l+params.delLam; else l=l*params.delLam;
	while (!pv->Done()) if (pv->Inc())	CheckEqu(pv->GetP(),l);
	pv->Reset();
	if (params.update_func) (*params.update_func)(step/num_steps);
}
// Record the time taken and close the output file
*params.pxifile<<"Simulation took "<<timer.ElapsedStr()<<'\n';
delete pv;
return 1;
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class ProbVect<double>;
TEMPLATE class ProbVect<gRational>;

TEMPLATE class GridSolveModule<double>;
TEMPLATE class GridSolveModule<gRational>;

TEMPLATE class GridParams<double>;
TEMPLATE class GridParams<gRational>;


