#include <stdio.h>
#include "onedot.h"
#include "gmisc.h"

//CopyFile
int CopyFile (const char *file1, const char *file2)
{
  FILE *fd1;
  FILE *fd2;
  int ch;

  if ((fd1 = fopen (file1, "rb")) == NULL)
    return FALSE;
  if ((fd2 = fopen (file2, "wb")) == NULL)
    {
      fclose (fd1);
      return FALSE;
    }

  while ((ch = getc (fd1)) != EOF)
    putc (ch, fd2);

  fclose (fd1);
  fclose (fd2);
  return TRUE;
}


//******************************* COPY HEADER **************************
// Note that this assumes that all the parameters come before the Data: line
// This just copies everything that comes before the Data: line to the output file
void OneDot::CopyHeader(gInput &in,gOutput &out)
{
  wxString tmp_str;
char c;
do
{
	tmp_str="";c=0;
	while (c!='\n') {in>>c;tmp_str+=c;}
	out<<tmp_str;
} while (strncmp(tmp_str,"Data:",strlen("Data:"))!=0 && !in.eof());
}
//****************************** GO ************************************
//Note: if input and output filenames are specified the same, the input file will be
//overwritten
void OneDot::Go(const char *in_filename,const char *out_filename)
{
double 	working_e;
int			new_equ,equ_num;
int			i;
char 		*temp_filename=new char[200];
// Initialize files
if (strcmp(in_filename,out_filename)==0)
	tmpnam(temp_filename);
else
	strcpy(temp_filename,out_filename);

gFileInput in(in_filename);
gFileOutput out(temp_filename);

CopyHeader(in,out);
// Initialize variables
EquTracker equs;
DataLine probs(in);
// Process the entire file
FindStringInFile(in,"Data:");
in>>probs;
while (!probs.Done() && in.IsValid() && out.IsValid())
{
	// Process one lambda value
	working_e=probs.E();			// now working on the new lambda
	for (i=1;i<=best_point.Length();i++) best_point[i].Delta()=LARGE_NUMBER;
	do
	{
		// Find which equlibrium this point belongs to
		equ_num=equs.Check_Equ(probs,&new_equ);
		// If this point is a new equilibrium, create new room for its data
		if (new_equ){best_point.Expand(1);best_point[equ_num].Delta()=LARGE_NUMBER;}
		// Record the smallest delta for this equlibrium,
		if (probs.Delta()<best_point[equ_num].Delta())
			best_point[equ_num]=probs;
		in>>probs;
	} while (probs.E()==working_e && !probs.Done() && in.IsValid() && out.IsValid());
	// Write one point per equlibrium per lambda
	for (i=1;i<=equs.Length();i++) out<<best_point[i];
}
// close both files
in=(FILE *)0;out=(FILE *)0;
// Now, if the files were the same, copy the temporary file over the input one
if (strcmp(in_filename,out_filename)==0)
{
	CopyFile(temp_filename,out_filename);
	remove(temp_filename);
}
delete [] temp_filename;

}
//******************************** CONSTRUCTOR *****************************
OneDot::OneDot(void) {}




