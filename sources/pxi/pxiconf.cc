//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Implementation of plot configuration class
//

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // WX_PRECOMP

#include "pxiconf.h"
#include "dlgpxi.h"

PxiDrawSettings::PxiDrawSettings(const FileHeader &p_header, int p_index)
  : clear_brush("BLACK",wxTRANSPARENT),
    exp_data_brush("BLACK",wxSOLID),
    color_mode(COLOR_PROB),
    connect_dots(FALSE), restart_overlay_colors(TRUE)
{ }


// Plot3 mode is now more flexible: it works great (by default) on 2d player
// games, each player having 3 strategies.  It has now been extended to work
// with N player games with 3+ strategies each
// In this case, the two desired strategies are selected in the dialog, and
// the third is calculated from 1-(first+second).  You can also plot more than
// one infoset per triangle now.

bool PxiDrawSettings::CheckPlot3Mode(void)
{
#ifdef NOT_IMPLEMENTED
  int i;
  // Check if there are 3 or over strategy_show for each player
  int page = whichpage;
  int bottom = 2*page, top = 2*page-1;
  int t, b;

  for (t=1;t<=GetMyPlot(top).Length();t++)
    if (strategy_show[GetMyPlot(top)[t]].Length()<3) {
      wxMessageBox("Each player must have at least 3 strategies!");
      return FALSE;
    }
  for (b=1;b<=GetMyPlot(bottom).Length();b++)
    if (strategy_show[GetMyPlot(bottom)[b]].Length()<3) {
      wxMessageBox("Each player must have at least 3 strategies!");
      return FALSE;
    }
  
  // Check if only two strategies have been selected for each player (add all of them up)
  int num_strategies=0;
  for (t=1;t<=GetMyPlot(top).Length();t++)
    for (i=1;i<=strategy_show[GetMyPlot(top)[t]].Length();i++)
      num_strategies+=(GetStrategyShow(GetMyPlot(top)[t],i)) ? 1 : 0;
  for (b=1;b<=GetMyPlot(bottom).Length();b++)
    for (i=1;i<=strategy_show[GetMyPlot(bottom)[b]].Length();i++)
      num_strategies+=(GetStrategyShow(GetMyPlot(bottom)[b],i)) ? 1 : 0;
  if (num_strategies>2*(GetMyPlot(top).Length()+GetMyPlot(bottom).Length())) {
    wxMessageBox("Only two strategies per player allowed!");
    return FALSE;
  }
#endif // NOT_IMPLEMENTED
  return TRUE;
}

// Check Plot_2 Mode:  Plot 2 mode plots a player's strategy against another
// strategy (the same or different player's).  Only one pair of strategies
// can be plotted at any one time.  Thus, only two of the checkboxes in the
// strategy selection section can be turned on.

bool PxiDrawSettings::CheckPlot2Mode(void)
{
#ifdef NOT_IMPLEMENTED
  int i,j,num_strategies=0;
  for (j=1;j<=strategy_show.Length();j++)
    for (i=1;i<=strategy_show[j].Length();i++)
      num_strategies+=(GetStrategyShow(j,i)) ? 1 : 0;
  if (num_strategies>2) {
    wxMessageBox("Only TWO strategies TOTAL can be checked\nin the strategy selection dialog");
    return FALSE;
  }
#endif // NOT_IMPLEMENTED
  return TRUE;
}


const int idSETTINGS_WHICH_PLOT_LISTBOX = 2018;
const int idSETTINGS_WHICH_INFOSET_LISTBOX = 2019;
const int idSETTINGS_INFOSET_LISTBOX = 2020;
const int idSETTINGS_ACTION_LISTBOX = 2021;
const int idSETTINGS_OVERLAY_BUTTON = 2022;
const int idSETTINGS_PLOT_MODE = 2023;
const int idSETTINGS_PLOT_BUTTON = 2024;

class dialogDrawSettings : public guiAutoDialog {
private:
  PxiDrawSettings &draw_settings;       // draw settings, see above
  wxListBox *m_actionItem, *m_plotItem, *m_isetItem;
  wxTextCtrl *m_minLam, *m_maxLam, *m_minY, *m_maxY;
  wxButton *m_overlayButton, *m_plotButton;
  wxRadioBox *m_plotMode, *m_colorMode;
  wxCheckBox *m_twoPlots, *m_connectDots, *m_restartColors;
  
  int whichiset;
  
  void OnWhichPlot(wxCommandEvent &);
  void OnWhichInfoset(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);
  void OnAction(wxCommandEvent &);
  void OnOverlay(wxCommandEvent &);
  void OnPlot(wxCommandEvent &);
  void OnPlotMode(wxCommandEvent &);

  void Run(void);
public:
  dialogDrawSettings(wxWindow *, PxiDrawSettings &);
  virtual ~dialogDrawSettings();
  
  DECLARE_EVENT_TABLE()
};

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
  //  int num_plots = 1;
  
  m_plotItem = new wxListBox(this, idSETTINGS_WHICH_PLOT_LISTBOX);
  for (int i = 1; i <= 1; i++) {
    tmp.Printf("Plot %d", i);
    m_plotItem->Append(tmp);
  }
  m_plotItem->SetSelection(0);
  //  const PlotInfo &thisplot(draw_settings.GetPlotInfo());
  
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
  //  tmp.Printf( "%f", thisplot.GetMinX());
  m_minLam = new wxTextCtrl(this, -1, tmp);
  // tmp.Printf( "%f", thisplot.GetMaxX());
  m_maxLam = new wxTextCtrl(this, -1, tmp);
  //  tmp.Printf( "%f", thisplot.GetMinY());
  m_minY = new wxTextCtrl(this, -1, tmp);
  // tmp.Printf( "%f", thisplot.GetMaxY());
  m_maxY = new wxTextCtrl(this, -1, tmp);
  
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

  //  m_plotMode->SetSelection(thisplot.GetPlotMode());
  
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
  
  // The multi-plot feature is undergoing changes; disable this item
  m_twoPlots = new wxCheckBox(this, -1, "Two Plots");
  m_twoPlots->SetValue(false);
  m_twoPlots->Enable(false);

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

void dialogDrawSettings::OnWhichPlot(wxCommandEvent &ev)
{
#ifdef NOT_PORTED_YET
  wxString tmp;
  const PlotInfo &thisplot(ThisPlot());

  m_isetItem->Clear();
  for (int iset = 1; iset <= thisplot.GetNumIsets(); iset++) {
    tmp.Printf("Infoset %d", iset);
    m_isetItem->Append(tmp);
  }
  bool mark = true;
  whichiset = 0;

  //  m_isetItem->SetSelection(thisplot.GetPlotNumber()-1, true);
  if (mark) {
    //    whichiset = thisplot.GetPlotNumber()-1; 
    mark = false;
  }

  //  m_plotMode->SetSelection(thisplot.GetPlotMode());
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
#endif  // NOT_PORTED_YET
}  

void dialogDrawSettings::OnPlotMode(wxCommandEvent &)
{
  /*
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
  */
}

void dialogDrawSettings::OnWhichInfoset(wxCommandEvent &)
{
#ifdef NOT_PORTED_YET
  PlotInfo &thisplot(ThisPlot());
  bool flag = false;
  wxString tmp;

  //  whichiset = thisplot.GetPlotNumber();
  
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
#endif // NOT_PORTED_YET
}

void dialogDrawSettings::OnAction(wxCommandEvent &)
{ 
#ifdef NOT_PORTED_YET
  PlotInfo &thisplot(ThisPlot());

  for (int act = 1; act <= thisplot.GetNumStrats(whichiset); act++) 
    thisplot.SetStrategyShow(whichiset, act, m_actionItem->Selected(act-1));
#endif  // NOT_PORTED_YET
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
  //  dialogPlotOptions dialog(this, ThisPlot());
}

void dialogDrawSettings::Run()
{ 
  wxCommandEvent event;
  OnWhichPlot(event);

  //  PlotInfo &thisplot(ThisPlot());

  if (ShowModal() == wxID_OK){ 
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

    draw_settings.SetConnectDots(m_connectDots->GetValue());
    draw_settings.SetRestartOverlayColors(m_restartColors->GetValue());
  };
}


void PxiDrawSettings::SetOptions(wxWindow *parent)
{
  dialogDrawSettings dialog(parent, *this);
}

