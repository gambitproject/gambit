//
// File: expdprm.c -- definitions for experiment data dialog class
//

#ifndef EXPDPRM_H
#define EXPDPRM_H

#include "wxmisc.h"
#include "gambitio.h"
#include "pxi.h"
#include "expdata.h"
#include "wx/slider.h"

// ----------------------------------------------------------------------
//
//     dialogOverlayData
//
// ----------------------------------------------------------------------

class dialogOverlayData: public guiAutoDialog
{
private:
  PxiCanvas *canvas;
  PxiChild *frame;
  
  wxString likename, pxiname, expname;
  gOutput *likefile;
  gInput *pxifile,*expfile;
  int save_like,load_now;
  //PxiFrame *frame;
  wxTextCtrl *m_pxiName, *m_expName, *m_likeName;
  wxCheckBox *m_saveLike, *m_loadNow;
  
  void OnBrowsePxi(wxCommandEvent &);
  void OnBrowseExp(wxCommandEvent &);
  
  void Run(void);
public:
  // Constructor
  dialogOverlayData(PxiChild *, PxiCanvas *);
  ~dialogOverlayData(void);
  
  // Data access
  void	GetParams(ExpDataParams &P);
  void	LoadNow(void);
  
  DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------
//
//     dialogDrawSettings
//
// ----------------------------------------------------------------------

class dialogDrawSettings : public guiAutoDialog {
private:
  PxiDrawSettings &draw_settings;       // draw settings, see above
  wxListBox *m_actionItem, *m_plotItem, *m_isetItem;
  wxNumberItem *m_minLam, *m_maxLam, *m_minY, *m_maxY;
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
  PlotInfo & ThisPlot(void);

  void Run(void);
public:
  dialogDrawSettings(wxWindow *, PxiDrawSettings &);
  virtual ~dialogDrawSettings();
  
  DECLARE_EVENT_TABLE()
};

// ----------------------------------------------------------------------
//
//     dialogPlotOptions
//
// ----------------------------------------------------------------------

class dialogPlotOptions : public guiAutoDialog {
private:
  PlotInfo &thisplot;       // information about plot
  wxCheckBox m_axis, m_labels, m_ticks, m_nums, m_square;

  const char *HelpString(void) const { return "Plot Options"; }
  
  void Run(void);
public:
  dialogPlotOptions(wxWindow *, PlotInfo &);
  virtual ~dialogPlotOptions();
};

// ----------------------------------------------------------------------
//
//     dialogOverlayOptions
//
// ----------------------------------------------------------------------

class dialogOverlayOptions : public guiAutoDialog {
private:
  PxiDrawSettings &draw_settings;
  wxRadioBox *m_token;
  wxCheckBox *m_connect;
  wxSlider *m_size;
  
  const char *HelpString(void) const { return "Overlay Options"; }
  void Run(void);
public:
  dialogOverlayOptions(wxWindow *, PxiDrawSettings &);
  virtual ~dialogOverlayOptions();
};

const int idEXPDATA_BROWSE1_BUTTON = 3051;
const int idEXPDATA_BROWSE2_BUTTON = 3052;

const int idSETTINGS_WHICH_PLOT_LISTBOX = 2018;
const int idSETTINGS_WHICH_INFOSET_LISTBOX = 2019;
const int idSETTINGS_INFOSET_LISTBOX = 2020;
const int idSETTINGS_ACTION_LISTBOX = 2021;
const int idSETTINGS_OVERLAY_BUTTON = 2022;
const int idSETTINGS_PLOT_MODE = 2023;
const int idSETTINGS_PLOT_BUTTON = 2024;

#endif EXPDPRM_H

