//
// FILE: dlmoveadd.h -- Dialog for adding a move to an efg
//
//
//

#ifndef DLMOVEADD_H
#define DLMOVEADD_H

typedef enum { NodeAddNew, NodeAddIset} NodeAddMode;

class dialogMoveAdd : public wxDialogBox {
private:
  Efg &m_efg;
  int m_completed, m_branches;

  wxListBox *m_playerItem, *m_infosetItem;
  wxText *m_actionItem;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((dialogMoveAdd *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((dialogMoveAdd *) p_object.GetClientData())->OnCancel(); }
  static void CallbackPlayer(wxListBox &p_object, wxCommandEvent &p_event)
    { ((dialogMoveAdd *) p_object.wxEvtHandler::GetClientData())->
	OnPlayer(p_event.commandInt); }
  static void CallbackInfoset(wxListBox &p_object, wxCommandEvent &p_event)
    { ((dialogMoveAdd *) p_object.wxEvtHandler::GetClientData())->
	OnInfoset(p_event.commandInt); }

  void OnOK(void);
  void OnCancel(void);
  void OnPlayer(int);
  void OnInfoset(int);
  Bool OnClose(void);

public:
  dialogMoveAdd(Efg &ef_,EFPlayer *player=0,Infoset *infoset=0,int branches=2,wxFrame *frame=0);
  virtual ~dialogMoveAdd();

  int Completed(void) const { return m_completed; }

  NodeAddMode GetAddMode(void) const;
  EFPlayer *GetPlayer(void) const;
  Infoset *GetInfoset(void) const;
  int GetActions(void) const;
};

#endif  // DLMOVEADD_H



