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
//                             Lifecycle
//-----------------------------------------------------------------------

ExpData::ExpData(void)
  : m_haveMLEs(false), m_numInfosets(0)
{ }

//-----------------------------------------------------------------------
//                   Reading and writing .agg files
//-----------------------------------------------------------------------

bool ExpData::LoadData(gInput &p_file)
{
  try {
    p_file >> m_numInfosets;
    m_numActions = gBlock<int>(m_numInfosets);
    for (int iset = 1; iset <= m_numInfosets; iset++) {
      p_file >> m_numActions[iset];
    }

    int numPoints;
    p_file >> numPoints;
    m_points = gRectBlock<gBlock<int> >(numPoints, m_numInfosets);
    
    for (int j = 1; j <= numPoints; j++) {
      for (int iset = 1; iset <= m_numInfosets; iset++) {
	m_points(j, iset) = gBlock<int>(m_numActions[iset]);
      }
    }

    for (int j = 1; j <= numPoints; j++) {
      for (int iset = 1; iset <= m_numInfosets; iset++) {
	for (int i = 1; i <= m_numActions[iset]; i++) {
	  p_file >> (m_points(j,iset))[i];
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

bool ExpData::SaveData(gOutput &p_file)
{
  try {
    p_file << m_numInfosets << ' ';
    for (int iset = 1; iset <= m_numInfosets; iset++) {
      p_file << m_numActions[iset] << ' ';
    }
    p_file << '\n';
    
    p_file << m_points.NumRows() << '\n';
    
    for (int i = 1; i <= m_points.NumRows(); i++) {
      for (int iset = 1; iset <= m_numInfosets; iset++) {
	for (int act = 1; act <= m_numActions[iset]; act++) {
	  p_file << m_points(i, iset)[act] << ' ';
	}
      }
      p_file << '\n';
    }
  }
  catch (...) {
    return false;
  }

  return true;
}

//-----------------------------------------------------------------------
//                        General data access
//-----------------------------------------------------------------------

int ExpData::NumActions(void) const
{
  int act = 0;
  for (int iset = 1; iset <= m_numInfosets; act += m_numActions[iset++]);
  return act;
}

double ExpData::GetDataProb(int p_point, int p_iset, int p_act) const
{
  int total = 0;
  for (int i = 1; i <= m_numActions[p_iset];
       total += m_points(p_point, p_iset)[i++]);
  return ((double) m_points(p_point, p_iset)[p_act] / (double) total);
}

//-----------------------------------------------------------------------
//                      Editing data points
//-----------------------------------------------------------------------

void ExpData::AddDataPoint(void)
{
  gArray<gBlock<int> > row(m_numInfosets);
  for (int iset = 1; iset <= m_numInfosets; iset++) {
    row[iset] = gBlock<int>(m_numActions[iset]);
  }
  m_points.AddRow(row);
}

void ExpData::RemoveDataPoint(int i)
{
  m_points.RemoveRow(i);
}

void ExpData::SetDataPoint(int i, int iset, int act, int value)
{
  m_points(i, iset)[act] = value;
  // set this flag after assigning, since bad indices will generate
  // an exception, and won't invalidate estimates.
  m_haveMLEs = false;
}

//-----------------------------------------------------------------------
//                Fitting data to QRE correspondence
//-----------------------------------------------------------------------

void ExpData::ComputeMLEs(FileHeader &p_qreFile, gOutput &p_likeFile)
{
  double likeMax = -1000, likeMin = 1000;
  
  // Output the likelihood file header
  OutputLikeHeader(p_likeFile, m_points.NumRows());
  
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
	  like += (((double) m_points(j, iset)[i]) *
		   log(gmax(qre[iset][i], ALMOST_ZERO)));
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

  m_haveMLEs = true;
}

gBlock<int> ExpData::FitPoints(double p_lambda) const
{
  const double DELTA_DOUBLE = 0.0001;

  gBlock<int> points;
  if (m_haveMLEs) {
    for (int i = 1; i <= NumPoints(); i++) {
      if (m_fitLambdas[i] > p_lambda - DELTA_DOUBLE &&
	  m_fitLambdas[i] < p_lambda + DELTA_DOUBLE) {
	points.Append(i);
      }
    }
  }
  return points;
}

