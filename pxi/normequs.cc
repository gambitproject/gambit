/****************************************************************************
 * Program operates on the following game (for example):
 *      q1     q2     q3
 * p1 [7,3    0,0    0,0] x1=7*q1+0*q2+0*q3
 * p2 [0,0    5,5    0,0] x2=5*q2+0*q1+0*q3
 * p3 [0,0    0,0    3,7] x3=3*q3+0*q2+0*q1
 *     y1      y2     y3
 *
 * Pick two indipendent q's, from them calculate p's, from them calculate y's
 * from them calculate q_calc's.  If q==q_calc, got an equilibrium.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include "probvect.h"
#include "normequs.h"
#include "stpwatch.h"
#include "gmisc.h"

#define TRUE	1
#define	FALSE	0
#define FLOAT_ROUND_UP	.0001
char nequs_tempstr[200];
/*************************** OUTPUT RESULT ********************************/
void NormalEquSolver::OutputResult(double l,double *q,double *p,double dist,output_modes mode)
{
static FILE	*fp=NULL;
static StopWatch *timer;
char	tempstr1[200];
int		i;

if (mode==MODE_NEW)
{
	timer=new StopWatch;
  timer->Start();
	fp=fopen(file_name,"w");	// Initialize the output file
	assert(fp!=NULL);
	fprintf(fp,"Dimensionality:\n");
	fprintf(fp,"2 %d %d\n",matrix->DimX(),matrix->DimY());
	fprintf(fp,"Game:\n");
	matrix->WriteMatrix(fp);	// Write the matrix to it
	// Write the current settings to it
	fprintf(fp,"Settings:\n");
	fprintf(fp,"%lf\n%lf\n%lf\n",l_start,l_stop,dl);
	fprintf(fp,"0\n1\n%d\n",data_type);	// note: convention changed: 0-Lin, 1-Log
	// Write extra information
	fprintf(fp,"Extra:\n");
	fprintf(fp,"1\n%lf\n%lf\n",m_error,dp);
	// write the column data
	fprintf(fp,"DataFormat:\n");
	fprintf(fp,"%d ",matrix->DimX()*2+2);
	fprintf(fp,"%d %d ",matrix->DimX()*2+2-1,matrix->DimX()*2+2);
	for (int i=1;i<matrix->DimX()*2+2-1;i++) fprintf(fp,"%d ",i);
	fprintf(fp,"\n");
	fprintf(fp,"Data:\n");
	return;
}
if (mode==MODE_DONE)
{
	sprintf(nequs_tempstr,"Simulation took %s\n",timer->ElapsedStr());
	fprintf(fp,"%s",nequs_tempstr);
	fclose(fp);
	fp=NULL;
	delete timer;
	return;
}

strcpy(tempstr1,"");        /* start with a blank line     */
for(i=0;i<dim;i++)        /* print the values of p       */
{
	sprintf(nequs_tempstr,"%4.4f ",p[i]);
	strcat(tempstr1,nequs_tempstr);
}
for(i=0;i<dim;i++)        /* print the values of q        */
{
	sprintf(nequs_tempstr,"%4.4f ",q[i]);
	strcat(tempstr1,nequs_tempstr);
}
sprintf(nequs_tempstr,"%4.4f %8.8f \n",l,dist);
strcat(tempstr1,nequs_tempstr);
fprintf(fp,"%s",tempstr1);
}


/******************************** CHECK EQU ********************************/
Bool NormalEquSolver::Check_Equ(double *q,double l)
{
double denom;					/* denominator of function--speedup!		*/
Bool   ok;                       /* was a solution found?                    */
int   i,j;                      /* just loop variables                      */
double dist;

assert(q!=NULL);
/*---------------------make X's---------------------*/
for (i=0;i<dim;i++)
{
		x[i]=0.0;
		for (j=0;j<dim;j++)	x[i]+=(*matrix)(i,j).row*q[j];
}
/*--------------------make P's----------------------*/
denom=0.0;
for (i=0;i<dim;i++) denom+=exp(l*x[i]);
for (i=0;i<dim;i++) p[i]=exp(l*x[i])/denom;
/*--------------------make Y's----------------------*/
for (i=0;i<dim;i++)
{
	y[i]=0.0;
	for (j=0;j<dim;j++)	y[i]+=(*matrix)(j,i).col*p[j];
}
/*--------------------make Q_CALC's-----------------*/
denom=0.0;
for (i=0;i<dim;i++) denom+=exp(l*y[i]);
for (i=0;i<dim;i++) q_calc[i]=exp(l*y[i])/denom;
/*--------------------check for equilibrium---------*/
			 /* Note: this uses the very crude method for finding*
			 * if two points are 'close enough.'  The correspon-*
			 * ding coordinates of each point are subtracted and*
			 * abs or result is compared to MERROR              *
			 * A more precise way would be to use the Distance  *
			 * function provided, but that would be very slow   *
			 * if Distance is not used, value of ok is either   *
			 * 1.0 or 0.0                                       */

ok=TRUE;dist=0.0;
for (i=0;i<dim;i++)
{
	dist+=fabs((double)(q[i]-q_calc[i]));
	if ((fabs((double)(q[i]-q_calc[i]))>=m_error)) ok=FALSE;
}

if (ok)	OutputResult(l,q,p,dist);

return (ok);
}

NormalEquSolver::NormalEquSolver(char *_matrix_file_name,double _l_start,double _l_stop,double _dl,double _dp,double _m_error,int _data_type,char *out_file_name)
{
update_func=NULL;
matrix=new NormalMatrix(_matrix_file_name);
assert(matrix!=NULL);
dim=matrix->Dim();
x=new double[dim];y=new double[dim];p=new double[dim];q_calc=new double[dim];
assert(x!=NULL);assert(y!=NULL);assert(p!=NULL);assert(q_calc!=NULL);
l_start=_l_start;l_stop=_l_stop;dl=_dl;dp=_dp;m_error=_m_error;data_type=_data_type;
done=FALSE;
if (out_file_name) strcpy(file_name,out_file_name);
}

NormalEquSolver::NormalEquSolver(NormalMatrix _matrix,double _l_start,double _l_stop,double _dl,double _dp,double _m_error,int _data_type,char *out_file_name)
{
update_func=NULL;
matrix=new NormalMatrix(_matrix);
assert(matrix!=NULL);
dim=matrix->Dim();
l_start=_l_start;l_stop=_l_stop;dl=_dl;dp=_dp;m_error=_m_error;data_type=_data_type;
x=new double[dim];y=new double[dim];p=new double[dim];q_calc=new double[dim];
assert(x!=NULL);assert(y!=NULL);assert(p!=NULL);assert(q_calc!=NULL);
done=FALSE;
if (out_file_name) strcpy(file_name,out_file_name);
}

NormalEquSolver::~NormalEquSolver(void)
{
assert(matrix!=NULL);
assert(x!=NULL);assert(y!=NULL);assert(p!=NULL);assert(q_calc!=NULL);
delete matrix;
delete [] p;
delete [] x;
delete [] y;
delete [] q_calc;
}

void NormalEquSolver::Go(void)
{
// Create the ProbVector to give us all sets of probability values
ProbVect *pv=new ProbVect(dim,(int)(1.0/dp+FLOAT_ROUND_UP));
assert(pv!=NULL);
// Initialize the output file
OutputResult(0,NULL,NULL,0,MODE_NEW);
for (double l=l_start;l<l_stop;l=(data_type==DATA_TYPE_ARITH) ? (l+dl) : (l*(1+dl)))
{
	while (!pv->Done()) if (pv->Inc())	Check_Equ(pv->GetP(),l);
	pv->Reset();
	if (update_func) (*update_func)();
}
// Close the output file
OutputResult(0,NULL,NULL,0,MODE_DONE);
done=TRUE;
delete pv;

}

//************************************************************************
// Normal Matrix Class
//************************************************************************
NormalMatrix::NormalMatrix(int _dim)
{
dim=_dim;
assert(dim>0);
data=new PayoffStruct *[dim];
assert(data!=NULL);
for (int i=0;i<dim;i++)
{
	data[i]=new PayoffStruct[dim];
	assert(data[i]!=NULL);
}
}

//********************** Reading matrixes from a data file****************
//* Data files have the following format:
//* number_of_players, n[0..number_of_players]--# of strategies for player[i]
//* Indexes [fastest,second,last]
//* data in the format (row,col) (row,col)
//*                    (row,col) (row,col)
NormalMatrix::NormalMatrix(char *file_name)
{
FILE	*matrixfile=fopen(file_name,"rt");
int	i,i1,x_dim,y_dim,dummy;
fscanf(matrixfile,"%d %d %d",&dummy,&x_dim,&y_dim);
fscanf(matrixfile,"%d %d %d",&dummy,&dummy,&dummy);
assert(x_dim==y_dim);
dim=x_dim;
data=new PayoffStruct *[dim];
assert(data!=NULL);
for (i=0;i<dim;i++)
{
	data[i]=new PayoffStruct[dim];
	assert(data[i]!=NULL);
}

for (i=0;i<dim;i++)
 for (i1=0;i1<dim;i1++)
	fscanf(matrixfile,"%lf %lf",&(data[i][i1].row),&(data[i][i1].col));
fclose(matrixfile);
}

NormalMatrix::NormalMatrix(FILE *matrixfile)
{
int	i,i1,num_players,x_dim,y_dim,dummy;
fscanf(matrixfile,"%d %d %d",&num_players,&x_dim,&y_dim);
fscanf(matrixfile,"%d %d %d",&dummy,&dummy,&dummy);
assert(num_players==2 && x_dim==y_dim);
dim=x_dim;
data=new PayoffStruct *[dim];
assert(data!=NULL);
for (i=0;i<dim;i++)
{
	data[i]=new PayoffStruct[dim];
	assert(data[i]!=NULL);
}

for (i=0;i<dim;i++)
 for (i1=0;i1<dim;i1++)
	fscanf(matrixfile,"%lf %lf",&(data[i][i1].row),&(data[i][i1].col));
}

NormalMatrix::NormalMatrix(gInput &in)
{
int	i,i1,num_players,x_dim,y_dim,dummy,ok;
ok=FindStringInFile(in,"Dimensionality:");assert(ok);
in>>num_players>>x_dim>>y_dim;
ok=FindStringInFile(in,"Game:");assert(ok);
in>>dummy>>dummy>>dummy;
assert(num_players==2 && x_dim==y_dim);
dim=x_dim;
data=new PayoffStruct *[dim];
assert(data!=NULL);
for (i=0;i<dim;i++)
{
	data[i]=new PayoffStruct[dim];
	assert(data[i]!=NULL);
}

for (i=0;i<dim;i++)
 for (i1=0;i1<dim;i1++)
	in>>data[i][i1].row>>data[i][i1].col;
}

NormalMatrix::NormalMatrix(NormalMatrix &m)
{
int i,j;
dim=m.Dim();
assert(dim>0);
data=new PayoffStruct *[dim];
assert(data!=NULL);
for (i=0;i<dim;i++)
{
	data[i]=new PayoffStruct[dim];
	assert(data[i]!=NULL);
}
for (i=0;i<dim;i++)
	for (j=0;j<dim;j++)
	{
		data[i][j].row=m(i,j).row;
		data[i][j].col=m(i,j).col;
	}
}
//**************************** Writing matrices to a data file
//* Same format as above
void NormalMatrix::WriteMatrix(char *file_name)
{
gFileOutput out(file_name);
WriteMatrix(out);
}

void NormalMatrix::WriteMatrix(FILE *matrixfile)
{
assert(data!=NULL);
int	i,i1;
fprintf(matrixfile,"%d %d %d\n",3,2,1);
for (i=0;i<dim;i++)
{
 for (i1=0;i1<dim;i1++)
	fprintf(matrixfile,"%3.3lf %3.3lf  ",data[i][i1].row,data[i][i1].col);
 fprintf(matrixfile,"\n");
}
}

void NormalMatrix::WriteMatrix(gOutput &out)
{
assert(data!=NULL);
int	i,i1;
out<<2<<' '<<dim<<' '<<dim<<'\n';
out<<3<<' '<<2<<' '<<1<<'\n';
for (i=0;i<dim;i++)
{
 for (i1=0;i1<dim;i1++)
	out<<data[i][i1].row<<' '<<data[i][i1].col;
 out<<'\n';
}
}


NormalMatrix::~NormalMatrix(void)
{
assert(data!=NULL);
if (data)
{
	for (int i=0;i<dim;i++)
	{
		assert(data[i]!=NULL);
		delete [] data[i];
		data[i]=NULL;
	}
	assert(data!=NULL);
	delete [] data;
}
}
