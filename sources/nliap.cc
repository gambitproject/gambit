//#
//# FILE: liap.cc -- Liapunov module
//#
//# $Id$
//#

#include "mixed.h"
#include "normal.h"
#include "gfunct.h"


class gLiapFunct : public gBFunct_nDim {
private:
	NormalForm *nf;
public:
	gLiapFunct(NormalForm &N)	: gBFunct_nDim(N.ProfileLength()) , nf(&N)	{ }
	double operator()(const dVector &x) const {return nf->LiapValue(x);}
};


int NormalForm::Liap(int number)
{
	MixedProfile p(Centroid());
	Liap(p);

/*	for(int i=1;i<number;i++)
		{
		p.Randomize();
		Liap(p);
		}
*/
return 1;
};

int NormalForm::Liap(MixedProfile &p)
{
	gLiapFunct f((*this));

// Ted -- we need a constructor in gvector to construct v from p,
// so we can replace all of this with dVector v(p);
// I wasn't sure where it would belong, since we don't want to include
// mixed.h in gvector.h

	dVector v(ProfileLength());
	int k=1;

	for(int i=1;i<=NumPlayers();i++)
		{
		for(int j=1;j<=NumStrats(i);j++)
			v[k]=p[i][j];
		k++;
		}
	f.MinPowell(v);
	gout << "\nv= " << v;
  return 1;
};

double NormalForm::LiapValue(const dVector &x) const
{
	assert(x.Length()==ProfileLength());

	MixedProfile m((*this));

	int k=x.First();
	for(int i=1;i<=NumPlayers();i++)
		{
		for(int j=1;j<=NumStrats(i);j++)
			m[i][j]=x[k];
		k++;
		}
	return LiapValue(m);

};

double NormalForm::LiapValue(const MixedProfile &p) const
{
	int i,j,num;
	MixedProfile tmp(p);
	gVector<double> payoff;
	double x,result,avg,sum;

	payoff.SetFirst(1);
	result=0;
	for(i=1;i<=NumPlayers();i++)
		{
		payoff.SetLast(NumStrats(i));
		payoff=0;
		tmp[i]=0;
		avg=sum=0;
				// then for each strategy for that player set it to 1 and evaluate
		for(j = 1; j <= payoff.Length(); j++)
			{
			tmp[i][j]=1;
			x=p[i][j];
			payoff[j] = (*this)(i,tmp);
			avg+=x*payoff[j];
			sum+=x;
			x= (x > 0 ? 0 : x);
			result += x*x;         // add penalty for neg probabilities
			tmp[i][j]=0;
			}
		tmp[i]=p[i];
		avg/=payoff.Length();
		for(j=1;j<=payoff.Length();j++)
			{
			x=payoff[j]-avg;
			x = (x > 0 ? x : 0);
			result += x*x;          // add penalty if not best response
			}
		x=sum-1;
		result += x*x ;          // add penalty for sum not equal to 1
		}

	return result;
}

