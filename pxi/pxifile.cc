//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of class for storing of PXI file data
//

#include "gmisc.h"   // for FindStringInFile
#include "pxifile.h"

//=========================================================================
//                         class DataLine
//=========================================================================

/*
void DataLine::Init(void)
{
  data = gBlock<gBlock>(header.NumInfosets());
  for (int iset=1;iset<=header.NumInfosets();iset++)
    data[iset]=	PointNd(header.NumStrategies(iset));
}
*/
#ifdef NOT_PORTED_YET
int DataLine::Read(gInput &in)
{
  gBlock<double> data_cols(header.NumColumns());
  try {
    for (int i = 1; i <= header.NumColumns(); i++)  {
      in >> data_cols[i];
    }
  }
  catch (gFileInput::ReadFailed &) {
    return 1;
  }
  e=data_cols[header.ECol()];
  if (header.DeltaCol()) delta=data_cols[header.DeltaCol()];
  double sum;
  for (int iset=1;iset<=header.NumInfosets();iset++) {
    sum=0.0;
    for (int i = 1; i <= header.NumStrategies(iset); i++) {
      if (header.Col(iset,i)) {
	data[iset][i]=data_cols[header.Col(iset,i)];
	sum+=data[iset][i];
      }
      else
	data[iset][i]=1.0-sum;
    }
  }
  return (in.eof() /* || !in.IsValid() */);
}

void DataLine::Write(gOutput &out) const
{
  int i;
  gBlock<double>	data_cols(header.NumColumns());
  for (i=1;i<=header.NumColumns();i++) data_cols[i]=0.00; // clear it first
  for (int iset=1;iset<=header.NumInfosets();iset++)
    for (i=1;i<=header.NumStrategies(iset);i++)
      if (header.Col(iset,i))
	data_cols[header.Col(iset,i)]=data[iset][i];
  data_cols[header.ECol()]=e;
  if (header.DeltaCol()) data_cols[header.DeltaCol()]=delta;
  for (i=1;i<=header.NumColumns();i++) out<<data_cols[i]<<' ';
  out<<'\n';
}
#endif  // NOT_PORTED_YET

//=========================================================================
//                         class FileHeader 
//=========================================================================

FileHeader::FileHeader(void)
{ }

FileHeader::~FileHeader()
{
  for (int i = 1; i <= m_data.Length(); i++) {
    delete m_data[i];
  }
}

void FileHeader::ReadFile(gInput &in)
{
  int iset,ok=0;
  merror=-1;
  q_step=-1;
  // Get dimensionality
  ok=FindStringInFile(in,"Dimensionality:");assert(ok);
  in >> num_infosets;
  strategies=gBlock<int>(num_infosets);
  for (iset=1;iset<=num_infosets;iset++) in>>strategies[iset];
  // Get the x/y data settings/scales
  ok=FindStringInFile(in,"Settings:");assert(ok);
  in >> e_start >> e_stop >> e_step >> data_min >> data_max >> data_type;
  // Get the data format
  ok=FindStringInFile(in,"DataFormat:");assert(ok);
  in >> num_columns;
  in >> e_column >> delta_column;
  prob_cols=gBlock< gBlock<int> >(num_infosets);
  for (iset=1;iset<=num_infosets;iset++)
    prob_cols[iset]=gBlock<int>(strategies[iset]);
  for (iset=1;iset<=num_infosets;iset++)
    for (int st=1;st<=strategies[iset];st++)
      in >> prob_cols[iset][st];
  // Get the optional data
  // Get the game matrix if available
  ok=FindStringInFile(in,"Game:");
  /*
  // The game matrix reading is only implemented for 2 player square games.
  if (ok && num_infosets==2 && strategies[1]==strategies[2]) 
    matrix=new NormalMatrix(in); 
  else matrix=0;
  // Get the extra data if available
  */
  ok=FindStringInFile(in,"Extra:");
  if (ok) in>>ok;	// Check ReadExtra
  if (ok) in>>merror>>q_step;

  if (!FindStringInFile(in, "Data:")) {
    return;
  }

  gBlock<gBlock<double> > probs(NumInfosets());
  for (int iset = 1; iset <= NumInfosets(); iset++) {
    probs[iset] = gBlock<double>(NumStrategies(iset));
  }

  while (!in.eof()) {
    gBlock<double> data_cols(NumColumns());
    try {
      for (int i = 1; i <= NumColumns(); i++)  {
	in >> data_cols[i];
      }
    }
    catch (...) {
      return;
    }

    DataLine *line = new DataLine(data_cols[ECol()],
				  (DeltaCol()) ? data_cols[DeltaCol()] : 0.0,
				  probs);
    double sum;
    for (int iset=1;iset<=NumInfosets();iset++) {
      sum=0.0;
      for (int i = 1; i <= NumStrategies(iset); i++) {
	if (Col(iset,i)) {
	  (*line)[iset][i]=data_cols[Col(iset,i)];
	  sum+=(*line)[iset][i];
	}
	else {
	  (*line)[iset][i]=1.0-sum;
	}
      }
    }

    m_data.Append(line);
  }
}

// Data Output for FileHeader
gOutput &operator<<(gOutput &op,const FileHeader &f)
{
  int i,j;
  // Output dimensinality
  op<<"Dimensionality:\n"<<f.num_infosets<<' ';
  for (i=1;i<=f.num_infosets;i++) op<<f.strategies[i]<<' ';
  op<<'\n';
  // Output the x/y data settings/scales
  op<<"Settings:\n";
  op<<f.e_start<<'\n'<<f.e_stop<<'\n'<<f.e_step<<'\n'<<f.data_min<<'\n'<<f.data_max<<'\n'<<f.data_type<<'\n';
  /*
  // Output the game matrix if we have it
  //  if (f.matrix) {op<<"Game:\n";f.matrix->WriteMatrix(op);}
  */
  // Output the extra stuff if we have it
  if (f.merror>0) op<<"Extra:\n"<<f.merror<<'\n'<<f.q_step<<'\n';
  // Output the data format
  op<<"DataFormat:\n";
  op<<f.num_columns<<' '<<f.e_column<<' '<<f.delta_column<<' ';
  for (i=1;i<=f.num_infosets;i++)
    for (j=1;i<=f.strategies[i];j++)
      op<<f.prob_cols[i][j]<<' ';
  op<<'\n';
  return op;
}

