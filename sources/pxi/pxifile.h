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
  gbtBlock<gbtBlock<double> > m_data;

public:
  DataLine(void) { }
  DataLine(double p_lambda, double p_delta,
	   const gbtBlock<gbtBlock<double> > &p_data)
    : m_lambda(p_lambda), m_delta(p_delta), m_data(p_data) { }

  bool operator==(const DataLine &) const { return false; }
  bool operator!=(const DataLine &) const { return true; }

  // Return probs for i'th infoset
  const gbtArray<double> &operator[](int iset) const { return m_data[iset]; }
  gbtArray<double> &operator[](int iset) { return m_data[iset]; }

  int NumInfosets(void) const { return m_data.Length(); }
  int NumStrategies(int iset) const { return m_data[iset].Length(); }

  double Lambda(void) const { return m_lambda; }
  double Delta(void) const { return m_delta; }
};

inline gbtOutput &operator<<(gbtOutput &p_file, const DataLine &)
{ return p_file; }

class PxiFile {
private:
  gbtBlock<int> m_numStrats;
  double m_minLambda, m_maxLambda, m_delLambda;
  int m_powLambda;
  double m_error, m_qStep;
  int m_numColumns, m_lambdaColumn, m_deltaColumn;
  bool m_shown;
  
  gbtBlock< gbtBlock<int> > m_columns;
  gbtList<DataLine *> m_data;

public:
  PxiFile(void);
  ~PxiFile();

  void ReadFile(gbtInput &);
  void WriteFile(gbtOutput &) const;

  // DATA ACCESS: GAME INFORMATION
  int NumInfosets(void) const { return m_numStrats.Length(); }
  int NumStrategies(int i) const { return m_numStrats[i]; }
  const gbtArray<int> &NumStrategies(void) const { return m_numStrats; }

  // DATA ACCESS: FILE STRUCTURE INFORMATION
  int NumColumns(void) const { return m_numColumns; }
  int Column(int iset, int st) const { return m_columns[iset][st];}
  int LambdaColumn(void) const { return m_lambdaColumn; }
  int DeltaColumn(void) const { return m_deltaColumn; }

  // DATA ACCESS: ALGORITHM INFORMATION
  double MinLambda(void) const { return m_minLambda; }
  double MaxLambda(void) const { return m_maxLambda; }
  // Note that m_delLambda is stored as m_delLambda-1 if log plots are used.
  // i.e. step of .1 is 1.1
  double DelLambda(void) const 
    { return (m_powLambda) ? 1.0 + m_delLambda : m_delLambda; }
  int PowLambda(void) const { return m_powLambda; }

  double MError(void) const { return m_error; }
  double QStep(void) const { return m_qStep; }

  bool IsShown(void) const { return m_shown; }
  void Show(bool p_shown) { m_shown = p_shown; }

  const gbtList<DataLine *> &GetData(void) const { return m_data; }
};

#endif  // PXIFILE_H
