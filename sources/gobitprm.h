#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"

#define DATA_TYPE_LOG 1
template <class T>
class GobitSolveParamsDialog : public MyDialogBox
{
private:
	float minLam, maxLam, delLam, tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent,nequilib;
	int type, plev;
	char *outfile,*pxifile;
	Bool run_pxi;
	char *pxi_command;
public:
	GobitSolveParamsDialog(void);
	~GobitSolveParamsDialog(void);
	GobitParams<T> GetParams(void);
	int RunPXI(void) {return run_pxi;}
	char *PXICommand(void) {return pxi_command;}
};


//******************************** Constructor/main ************************
template <class T>
GobitSolveParamsDialog<T>::GobitSolveParamsDialog(void)
												:MyDialogBox(NULL,"Gobit Params")
{
// The default output filename is a temp file name
outfile=new char[250];pxifile=new char[250];pxi_command=new char[250];
strcpy(outfile,"gobit.xxx");strcpy(pxi_command,"pxi");
char	t_outfile[250];
wxGetTempFileName("gobit",t_outfile);
strcpy(pxifile,FileNameFromPath(t_outfile));
minLam=0.01;maxLam=30.0;delLam=.01;type=DATA_TYPE_LOG;
tolOpt=.0001;tolBrent=0.0001;maxitsBrent=100;maxitsOpt=200;
nequilib=1;
run_pxi=FALSE;
Form()->Add(wxMakeFormShort("# Equilibria",&nequilib));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("L Start",&minLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("L Stop",&maxLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("L Step",&delLam,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("Tolerance n-D",&tolOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance 1-D",&tolBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Iterations n-D",&maxitsOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("Iterations 1-D",&maxitsBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormString("OutFile",&outfile,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
Form()->Add(wxMakeFormString("PxiFile",&pxifile,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
Form()->Add(wxMakeFormNewLine());
wxStringList *type_list=new wxStringList;
type_list->Add("Log plot");type_list->Add("Arith plot");
char *type_str=new char[20];
strcpy(type_str,"Log plot");
wxFormItem *type_item=wxMakeFormString("Plot Type",&type_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(type_list), 0));
Form()->Add(type_item);
Form()->Add(wxMakeFormNewLine());
run_pxi=FALSE;
Form()->Add(wxMakeFormBool("Run PXI",&run_pxi));
Form()->Add(wxMakeFormString("PXI Command",&pxi_command));
Form()->Add(wxMakeFormNewLine());
Form()->AssociatePanel(this);
Go1();
type=wxListFindString(type_list,type_str);
}

template <class T>
GobitSolveParamsDialog<T>::~GobitSolveParamsDialog(void)
{
delete [] outfile;
}
template <class T>
GobitParams<T> GobitSolveParamsDialog<T>::GetParams(void)
{
GobitParams<T> P;
P.minLam=minLam;P.maxLam=maxLam;P.delLam=delLam;
P.tolBrent=tolBrent;P.maxitsBrent=maxitsBrent;
P.nequilib=nequilib;
//P.tolPOW=tolOpt;P.maxitsPOW=maxitsOpt;
//P.outfile=pxifile;P.errfile=outfile;
P.type=type;P.plev=0;
return P;
}

#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class GobitSolveParamsDialog<double> ;
TEMPLATE class GobitSolveParamsDialog<gRational> ;
#ifdef __BORLANDC__
#pragma -Jgx
#endif
