#include "wxmisc.h"

class dialogExpData: public guiAutoDialog
{
private:
  wxString likename, pxiname, expname;
  gOutput *likefile;
  gInput *pxifile,*expfile;
  int save_like,load_now;
  PxiFrame *frame;
  wxTextCtrl *m_pxiName, *m_expName, *m_likeName;
  wxCheckBox *m_saveLike, *m_loadNow;

  void OnBrowsePxi(wxCommandEvent &);
  void OnBrowseExp(wxCommandEvent &);
  
  //  static void browse_pxi_func(wxButton &ob,wxEvent &ev);
  //  static void browse_exp_func(wxButton &ob,wxEvent &ev);
  //  static void save_item_func(wxCheckBox &ob,wxEvent &ev);
public:
  // Constructor
  dialogExpData(const char *pxi_name=0,PxiFrame *parent=0);
  // Destructor
  ~dialogExpData(void);
  // Data access
  void	GetParams(ExpDataParams &P);
  void	LoadNow(void);

  DECLARE_EVENT_TABLE()
};

const int idEXPDATA_BROWSE1_BUTTON = 3051;
const int idEXPDATA_BROWSE2_BUTTON = 3052;

BEGIN_EVENT_TABLE(dialogExpData, wxDialog)
  EVT_BUTTON(idEXPDATA_BROWSE1_BUTTON, dialogExpData::OnBrowsePxi)
  EVT_BUTTON(idEXPDATA_BROWSE2_BUTTON, dialogExpData::OnBrowseExp)
END_EVENT_TABLE()

dialogExpData::dialogExpData(const char *pxi_name,PxiFrame *parent)
  :guiAutoDialog(parent,"Likelihood"), likename("like.out"), pxiname(pxi_name), 
   expname(""), frame(parent), likefile(0), 
   expfile(0),pxifile(0)
{
  //  likename=new char[250];strcpy(likename,"like.out");
  //  pxiname=new char[250];strcpy(pxiname,pxi_name);
  //  expname=new char[250];strcpy(expname,"");

  save_like=TRUE;load_now=FALSE;
  
  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);

  wxBoxSizer *pxinameSizer = new wxBoxSizer(wxHORIZONTAL);
  pxinameSizer->Add(new wxStaticText(this, -1, "PxiName"), 0,
		     wxCENTER | wxALL, 5);
  m_pxiName = new wxTextCtrl(this, -1, pxiname);
  pxinameSizer->Add(m_pxiName, 1, wxEXPAND | wxALL, 5);
  wxButton *browse1Button = new wxButton(this, idEXPDATA_BROWSE1_BUTTON, "Browse...");
  pxinameSizer->Add(browse1Button, 0, wxALL, 5);

  wxBoxSizer *expnameSizer = new wxBoxSizer(wxHORIZONTAL);
  expnameSizer->Add(new wxStaticText(this, -1, "ExpName"), 0,
		     wxCENTER | wxALL, 5);
  m_expName = new wxTextCtrl(this, -1, expname);
  expnameSizer->Add(m_expName, 1, wxEXPAND | wxALL, 5);
  wxButton *browse2Button = new wxButton(this, idEXPDATA_BROWSE2_BUTTON, "Browse...");
  expnameSizer->Add(browse2Button, 0, wxALL, 5);

  wxBoxSizer *likenameSizer = new wxBoxSizer(wxHORIZONTAL);
  likenameSizer->Add(new wxStaticText(this, -1, "Likename"), 0,
		     wxCENTER | wxALL, 5);
  m_likeName = new wxTextCtrl(this, -1, likename);
  likenameSizer->Add(m_likeName, 1, wxEXPAND | wxALL, 5);
  m_saveLike = new wxCheckBox(this, -1, "Save Like");
  m_saveLike->SetValue(true);
  likenameSizer->Add(m_saveLike, 1, wxEXPAND | wxALL, 5);

  m_loadNow = new wxCheckBox(this, -1, "Load Now");
  m_loadNow->SetValue(false);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  allSizer->Add(pxinameSizer, 1, wxEXPAND | wxALL, 5);
  allSizer->Add(expnameSizer, 1, wxEXPAND | wxALL, 5);
  allSizer->Add(likenameSizer, 1, wxEXPAND | wxALL, 5);
  allSizer->Add(m_loadNow, 1, wxEXPAND | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);

  SetAutoLayout(TRUE);
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();

#ifdef UNUSED
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
#endif //UNUSED
}

void dialogExpData::OnBrowsePxi(wxCommandEvent &) 
{
  Enable(false); // Don't allow anything while the dialog is up.
  wxString file = wxFileSelector("Load Data File", 
				    wxPathOnly(m_pxiName->GetValue()),
				    wxFileNameFromPath(m_pxiName->GetValue()),
				    ".pxi", "*.pxi");
  Enable(true);

  if (file) {
    m_pxiName->SetValue(file);
  }
}

void dialogExpData::OnBrowseExp(wxCommandEvent &) 
{
  wxString file = wxFileSelector("Load Observation File", 
				    wxPathOnly(m_pxiName->GetValue()),
				    wxFileNameFromPath(m_pxiName->GetValue()),
				    ".agg", "*.agg");

  if (file) {
    m_expName->SetValue(file);
  }
}

#ifdef UNUSED
void dialogExpData::browse_pxi_func(wxButton &ob,wxEvent &)
{
  char *s=wxFileSelector("Load data file", NULL, NULL, NULL, "*.out");
  if (s) ((wxText *)ob.GetClientData())->SetValue(s);
}

void dialogExpData::browse_exp_func(wxButton &ob,wxEvent &)
{
  char *s=wxFileSelector("Load observation file", NULL, NULL, NULL, "*.exp");
  if (s) ((wxText *)ob.GetClientData())->SetValue(s);
}

void dialogExpData::save_item_func(wxCheckBox &ob,wxEvent &)
{((wxString *)ob.GetClientData())->Enable(ob.GetValue());}
#endif // UNUSED

void dialogExpData::GetParams(ExpDataParams &P)
{
  expname = m_expName->GetValue();
  pxiname = m_pxiName->GetValue();
  likename = m_likeName->GetValue();
  assert(pxiname && expname);
  pxifile=new gFileInput(pxiname);expfile=new gFileInput(expname);
  P.pxifile=pxifile;P.expfile=expfile;
  if (likename && save_like) {
    likefile=new gFileOutput(likename);
    P.likefile=likefile;
  }
}

void dialogExpData::LoadNow(void)
{
  if (likefile && load_now) {
    delete likefile;likefile=0;
    frame->LoadFile(likename);
    //    frame->MakeChild(likename);
  }
}

dialogExpData::~dialogExpData(void)
{
  if (pxifile) delete pxifile;
  if (expfile) delete expfile;
  if (likefile) delete likefile;
}

