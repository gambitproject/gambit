//
// FILE: dlmoveadd.h -- Dialog for adding a move to an efg
//
// $Id$
//

#ifndef DLMOVEADD_H
#define DLMOVEADD_H

typedef enum { NodeAddNew, NodeAddIset } NodeAddMode;

class dialogMoveAdd : public guiAutoDialog {
private:
  Efg &m_efg;
  int m_branches;

  wxListBox *m_playerItem, *m_infosetItem;
  wxText *m_actionItem;

  static void CallbackPlayer(wxListBox &p_object, wxCommandEvent &p_event)
    { ((dialogMoveAdd *) p_object.wxEvtHandler::GetClientData())->
	OnPlayer(p_event.commandInt); }
  static void CallbackInfoset(wxListBox &p_object, wxCommandEvent &p_event)
    { ((dialogMoveAdd *) p_object.wxEvtHandler::GetClientData())->
	OnInfoset(p_event.commandInt); }

  void OnPlayer(int);
  void OnInfoset(int);

  const char *HelpString(void) const { return "Node Menu"; }
  
public:
  dialogMoveAdd(Efg &, const gText &, EFPlayer *player=0, Infoset *infoset=0,
                int branches=2, wxFrame *frame=0);
  virtual ~dialogMoveAdd();

  NodeAddMode GetAddMode(void) const;
  EFPlayer *GetPlayer(void) const;
  Infoset *GetInfoset(void) const;
  int GetActions(void) const;
};

#endif  // DLMOVEADD_H



