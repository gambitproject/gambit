//
// FILE: algdlgs.h -- declaration of base classes for all of the algorithm
//                    parameter dialogs
//
// $Id$
//

#ifndef ALGDLGS_H
#define ALGDLGS_H

#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"

#include "gnumber.h"  

#define PARAMS_SECTION  "Algorithm Params"      // section in .ini file

//=========================================================================
//                 dialogAlgorithm: Class declaration
//=========================================================================

class dialogAlgorithm : public wxDialogBox {
private:
  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogAlgorithm *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogAlgorithm *) p_object.GetClientData())->OnCancel(); }

  static void CallbackDepth(wxRadioBox &p_object, wxEvent &)
    { ((dialogAlgorithm *) p_object.GetClientData())->OnDepth(); }
  static void CallbackAll(wxCheckBox &p_object, wxEvent &)
    { ((dialogAlgorithm *) p_object.GetClientData())->OnAll(); } 

  static void CallbackTrace(wxRadioBox &p_object, wxEvent &)
    { ((dialogAlgorithm *) p_object.GetClientData())->OnTrace(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

  void OnDepth(void);
  void OnAll(void);
  void OnTrace(void);

protected:
  bool m_usesNfg, m_subgames;
  int m_completed;
  wxRadioBox *m_depthChoice, *m_typeChoice, *m_methodChoice;
  wxCheckBox *m_markSubgames, *m_selectSolutions;

  wxIntegerItem *m_stopAfter;
  wxCheckBox *m_findAll;
  wxRadioBox *m_precision;

  wxRadioBox *m_traceDest;
  wxText *m_traceFile;
  wxIntegerItem *m_traceLevel;

  void DominanceFields(bool p_mixed);
  void SubgameFields(void);
  void TraceFields(void);

  void StopAfterField(void);
  void PrecisionField(void);

  virtual void AlgorithmFields(void) { }

  void MakeCommonFields(bool p_dominance, bool p_subgames, bool p_vianfg);

  void Go(void) { Fit(); Show(TRUE); }

public:
  dialogAlgorithm(const gText &, bool, wxWindow *parent = 0, 
		  const char *help_str = 0);
  virtual ~dialogAlgorithm();

  int Completed(void) const { return m_completed; }

  bool MarkSubgames(void) const 
    { return (m_markSubgames && m_markSubgames->GetValue()); }

  bool Eliminate(void) const
    { return (m_depthChoice->GetSelection() > 0); }
  bool EliminateAll(void) const 
    { return (m_depthChoice->GetSelection() == 2); }
  bool EliminateWeak(void) const
    { return (m_typeChoice->GetSelection() == 0); }
  bool EliminateMixed(void) const
    { return (m_methodChoice && m_methodChoice->GetSelection() == 1); }

  gOutput *TraceFile(void) const;
  int TraceLevel(void) const;

  virtual gPrecision Precision(void) const { return precDOUBLE; }
};


//=========================================================================
//                      dialogPxi: Class declaration
//=========================================================================

class dialogPxi : public dialogAlgorithm {
protected:
  gText m_defaultPxiFile;
  wxRadioBox *m_plotType;
  wxText *m_pxiFile, *m_pxiCommand;
  wxCheckBox *m_runPxi;

  static void CallbackRun(wxCheckBox &p_object, wxEvent &)
    { ((dialogPxi *) p_object.GetClientData())->OnRun(); }

  void OnRun(void);

  void PxiFields(void);

public:
  dialogPxi(const char *p_label = 0, const char *p_filename = "pxi.out",
	    wxWindow *p_parent = 0, const char *p_helpStr = 0);
  virtual ~dialogPxi();

  bool LinearPlot(void) const 
    { return (m_plotType->GetSelection() == 1); }
  gOutput *PxiFile(void) const;
  gText PxiFilename(void) const
    { return m_pxiFile->GetValue(); }
  bool RunPxi(void) const
    { return m_runPxi->GetValue(); }
  gText PxiCommand(void) const
    { return m_pxiCommand->GetValue(); }
};

#endif   // ALGDLGS_H

