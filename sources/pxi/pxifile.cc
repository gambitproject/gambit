//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of class for storing of PXI file data
//

#include "pxifile.h"

int FindStringInFile(gInput &, const char *);

//=========================================================================
//                            class PxiFile 
//=========================================================================

PxiFile::PxiFile(void)
{ }

PxiFile::~PxiFile()
{
  for (int i = 1; i <= m_data.Length(); i++) {
    delete m_data[i];
  }
}

//-------------------------------------------------------------------------
//                  PxiFile: Reading and writing files
//-------------------------------------------------------------------------

void PxiFile::ReadFile(gInput &in)
{
  m_error = -1;
  m_qStep = -1;

  m_minLambda = 1000000000.0;
  m_maxLambda = -1.0;

  // Get dimensionality
  if (!FindStringInFile(in,"Dimensionality:")) {
    return;
  }

  int numInfosets;
  in >> numInfosets;
  m_numStrats = gBlock<int>(numInfosets);
  for (int iset = 1; iset <= numInfosets; iset++)  {
    in >> m_numStrats[iset];
  }

  if (!FindStringInFile(in,"Settings:")) {
    return;
  }

  double dataMin, dataMax, eStart, eStop;
  in >> eStart >> eStop >> m_delLambda >> dataMin >> dataMax >> m_powLambda;
  // Get the data format

  if (!FindStringInFile(in,"DataFormat:")) {
    return;
  }

  in >> m_numColumns;
  in >> m_lambdaColumn >> m_deltaColumn;
  m_columns = gBlock< gBlock<int> >(numInfosets);
  for (int iset = 1; iset <= numInfosets; iset++) {
    m_columns[iset] = gBlock<int>(m_numStrats[iset]);
  }

  for (int iset = 1; iset <= numInfosets; iset++) {
    for (int st = 1; st <= m_numStrats[iset]; st++) {
      in >> m_columns[iset][st];
    }
  }
  // Get the optional data
  // Get the game matrix if available

  if (FindStringInFile(in,"Extra:")) {
    int ok;
    in >> ok;	// Check ReadExtra
    if (ok) {
      in >> m_error >> m_qStep;
    }
  }

  if (!FindStringInFile(in, "Data:")) {
    return;
  }

  gBlock<gBlock<double> > probs(NumInfosets());
  for (int iset = 1; iset <= NumInfosets(); iset++) {
    probs[iset] = gBlock<double>(NumStrategies(iset));
  }

  while (!in.eof()) {
    gBlock<double> data(NumColumns());
    try {
      for (int i = 1; i <= NumColumns(); i++)  {
	in >> data[i];
      }
    }
    catch (...) {
      return;
    }

    DataLine *line = new DataLine(data[LambdaColumn()],
				  (DeltaColumn()) ? data[DeltaColumn()] : 0.0,
				  probs);
    for (int iset = 1; iset <= NumInfosets(); iset++) {
      double sum = 0.0;
      for (int i = 1; i <= NumStrategies(iset); i++) {
	if (Column(iset, i)) {
	  (*line)[iset][i] = data[Column(iset, i)];
	  sum += (*line)[iset][i];
	}
	else {
	  (*line)[iset][i] = 1.0 - sum;
	}
      }
    }

    m_data.Append(line);

    if (data[LambdaColumn()] < m_minLambda) {
      m_minLambda = data[LambdaColumn()];
    }
    if (data[LambdaColumn()] > m_maxLambda) {
      m_maxLambda = data[LambdaColumn()];
    }
  }
}

void PxiFile::WriteFile(gOutput &p_file) const
{
  p_file << "Dimensionality:\n" << NumInfosets() << ' ';
  for (int i = 1; i <= NumInfosets(); i++) {
    p_file << NumStrategies(i) <<' ';
  }

  p_file << '\n';

  p_file << "Settings:\n";
  p_file << MinLambda() << '\n';
  p_file << MaxLambda() << '\n';
  p_file << DelLambda() << '\n';
  p_file << MinLambda() << '\n';
  p_file << MaxLambda() << '\n';
  p_file << PowLambda() <<'\n';

  if (m_error > 0) {
    p_file << "Extra:\n";
    p_file << MError() <<'\n';
    p_file << QStep() <<'\n';
  }

  p_file << "DataFormat:\n";
  p_file << NumColumns() << ' ';
  p_file << LambdaColumn() << ' ';
  p_file << DeltaColumn() << ' ';
  for (int i = 1; i <= NumInfosets(); i++) {
    for (int j = 1; j <= NumStrategies(i); j++) {
      p_file << Column(i, j) << ' ';
    }
  }
  p_file << '\n';
}

