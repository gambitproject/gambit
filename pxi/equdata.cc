#include "gmisc.h"
#include "equdata.h"


//***********************CONSTRUCTORS FOR FILE HEADER***********************
FileHeader::FileHeader(const char *in_filename)
{file_name=in_filename;gFileInput in(in_filename);Init(in);}

FileHeader::FileHeader(gInput &in)
{Init(in);}

FileHeader::FileHeader(const FileHeader &F):
  merror(F.merror),q_step(F.q_step),e_step(F.e_step),e_start(F.e_start),e_stop(F.e_stop),data_type(F.data_type),
  strategies(F.strategies),num_columns(F.num_columns),num_infosets(F.num_infosets),
  e_column(F.e_column),delta_column(F.delta_column),prob_cols(F.prob_cols),
  matrix((F.matrix) ? new NormalMatrix(*F.matrix) : 0),file_name(F.file_name),
  data_max(F.data_max),data_min(F.data_min)
{ }

FileHeader &FileHeader::operator=(const FileHeader &F)
{
  merror=F.merror;q_step=F.q_step;e_step=F.e_step;e_start=F.e_start;e_stop=F.e_stop;data_type=F.data_type;
  strategies=F.strategies;
  num_columns=F.num_columns;num_infosets=F.num_infosets;
  e_column=F.e_column;delta_column=F.delta_column;prob_cols=F.prob_cols;
  matrix=(F.matrix) ? new NormalMatrix(*F.matrix) : 0;
  file_name=F.file_name;
  data_max=F.data_max;data_min=F.data_min;
  return (*this);
}

void FileHeader::Init(gInput &in)
{
  int iset,ok=0;
  merror=-1;q_step=-1;
  // Get dimensionality
  ok=FindStringInFile(in,"Dimensionality:");assert(ok);
  in>>num_infosets;
  strategies=gBlock<int>(num_infosets);
  for (iset=1;iset<=num_infosets;iset++) in>>strategies[iset];
  // Get the x/y data settings/scales
  ok=FindStringInFile(in,"Settings:");assert(ok);
  in>>e_start>>e_stop>>e_step>>data_min>>data_max>>data_type;
  // Get the data format
  ok=FindStringInFile(in,"DataFormat:");assert(ok);
  in>>num_columns;
  in>>e_column>>delta_column;
  prob_cols=gBlock< gBlock<int> >(num_infosets);
  for (iset=1;iset<=num_infosets;iset++)
    prob_cols[iset]=gBlock<int>(strategies[iset]);
  for (iset=1;iset<=num_infosets;iset++)
    for (int st=1;st<=strategies[iset];st++)
      in>>prob_cols[iset][st];
  // Get the optional data
  // Get the game matrix if available
  ok=FindStringInFile(in,"Game:");
  // The game matrix reading is only implemented for 2 player square games.
  if (ok && num_infosets==2 && strategies[1]==strategies[2]) 
    matrix=new NormalMatrix(in); 
  else matrix=0;
  // Get the extra data if available
  ok=FindStringInFile(in,"Extra:");
  if (ok) in>>ok;	// Check ReadExtra
  if (ok) in>>merror>>q_step;
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
  // Output the game matrix if we have it
  if (f.matrix) {op<<"Game:\n";f.matrix->WriteMatrix(op);}
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

//***************************** CONSTRUCTORS FOR DATALINE ****************
void DataLine::Init(void)
{
  data=gBlock<PointNd>(header.NumInfosets());
  for (int iset=1;iset<=header.NumInfosets();iset++)
    data[iset]=	PointNd(header.NumStrategies(iset));
}
//************************** INPUT/OUTPUT FOR DATALINE *******************
int	DataLine::Read(gInput &in)
{
  int i;
  gBlock<double>	data_cols(header.NumColumns());
  for (i=1;i<=header.NumColumns();i++) in>>data_cols[i];
  e=data_cols[header.ECol()];
  if (header.DeltaCol()) delta=data_cols[header.DeltaCol()];
  double sum;
  for (int iset=1;iset<=header.NumInfosets();iset++) {
    sum=0.0;
    for (i=1;i<=header.NumStrategies(iset);i++) {
      if (header.Col(iset,i)) {
	data[iset][i]=data_cols[header.Col(iset,i)];
	sum+=data[iset][i];
      }
      else
	data[iset][i]=1.0-sum;
    }
  }
  return (in.eof() || !in.IsValid());
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
