//***************************************************************************
//* File: probvect.h.  Header file for probvect.c.
//* The ProbVect class is based in part on the gVector class.  ProbVect is
//* designed to allow the user to solve problems pertaining to normal form
//* games using the grid search method.  The class is initialized with the
//* size of the normal form matrix (NxN) and the number of steps desired per
//* each strategie's probability (delta p).  The class then steps through all
//* values of p[1..n-1].  To go to the next set of values, call the Inc()
//* function.  This function will return TRUE if this set is valid, i.e. if
//* the sum adds up to 1.0 or FALSE if the sum is greater than 1.0 or less
//* than 0.  Each value of p can be accessed by 'indexing' into the class--
//* the [] operator is overloaded.  The user MUST test all the values have
//* been processed by calling the Done() function.  Perhaps a more efficient
//* algorithm will be developed in the future.

#include <math.h>
#include <assert.h>
#include <stdio.h>

#ifndef BOOL_DEFINED
	typedef int Bool;
#endif
class ProbVect
{
#define PV_FLT	0.4
private:
	double *p;
	long *order;
	double dl;
	int		dim,l_steps;
	long	max_count,count;
	int		j,i;

	double Sum(void);
public:
	ProbVect(int _dim,int _l_steps);
	~ProbVect(void) {assert(p!=NULL);assert(order!=NULL);delete [] p; delete [] order;}

	Bool Inc(void);

	void Reset(void) {count=0;}

	Bool Done(void) {return count>=max_count;}

	double operator[](int index) const {assert(index<=dim);return p[index];}
	double *GetP(void) {assert(p!=NULL);return p;}

};
