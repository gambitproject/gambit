//
// File: expdprm.c -- definitions for experiment data dialog class
//

#include "dlgpxi.h"
#include "axis.h"


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
  EVT_LISTBOX(idSETTINGS_INFOSET_LISTBOX, dialogDrawSettings::OnInfoset)
  EVT_LISTBOX(idSETTINGS_ACTION_LISTBOX, dialogDrawSettings::OnAction)
  EVT_BUTTON(idSETTINGS_OVERLAY_BUTTON, dialogDrawSettings::OnOverlay)
  EVT_BUTTON(idSETTINGS_FONT_BUTTON, dialogDrawSettings::OnFont)
  EVT_BUTTON(idSETTINGS_PLOT_BUTTON, dialogDrawSettings::OnPlot)
END_EVENT_TABLE()

dialogDrawSettings::dialogDrawSettings(wxWindow *p_parent, PxiDrawSettings &s)
  : guiAutoDialog(p_parent, "Draw Settings"), 
    draw_settings(s)
{
  int num_plots = 1;
  if(draw_settings.one_or_two==2) num_plots = 2;
  
  m_whichPlotItem = new wxListBox(this, idSETTINGS_WHICH_PLOT_LISTBOX);
  for (int i = 1; i <= draw_settings.GetNumPlots(); i++) {
    char tmp[80];
    sprintf(tmp, "Plot %d", i);
    m_whichPlotItem->Append(tmp);
  }
  m_whichPlotItem->SetSelection(0);
  
  m_whichIsetItem = new wxListBox(this, idSETTINGS_WHICH_INFOSET_LISTBOX, 
				  wxDefaultPosition, wxDefaultSize,0,0,
#ifdef __WXGTK__  // the wxGTK multiple-selection listbox is flaky (2.1.11)
				  wxLB_EXTENDED
#else
				  wxLB_MULTIPLE
#endif // __WXGTK__
				  );
  wxCommandEvent event;
  OnWhichPlot(event);
  wxString tmp;

  m_infosetItem = new wxListBox(this, idSETTINGS_INFOSET_LISTBOX);
  for (int iset = 1; iset <= draw_settings.num_infosets; iset++) {
    tmp.Printf("Infoset %d", iset);
    m_infosetItem->Append(tmp);
  }
  m_infosetItem->SetSelection(0);
  
  m_actionItem = new wxListBox(this, idSETTINGS_ACTION_LISTBOX, 
			       wxDefaultPosition, wxDefaultSize,0,0,
#ifdef __WXGTK__  // the wxGTK multiple-selection listbox is flaky (2.1.11)
			       wxLB_EXTENDED
#else
                               wxLB_MULTIPLE
#endif // __WXGTK__
			       );
  OnInfoset(event);
  
  tmp.Printf( "%f", draw_settings.stop_min);
  m_minLam = new wxNumberItem(this, "minLam", tmp);
  tmp.Printf( "%f", draw_settings.stop_max);
  m_maxLam = new wxNumberItem(this, "maxLam", tmp);
  tmp.Printf( "%f", draw_settings.data_min);
  m_minY = new wxNumberItem(this, "minLam", tmp);
  tmp.Printf( "%f", draw_settings.data_max);
  m_maxY = new wxNumberItem(this, "maxLam", tmp);
  
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
  plotSizer->Add(m_whichPlotItem, 0, wxCENTRE | wxALL, 5);
  plotSizer->Add(m_whichIsetItem, 0, wxCENTRE | wxALL, 5);
  
  wxBoxSizer *selectSizer = new wxBoxSizer(wxHORIZONTAL);
  selectSizer->Add(m_infosetItem, 0, wxCENTRE | wxALL, 5);
  selectSizer->Add(m_actionItem, 0, wxCENTRE | wxALL, 5);
  
  wxString plotModeChoices[] = { "Plot X", "Plot 2", "Plot 3" };
  m_plotMode = new wxRadioBox(this, -1, "Plot Mode",
			      wxDefaultPosition, 
#ifdef __WXMOTIF__ // bug in wxmotif
			      wxSize(250,25),3, plotModeChoices, 0, wxRA_SPECIFY_COLS
#else
			      wxDefaultSize,3, plotModeChoices, 0, wxRA_SPECIFY_ROWS
#endif
			      );
  if (draw_settings.plot_mode == PXI_PLOT_X) 
    m_plotMode->SetSelection(0);
  else if (draw_settings.plot_mode == PXI_PLOT_2) 
    m_plotMode->SetSelection(1);
  else if (draw_settings.plot_mode == PXI_PLOT_3) 
    m_plotMode->SetSelection(2);
  
  wxString colorModeChoices[] = { "Equ", "Prob", "None" };
  m_colorMode = new wxRadioBox(this, -1, "Color Mode",
			       wxDefaultPosition, 
#ifdef __WXMOTIF__ // bug in wxmotif
			       wxSize(250,25),3, colorModeChoices, 0, wxRA_SPECIFY_COLS
#else
			       wxDefaultSize,3, colorModeChoices, 0, wxRA_SPECIFY_ROWS
#endif
			       );  
  if (draw_settings.color_mode == COLOR_EQU) 
    m_colorMode->SetSelection(0);
  else if (draw_settings.color_mode == COLOR_PROB) 
    m_colorMode->SetSelection(1);
  else if (draw_settings.color_mode == COLOR_NONE) 
    m_colorMode->SetSelection(2);
  
  wxBoxSizer *modeSizer = new wxBoxSizer(wxHORIZONTAL);
  modeSizer->Add(m_plotMode, 0, wxALL, 5);
  modeSizer->Add(m_colorMode, 0, wxALL, 5);
  
  m_overlayButton = new wxButton(this, idSETTINGS_OVERLAY_BUTTON, "Overlay");
  m_fontButton = new wxButton(this, idSETTINGS_FONT_BUTTON, "Font");
  m_plotButton = new wxButton(this, idSETTINGS_PLOT_BUTTON, "Plot");
  
  m_twoPlots = new wxCheckBox(this, -1, "Two Plots");
  if(draw_settings.one_or_two==2)
    m_twoPlots->SetValue(true);
  else
    m_twoPlots->SetValue(false);

  m_connectDots = new wxCheckBox(this, -1, "Connect Dots");
  m_connectDots->SetValue(draw_settings.ConnectDots());

  m_restartColors = new wxCheckBox(this, -1, "Restart Colors");
  m_restartColors->SetValue(draw_settings.restart_overlay_colors);

  wxBoxSizer *miscSizer = new wxBoxSizer(wxHORIZONTAL);
  miscSizer->Add(m_twoPlots, 0, wxALL, 5);
  miscSizer->Add(m_connectDots, 0, wxALL, 5);
  miscSizer->Add(m_restartColors, 0, wxALL, 5);

  wxBoxSizer *botSizer = new wxBoxSizer(wxHORIZONTAL);
  botSizer->Add(m_overlayButton, 0, wxALL, 5);
  botSizer->Add(m_fontButton, 0, wxALL, 5);
  botSizer->Add(m_plotButton, 0, wxALL, 5);

  wxBoxSizer *allSizer = new wxBoxSizer(wxVERTICAL);
  allSizer->Add(new wxStaticText(this, -1, "Display Range:"), 0, wxCENTRE | wxALL, 5);
  allSizer->Add(lambdaSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(dataSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(plotSizer, 0, wxCENTRE | wxALL, 5);
  allSizer->Add(selectSizer, 0, wxCENTRE | wxALL, 5);
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

void dialogDrawSettings::OnWhichPlot(wxCommandEvent &)
{
  int whichplot = m_whichPlotItem->GetSelection()+1; 

  m_whichIsetItem->Clear();
  for (int iset = 1; iset <= draw_settings.num_infosets; iset++) {
    char tmp[80];
    sprintf(tmp, "Infoset %d", iset);
    m_whichIsetItem->Append(tmp);
  }
  for (int iset = 1; iset <= draw_settings.num_infosets; iset++) {
    if (draw_settings.GetMyPlot(whichplot).Contains(iset)) 
      m_whichIsetItem->SetSelection(iset-1, true);
  }
}

void dialogDrawSettings::OnWhichInfoset(wxCommandEvent &)
{
  int whichplot = m_whichPlotItem->GetSelection()+1; 
  int j = 0;

  for (int iset = 1; iset <= draw_settings.num_infosets; iset++) {
    bool flag = m_whichIsetItem->Selected(iset-1);
    bool member = draw_settings.GetMyPlot(whichplot).Contains(iset);
    if (flag && !member) {
      draw_settings.GetMyPlot(whichplot).Append(iset);
      j = iset;
    }
    else if (!flag && member) 
      draw_settings.GetMyPlot(whichplot).Remove(draw_settings.GetMyPlot(whichplot).Find(iset));
  }

  if(j) {
    m_infosetItem->SetSelection(j-1);
    wxCommandEvent event;
    OnInfoset(event);
  }
}

void dialogDrawSettings::OnInfoset(wxCommandEvent &)
{
  int iset = m_infosetItem->GetSelection()+1; 

  m_actionItem->Clear();
  for (int act = 1; act <= draw_settings.strategy_show[iset].Length(); act++) {
    char tmp[80];
    sprintf(tmp, "Action %d", act);
    m_actionItem->Append(tmp);
  }
  for (int act = 1; act <= draw_settings.strategy_show[iset].Length(); act++) {
    if(draw_settings.strategy_show[iset][act])
      m_actionItem->SetSelection(act - 1, true);
  }
  m_actionItem->Enable(true);
}

void dialogDrawSettings::OnAction(wxCommandEvent &)
{ 
  int iset = m_infosetItem->GetSelection()+1; 

  for (int act = 1; act <= draw_settings.strategy_show[iset].Length(); act++) 
    draw_settings.strategy_show[iset][act] = m_actionItem->Selected(act-1);
  
}

void dialogDrawSettings::OnOverlay(wxCommandEvent &)
{
  dialogOverlayOptions dialog(this, draw_settings);
}

void dialogDrawSettings::OnFont(wxCommandEvent &)
{ 
  //  draw_settings.label_font_func();
}

void dialogDrawSettings::OnPlot(wxCommandEvent &)
{ 
  dialogPlotOptions dialog(this, draw_settings);
}

void dialogDrawSettings::Run()
{ 
  if(ShowModal() == wxID_OK){ 
    draw_settings.stop_min = m_minLam->GetNumber();
    draw_settings.stop_max = m_maxLam->GetNumber();
    draw_settings.data_min =  m_minY->GetNumber();
    draw_settings.data_max = m_maxY->GetNumber();

    int mode = m_plotMode->GetSelection();
    if(mode==0)draw_settings.plot_mode=PXI_PLOT_X;
    if(mode==1)draw_settings.plot_mode=PXI_PLOT_2;
    if(mode==2)draw_settings.plot_mode=PXI_PLOT_3;

    mode = m_colorMode->GetSelection();
    if(mode==0)draw_settings.color_mode=COLOR_EQU;
    if(mode==1)draw_settings.color_mode=COLOR_PROB;
    if(mode==2)draw_settings.color_mode=COLOR_NONE;

    draw_settings.SetPlotsPerPage(m_twoPlots->GetValue()+1);
    draw_settings.connect_dots = m_connectDots->GetValue();
    draw_settings.restart_overlay_colors = m_restartColors->GetValue();
  };
}

// ----------------------------------------------------------------------
//
//     dialogPlotOptions
//
// ----------------------------------------------------------------------

dialogPlotOptions::dialogPlotOptions(wxWindow *p_parent, PxiDrawSettings &s)
  : guiAutoDialog(p_parent, "Plot Options"),  draw_settings(s),
    m_axis(this, -1, "Draw Axis"), m_labels(this, -1, "Draw Labels"), 
    m_ticks(this, -1, "Draw Ticks"), m_nums(this, -1, "Draw Nums"), 
    m_square(this, -1, "Square Axis")
{
  unsigned int feat = draw_settings.PlotFeatures();
  int type = draw_settings.GetPlotMode();

  m_axis.SetValue((feat&DRAW_AXIS) ? true : false);
  m_labels.SetValue((feat&DRAW_LABELS) ? true : false);
  m_ticks.SetValue((feat&DRAW_TICKS) ? true : false);
  m_nums.SetValue((feat&DRAW_NUMS) ? true : false);
  m_square.SetValue((feat&DRAW_SQUARE) ? true : false);

  switch (type) {
  case PXI_PLOT_3: 
    m_ticks.Enable(false);
    m_square.Enable(false);
    m_nums.Enable(false);
    break;
  case PXI_PLOT_X: 
    m_labels.Enable(false);
    m_square.Enable(false);
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
  unsigned int feat=draw_settings.PlotFeatures();
  
  if (ShowModal() == wxID_OK) {
    feat=0;
    switch (draw_settings.GetPlotMode()) {
    case PXI_PLOT_3: 
      if (m_axis.GetValue()) feat|=DRAW_AXIS;
      if (m_labels.GetValue()) feat|=DRAW_LABELS;
      break;
    case PXI_PLOT_X: 
      if (m_axis.GetValue()) feat|=DRAW_AXIS;
      if (m_ticks.GetValue()) feat|=DRAW_TICKS;
      if (m_nums.GetValue()) feat|=DRAW_NUMS;
      break;
    case PXI_PLOT_2: 
      if (m_axis.GetValue()) feat|=DRAW_AXIS;
      if (m_ticks.GetValue()) feat|=DRAW_TICKS;
      if (m_nums.GetValue()) feat|=DRAW_NUMS;
      if (m_square.GetValue()) feat|=DRAW_SQUARE;
      break;
    }
    draw_settings.SetPlotFeatures(feat);
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

