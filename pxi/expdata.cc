//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of experimental data storage and analysis class
//

#include <math.h>

#include "math/math.h"
#include "expdata.h"

const double ALMOST_INFINITY = 100000.0;
const double ALMOST_ZERO = 0.0000001;

//========================================================================
//       Static auxiliary functions for outputting likelihood data
//========================================================================

static void OutputLikeData(gOutput &p_file,
			   gBlock<double> &p_likes,
			   double p_lambda, 
			   double &p_likeMin, double &p_likeMax)
{
  for (int j = 1; j <= p_likes.Length(); j++) {
    p_file << p_likes[j] << ' ';
    p_likeMin = gmin(p_likeMin, p_likes[j]);
    p_likeMax = gmax(p_likeMax, p_likes[j]);
    p_likes[j] = ALMOST_INFINITY;
  }
  p_file << p_lambda <<'\n';
}

static void OutputLikeHeader(gOutput &p_file, int p_numPoints)
{
  p_file << "Dimensionality:\n" << 1 << ' ' << p_numPoints << '\n';
  p_file << "DataFormat:\n";
  p_file << (p_numPoints+1) << ' ' << (p_numPoints+1) << ' ' << 0 << ' ';
  for (int i = 1; i <= p_numPoints; i++)  {
    p_file << i <<' ';
  }
  p_file << '\n';
  p_file << "Data:\n";
}

//=======================================================================
//                  class ExpData: Member functions
//=======================================================================

//-----------------------------------------------------------------------
//                 Lifecycle and data reading/writing
//-----------------------------------------------------------------------

ExpData::ExpData(void)
{ }

bool ExpData::LoadData(gInput &p_file)
{
  try {
    int numInfosets, numPoints;
  
    p_file >> numInfosets;
    gBlock<int> strategies(numInfosets);
    for (int iset = 1; iset <= numInfosets; iset++) {
      p_file >> strategies[iset];
    }

    p_file >> numPoints;
    m_probs = gRectArray<gBlock<double> >(numPoints, numInfosets);
    
    for (int j = 1; j <= numPoints; j++) {
      for (int iset = 1; iset <= numInfosets; iset++) {
	m_probs(j, iset) = gBlock<double>(strategies[iset]);
      }
    }

    for (int j = 1; j <= numPoints; j++) {
      for (int iset = 1; iset <= numInfosets; iset++) {
	for (int i = 1; i <= strategies[iset]; i++) {
	  p_file >> (m_probs(j,iset))[i];
	}
      }
    }

    m_fitLambdas = gBlock<double>(numPoints);
    m_fitLikes = gBlock<double>(numPoints);

  }
  catch (...) {
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------
//                Fitting data to QRE correspondence
//-----------------------------------------------------------------------

void ExpData::ComputeMLEs(FileHeader &p_qreFile, gOutput &p_likeFile)
{
  double likeMax = -1000, likeMin = 1000;
  
  // Output the likelihood file header
  OutputLikeHeader(p_likeFile, m_probs.NumRows());
  
  gBlock<double> likes(NumPoints());
  for (int j = 1; j <= NumPoints(); j++) {
    likes[j] = ALMOST_INFINITY;
    m_fitLikes[j] = ALMOST_INFINITY;
  }

  for (int eq = 1; eq <= p_qreFile.GetData().Length(); eq++) {
    const DataLine &qre = *p_qreFile.GetData()[eq];

    for (int j = 1; j <= NumPoints(); j++) {
      // Calculate the (log) likelihood function
      double like = 0.0;
      for (int iset = 1; iset <= qre.NumInfosets(); iset++) {
	for (int i = 1; i <= qre.NumStrategies(iset); i++) {
	  like += m_probs(j, iset)[i] * log(gmax(qre[iset][i], ALMOST_ZERO));
	}
      }
      like = -like;	// taking -log of like.... Minimum is the best fit
      if (m_fitLikes[j] > like) {
	m_fitLikes[j] = like;
	m_fitLambdas[j] = qre.Lambda();
      }
      likes[j] = gmin(likes[j], like);
    }

    OutputLikeData(p_likeFile, likes, qre.Lambda(), likeMin, likeMax);
  }

  // Save likelihood settings (must do now since like_min/max were not available til now)
  p_likeFile << "Settings:\n";
  p_likeFile << p_qreFile.EStart() << '\n';
  p_likeFile << p_qreFile.EStop() << '\n';
  double e_step = (p_qreFile.DataType() == DATA_TYPE_ARITH) ? p_qreFile.EStep() : p_qreFile.EStep() - 1;
  p_likeFile << e_step << '\n';
  p_likeFile << likeMin << '\n';
  p_likeFile << likeMax << '\n';
  p_likeFile << p_qreFile.DataType() << '\n';

  // Normalize the probs (experimental data) to be used for plotting.  Is this right?!
  for (int j = 1; j <= NumPoints(); j++) {
    for (int iset = 1; iset <= m_probs.NumColumns(); iset++) {
      double sum = 0.0;
      for (int i = 1; i <= m_probs(j, iset).Length(); i++) {
	sum += m_probs(j, iset)[i];
      }
      for (int i = 1; i <= m_probs(j, iset).Length(); i++) {
	m_probs(j, iset)[i] /= sum;
      }
    }
  }
  m_solved = true;
}

gBlock<int> ExpData::FitPoints(double p_lambda) const
{
  const double DELTA_DOUBLE = 0.0001;

  gBlock<int> points;
  if (m_solved) {
    for (int i = 1; i <= NumPoints(); i++) {
      if (m_fitLambdas[i] > p_lambda - DELTA_DOUBLE &&
	  m_fitLambdas[i] < p_lambda + DELTA_DOUBLE) {
	points.Append(i);
      }
    }
  }
  return points;
}


