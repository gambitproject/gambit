//
// FILE: dlefgreveal.h -- Selection of players for Edit->Infoset->Reveal
//
//
//

#ifndef DLEFGREVEAL_H
#define DLEFGREVEAL_H

class dialogInfosetReveal : public wxDialogBox {
private:
  const Efg &m_efg;
  int m_completed;

  wxListBox *m_playerNameList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogInfosetReveal *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogInfosetReveal *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  dialogInfosetReveal(const Efg &, wxWindow *);
  virtual ~dialogInfosetReveal();

  int Completed(void) const { return m_completed; }
  gArray<EFPlayer *> GetPlayers(void) const;
};

#endif  // DLEFGREVEAL_H
