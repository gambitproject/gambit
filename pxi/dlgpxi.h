//
// File: expdprm.c -- definitions for experiment data dialog class
//

#ifndef EXPDPRM_H
#define EXPDPRM_H

#include "wxmisc.h"
#include "gambitio.h"
#include "pxi.h"
#include "expdata.h"

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

#endif EXPDPRM_H

