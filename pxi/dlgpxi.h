//
// File: expdprm.c -- definitions for experiment data dialog class
//

#ifndef EXPDPRM_H
#define EXPDPRM_H

#include "wx/slider.h"

#include "wxmisc.h"
#include "base/gstream.h"
#include "pxi.h"
#include "pxicanvas.h"
#include "expdata.h"

// ----------------------------------------------------------------------
//
//     dialogOverlayData
//
// ----------------------------------------------------------------------

class dialogOverlayData: public guiAutoDialog {
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

class dialogSelectScale : public guiAutoDialog {
private:
  wxRadioBox *m_size;
  
  const char *HelpString(void) const { return "Change Scale"; }
public:
  dialogSelectScale(wxWindow *, double scale);
  virtual ~dialogSelectScale();
  double GetScale(void);
};

const int idEXPDATA_BROWSE1_BUTTON = 3051;
const int idEXPDATA_BROWSE2_BUTTON = 3052;

#endif 

