//
// File: expdprm.c -- definitions for experiment data dialog class
//

#include "dlgpxi.h"
#include "axis.h"
#include "pxichild.h"

// ----------------------------------------------------------------------
//
//     dialogOverlayData
//
// ----------------------------------------------------------------------

BEGIN_EVENT_TABLE(dialogOverlayData, wxDialog)
  EVT_BUTTON(idEXPDATA_BROWSE1_BUTTON, dialogOverlayData::OnBrowsePxi)
  EVT_BUTTON(idEXPDATA_BROWSE2_BUTTON, dialogOverlayData::OnBrowseExp)
END_EVENT_TABLE()

dialogOverlayData::dialogOverlayData(PxiChild *parent, PxiCanvas *p_canvas)
  :guiAutoDialog(parent,"Likelihood"), canvas(p_canvas), frame(parent),
   likename("like.out"), pxiname(p_canvas->PxiName()), 
   expname(""),  likefile(0), expfile(0),pxifile(0)
{
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
  Run();
}

void dialogOverlayData::OnBrowsePxi(wxCommandEvent &) 
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

void dialogOverlayData::OnBrowseExp(wxCommandEvent &) 
{
  wxString file = wxFileSelector("Load Observation File", 
				    wxPathOnly(m_pxiName->GetValue()),
				    wxFileNameFromPath(m_pxiName->GetValue()),
				    ".agg", "*.agg");

  if (file) {
    m_expName->SetValue(file);
  }
}

void dialogOverlayData::GetParams(ExpDataParams &P)
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

void dialogOverlayData::LoadNow(void)
{
  if (likefile && load_now) {
    delete likefile;likefile=0;
    //    frame->LoadFile(likename);
    //    frame->MakeChild(likename);
  }
}

dialogOverlayData::~dialogOverlayData(void)
{
  if (pxifile) delete pxifile;
  if (expfile) delete expfile;
  if (likefile) delete likefile;
}

void dialogOverlayData::Run(void)
{
  if (ShowModal() == wxID_OK) {
    ExpDataParams P;
    GetParams(P);
    canvas->NewExpData(P);
    //    if (canvas->exp_data) {delete canvas->exp_data;canvas->exp_data=NULL;}
    //    canvas->exp_data=new ExpData(P);
    LoadNow();
  }
}

// ----------------------------------------------------------------------
//
//     dialogPlotOptions
//
// ----------------------------------------------------------------------

dialogPlotOptions::dialogPlotOptions(wxWindow *p_parent, PlotInfo &plot)
  : guiAutoDialog(p_parent, "Plot Options"),  thisplot(plot),
    m_axis(this, -1, "Draw Axis"), m_labels(this, -1, "Draw Labels"), 
    m_ticks(this, -1, "Draw Ticks"), m_nums(this, -1, "Draw Nums"), 
    m_square(this, -1, "Square Axis")
{
  int type = thisplot.GetPlotMode();

  m_axis.SetValue(thisplot.ShowAxis());
  m_labels.SetValue(thisplot.ShowLabels());
  m_ticks.SetValue(thisplot.ShowTicks());
  m_nums.SetValue(thisplot.ShowNums());
  m_square.SetValue(thisplot.ShowSquare());

  switch (type) {
  case PXI_PLOT_3: 
    m_ticks.Enable(false);
    m_square.Enable(false);
    m_nums.Enable(false);
    break;
  case PXI_PLOT_X: 
    m_labels.Enable(false);
    m_square.Enable(false);
    break;
  case PXI_PLOT_2: 
    m_labels.Enable(false);
    break;
  }

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(&m_axis, 0, wxALL, 5);
  topSizer->Add(&m_labels, 0, wxALL, 5);
  topSizer->Add(&m_nums, 0, wxALL, 5);
  topSizer->Add(&m_ticks, 0, wxALL, 5);
  topSizer->Add(&m_square, 0, wxALL, 5);

  allSizer->Add(topSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetAutoLayout(TRUE);
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();
  Run();
}

dialogPlotOptions::~dialogPlotOptions()
{ }


void dialogPlotOptions::Run()
{
  if (ShowModal() == wxID_OK) {
    
    switch (thisplot.GetPlotMode()) {
    case PXI_PLOT_3: 
      thisplot.SetShowAxis(m_axis.GetValue());
      thisplot.SetShowLabels(m_labels.GetValue());
      break;
    case PXI_PLOT_X: 
      thisplot.SetShowAxis(m_axis.GetValue());
      thisplot.SetShowTicks(m_ticks.GetValue());
      thisplot.SetShowNums(m_nums.GetValue());
      break;
    case PXI_PLOT_2: 
      thisplot.SetShowAxis(m_axis.GetValue());
      thisplot.SetShowTicks(m_ticks.GetValue());
      thisplot.SetShowNums(m_nums.GetValue());
      thisplot.SetShowSquare(m_square.GetValue());
      break;
    }
  }
}

// ----------------------------------------------------------------------
//
//     dialogOverlayOptions
//
// ----------------------------------------------------------------------

dialogOverlayOptions::dialogOverlayOptions(wxWindow *p_parent, 
					   PxiDrawSettings &s)
  : guiAutoDialog(p_parent, "Overlay Options"), draw_settings(s)
{
  wxString tokenChoices[] = { "Numbers", "Tokens"};
  m_token = new wxRadioBox(this, -1, "Draw Axis", wxDefaultPosition, 
#ifdef __WXMOTIF__ // bug in wxmotif
			   wxSize(200,25),2, tokenChoices, 0, wxRA_SPECIFY_COLS
#else

			   wxDefaultSize,2, tokenChoices, 0, wxRA_SPECIFY_ROWS
#endif
			   );
  m_connect = new wxCheckBox(this, -1, "Connect Points"); 
  m_size = new wxSlider(this, -1, 4,2,10, wxDefaultPosition, 
#if defined(__WXMOTIF__) // bug in wxmotif
			wxSize(200,25),wxSL_HORIZONTAL | wxSL_LABELS
#elif defined(__WXGTK__) // bug in wxgtk
			wxSize(300,50), wxSL_HORIZONTAL | wxSL_LABELS
#else
			wxDefaultSize, wxSL_HORIZONTAL | wxSL_LABELS
#endif
			);

  // set initial values
  m_token->SetSelection(0);
  if(s.GetOverlaySym()==OVERLAY_TOKEN)
    m_token->SetSelection(1);
  
  m_connect->SetValue(false);
  if(s.GetOverlayLines())
    m_connect->SetValue(true);
  
  //  m_size.SetValue(s.GetTokenSize());
  
  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  allSizer->Add(m_token, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_connect, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_size, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetAutoLayout(TRUE);
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();
  Run();
}

dialogOverlayOptions::~dialogOverlayOptions()
{ }


void dialogOverlayOptions::Run()
{
  if (ShowModal() == wxID_OK) {
    draw_settings.SetOverlayLines(m_connect->GetValue());
    draw_settings.SetOverlaySym((m_token->GetSelection()==0) ? OVERLAY_NUMBER : OVERLAY_TOKEN);
    if(draw_settings.GetOverlaySym()==OVERLAY_TOKEN)
      draw_settings.SetTokenSize( m_size->GetValue());
  }
}


// ----------------------------------------------------------------------
//
//     dialogSelectScale
//
// ----------------------------------------------------------------------

dialogSelectScale::dialogSelectScale(wxWindow *p_parent, double scale)
  : guiAutoDialog(p_parent, "Change Scale")
{
  wxString scaleChoices[] = { ".25", 
			      ".50", 
			      ".75", 
			      "1.00", 
			      "1.25", 
			      "1.50", 
			      "1.75", 
			      "2.00" };
  m_size = new wxRadioBox(this, -1,"Plot Scale",
			      wxDefaultPosition, 
#ifdef __WXMOTIF__ // bug in wxmotif
			      wxSize(250,25),8, scaleChoices, 1, wxRA_SPECIFY_COLS
#else
			      wxDefaultSize,8, scaleChoices, 1, wxRA_SPECIFY_COLS
#endif
			      );


  double s;
  for(int i=0;i<m_size->Number();i++) {
    m_size->GetLabel(i).ToDouble(&s);
    if(scale == s)m_size->SetSelection(i);
  }
  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  allSizer->Add(m_size, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetAutoLayout(TRUE);
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();
}

dialogSelectScale::~dialogSelectScale()
{ }

double dialogSelectScale::GetScale(void)
{
  double scale;
  m_size->GetLabel(m_size->GetSelection()).ToDouble(&scale);
  return scale;
}


