#include <assert.h>
#include "probvect.h"
#define ALMOST_ZERO	0.000001

ProbVect::ProbVect(int _dim,int _l_steps)
{
dim=_dim-1;l_steps=_l_steps;dl=1.0/l_steps;
p=new double[dim+1];order=new long[dim+1];
assert(p!=NULL);assert(order!=NULL);
max_count=(long)(pow((double)(l_steps+1),(double)dim)+PV_FLT);
for (i=0;i<=dim;i++) order[i]=(long)(pow((double)(l_steps+1),(double)i)+PV_FLT);
count=0;
}

double ProbVect::Sum(void)
{
double sum=0.0;
for(i=1;i<=dim;i++) sum+=p[i];
return sum;
}

Bool ProbVect::Inc(void)
{
	count++;
	for (j=1;j<=dim;j++)
		p[j]=(((count-1)/order[j-1])%order[1])*dl;
	return ((p[0]=1.00-Sum())>=-.000001);
}
