#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#pragma hdrstop
#include <stdlib.h>
#include <string.h>
#include "general.h"
#include "normequs.h"
#include "ncalcset.h"

void save_game_func(wxButton &ob,wxCommandEvent &ev)
{
enter_game_struct *egs=(enter_game_struct *)ob.GetClientData();
NormalCalcSettings *calc=egs->dl;
if (!calc->GotMatrix())
{
	wxMessageBox("Matrix not defined","Error",wxOK | wxCENTRE,(wxFrame *)ob.GetParent());
	return;
}
else
{
	NormalMatrix &matrix=calc->GetMatrix();
	char *s=copystring(wxFileSelector("Save matrix",NULL,NULL,"nfg","*.nfg"));
	if (s)
	{
		matrix.WriteMatrix(s);
		wxMessageBox("Matrix saved","OK",wxOK | wxCENTRE,(wxFrame *)ob.GetParent());
	}
}
}



void enter_game_func(wxButton &ob,wxCommandEvent &ev)
{
int i,j,dim;
egf_payoff_struct **array;
//
enter_game_struct *egs=(enter_game_struct *)ob.GetClientData();

NormalCalcSettings *calc=egs->dl;
if (calc->GotMatrix())		// if a matrix is already defined, use it
{
	NormalMatrix &matrix=calc->GetMatrix();
	dim=matrix.Dim();
	// make a 2D array of egf_payoff_struct
	array=new egf_payoff_struct *[dim];
	for (i=0;i<dim;i++) array[i]=new egf_payoff_struct[dim];
	for (i=0;i<dim;i++)
		for (j=0;j<dim;j++)
    {array[i][j].row=matrix(i,j).row;array[i][j].col=matrix(i,j).col;}

}
else
{
	dim=atoi(egs->dim_item->GetValue());
  if (dim<2 || dim>10)
	{
		wxMessageBox("Minimal size is 2x2","Error",wxOK | wxCENTRE,(wxFrame *)ob.GetParent());
		return;
	}
	// make a 2D array of egf_payoff_struct
	array=new egf_payoff_struct *[dim];
	for (i=0;i<dim;i++) array[i]=new egf_payoff_struct[dim];
	for (i=0;i<dim;i++)
		for (j=0;j<dim;j++)
    {array[i][j].row=0.0;array[i][j].col=0.0;}
}
// Create the dialog;
MyDialogBox *d=new MyDialogBox((wxFrame *)ob.GetParent(),"Enter Game");
for (i=0;i<dim;i++)	// rows
{
	for (j=0;j<dim;j++)	// cols
	{
		d->Form()->Add(wxMakeFormFloat(NULL,&(array[i][j].row),wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,50));
		d->Form()->Add(wxMakeFormFloat(NULL,&(array[i][j].col),wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,50));
		d->Form()->Add(wxMakeFormMessage(" "));
	}
	d->Form()->Add(wxMakeFormNewLine());
	d->Form()->Add(wxMakeFormNewLine());
}
d->Go();
calc->SetMatrix(array,dim);
for (i=0;i<dim;i++) delete [] array[i];
delete [] array;
delete d;
}

void load_game_func(wxButton &ob,wxCommandEvent &ev)
{
char *s=copystring(wxFileSelector("Load Game",NULL,NULL,"out","*.out"));
if (s)
{
	FILE *fp=fopen(s,"r");
  char *dim_str=new char[20]; 
	enter_game_struct *egs=(enter_game_struct *)ob.GetClientData();
	egs->dl->SetMatrix(fp);
  sprintf(dim_str,"%d",egs->dl->GetMatrix().Dim());
  egs->dim_item->SetValue(dim_str);
	if (StringMatch(".out",FileNameFromPath(s)))	// if reading data from an output file
	{
		float m_error_f,p_step_f,l_step_f,l_start_f,l_stop_f;
    int		data_type_i;
		char *l_start_str=new char[20];
		char *l_stop_str=new char[20];
		char *l_step_str=new char[20];
		char *p_step_str=new char[20];
		char *m_error_str=new char[20];

		FindStringInFile(fp,"Settings:");
		fscanf(fp,"%f %f %f %f %f",&m_error_f,&p_step_f,&l_step_f,&l_start_f,&l_stop_f);
		sprintf(m_error_str,"%3.3f",m_error_f);
		sprintf(p_step_str,"%3.3f",p_step_f);
		sprintf(l_step_str,"%3.3f",l_step_f);
    sprintf(l_start_str,"%3.3f",l_start_f);
		sprintf(l_stop_str,"%3.3f",l_stop_f);
		fscanf(fp,"%d",&data_type_i);

		egs->l_start_item->SetValue(l_start_str);
		egs->l_stop_item->SetValue(l_stop_str);
		egs->l_step_item->SetValue(l_step_str);
		egs->p_step_item->SetValue(p_step_str);
		egs->m_error_item->SetValue(m_error_str);
		egs->data_type_item->SetSelection(data_type_i);

		delete [] l_start_str;
		delete [] l_stop_str;
		delete [] l_step_str;
		delete [] p_step_str;
		delete [] m_error_str;
  }
	fclose(fp);
  delete [] dim_str;
	delete [] s;
}
}

//******************************** Constructor/main ************************
NormalCalcSettings::NormalCalcSettings(void)
{
matrix=NULL;
char	t_file_name[250];
file_name=new char[250];
wxGetTempFileName("pxi",t_file_name);
strcpy(file_name,FileNameFromPath(t_file_name));
one_dot=FALSE;
dim=0;l_start=0.01;l_stop=3.0;dl=.1;dp=.01;m_error=.01;data_type=DATA_TYPE_LOG;
MyDialogBox *d=new MyDialogBox(NULL,"Calculation Settings");
wxFormItem *l_start_item=wxMakeFormFloat("L Start",&l_start,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100);
d->Form()->Add(l_start_item);
wxFormItem *l_stop_item=wxMakeFormFloat("L Stop",&l_stop,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100);
d->Form()->Add(l_stop_item);
wxFormItem *l_step_item=wxMakeFormFloat("L Step",&dl,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100);
d->Form()->Add(l_step_item);
d->Form()->Add(wxMakeFormNewLine());
wxFormItem *p_step_item=wxMakeFormFloat("P Step",&dp,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100);
d->Form()->Add(p_step_item);
wxFormItem *m_error_item=wxMakeFormFloat("M Error",&m_error,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100);
d->Form()->Add(m_error_item);
d->Form()->Add(wxMakeFormNewLine());
wxFormItem *dim_item=wxMakeFormShort("Game Dimension",&dim,wxFORM_TEXT,NULL,NULL,wxVERTICAL,30);
d->Form()->Add(dim_item);
d->Form()->Add(wxMakeFormString("File",&file_name,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
d->Form()->Add(wxMakeFormNewLine());
wxStringList *data_type_list=new wxStringList;
data_type_list->Add("Lin plot");data_type_list->Add("Log plot");
char *data_type_str=new char[20];
strcpy(data_type_str,"Log plot");
wxFormItem *data_type_item=wxMakeFormString("Plot Type",&data_type_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(data_type_list), 0),NULL,wxVERTICAL);
d->Form()->Add(data_type_item);
d->Form()->Add(wxMakeFormBool("OneDot",&one_dot));
d->Form()->Add(wxMakeFormNewLine());
wxFormItem *enter_game_item=wxMakeFormButton("Enter Game",(wxFunction)enter_game_func);
d->Form()->Add(enter_game_item);
wxFormItem *load_game_item=wxMakeFormButton("Load Game",(wxFunction)load_game_func);
d->Form()->Add(load_game_item);
wxFormItem *save_game_item=wxMakeFormButton("Save Game",(wxFunction)save_game_func);
d->Form()->Add(save_game_item);
d->Form()->AssociatePanel(d);
enter_game_struct egs={this,
											(wxText *)dim_item->GetPanelItem(),
											(wxText *)l_start_item->GetPanelItem(),
											(wxText *)l_stop_item->GetPanelItem(),
											(wxText *)l_step_item->GetPanelItem(),
											(wxText *)p_step_item->GetPanelItem(),
											(wxText *)m_error_item->GetPanelItem(),
											(wxRadioBox *)data_type_item->GetPanelItem()
											};
((wxButton *)enter_game_item->GetPanelItem())->SetClientData((char *)&egs);
((wxButton *)load_game_item->GetPanelItem())->SetClientData((char *)&egs);
((wxButton *)save_game_item->GetPanelItem())->SetClientData((char *)&egs);
d->Go1();
completed=d->Completed();
data_type=wxListFindString(data_type_list,data_type_str);
delete d;
}

NormalCalcSettings::~NormalCalcSettings(void)
{
if (matrix) delete matrix;
delete [] file_name;
}

void NormalCalcSettings::SetMatrix(egf_payoff_struct **array,int _dim)
{
if (matrix) delete matrix;
dim=_dim;
matrix=new NormalMatrix(dim);
for (int i=0;i<dim;i++)
	for (int j=0;j<dim;j++)
	{
		matrix->SetValue(i,j,(double)array[i][j].row,(double)array[i][j].col);
	}
}

void NormalCalcSettings::SetMatrix(char *file_name)
{
if (matrix) delete matrix;
matrix=new NormalMatrix(file_name);
}

void NormalCalcSettings::SetMatrix(FILE *fp)
{
if (matrix) delete matrix;
matrix=new NormalMatrix(fp);
}

