#include "gmisc.h"
#include "expdata.h"
#include <math.h>

#define ALMOST_INFINITY  100000
#define	ALMOST_ZERO      0.0000001

ExpData::ExpData(void)
{ }

gOutput &operator<<(gOutput &op,const ExpData::BEST_POINT &e) {op<<e.like;return op;}

void ExpData::OutputLikeData(gOutput &p_file,
			     gBlock<double> &like_m,
			     double cur_e,double &like_min,double &like_max)
{
  for (int j=1;j<=num_points;j++) {
    p_file<<like_m[j]<< ' ';
    like_min=gmin(like_min,like_m[j]);
    like_max=gmax(like_max,like_m[j]);
    like_m[j]=ALMOST_INFINITY;
  }
  p_file<<cur_e<<'\n';
}

void ExpData::OutputLikeHeader(gOutput &p_file)
{
  p_file << "Dimensionality:\n" << 1 << ' ' << num_points << '\n';
  p_file << "DataFormat:\n";
  p_file << (num_points+1) << ' ' << (num_points+1) << ' ' << 0 << ' ';
  for (int i = 1; i <= num_points; i++)  {
    p_file << i <<' ';
  }
  p_file << '\n';
  p_file << "Data:\n";
}

// Fit to the calculated data file
void ExpData::ComputeMLEs(FileHeader &p_qreFile, gOutput &p_likeFile)
{
  int ok;
  int i,j,iset;
  double  like;
  // stuff to create a likelihood file
  double like_max=-1000,like_min=1000;
  
  // Output the likelihood file header
  OutputLikeHeader(p_likeFile);
  
  gBlock<double> like_m(num_points);
  for (j=1;j<=num_points;j++) {
    like_m[j]=ALMOST_INFINITY;
    points[j].like=ALMOST_INFINITY;
  }

  for (int eq = 1; eq <= p_qreFile.GetData().Length(); eq++) {
    const DataLine &qre = *p_qreFile.GetData()[eq];
    double cur_e = qre.Lambda();
  
    for (j=1;j<=num_points;j++) {
      // Calculate the likelihood function
      like=0.0;
      for (int iset = 1; iset <= qre.NumInfosets(); iset++) {
	for (int i = 1; i <= qre.NumStrategies(iset); i++) {
	  like += probs(j,iset)[i]*log(gmax(qre[iset][i],ALMOST_ZERO));
	}
      }
      like = -like;	// taking -log of like.... Minimum is the best fit
      if (points[j].like > like) {
	points[j].like = like;
	points[j].e = qre.Lambda();
      }
      like_m[j] = gmin(like_m[j],like);
    }

    OutputLikeData(p_likeFile, like_m,cur_e,like_min,like_max);
  }

  // Save likelihood settings (must do now since like_min/max were not available til now)
  p_likeFile << "Settings:\n";
  p_likeFile << p_qreFile.EStart() << '\n';
  p_likeFile << p_qreFile.EStop() << '\n';
  double e_step = (p_qreFile.DataType() == DATA_TYPE_ARITH) ? p_qreFile.EStep() : p_qreFile.EStep() - 1;
  p_likeFile << e_step << '\n';
  p_likeFile << like_min << '\n';
  p_likeFile << like_max << '\n';
  p_likeFile << p_qreFile.DataType() << '\n';

  // Normalize the probs (experimental data) to be used for plotting.  Is this right?!
  for (int j = 1; j <= num_points; j++) {
    for (int iset = 1; iset <= probs.NumColumns(); iset++) {
      double sum = 0;
      for (int i = 1; i <= probs(1, iset).Length(); i++) {
	sum += probs(j,iset)[i];
      }
      for (int i = 1; i <= probs(1, iset).Length(); i++) {
	probs(j,iset)[i] /= sum;
      }
    }
  }
  m_solved = true;
}

#define	DELTA_DOUBLE	0.0001
gBlock<int> ExpData::HaveL(double _e) const
{
  gBlock<int> have;
  if (m_solved)
    for (int i=1;i<=num_points;i++)
      if (points[i].e>_e-DELTA_DOUBLE && points[i].e<_e+DELTA_DOUBLE)
	have.Append(i);
  return have;
}

bool ExpData::LoadData(gInput &p_file)
{
  try {
    int num_infosets;
  
    p_file >> num_infosets;
    gBlock<int> strategies(num_infosets);
    for (int iset = 1; iset <= num_infosets; iset++) {
      p_file >> strategies[iset];
    }

    p_file >> num_points;
    probs = gRectArray<gBlock<double> >(num_points,num_infosets);
  
    for (int j = 1; j <= num_points; j++) {
      for (int iset = 1; iset <= num_infosets; iset++) {
	probs(j, iset) = gBlock<double>(strategies[iset]);
      }
    }

    points = gBlock<best_point_struct>(num_points);
    
    for (int j = 1; j <= num_points; j++) {
      for (int iset = 1; iset <= num_infosets; iset++) {
	for (int i = 1; i <= strategies[iset]; i++) {
	  p_file >> (probs(j,iset))[i];
	}
      }
    }
  }
  catch (...) {
    return false;
  }

  return true;
}

