#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"

#define DATA_TYPE_LOG 1
template <class T>
class LiapSolveParamsDialog : public MyDialogBox
{
private:
	float tolOpt, tolBrent;
	int maxitsOpt,maxitsBrent,nequilib,ntries;
	int plev;
	char *outfile,*errfile;
public:
	LiapSolveParamsDialog(void);
	~LiapSolveParamsDialog(void);
	LiapParams<T> GetParams(void);
};


//******************************** Constructor/main ************************
template <class T>
LiapSolveParamsDialog<T>::LiapSolveParamsDialog(void)
												:MyDialogBox(NULL,"Liap Params")
{
// The default output filename is a temp file name
outfile=new char[250];errfile=new char[250];
strcpy(outfile,"liap.out");strcpy(errfile,"liap.err");
tolOpt=.0001;tolBrent=0.0001;maxitsBrent=100;maxitsOpt=200;
nequilib=1;ntries=10;
Form()->Add(wxMakeFormShort("# Equilibria",&nequilib));
Form()->Add(wxMakeFormShort("# Tries",&ntries));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormFloat("Tolerance n-D",&tolOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormFloat("Tolerance 1-D",&tolBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormShort("Iterations n-D",&maxitsOpt,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormShort("Iterations 1-D",&maxitsBrent,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL,100));
Form()->Add(wxMakeFormNewLine());
Form()->Add(wxMakeFormString("OutFile",&outfile,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
Form()->Add(wxMakeFormString("ErrFile",&errfile,wxFORM_DEFAULT,NULL,NULL,wxVERTICAL));
Go();
}

template <class T>
LiapSolveParamsDialog<T>::~LiapSolveParamsDialog(void)
{
delete [] outfile;delete [] errfile;
}
template <class T>
LiapParams<T> LiapSolveParamsDialog<T>::GetParams(void)
{
LiapParams<T> P;
P.tolBrent=tolBrent;P.maxitsBrent=maxitsBrent;
P.nequilib=nequilib;P.ntries=ntries;
P.outfile=new gFileOutput(outfile);P.errfile=new gFileOutput(errfile);
P.plev=0;
return P;
}
#ifdef __GNUG__
#define TEMPLATE template
#elif defined __BORLANDC__
#pragma option -Jgd
#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class LiapSolveParamsDialog<double> ;
TEMPLATE class LiapSolveParamsDialog<gRational> ;
#ifdef __BORLANDC__
#pragma -Jgx
#endif
