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
  FullEfg &m_efg;
  int m_branches;

  wxListBox *m_playerItem, *m_infosetItem;
  wxTextCtrl *m_actionItem;

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);

  const char *HelpString(void) const { return "Add move"; }
  
public:
  dialogMoveAdd(wxWindow *, FullEfg &, const gText &,
		EFPlayer *, Infoset *, int branches);
  virtual ~dialogMoveAdd() { } 

  NodeAddMode GetAddMode(void) const;
  EFPlayer *GetPlayer(void) const;
  Infoset *GetInfoset(void) const;
  int GetActions(void) const;

  DECLARE_EVENT_TABLE()
};


class dialogMoveAddAfterAction : public guiAutoDialog {
private:
  FullEfg &m_efg;
  int m_branches;

  wxListBox *m_playerItem, *m_infosetItem, *m_ancestorItem;
  wxTextCtrl *m_actionItem;

  void OnPlayer(wxCommandEvent &);
  void OnInfoset(wxCommandEvent &);

  const char *HelpString(void) const { return "Add move"; }
  
public:
  dialogMoveAddAfterAction(wxWindow *, FullEfg &, const gText &,
			   EFPlayer *, Infoset *, int branches);
  virtual ~dialogMoveAddAfterAction() { } 

  NodeAddMode GetAddMode(void) const;
  EFPlayer *GetPlayer(void) const;
  Infoset *GetInfoset(void) const;
  Action *GetAncestor(void) const;
  int GetActions(void) const;

  DECLARE_EVENT_TABLE()
};

#endif  // DLMOVEADD_H



