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
    frame->LoadFile(likename);
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
//     dialogDrawSettings
//
// ----------------------------------------------------------------------

BEGIN_EVENT_TABLE(dialogDrawSettings, guiAutoDialog)
  EVT_LISTBOX(idSETTINGS_WHICH_PLOT_LISTBOX, dialogDrawSettings::OnWhichPlot)
  EVT_LISTBOX(idSETTINGS_WHICH_INFOSET_LISTBOX, dialogDrawSettings::OnWhichInfoset)
  EVT_LISTBOX(idSETTINGS_ACTION_LISTBOX, dialogDrawSettings::OnAction)
  EVT_BUTTON(idSETTINGS_OVERLAY_BUTTON, dialogDrawSettings::OnOverlay)
  EVT_BUTTON(idSETTINGS_PLOT_BUTTON, dialogDrawSettings::OnPlot)
  EVT_RADIOBOX(idSETTINGS_PLOT_MODE, dialogDrawSettings::OnPlotMode)
END_EVENT_TABLE()

dialogDrawSettings::dialogDrawSettings(wxWindow *p_parent, PxiDrawSettings &s)
  : guiAutoDialog(p_parent, "Draw Settings"), 
    draw_settings(s), whichiset(1)
{
  wxString tmp;
  int num_plots = 1;
  if(draw_settings.GetPlotsPerPage()==2) num_plots = 2;
  
  m_plotItem = new wxListBox(this, idSETTINGS_WHICH_PLOT_LISTBOX);
  for (int i = 1; i <= draw_settings.GetNumPlots(); i++) {
    tmp.Printf("Plot %d", i);
    m_plotItem->Append(tmp);
  }
  m_plotItem->SetSelection(0);
  const PlotInfo &thisplot(draw_settings.GetPlotInfo(1));
  
  m_isetItem = new wxListBox(this, idSETTINGS_WHICH_INFOSET_LISTBOX, 
				  wxDefaultPosition, wxDefaultSize,0,0,
#ifdef __WXGTK__  // the wxGTK multiple-selection listbox is flaky (2.1.11)
				  wxLB_EXTENDED
#else
				  wxLB_MULTIPLE
#endif // __WXGTK__
				  );
  m_actionItem = new wxListBox(this, idSETTINGS_ACTION_LISTBOX, 
			       wxDefaultPosition, wxDefaultSize,0,0,
#ifdef __WXGTK__  // the wxGTK multiple-selection listbox is flaky (2.1.11)
			       wxLB_EXTENDED
#else
                               wxLB_MULTIPLE
#endif // __WXGTK__
			       );
  tmp.Printf( "%f", thisplot.GetMinX());
  m_minLam = new wxNumberItem(this, "minLam", tmp);
  tmp.Printf( "%f", thisplot.GetMaxX());
  m_maxLam = new wxNumberItem(this, "maxLam", tmp);
  tmp.Printf( "%f", thisplot.GetMinY());
  m_minY = new wxNumberItem(this, "minY", tmp);
  tmp.Printf( "%f", thisplot.GetMaxY());
  m_maxY = new wxNumberItem(this, "maxY", tmp);
  
  wxBoxSizer *lambdaSizer = new wxBoxSizer(wxHORIZONTAL);
  lambdaSizer->Add(new wxStaticText(this, -1, "X Min"),
		   0, wxALL | wxCENTER, 5);
  lambdaSizer->Add(m_minLam, 0, wxALL, 5);
  lambdaSizer->Add(new wxStaticText(this, -1, "X Max"),
		   0, wxALL | wxCENTER, 5);
  lambdaSizer->Add(m_maxLam, 0, wxALL, 5);
  
  wxBoxSizer *dataSizer = new wxBoxSizer(wxHORIZONTAL);
  dataSizer->Add(new wxStaticText(this, -1, "Y Min"),
		   0, wxALL | wxCENTER, 5);
  dataSizer->Add(m_minY, 0, wxALL, 5);
  dataSizer->Add(new wxStaticText(this, -1, "Y Max"),
		   0, wxALL | wxCENTER, 5);
  dataSizer->Add(m_maxY, 0, wxALL, 5);
  
  wxBoxSizer *plotSizer = new wxBoxSizer(wxHORIZONTAL);
  plotSizer->Add(m_plotItem, 0, wxCENTRE | wxALL, 5);
  plotSizer->Add(m_isetItem, 0, wxCENTRE | wxALL, 5);
  plotSizer->Add(m_actionItem, 0, wxCENTRE | wxALL, 5);
  
  wxString plotModeChoices[] = { "Plot X", "Plot 2", "Plot 3" };
  m_plotMode = new wxRadioBox(this, idSETTINGS_PLOT_MODE,"Plot Mode",
			      wxDefaultPosition, 
#ifdef __WXMOTIF__ // bug in wxmotif
			      wxSize(250,25),3, plotModeChoices, 1, wxRA_SPECIFY_ROWS
#else
			      wxDefaultSize,3, plotModeChoices, 1, wxRA_SPECIFY_ROWS
#endif
			      );

  m_plotMode->SetSelection(thisplot.GetPlotMode());
  
  wxString colorModeChoices[] = { "Equ", "Prob", "None" };
  m_colorMode = new wxRadioBox(this, -1, "Color Mode",
			       wxDefaultPosition, 
#ifdef __WXMOTIF__ // bug in wxmotif
			       wxSize(250,25),3, colorModeChoices, 1, wxRA_SPECIFY_ROWS
#else
			       wxDefaultSize,3, colorModeChoices, 1, wxRA_SPECIFY_ROWS
#endif
			       );  
  switch(draw_settings.GetColorMode()) {
  case COLOR_EQU: 
    m_colorMode->SetSelection(0);
    break;
  case COLOR_PROB:
    m_colorMode->SetSelection(1);
    break;
  case COLOR_NONE: 
    m_colorMode->SetSelection(2);
    break;
  }

  wxBoxSizer *modeSizer = new wxBoxSizer(wxHORIZONTAL);
  modeSizer->Add(m_plotMode, 0, wxALL, 5);
  modeSizer->Add(m_colorMode, 0, wxALL, 5);
  
  m_overlayButton = new wxButton(this, idSETTINGS_OVERLAY_BUTTON, "Overlay");
  m_plotButton = new wxButton(this, idSETTINGS_PLOT_BUTTON, "Plot");
  
  m_twoPlots = new wxCheckBox(this, -1, "Two Plots");
  if(draw_settings.GetPlotsPerPage()==2)
    m_twoPlots->SetValue(true);
  else
    m_twoPlots->SetValue(false);

  m_connectDots = new wxCheckBox(this, -1, "Connect Dots");
  m_connectDots->SetValue(draw_settings.ConnectDots());

  m_restartColors = new wxCheckBox(this, -1, "Restart Colors");
  m_restartColors->SetValue(draw_settings.RestartOverlayColors());

  wxBoxSizer *miscSizer = new wxBoxSizer(wxHORIZONTAL);
  miscSizer->Add(m_twoPlots, 0, wxALL, 5);
  miscSizer->Add(m_connectDots, 0, wxALL, 5);
  miscSizer->Add(m_restartColors, 0, wxALL, 5);

  wxBoxSizer *botSizer = new wxBoxSizer(wxHORIZONTAL);
  botSizer->Add(m_overlayButton, 0, wxALL, 5);
  botSizer->Add(m_plotButton, 0, wxALL, 5);

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  allSizer->Add(new wxStaticText(this, -1, "Display Range:"), 0, wxCENTRE | wxALL, 5);
  allSizer->Add(lambdaSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(dataSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(plotSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(modeSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(miscSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(botSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(m_buttonSizer, 0, wxCENTRE | wxALL, 5);
  
  SetAutoLayout(TRUE);
  SetSizer(allSizer); 
  allSizer->Fit(this);
  allSizer->SetSizeHints(this); 
  Layout();
  Run();
}

dialogDrawSettings::~dialogDrawSettings()
{ }

PlotInfo & dialogDrawSettings::ThisPlot(void)
{
  return draw_settings.GetPlotInfo(m_plotItem->GetSelection()+1);
}

void dialogDrawSettings::OnWhichPlot(wxCommandEvent &ev)
{
  wxString tmp;
  const PlotInfo &thisplot(ThisPlot());

  m_isetItem->Clear();
  for (int iset = 1; iset <= thisplot.GetNumIsets(); iset++) {
    tmp.Printf("Infoset %d", iset);
    m_isetItem->Append(tmp);
  }
  bool mark = true;
  whichiset = 0;
  for (int iset = 1; iset <= thisplot.GetNumIsets(); iset++) {
    if (thisplot.Contains(iset)) {
      m_isetItem->SetSelection(iset-1, true);
      if(mark) {whichiset = iset; mark = false;}
    }
  }

  m_plotMode->SetSelection(thisplot.GetPlotMode());
  m_plotMode->Show(true);

  m_actionItem->Clear();
  if(whichiset) {
    for (int act = 1; act <= thisplot.GetNumStrats(whichiset); act++) {
      tmp.Printf("Action %d", act);
      m_actionItem->Append(tmp);
    }
    
    for (int act = 1; act <= thisplot.GetNumStrats(whichiset); act++) 
      if(thisplot.GetStrategyShow(whichiset, act))
	m_actionItem->SetSelection(act - 1, true);
  }
}  

void dialogDrawSettings::OnPlotMode(wxCommandEvent &)
{
  switch(m_plotMode->GetSelection()) {
  case 0: 
    ThisPlot().SetPlotMode(PXI_PLOT_X);
    break;
  case 1: 
    ThisPlot().SetPlotMode(PXI_PLOT_2);
    break;
  case 2: 
    ThisPlot().SetPlotMode(PXI_PLOT_3);
    break;
  }
}

void dialogDrawSettings::OnWhichInfoset(wxCommandEvent &)
{
  PlotInfo &thisplot(ThisPlot());
  bool flag = false;
  wxString tmp;

  for (int iset = 1; iset <= thisplot.GetNumIsets(); iset++) {
    bool selected = m_isetItem->Selected(iset-1);
    bool member = thisplot.Contains(iset);
    if (selected && !member) {
      thisplot.AddInfoset(iset);
      flag = true;
      whichiset = iset;
    }
    else if (!selected && member) {
      thisplot.RemoveInfoset(iset);
    } 
  }
  
  if(flag) {
    m_actionItem->Clear();
    for (int act = 1; act <= thisplot.GetNumStrats(whichiset); act++) {
      tmp.Printf("Action %d", act);
      m_actionItem->Append(tmp);
    }
    
    for (int act = 1; act <= thisplot.GetNumStrats(whichiset); act++) 
      if(thisplot.GetStrategyShow(whichiset, act))
	m_actionItem->SetSelection(act - 1, true);

    wxCommandEvent event;
    OnAction(event);
  }
}

void dialogDrawSettings::OnAction(wxCommandEvent &)
{ 
  PlotInfo &thisplot(ThisPlot());

  for (int act = 1; act <= thisplot.GetNumStrats(whichiset); act++) 
    thisplot.SetStrategyShow(whichiset, act, m_actionItem->Selected(act-1));
}

void dialogDrawSettings::OnOverlay(wxCommandEvent &)
{
  dialogOverlayOptions dialog(this, draw_settings);
}

/*
void dialogDrawSettings::OnFont(wxCommandEvent &)
{ 
  //  draw_settings.label_font_func();
}
*/

void dialogDrawSettings::OnPlot(wxCommandEvent &)
{ 
  dialogPlotOptions dialog(this, ThisPlot());
}

void dialogDrawSettings::Run()
{ 
  wxCommandEvent event;
  OnWhichPlot(event);

  PlotInfo &thisplot(ThisPlot());

  if(ShowModal() == wxID_OK){ 
    thisplot.SetMinX(m_minLam->GetNumber());
    thisplot.SetMaxX(m_maxLam->GetNumber());
    thisplot.SetMinY(m_minY->GetNumber());
    thisplot.SetMaxY(m_maxY->GetNumber());

    /*
    int mode = m_plotMode->GetSelection();
    if(mode==0)thisplot.SetPlotMode(PXI_PLOT_X);
    if(mode==1)thisplot.SetPlotMode(PXI_PLOT_2);
    if(mode==2)thisplot.SetPlotMode(PXI_PLOT_3);
    */

    int mode = m_colorMode->GetSelection();
    if(mode==0)draw_settings.SetColorMode(COLOR_EQU);
    if(mode==1)draw_settings.SetColorMode(COLOR_PROB);
    if(mode==2)draw_settings.SetColorMode(COLOR_NONE);

    draw_settings.SetPlotsPerPage(m_twoPlots->GetValue()+1);
    draw_settings.SetConnectDots(m_connectDots->GetValue());
    draw_settings.SetRestartOverlayColors(m_restartColors->GetValue());
  };
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


