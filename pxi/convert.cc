#include <stdio.h>
#include <string.h>
#include "gmisc.h"

void main(int argc,char **argv)
{
int i;
if (argc!=3)
{
	printf("Usage: convert [infile] [outfile]\n");
	exit(1);
}
gFileInput in(argv[1]);
gFileOutput out(argv[2]);
if (!in.IsValid() || !out.IsValid())
{
	printf("A file error occured\n");
	exit(1);
}

out<<"Dimensionality:\n";
int datasets,datapoints;
in>>datasets;
out<<datasets<<' ';
for (i=1;i<=datasets;i++)
{
	in>>datapoints;
	out<<datapoints<<' ';
}
out<<'\n';
if (datasets==2)	// we should have a game matrix ....
{
	in.seekp(0L);	// go to the start of the file
	int x_dim,y_dim,dummy;
	double val1,val2;
	in>>datasets>>x_dim>>y_dim;
	in>>dummy>>dummy>>dummy;
	out<<"Game:\n";
	out<<"1 2 3\n";
	for (i=1;i<=x_dim;i++)
	{
		for (int j=1;j<=y_dim;j++)
		{
		 in>>val1>>val2;
		 out<<val1<<' '<<val2<<' ';
		}
		out<<'\n';
	}
}
out<<"Settings:\n";
FindStringInFile(in,"Settings:");
double merror,qstep;
double estart,estop,estep;
int type;
in>>merror>>qstep;
in>>estep>>estart>>estop>>type;
if (type==0) estep-=1;
out<<estart<<'\n'<<estop<<'\n'<<estep<<'\n'<<0<<'\n'<<1<<'\n'<<(1-type)<<'\n';
out<<"Extra:\n";
out<<1<<'\n';
out<<merror<<'\n'<<qstep<<'\n';

int num_columns,column;
in>>num_columns;
out<<"DataFormat:\n";
out<<num_columns<<' ';
in>>column;		// e
out<<column<<' ';
in>>column;		// delta
out<<column<<' ';

for (i=1;i<=num_columns-2;i++)
{
	in>>column;
	out<<column<<' ';
}
out<<'\n';
FindStringInFile(in,"Data:");
out<<"Data:\n";

double val;
while (in.IsValid())
{
	for (i=1;i<=num_columns;i++)
	{
		in>>val;
		out<<val<<' ';
	}
	out<<'\n';
}
}


