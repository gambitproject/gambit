//
// FILE: efgsolvd.h -- Selecting algorithm dialogs
//
// $Id$
//

#ifndef EFGSOLVD_H
#define EFGSOLVD_H

typedef enum {
  efgSTANDARD_NASH = 0, efgSTANDARD_PERFECT = 1, efgSTANDARD_SEQUENTIAL = 2
} efgStandardType;

typedef enum {
  efgSTANDARD_ONE = 0, efgSTANDARD_TWO = 1, efgSTANDARD_ALL = 2
} efgStandardNum;

class dialogEfgSolveStandard : public wxDialogBox {
private:
  int m_completed;
  wxRadioBox *m_standardType, *m_standardNum, *m_precision;
  wxText *m_description;
  const Efg &m_efg;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogEfgSolveStandard *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogEfgSolveStandard *) p_object.GetClientData())->OnCancel(); }
  static void CallbackHelp(wxButton &p_object, wxEvent &)
    { ((dialogEfgSolveStandard *) p_object.GetClientData())->OnHelp(); }
  static void CallbackChanged(wxRadioBox &p_object, wxEvent &)
    { ((dialogEfgSolveStandard *) p_object.GetClientData())->OnChanged(); }

  void OnOK(void);
  void OnCancel(void);
  void OnHelp(void);
  Bool OnClose(void);
  
  void OnChanged(void);

public:
  dialogEfgSolveStandard(const Efg &p_efg, wxWindow *p_parent);
  virtual ~dialogEfgSolveStandard();

  int Completed(void) const { return m_completed; }
  efgStandardType Type(void) const;
  efgStandardNum Number(void) const;
  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }
};

#endif   // EFGSOLVD_H
