//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Interface to class to store data from a PXI file
//

#ifndef PXIFILE_H
#define PXIFILE_H

#include "base/base.h"

class DataLine {
private:
  double m_lambda, m_delta;
  gBlock<gBlock<double> > m_data;

public:
  DataLine(void) { }
  DataLine(double p_lambda, double p_delta,
	   const gBlock<gBlock<double> > &p_data)
    : m_lambda(p_lambda), m_delta(p_delta), m_data(p_data) { }

  bool operator==(const DataLine &) const { return false; }
  bool operator!=(const DataLine &) const { return true; }

  // Return probs for i'th infoset
  const gArray<double> &operator[](int iset) const { return m_data[iset]; }
  gArray<double> &operator[](int iset) { return m_data[iset]; }

  int NumInfosets(void) const { return m_data.Length(); }
  int NumStrategies(int iset) const { return m_data[iset].Length(); }

  double Lambda(void) const { return m_lambda; }
  double Delta(void) const { return m_delta; }
};

inline gOutput &operator<<(gOutput &p_file, const DataLine &)
{ return p_file; }

const int DATA_TYPE_ARITH = 0;
const int DATA_TYPE_LOG = 1;

class FileHeader {
friend gOutput &operator<<(gOutput &op,const FileHeader &f);
private:
  int num_infosets;
  gBlock<int> strategies;
  double e_step,e_start,e_stop;
  double data_max,data_min;
  int data_type;
  double merror,q_step;	// optional
  int num_columns;
  int e_column,delta_column;
  gBlock< gBlock<int> > prob_cols;            // [iset][strategy]
  //  NormalMatrix *matrix;	// optional
  gList<DataLine *> m_data;

public:
  FileHeader(void);
  ~FileHeader();

  void ReadFile(gInput &in);

  // Comparison operators
  int operator==(const FileHeader &H)
    {return (strategies==H.strategies && num_infosets==H.num_infosets);}
  int operator!=(const FileHeader &H)
    {return !(*this==H);}
  // Data access functions
  int NumColumns(void) const {return num_columns;}
  int NumInfosets(void) const {return num_infosets;}
  int NumStrategies(int i) const {return strategies[i];}
  gBlock<int> NumStrategies(void) const {return strategies;}
  int Col(int iset,int st) const {return prob_cols[iset][st];}
  int ECol(void) const {return e_column;}
  int DeltaCol(void) const {return delta_column;}
  int DataType(void) const {return data_type;}
  double MError(void) const {return merror;}
  double QStep(void) const {return q_step;}
  // Note that e_step is stored as e_step-1 if log plots are used. i.e. step of .1 is 1.1
  double EStep(void) const {return (data_type) ? 1+e_step : e_step;}
  double EStart(void) const {return e_start;}
  double EStop(void) const {return e_stop;}
  double DataMax(void) const {return data_max;}
  double DataMin(void) const {return data_min;}

  const gList<DataLine *> &GetData(void) const { return m_data; }
};

#endif  // PXIFILE_H
