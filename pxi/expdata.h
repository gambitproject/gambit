// File: expdata.h, header file for expdata.cc
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


#ifndef	EXPDATA_H
#define	EXPDATA_H
#include <math.h>
#include <stdio.h>
#include "general.h"
#include "gambitio.h"
#include "gblockm.h"
#include "equdata.h"


typedef struct EXPDATASTRUCT
{
  gBlock<PointNd> probs;
  double e;
  EXPDATASTRUCT(const gBlock<PointNd> &_probs,double _e)
    {probs=_probs;e=_e;}
  EXPDATASTRUCT(void) {;}
  EXPDATASTRUCT(const EXPDATASTRUCT &A):probs(A.probs),e(A.e)	{;}
  ~EXPDATASTRUCT(void) {;}
  int operator==(const EXPDATASTRUCT &A) {return (e==A.e && probs==A.probs);}
  int operator!=(const EXPDATASTRUCT &A) {return !(*this==A);}
  EXPDATASTRUCT &operator=(const EXPDATASTRUCT &A)
    {probs=A.probs;e=A.e; return (*this);}
} exp_data_struct;

gOutput &operator<<(gOutput &op,const EXPDATASTRUCT &e);
gOutput &operator<<(gOutput &op,const gBlock<double> &d);

class ExpDataParams
{
public:
  gOutput *likefile;
  gInput *pxifile,*expfile;
  ExpDataParams(void) : likefile(0),pxifile(0),expfile(0) {}
};

class ExpData
{
public:
  typedef struct BEST_POINT {
    friend gOutput &operator<<(gOutput &op,const BEST_POINT &e);
    double e;
    double like;
    BEST_POINT(void):e(-1.0),like(-1.0) {;}
    BEST_POINT(const BEST_POINT &P):e(P.e),like(P.like) {;}
    BEST_POINT &operator=(const BEST_POINT &p)
      {e=p.e;like=p.like;return (*this);}
    int operator==(const BEST_POINT &p) {return (e==p.e && like==p.like);}
    int operator!=(const BEST_POINT &p) {return !(*this==p);}
  } best_point_struct;
private:
  // variables
  Bool	solved;
  int num_points;
  gBlock<best_point_struct> points;
  gMatrix1<PointNd> probs;
  ExpDataParams &P;
  // functions
  void Go(void);
  void OutputLikeHeader(void);
  void OutputLikeData(gBlock<double> &like_m,double cur_e,double &like_min,double &like_max);
public:
  // Constructor, Calculate the closes values of lambda based on data in file_name
  ExpData(ExpDataParams &P);
  // Destructor
  ~ExpData(void) { ;}
  // Data access functions
  int NumPoints(void) {return num_points;}
  gBlock<int> HaveL(double l);
  // Accessing the calculated results
  exp_data_struct *operator[](int i) const
    {
      assert(i>0 && i<=num_points);
      assert(solved);
      return (new exp_data_struct(probs[i],points[i].e));
    }
};
#endif
