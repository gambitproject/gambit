//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// The class will read in a file with experimental data, in the following
// format:
// 	#_of_players  #_of_strategies_for_player_1 #_of_strategies_for_player_2
// 	#_of_data points
// 	data....
//  Same format as the matrix file in pxi:
// and using the file with data calculated by PXI, attempt to fit the data
// points to those in the calculated file.  The result will be the p[1..dim],
// q[1..dim] and the lambda value that corresponds to the data points in the
// calculated file that are closest to those in the experimental data file.
// To use the class, initialize it with the properly filled out ExpDataParams.
// The resulting data is stored in exp_data_struct structure which contains
// the experimental p,q and the lambda value it fitted to.  Access that data
// with the overloaded [] operator which will return the fitted points up to
// the number of points defined in the experimental data file

// The algorithm determines the best likelihood by
// like=Sum[num_obs*Log[like_calc]] and using the minimum value of like.
// The module can also save the likelihood information for each value of
// lambda processed.  Note that this feature does not save like for each lambda
// processed, but only for each UNIQUE value of lambda.  If the calcfile
// contains more than one set of probs for the same lambda, only the max values
// of like are saved for each data point for this lambda.
//

#ifndef	EXPDATA_H
#define	EXPDATA_H

#include <math.h>
#include <stdio.h>
#include "base/gstream.h"
#include "base/grarray.h"
#include "pxifile.h"

gOutput &operator<<(gOutput &op,const gBlock<double> &d);

class ExpData {
private:
  bool m_solved;
  gBlock<double> m_fitLambdas, m_fitLikes;
  gRectArray<gBlock<double> > m_probs;

public:
  ExpData(void);

  int NumPoints(void) const { return m_probs.NumRows(); }
  gBlock<int> FitPoints(double p_lambda) const;
  
  double MLELambda(int i) const { return m_fitLambdas[i]; }
  double MLEValue(int i) const { return m_fitLikes[i]; } 
  double GetDataPoint(int i, int iset, int act) const
    { return m_probs(i, iset)[act]; }

  bool HaveMLEs(void) const { return m_solved; }
  void ComputeMLEs(FileHeader &, gOutput &p_likeFile);

  bool LoadData(gInput &);
};

#endif  // EXPDATA_H


