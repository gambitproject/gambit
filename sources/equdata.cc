#include "equdata.h"
#include "equdflt.h"

void FindStringInFile(gInput &in,const char *s);

//***********************CONSTRUCTORS FOR FILE HEADER***********************
FileHeader::FileHeader(const char *in_filename,Bool want_matrix)
{file_name=in_filename;gFileInput in(in_filename);Init(in,want_matrix);}

FileHeader::FileHeader(gInput &in,Bool want_matrix)
{Init(in,want_matrix);}

FileHeader::FileHeader(double _merror,double _qstep,double _estep,double _estart,
						double _estop,int _data_type,int _num_infosets,gBlock<int> _strategies)
{
merror=(_merror>0) ? _merror : DEF_MERROR;
q_step=(_qstep>0) ? _qstep: DEF_QSTEP;
e_step=(_estep>0) ? _estep: DEF_ESTEP;
e_start=(_estart>0) ? _estart : DEF_ESTART;
e_stop=(_estop>0) ? _estop : DEF_ESTOP;
data_type=(_data_type>0) ? _data_type : DEF_DATATYPE;
num_infosets=(_num_infosets>0) ? _num_infosets : -1;
if (num_infosets>0)
{
	int num_columns=num_infosets+2;
	int i;
	delta_column=num_columns;
	e_column=num_columns-1;
	strategies=gBlock<int>(num_infosets);
	for(i=1;i<=num_infosets;i++) strategies[i]=0;
	prob_cols=gBlock< gBlock<int> >(num_infosets);
	if (_strategies!=gBlock<int>())
	{
		assert(_strategies.Length()==num_infosets);
		for (int iset=1;iset<=num_infosets;iset++)
		{
			strategies[iset]=_strategies[iset];
			prob_cols[iset]=gBlock<int>(strategies[iset]);
			int cur_col=0;for (int i=1;i<=num_infosets;i++) cur_col+=strategies[i];
			for (i=1;i<=strategies[iset];i++) prob_cols[iset][i]=++cur_col;
		}
	}
}
}
FileHeader::FileHeader(const FileHeader &F):
merror(F.merror),q_step(F.q_step),e_step(F.e_step),e_start(F.e_start),e_stop(F.e_stop),data_type(F.data_type),
strategies(F.strategies),num_columns(F.num_columns),num_infosets(F.num_infosets),
e_column(F.e_column),delta_column(F.delta_column),prob_cols(F.prob_cols),
matrix((F.matrix) ? new NormalMatrix(*F.matrix) : 0)
{ }

FileHeader &FileHeader::operator=(const FileHeader &F)
{
merror=F.merror;q_step=F.q_step;e_step=F.e_step;e_start=F.e_start;e_stop=F.e_stop;data_type=F.data_type;
strategies=F.strategies;
num_columns=F.num_columns;num_infosets=F.num_infosets;
e_column=F.e_column;delta_column=F.delta_column;prob_cols=F.prob_cols;
matrix=(F.matrix) ? new NormalMatrix(*F.matrix) : 0;
return (*this);
}

void FileHeader::Init(gInput &in,Bool want_matrix)
{
int iset;
in.seekp(0L);	// go to the start of the file
in>>num_infosets;
strategies=gBlock<int>(num_infosets);
for (iset=1;iset<=num_infosets;iset++) in>>strategies[iset];
if (want_matrix && num_infosets==2) matrix=new NormalMatrix(in); else matrix=0;
FindStringInFile(in,"Settings:");		// go to start of settings
in>>merror>>q_step>>e_step>>e_start>>e_stop>>data_type;
in>>num_columns;
in>>e_column>>delta_column;
prob_cols=gBlock< gBlock<int> >(num_infosets);
for (iset=1;iset<=num_infosets;iset++)
	prob_cols[iset]=gBlock<int>(strategies[iset]);
for (iset=1;iset<=num_infosets;iset++)
	for (int st=1;st<=strategies[iset];st++)
		in>>prob_cols[iset][st];
}

// Data Output for FileHeader
gOutput &operator<<(gOutput &op,const FileHeader &f)
{
int i,j;
// This info can come from either the matrix of the FileHeader.  If a matrix
// exists, it is used as it provides additional information as well
if (f.matrix)
	f.matrix->WriteMatrix(op);
else
{
	op<<f.num_infosets<<' ';
	for (i=1;i<=f.num_infosets;i++) op<<f.strategies[i]<<' ';
	op<<'\n';
}
op<<"Settings:\n";
op<<f.merror<<'\n'<<f.q_step<<'\n'<<f.e_step<<'\n'<<f.e_start<<'\n'<<f.e_stop<<'\n'<<f.data_type<<'\n';
op<<f.num_columns<<' '<<f.e_column<<' '<<f.delta_column<<' ';
for (i=1;i<=f.num_infosets;i++)
	for (j=1;j<=f.strategies[i];j++)
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
delta=data_cols[header.DeltaCol()];
double sum;
for (int iset=1;iset<=header.NumInfosets();iset++)
{
	sum=0.0;
	for (i=1;i<=header.NumStrategies(iset);i++)
	{
		if (header.Col(iset,i))
		{
			data[iset][i]=data_cols[header.Col(iset,i)];
			sum+=data[iset][i];
		}
		else
			data[iset][i]=1.0-sum;
	}
}
return (in.eof());
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
data_cols[header.DeltaCol()]=delta;
for (i=1;i<=header.NumColumns();i++) out<<data_cols[i]<<' ';
out<<'\n';
}
