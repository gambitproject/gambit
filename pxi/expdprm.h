#include "wxmisc.h"

class ExpDataDialog:public MyDialogBox
{
private:
  char *likename,*pxiname,*expname;
  gOutput *likefile;
  gInput *pxifile,*expfile;
  int save_like,load_now;
  PxiFrame *frame;
  static void browse_pxi_func(wxButton &ob,wxEvent &ev);
  static void browse_exp_func(wxButton &ob,wxEvent &ev);
  static void save_item_func(wxCheckBox &ob,wxEvent &ev);
public:
  // Constructor
  ExpDataDialog(const char *pxi_name=0,PxiFrame *parent=0);
  // Destructor
  ~ExpDataDialog(void);
  // Data access
  void	GetParams(ExpDataParams &P);
  void	LoadNow(void);
};

ExpDataDialog::ExpDataDialog(const char *pxi_name,PxiFrame *parent)
  :MyDialogBox(parent,"Likelihood"),frame(parent),likefile(0),expfile(0),pxifile(0)
{
  likename=new char[250];strcpy(likename,"like.out");
  pxiname=new char[250];strcpy(pxiname,pxi_name);
  expname=new char[250];strcpy(expname,"");
  save_like=TRUE;load_now=FALSE;
  
  wxFormItem *name_pxi=wxMakeFormString("PxiName ",&pxiname);
Form()->Add(name_pxi);
 wxFormItem *browse_pxi=wxMakeFormButton("Browse",(wxFunction)browse_pxi_func);
 Form()->Add(browse_pxi);
 Form()->Add(wxMakeFormNewLine());
 
 wxFormItem *name_exp=wxMakeFormString("ExpName",&expname);
 Form()->Add(name_exp);
 wxFormItem *browse_exp=wxMakeFormButton("Browse",(wxFunction)browse_exp_func);
 Form()->Add(browse_exp);
 Form()->Add(wxMakeFormNewLine());
 
 wxFormItem *name_like=wxMakeFormString("LikeFile",&likename);
 Form()->Add(name_like);
 wxFormItem *save_item=wxMakeFormBool("Save Like",&save_like);
 Form()->Add(save_item);
 Form()->Add(wxMakeFormNewLine());
 
 Form()->Add(wxMakeFormBool("Load Now",&load_now));
 Form()->AssociatePanel(this);
 ((wxButton *)browse_pxi->GetPanelItem())->SetClientData((char *)name_pxi->GetPanelItem());
 ((wxButton *)browse_exp->GetPanelItem())->SetClientData((char *)name_exp->GetPanelItem());
 ((wxCheckBox *)save_item->GetPanelItem())->SetClientData((char *)name_like->GetPanelItem());
 ((wxCheckBox *)save_item->GetPanelItem())->Callback((wxFunction)save_item_func);
 Go1();
}

void ExpDataDialog::browse_pxi_func(wxButton &ob,wxEvent &)
{
  char *s=wxFileSelector("Load data file", NULL, NULL, NULL, "*.out");
  if (s) ((wxText *)ob.GetClientData())->SetValue(s);
}

void ExpDataDialog::browse_exp_func(wxButton &ob,wxEvent &)
{
  char *s=wxFileSelector("Load observation file", NULL, NULL, NULL, "*.exp");
  if (s) ((wxText *)ob.GetClientData())->SetValue(s);
}

void ExpDataDialog::save_item_func(wxCheckBox &ob,wxEvent &)
{((wxText *)ob.GetClientData())->Enable(ob.GetValue());}

void ExpDataDialog::GetParams(ExpDataParams &P)
{
  assert(pxiname && expname);
  pxifile=new gFileInput(pxiname);expfile=new gFileInput(expname);
  P.pxifile=pxifile;P.expfile=expfile;
  if (likename && save_like) {
    likefile=new gFileOutput(likename);
    P.likefile=likefile;
  }
}

void ExpDataDialog::LoadNow(void)
{
  if (likefile && load_now) {
    delete likefile;likefile=0;
    frame->MakeChild(likename);
  }
}

ExpDataDialog::~ExpDataDialog(void)
{
  delete [] likename;
  delete [] pxiname;
  delete [] expname;
  if (pxifile) delete pxifile;
  if (expfile) delete expfile;
  if (likefile) delete likefile;
}

