//
// FILE: dlelim.h -- Parameters for dominance elimination
//
// $Id$
//

#ifndef DLELIM_H
#define DLELIM_H

typedef enum {
  elimWEAK = 0, elimSTRONG = 1
} elimStrengthType;

typedef enum {
  elimPURE = 0, elimMIXED = 1
} elimMethodType;

class dialogElim : public wxDialogBox {
private:
  wxListBox *m_playerBox;
  wxCheckBox *m_allBox, *m_compressBox;
  wxRadioBox *m_domTypeBox, *m_domMethodBox, *m_domPrecisionBox;
  Bool m_compress, m_all;
  int m_completed, m_numPlayers;
  gArray<int> m_players;
  int m_domType, m_domMethod, m_domPrecision;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogElim *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogElim *) p_object.GetClientData())->OnCancel(); }
  static void CallbackHelp(wxButton &, wxEvent &);

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);
  
public:
  dialogElim(int p_numPlayers, wxWindow *p_parent = NULL);
  virtual ~dialogElim();

  const gArray<int> &Players(void) const { return m_players; }
  bool Compress(void) const { return m_compress; }
  int Completed(void) const { return m_completed; }

  bool FindAll(void) const { return m_all; }
  bool DomStrong(void) const { return (m_domType == elimSTRONG); }
  bool DomMixed(void) const { return (m_domMethod == elimMIXED); }
  gPrecision Precision(void) const
    { return (m_domPrecision == 0) ? precDOUBLE : precRATIONAL; }
};


class SupportRemoveDialog: public wxDialogBox {
private:
  int num_sups;
  wxListBox *sups_item;
  int completed;
	
  static void all_func(wxCheckBox &ob,wxEvent &);
  static void ok_func(wxButton &ob,wxEvent &);
  static void cancel_func(wxButton &ob,wxEvent &);

public:
  SupportRemoveDialog(wxWindow *parent,int num_sups_);

  gArray<bool> Selected(void);
  int Completed(void) { return completed; }
};

#endif   // DLELIM_H

