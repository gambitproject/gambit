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
#include "base/grblock.h"
#include "pxifile.h"

class ExpData {
private:
  bool m_haveMLEs;
  int m_numInfosets;
  gBlock<int> m_numActions;
  gBlock<double> m_fitLambdas, m_fitLikes;
  gRectBlock<gBlock<int> > m_points;

public:
  // LIFECYCLE
  ExpData(void);

  // READING AND WRITING .AGG FILES
  bool LoadData(gInput &);
  bool SaveData(gOutput &);

  // GENERAL DATA ACCESS
  int NumPoints(void) const { return m_points.NumRows(); }
  int NumInfosets(void) const { return m_points.NumColumns(); }
  int NumActions(int iset) const { return m_points(1, iset).Length(); }
  int NumActions(void) const;

  int GetDataPoint(int i, int iset, int act) const
    { return m_points(i, iset)[act]; }

  double GetDataProb(int i, int iset, int act) const;

  // EDITING DATA POINTS
  void AddDataPoint(void);
  void RemoveDataPoint(int i);

  void SetDataPoint(int i, int iset, int act, int value);

  // COMPUTING ESTIMATES
  void ComputeMLEs(FileHeader &, gOutput &p_likeFile);

  // ACCESSING ESTIMATES
  bool HaveMLEs(void) const { return m_haveMLEs; }

  gBlock<int> FitPoints(double p_lambda) const;

  double MLELambda(int i) const { return m_fitLambdas[i]; }
  double MLEValue(int i) const { return m_fitLikes[i]; } 

};

#endif  // EXPDATA_H


