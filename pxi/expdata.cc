#include "gmisc.h"
#include "expdata.h"
#include <math.h>

#define ALMOST_INFINITY  100000
#define	ALMOST_ZERO      0.0000001

gOutput &operator<<(gOutput &op,const ExpData::BEST_POINT &e) {op<<e.like;return op;}

void ExpData::OutputLikeData(gBlock<double> &like_m,double cur_e,double &like_min,double &like_max)
{
  for (int j=1;j<=num_points;j++) {
    *P.likefile<<like_m[j]<< ' ';
    like_min=gmin(like_min,like_m[j]);
    like_max=gmax(like_max,like_m[j]);
    like_m[j]=ALMOST_INFINITY;
  }
  *P.likefile<<cur_e<<'\n';
}

void ExpData::OutputLikeHeader(void)
{
  *P.likefile<<"Dimensionality:\n"<<1<<' '<<num_points<<'\n';
  *P.likefile<<"DataFormat:\n";
  *P.likefile<<(num_points+1)<<' '<<(num_points+1)<<' '<<0<<' ';
  for (int i=1;i<=num_points;i++) *P.likefile<<i<<' ';
  *P.likefile<<'\n';
  *P.likefile<<"Data:\n";
}

void ExpData::Go(void)		// Fit to the calculated data file
{
  int ok;
  int i,j,iset;
  double  like;
  // stuff to create a likelihood file
  double like_max=-1000,like_min=1000;
  
  DataLine calc_probs(*P.pxifile);
  ok=FindStringInFile(*P.pxifile,"Data:");assert(ok);
  *P.pxifile>>calc_probs;
  
  // Output the likelihood file header
  if (P.likefile) OutputLikeHeader();
  
  gBlock<double> like_m(num_points);
  for (j=1;j<=num_points;j++) {
    like_m[j]=ALMOST_INFINITY;
    points[j].like=ALMOST_INFINITY;
  }
  double cur_e=calc_probs.E();
  
  while (!calc_probs.Done() && P.pxifile->IsValid()) { // read til the end of the file
    for (j=1;j<=num_points;j++) {
      // Calculate the likelihood function
      like=0.0;
      for (iset=1;iset<=calc_probs.NumInfosets();iset++)
	for (i=1;i<=calc_probs.NumStrategies(iset);i++)
	  like+=probs[j][iset][i]*log(gmax(calc_probs[iset][i],ALMOST_ZERO));
      like=-like;	// taking -log of like.... Minimum is the best fit
      if (points[j].like>like) {points[j].like=like;points[j].e=calc_probs.E();}
      if (P.likefile) like_m[j]=gmin(like_m[j],like);
    }
    *P.pxifile>>calc_probs;
    if (calc_probs.E()!=cur_e && P.likefile) {	// new E read.  Write out like's
      OutputLikeData(like_m,cur_e,like_min,like_max);
      cur_e=calc_probs.E();
    }
    
  }
  // Save likelihood settings (must do now since like_min/max were not available til now)
  if (P.likefile) {
    *P.likefile<<"Settings:\n";
    *P.likefile<<calc_probs.Header().EStart()<<'\n'<<calc_probs.Header().EStop()<<'\n';
    double e_step=(calc_probs.Header().DataType()==DATA_TYPE_ARITH) ? calc_probs.Header().EStep() : calc_probs.Header().EStep()-1;
    *P.likefile<<e_step<<'\n';
    *P.likefile<<like_min<<'\n'<<like_max<<'\n';
    *P.likefile<<calc_probs.Header().DataType()<<'\n';
  }
  solved=TRUE;
}

#define	DELTA_DOUBLE	0.0001
gBlock<int> ExpData::HaveL(double _e)
{
  gBlock<int> have;
  if (solved)
    for (int i=1;i<=num_points;i++)
      if (points[i].e>_e-DELTA_DOUBLE && points[i].e<_e+DELTA_DOUBLE)
	have.Append(i);
  return have;
}

// Constructor
ExpData::ExpData(ExpDataParams &params):P(params)
{
  int i,j,iset,num_infosets;
  
  *P.expfile>>num_infosets;
  gBlock<int> strategies(num_infosets);
  for (iset=1;iset<=num_infosets;iset++)	*P.expfile>>strategies[iset];
  
  *P.expfile>>num_points;
  probs=gMatrix1<PointNd>(num_points,num_infosets);
  
  for (j=1;j<=num_points;j++)
    for (iset=1;iset<=num_infosets;iset++)
      probs[j][iset]=PointNd(strategies[iset]);
  
  points=gBlock<best_point_struct>(num_points);
  
  for (j=1;j<=num_points;j++)
    for (iset=1;iset<=num_infosets;iset++)
      for (i=1;i<=strategies[iset];i++)
	*P.expfile>>(probs[j][iset])[i];
  // Start the actual Calculation
  Go();
  // Normalize the probs (experimental data) to be used for plotting.  Is this right?!
  double sum;
  for (j=1;j<=num_points;j++)
    for (iset=1;iset<=num_infosets;iset++) {
      sum=0;
      for (i=1;i<=strategies[iset];i++)
	sum+=probs[j][iset][i];
      for (i=1;i<=strategies[iset];i++)
	probs[j][iset][i]/=sum;
    }
}
