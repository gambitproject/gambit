//
// FILE: nfgsolvd.h -- the main dialog for running NormalForm solution
//                     algorithms.
//
// $Id$
//

#ifndef NFGSOLVD_H
#define NFGSOLVD_H

typedef enum {
  nfgSTANDARD_NASH = 0, nfgSTANDARD_PERFECT = 1
} nfgStandardType;

typedef enum {
  nfgSTANDARD_ONE = 0, nfgSTANDARD_TWO = 1, nfgSTANDARD_ALL = 2
} nfgStandardNum;

class dialogNfgSolveStandard : public wxDialogBox {
private:
  int m_completed;
  wxRadioBox *m_standardType, *m_standardNum, *m_precision;
  wxText *m_description;
  const Nfg &m_nfg;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogNfgSolveStandard *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogNfgSolveStandard *) p_object.GetClientData())->OnCancel(); }
  static void CallbackChanged(wxRadioBox &p_object, wxEvent &)
    { ((dialogNfgSolveStandard *) p_object.GetClientData())->OnChanged(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);
  
  void OnChanged(void);

public:
  dialogNfgSolveStandard(const Nfg &p_nfg, wxWindow *p_parent);
  virtual ~dialogNfgSolveStandard();

  int Completed(void) const { return m_completed; }
  nfgStandardType Type(void) const;
  nfgStandardNum Number(void) const;
  gPrecision Precision(void) const
   { return ((m_precision->GetSelection() == 0) ? precDOUBLE : precRATIONAL); }
};


class guiBadStandardSolve : public gException {
private:
  gText m_description;

public:
  guiBadStandardSolve(const gText &p_description)
    : m_description(p_description) { }
  virtual ~guiBadStandardSolve() { }

  gText Description(void) const { return m_description; }
};
  

#endif // NFGSOLVD_H
