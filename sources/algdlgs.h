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

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

  void OnDepth(void);

protected:
  int m_completed;
  wxRadioBox *m_depthChoice, *m_typeChoice, *m_methodChoice;
  wxCheckBox *m_markSubgames, *m_selectSolutions;

  void DominanceFields(bool p_mixed);
  void SubgameFields(void);
  virtual void AlgorithmFields(void) { }

  void MakeCommonFields(bool p_dominance, bool p_subgames, bool p_vianfg);

  void Go(void) { Fit(); Show(TRUE); }

public:
  dialogAlgorithm(const gText &, wxWindow *parent = 0, 
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

  virtual gPrecision Precision(void) const { return precDOUBLE; }
};


//=========================================================================
//                 PxiParamsDialog: Class declaration
//=========================================================================

class PxiParamsDialog : public dialogAlgorithm {
public:
  PxiParamsDialog(const char *alg = "Pxi", const char *label = 0, 
		  const char *filename = "pxi.out", wxWindow *parent = 0, 
		  const char *help_str = 0);
  virtual ~PxiParamsDialog(void);

  void MakePxiFields(void);
  virtual bool FromDialog(void) { return true; }
};

#endif   // ALGDLGS_H

