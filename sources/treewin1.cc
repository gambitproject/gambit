//
// FILE: treewin1.cc: Companion file to treewin.cc.  This file includes all the
//                    functions for dealing with various menu choices in
//                    treewin.
//
// $Id$
//

#include "wx.h"
#include "wx_mf.h"
#include "wx_clipb.h"
#include "wxmisc.h"
#include "efg.h"
#include "treewin.h"
#include "nodeaddd.h"
#include "infosetd.h"


extern void guiExceptionDialog(const gText &p_message, wxWindow *p_parent,
			       long p_style = wxOK | wxCENTRE);


#define MAX_LABEL_LENGTH    25

// Validation function for string dialogs

Bool StringConstraint(int type, char *value, char *label, char *msg_buffer)
{
    if (value && (strlen(value) >= MAX_LABEL_LENGTH) && (type == wxFORM_STRING))
    {
        sprintf(msg_buffer, "Value for %s should be %d characters or less\n",
                label, MAX_LABEL_LENGTH-1);
        return FALSE;
    }
    else
        return TRUE;
}


// This function used to be in the ef and is used frequently
EFPlayer *EfgGetPlayer(const Efg &ef, const gText &name)
{
    for (int i = 1; i <= ef.NumPlayers(); i++)
    {
        if (ef.Players()[i]->GetName() == name) 
            return ef.Players()[i];
    }

    if (ef.GetChance()->GetName() == name) 
        return ef.GetChance();

    return 0;
}

//-----------------------------------------------------------------------
//                    NODE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                        NODE-ADD MENU HANDLER
//***********************************************************************
void TreeWindow::node_add(void)
{
  static int branches = 2; // make this static so it remembers the last entry
  static EFPlayer *player = 0;
  static Infoset *infoset = 0;
  static Efg *last_ef = 0; // need this to make sure player,infoset are valid

  if (last_ef != &ef)  {
    player = 0;
    infoset = 0;
    last_ef = &ef;
  }
    
  NodeAddDialog node_add_dialog(ef, player, infoset, branches, pframe);

  if (node_add_dialog.Completed() == wxOK)  {
    nodes_changed = TRUE;
    NodeAddMode mode = node_add_dialog.GetAddMode();
    player = node_add_dialog.GetPlayer();
    infoset = node_add_dialog.GetInfoset();
    branches = node_add_dialog.GetBranches();
    Bool set_names = FALSE;

    try {
      if (mode == NodeAddNew) {
	ef.AppendNode(Cursor(), player, branches);
	set_names = node_add_dialog.SetNames();
      }
      else
	ef.AppendNode(Cursor(), infoset);

      if (set_names) {
	node_label();
	infoset_label();
      }

      // take care of probs for chance nodes.
      if (player == ef.GetChance())
	action_probs();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }
}

//***********************************************************************
//                        NODE-INSERT MENU HANDLER
//***********************************************************************
void TreeWindow::node_insert(void)
{
  static int branches = 2; // make this static so it remembers the last entry
  static EFPlayer *player = 0;
  static Infoset *infoset = 0;
  static Efg *last_ef = 0; // need this to make sure player,infoset are valid

  if (last_ef != &ef)  {
    player = 0;
    infoset = 0;
    last_ef = &ef;
  }
    
  NodeAddDialog node_add_dialog(ef, player, infoset, branches, pframe);

  if (node_add_dialog.Completed() == wxOK)  {
    nodes_changed = TRUE;
    NodeAddMode mode = node_add_dialog.GetAddMode();
    player = node_add_dialog.GetPlayer();
    infoset = node_add_dialog.GetInfoset();
    branches = node_add_dialog.GetBranches();
    Bool set_names = FALSE;

    try {
      if (mode == NodeAddNew) {
	ef.InsertNode(Cursor(), player, branches);
	set_names = node_add_dialog.SetNames();
      }
      else
	ef.InsertNode(Cursor(), infoset);

      SetCursorPosition(Cursor()->GetParent());
      if (set_names) {
	node_label();
	infoset_label();
      }

      // take care of probs for chance nodes.
      if (player == ef.GetChance())
	action_probs();
    }
    catch (gException &E) {
      guiExceptionDialog(E.Description(), pframe);
    }
  }

}
//***********************************************************************
//                      NODE-DELETE MENU HANDLER
//***********************************************************************

class efgNodeDeleteDialog : public wxDialogBox {
private:
  Node *m_node;
  int m_completed;
  wxListBox *m_branchList;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((efgNodeDeleteDialog *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((efgNodeDeleteDialog *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  efgNodeDeleteDialog(Node *, wxWindow *);
  virtual ~efgNodeDeleteDialog() { }

  int Completed(void) const { return m_completed; }
  Node *KeepNode(void) const
    { return m_node->GetChild(m_branchList->GetSelection() + 1); }
};

efgNodeDeleteDialog::efgNodeDeleteDialog(Node *p_node, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select Branch", TRUE), m_node(p_node)
{
  m_branchList = new wxListBox(this, 0, "Keep branch");
  for (int act = 1; act <= p_node->NumChildren(); act++) {
    m_branchList->Append(ToText(act) + ": " +
			 p_node->GetInfoset()->Actions()[act]->GetName());
  }
  m_branchList->SetSelection(0);

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void efgNodeDeleteDialog::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void efgNodeDeleteDialog::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool efgNodeDeleteDialog::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void TreeWindow::node_delete(void)
{
  try {
    efgNodeDeleteDialog dialog(Cursor(), this);

    if (dialog.Completed() == wxOK) {
      Node *keep = dialog.KeepNode();
      nodes_changed = TRUE;
      SetCursorPosition(ef.DeleteNode(Cursor(), keep));
    }
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                       NODE-LABEL MENU HANDLER
//***********************************************************************

class efgNodeLabelDialog : public wxDialogBox {
private:
  int m_completed;
  wxText *m_label;

  static void CallbackOK(wxButton &p_object, wxEvent &)
    { ((efgNodeLabelDialog *) p_object.GetClientData())->OnOK(); }
  static void CallbackCancel(wxButton &p_object, wxEvent &)
    { ((efgNodeLabelDialog *) p_object.GetClientData())->OnCancel(); }

  void OnOK(void);
  void OnCancel(void);
  Bool OnClose(void);

public:
  efgNodeLabelDialog(Node *, wxWindow *);
  virtual ~efgNodeLabelDialog() { }

  int Completed(void) const { return m_completed; }
  gText Label(void) const { return m_label->GetValue(); }
};

efgNodeLabelDialog::efgNodeLabelDialog(Node *p_node, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Label Node", TRUE)
{
  m_label = new wxText(this, 0, "Label");
  m_label->SetValue(p_node->GetName());

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

void efgNodeLabelDialog::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void efgNodeLabelDialog::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool efgNodeLabelDialog::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void TreeWindow::node_label(void)
{
  try {
    efgNodeLabelDialog dialog(Cursor(), this);

    if (dialog.Completed() == wxOK) {
      Cursor()->SetName(dialog.Label());
    }
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                       NODE-OUTCOME MENU HANDLER
//***********************************************************************

class EFChangePayoffs : public MyDialogBox {
private:
  EFOutcome *outcome;
  Efg &ef;

  wxText *name_item;
  wxText **payoff_items;

public:
  EFChangePayoffs(Efg &, EFOutcome *, wxWindow *parent);

  gArray<gNumber> Payoffs(void);
  gText Name(void);
};

EFChangePayoffs::EFChangePayoffs(Efg &p_efg, EFOutcome *p_outcome,
				 wxWindow *p_parent)
  : MyDialogBox(p_parent, "Change Payoffs"), outcome(p_outcome), ef(p_efg)
{
  Add(wxMakeFormMessage("Change payoffs for outcome:"));
  Add(wxMakeFormNewLine());

  char *new_name = new char[40];
  wxFormItem *name_fitem = Add(wxMakeFormString("Outcome:", &new_name, wxFORM_TEXT, 0, 0, 0, 160));
  Add(wxMakeFormNewLine());

  const int ENTRIES_PER_ROW = 3;

  // Payoff items
  char **new_payoffs = new char *[ef.NumPlayers()+1];
  wxFormItem **payoff_fitems = new wxFormItem *[ef.NumPlayers()+1];
  payoff_items = new wxText *[ef.NumPlayers()+1];

  for (int i = 1; i <= ef.NumPlayers(); i++) {
    new_payoffs[i] = new char[40];
    if (ef.Players()[i]->GetName() != "")
      payoff_fitems[i] = Add(wxMakeFormString(ef.Players()[i]->GetName() + ":", &(new_payoffs[i]), wxFORM_TEXT, 0, 0, 0, 160));
    else
      payoff_fitems[i] = Add(wxMakeFormString(ToText(i) + ":", &(new_payoffs[i]), wxFORM_TEXT, 0, 0, 0, 160));

    if (i % ENTRIES_PER_ROW == 0)
      Add(wxMakeFormNewLine());
  }

  AssociatePanel();

  for (int i = 1; i <= ef.NumPlayers(); i++) {
    payoff_items[i] = (wxText *)payoff_fitems[i]->GetPanelItem();
    gNumber payoff = 0;
    if (outcome)
      payoff = ef.Payoff(outcome, i);
    payoff_items[i]->SetValue(ToText(payoff));
  }
  
  if (p_outcome && ef.NumPlayers() > 0) {
    payoff_items[1]->SetFocus();
  }

  name_item = (wxText *) name_fitem->GetPanelItem();
  if (outcome)
    name_item->SetValue(outcome->GetName());
  else
    name_item->SetValue("Outcome " + ToText(ef.NumOutcomes() + 1));

  Go1();

  for (int i = 1; i <= ef.NumPlayers(); i++) 
    delete [] new_payoffs[i];

  delete [] new_payoffs;

  delete [] new_name;
}

gArray<gNumber> EFChangePayoffs::Payoffs(void)
{
  gArray<gNumber> payoffs(ef.NumPlayers());

  for (int i = 1; i <= ef.NumPlayers(); i++)
    FromText(payoff_items[i]->GetValue(), payoffs[i]);

  return payoffs;
}

gText EFChangePayoffs::Name(void)
{
  return name_item->GetValue();
} 

void TreeWindow::ChangePayoffs(void)
{
  EFChangePayoffs *dialog = new EFChangePayoffs(ef, Cursor()->GetOutcome(),
						pframe);

  if (dialog->Completed() == wxOK) {
    EFOutcome *outc = Cursor()->GetOutcome();
    gArray<gNumber> payoffs(dialog->Payoffs());

    if (!outc) {
      outc = ef.NewOutcome();
      Cursor()->SetOutcome(outc);
    }

    for (int i = 1; i <= ef.NumPlayers(); i++)
      ef.SetPayoff(outc, i, payoffs[i]);
    outc->SetName(dialog->Name());

    outcomes_changed = 1;
  }
  
  delete dialog;
}

void TreeWindow::EditOutcomeAttach(void)
{
  MyDialogBox *dialog = new MyDialogBox(pframe, "Attach Outcome");
    
  wxStringList *outcome_list = new wxStringList;
  char *outcome_name = new char[256];
        
  for (int outc = 1; outc <= ef.NumOutcomes(); outc++) {
    EFOutcome *outcome = ef.Outcomes()[outc];
    gText tmp = ToText(outc) + ": " + outcome->GetName() + " (";
    tmp += ToText(ef.Payoff(outcome, 1)) + ", " + ToText(ef.Payoff(outcome, 2));
    if (ef.NumPlayers() > 2) {
      tmp += ", " + ToText(ef.Payoff(outcome, 3));
      if (ef.NumPlayers() > 3) 
	tmp += ",...)";
      else
	tmp += ")";
    }
    else
      tmp += ")";
  
    outcome_list->Add(tmp);
  }

  dialog->Add(wxMakeFormString("Outcome", &outcome_name,
			       wxFORM_CHOICE,
			       new wxList(wxMakeConstraintStrings(outcome_list), 0)));

  dialog->Go();
  
  if (dialog->Completed() == wxOK) {
    for (int i = 0; ; i++) {
      if (outcome_name[i] == ':') {
	outcome_name[i] = '\0';
	break;
      }
    }
    
    int outc = (int) ToDouble(outcome_name);
    Cursor()->SetOutcome(ef.Outcomes()[outc]);
    outcomes_changed = 1;
    OnPaint();
  }

  delete dialog;
  delete [] outcome_name;
}

void TreeWindow::EditOutcomeDetach(void)
{
  Cursor()->SetOutcome(0);
  outcomes_changed = 1;
  OnPaint();
}

void TreeWindow::EditOutcomeLabel(void)
{
  char *name = new char[40];
  strncpy(name, Cursor()->GetOutcome()->GetName(), 40);

  MyDialogBox *dialog = new MyDialogBox(pframe, "Label outcome");
  dialog->Form()->Add(wxMakeFormString("New outcome label", &name, wxFORM_TEXT,
				       0, 0, 0, 220));
  dialog->Go();

  if (dialog->Completed() == wxOK) {
    Cursor()->GetOutcome()->SetName(name);
    outcomes_changed = 1;
  }
  
  delete dialog;
  delete [] name;
}

void TreeWindow::EditOutcomeNew(void)
{
  EFChangePayoffs *dialog = new EFChangePayoffs(ef, 0, pframe);

  if (dialog->Completed() == wxOK) {
    EFOutcome *outc = ef.NewOutcome();
    gArray<gNumber> payoffs(dialog->Payoffs());

    for (int i = 1; i <= ef.NumPlayers(); i++)
      ef.SetPayoff(outc, i, payoffs[i]);
    outc->SetName(dialog->Name());

    outcomes_changed = 1;
  }
  
  delete dialog;
}

void TreeWindow::EditOutcomeDelete(void)
{
  MyDialogBox *dialog = new MyDialogBox(pframe, "Delete Outcome");
    
  wxStringList *outcome_list = new wxStringList;
  char *outcome_name = new char[256];
        
  for (int outc = 1; outc <= ef.NumOutcomes(); outc++) {
    EFOutcome *outcome = ef.Outcomes()[outc];
    gText tmp = ToText(outc) + ": " + outcome->GetName() + " (";
    tmp += ToText(ef.Payoff(outcome, 1)) + ", " + ToText(ef.Payoff(outcome, 2));
    if (ef.NumPlayers() > 2) {
      tmp += ", " + ToText(ef.Payoff(outcome, 3));
      if (ef.NumPlayers() > 3) 
	tmp += ",...)";
      else
	tmp += ")";
    }
    else
      tmp += ")";
  
    outcome_list->Add(tmp);
  }

  dialog->Add(wxMakeFormString("Outcome", &outcome_name,
			       wxFORM_CHOICE,
			       new wxList(wxMakeConstraintStrings(outcome_list), 0)));

  dialog->Go();
  
  if (dialog->Completed() == wxOK) {
    for (int i = 0; ; i++) {
      if (outcome_name[i] == ':') {
	outcome_name[i] = '\0';
	break;
      }
    }
    
    int outc = (int) ToDouble(outcome_name);
    ef.DeleteOutcome(ef.Outcomes()[outc]);
    outcomes_changed = 1;
    OnPaint();
  }

  delete dialog;
  delete [] outcome_name;
}  


#define DRAG_OUTCOME_END      7

void TreeWindow::node_outcome(int out, int x, int y)
{
  if (out > ef.NumOutcomes())
    MyMessageBox("This outcome is not defined yet", "Outcome",
		 EFG_OUTCOME_HELP, pframe);
  else {
    if (x != -1) { // dropped an outcome at the coordinates (x,y)
      ScreenToClient(&x, &y); // x,y are absolute screen coordinates
      float xf = x, yf = y;
      Node *tmp = GotObject(xf, yf, DRAG_OUTCOME_END);
      if (tmp)
	SetCursorPosition(tmp);
      else
	return;
    }
    
    if (out > 0)
      Cursor()->SetOutcome(ef.Outcomes()[out]);

    if (out == 0)
      Cursor()->SetOutcome(0);

    // if (out == -1) just update all outcomes
    outcomes_changed = 1;
    OnPaint();
  }
}


//***********************************************************************
//                       NODE-SET-MARK MENU HANDLER
//***********************************************************************

void TreeWindow::node_set_mark(void)
{
  old_mark_node = mark_node;
  if (mark_node != Cursor())
    mark_node = Cursor();
  else
    mark_node = 0;                                   
}

//***********************************************************************
//                      NODE-GOTO-MARK MENU HANDLER
//***********************************************************************

void TreeWindow::node_goto_mark(void)
{
  if (mark_node) {
    SetCursorPosition(mark_node);
    ProcessCursor();
  }
}

//-----------------------------------------------------------------------
//                   INFOSET MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                    INFOSET-MEMBER MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_merge(void)
{
  try {
    ef.MergeInfoset(mark_node->GetInfoset(), Cursor()->GetInfoset());
    infosets_changed = TRUE;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                      INFOSET-BREAK MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_break(void)
{
  try {
    char *iset_name = wxGetTextFromUser("New infoset name",
					"Infoset Name",
					"Infoset" +
					ToText(Cursor()->GetPlayer()->NumInfosets()+1));
    ef.LeaveInfoset(Cursor())->SetName(iset_name);
    infosets_changed = TRUE;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                      INFOSET-SPLIT MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_split(void)
{
  try {
    char *iset_name = wxGetTextFromUser("New infoset name");
    Infoset *new_iset = ef.SplitInfoset(Cursor());
    if (iset_name)
      new_iset->SetName(iset_name);
    else
      new_iset->SetName("Infoset" + 
			ToText(new_iset->GetPlayer()->NumInfosets()));
    infosets_changed = TRUE;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                       INFOSET-JOIN MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_join(void)
{
  try {
    ef.JoinInfoset(mark_node->GetInfoset(), Cursor());
    infosets_changed = TRUE;
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                       INFOSET-LABEL MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_label(void)
{
  char *label = 0;
  MyDialogBox *label_dialog = 0;
    
  try {
    label = new char[MAX_LABEL_LENGTH];
    Bool label_actions = TRUE;
    if (Cursor()->GetInfoset()->GetName() != "")
      strcpy(label, Cursor()->GetInfoset()->GetName());
    else
      strcpy(label, "Infoset"+ToText(Cursor()->GetPlayer()->NumInfosets()));
    
    label_dialog = new MyDialogBox(pframe, "Label Infoset", EFG_INFOSET_HELP);
    label_dialog->Add(wxMakeFormString(
       "Label", &label, wxFORM_DEFAULT,
       new wxList(wxMakeConstraintFunction(StringConstraint), 0)));
    label_dialog->Add(wxMakeFormNewLine());
    label_dialog->Add(wxMakeFormBool("Label Actions", &label_actions));
    label_dialog->Go();

    if (label_dialog->Completed() == wxOK) {
      Cursor()->GetInfoset()->SetName(label);
      if (label_actions) action_label();
    }
    
    delete label_dialog;
    delete [] label;
  }
  catch (gException &E) {
    if (label_dialog)   
      delete label_dialog;

    if (label)    
      delete [] label;
 
    guiExceptionDialog(E.Description(), pframe);
  }
}


//***********************************************************************
//                       INFOSET-SWITCH-PLAYER MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_switch_player(void)
{
  MyDialogBox *infoset_switch_dialog = 0;
  char *player_name = 0;
    
  try {
    infoset_switch_dialog = new MyDialogBox(this, "Set New Player", EFG_INFOSET_HELP);
    wxStringList *player_list = new wxStringList;
    player_name = new char[20];
        
    if (ef.GetChance() != Cursor()->GetPlayer())
      player_list->Add(ef.GetChance()->GetName());

    for (int pl = 1; pl <= ef.NumPlayers(); pl++) {
      if (ef.Players()[pl] != Cursor()->GetPlayer())
	player_list->Add(ef.Players()[pl]->GetName());
    }

    infoset_switch_dialog->Add(
            wxMakeFormString("Player", &player_name,
                             wxFORM_CHOICE,
                             new wxList(wxMakeConstraintStrings(player_list), 0)));
    infoset_switch_dialog->Go();
        
    if (infoset_switch_dialog->Completed() == wxOK) {
      if (strcmp(player_name, ef.GetChance()->GetName())) {
	ef.SwitchPlayer(Cursor()->GetInfoset(), EfgGetPlayer(ef, player_name));
	infosets_changed = TRUE;
      }
      else {
	Infoset *s = ef.CreateInfoset(ef.GetChance(), Cursor()->NumChildren());
	ef.JoinInfoset(s, Cursor());
	infosets_changed = TRUE;
      }
    }
        
    delete infoset_switch_dialog;
    delete [] player_name;
  }
  catch (gException &E) {
    if (infoset_switch_dialog)  
      delete infoset_switch_dialog;

    if (player_name)   
      delete [] player_name;

    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                       INFOSET-REVEAL MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_reveal(void)
{
  MyDialogBox *infoset_reveal_dialog = 0;
  char **player_names = 0;
    
  try {
    infoset_reveal_dialog = new MyDialogBox(this, "Reveal to Players",
					    EFG_INFOSET_HELP);
    player_names = new char *[ef.NumPlayers()];

    for (int i = 1; i <= ef.NumPlayers(); i++)
      player_names[i - 1] = 0;

    for (int i = 1; i <= ef.NumPlayers(); i++)
      player_names[i - 1] = copystring(ef.Players()[i]->GetName());
        
    wxListBox *player_item = new wxListBox(infoset_reveal_dialog, 0, "Players",
					   wxMULTIPLE, -1, -1, -1, -1,
					   ef.NumPlayers(), player_names);
    infoset_reveal_dialog->Go();

    if (infoset_reveal_dialog->Completed() == wxOK) {
      gBlock<EFPlayer *> players;

      for (int i = 1; i <= ef.NumPlayers(); i++) {
	if (player_item->Selected(i-1)) 
	  players.Append(ef.Players()[i]);
      }

      ef.Reveal(Cursor()->GetInfoset(), players);
      infosets_changed = TRUE;
    }
        
    delete infoset_reveal_dialog;

    for (int i = 1; i <= ef.NumPlayers(); i++)
      delete [] player_names[i - 1];

    delete [] player_names;
  }
  catch (gException &E) {
    if (infoset_reveal_dialog)
      delete infoset_reveal_dialog;

    if (player_names) {
      for (int i = 1; i <= ef.NumPlayers(); i++) {
	if (player_names[i - 1])
	  delete [] player_names[i - 1];
      }

      if (player_names)   
	delete [] player_names;
    }

    guiExceptionDialog(E.Description(), pframe);
  }
}



//-----------------------------------------------------------------------
//                    ACTION MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                      ACTION-LABEL MENU HANDLER
//***********************************************************************

// The text input fields are stacked vertically up to ENTRIES_PER_DIALOG.
// If there are more than ENTRIES_PER_DIALOG actions for this infoset,
// consequtive dialogs will be created.

#define ENTRIES_PER_DIALOG 10

void TreeWindow::action_label(void)
{
  int num_actions = Cursor()->GetInfoset()->NumActions();
  int num_d = num_actions / ENTRIES_PER_DIALOG -
        ((num_actions % ENTRIES_PER_DIALOG) ? 0 : 1);
  char **action_names = 0;
  MyDialogBox *branch_label_dialog = 0;

  try {
    for (int d = 0; d <= num_d; d++) {
      branch_label_dialog = new MyDialogBox(pframe, "Action Label",
					    EFG_ACTION_HELP);
      int actions_now = gmin(num_actions-d*ENTRIES_PER_DIALOG, ENTRIES_PER_DIALOG);
      action_names = new char *[actions_now];
    
      int i;
      for (i = 1; i <= actions_now; i++) {
	action_names[i-1] = new char[MAX_LABEL_LENGTH];
	strcpy(action_names[i-1], 
	       Cursor()->GetInfoset()->GetActionName(i+d*ENTRIES_PER_DIALOG));
	branch_label_dialog->Add(wxMakeFormString(
		"Action " + ToText(i + d * ENTRIES_PER_DIALOG), 
                &action_names[i-1], 
                wxFORM_DEFAULT,
                new wxList(wxMakeConstraintFunction(StringConstraint), 0), 0, 0, 220));
	branch_label_dialog->Add(wxMakeFormNewLine());
      }
      
      if (num_actions-(d+1)*ENTRIES_PER_DIALOG > 0)
	branch_label_dialog->Add(wxMakeFormMessage("Continued..."));

      branch_label_dialog->Go();

      if (branch_label_dialog->Completed() == wxOK) {
	for (i = 1; i <= actions_now; i++)
	  Cursor()->GetInfoset()->SetActionName(i+d*ENTRIES_PER_DIALOG, 
					      action_names[i-1]);
      }
      
      delete branch_label_dialog;
      
      for (i = 1; i <= actions_now; i++)
	delete [] action_names[i-1];
      
      delete [] action_names;
    }
  }
  catch (gException &E) {
    if (action_names)
      delete [] action_names;
    if (branch_label_dialog)
      delete branch_label_dialog;
    
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                     ACTION-INSERT MENU HANDLER
//***********************************************************************

void TreeWindow::action_insert(void)
{
  MyDialogBox *branch_insert_dialog = 0;
  char *action_str = 0;

  try {
    branch_insert_dialog = new MyDialogBox(pframe, "Insert Branch",
					   EFG_ACTION_HELP);
    Infoset *iset = Cursor()->GetInfoset();
    wxStringList *action_list = new wxStringList;
    action_str = new char[30];

    for (int i = 1; i <= iset->NumActions(); i++) 
      action_list->Add(iset->GetActionName(i));

    branch_insert_dialog->Add(wxMakeFormString(
        "Where", &action_str,
        wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(action_list), 0)));
    branch_insert_dialog->Go();

    if (branch_insert_dialog->Completed() == wxOK) {
      nodes_changed = TRUE;
      
      for (int act = 1; act <= iset->NumActions(); act++) {
	if (iset->Actions()[act]->GetName() == action_str)
	  ef.InsertAction(iset, iset->Actions()[act]);
      }
    }
    
    delete branch_insert_dialog;
    delete [] action_str;
  }
  catch (gException &E) {
    if (action_str)
      delete [] action_str;

    if (branch_insert_dialog)
      delete branch_insert_dialog;

    guiExceptionDialog(E.Description(), pframe);
  }    
}

//***********************************************************************
//                       ACTION-DELETE MENU HANDLER
//***********************************************************************

void TreeWindow::action_delete(void)
{
  MyDialogBox *branch_delete_dialog = 0;
  char *action_str = 0;

  try {
    branch_delete_dialog = 
      new MyDialogBox(pframe, "Delete Branch", EFG_ACTION_HELP);
    Infoset *iset = Cursor()->GetInfoset();
    wxStringList *action_list = new wxStringList;
    action_str = new char[30];

    for (int i = 1; i <= iset->NumActions(); i++) 
      action_list->Add(iset->GetActionName(i));

    branch_delete_dialog->Add(wxMakeFormString(
        "Which", &action_str,
        wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(action_list), 0)));
    branch_delete_dialog->Go();

    if (branch_delete_dialog->Completed() == wxOK) {
      nodes_changed = TRUE;
      for (int act = 1; act <= iset->NumActions(); act++)
	if (iset->Actions()[act]->GetName() == action_str)
	  ef.DeleteAction(iset, iset->Actions()[act]);
    }

    delete branch_delete_dialog;
    delete [] action_str;
  }
  catch (gException &E) {
    if (branch_delete_dialog)
      delete branch_delete_dialog;

    if (action_str)
      delete [] action_str;
  }
}

//***********************************************************************
//                      NODE-PROBS MENU HANDLER
//***********************************************************************

// The text input fields are stacked vertically up to ENTRIES_PER_DIALOG.
// If there are more than ENTRIES_PER_DIALOG actions for this infoset,
// consecutive dialogs will be created.

#define ENTRIES_PER_DIALOG  10

void TreeWindow::action_probs(void)
{
  int i;
    
  int num_actions = Cursor()->NumChildren();
  int num_d = num_actions/ENTRIES_PER_DIALOG-((num_actions%ENTRIES_PER_DIALOG) ? 0 : 1);

  MyDialogBox *node_probs_dialog = 0;
  char **prob_vector = 0;
  Node *n = Cursor();

  try {
    for (int d = 0; d <= num_d; d++) {
      node_probs_dialog = new MyDialogBox(pframe, "Node Probabilities");
      int actions_now = gmin(num_actions-d*ENTRIES_PER_DIALOG, ENTRIES_PER_DIALOG);
      prob_vector = new char *[actions_now+1];
    
      for (i = 1; i <= actions_now; i++) {
	gNumber temp_p = ef.GetChanceProb(n->GetInfoset(),
					  i + d * ENTRIES_PER_DIALOG);
	gText temp_s = ToText(temp_p);
	prob_vector[i] = new char[temp_s.Length()+1];
	strcpy(prob_vector[i], temp_s);
	node_probs_dialog->Add(wxMakeFormString(
                "", &(prob_vector[i]), wxFORM_TEXT, NULL, NULL, wxVERTICAL, 80));
	node_probs_dialog->Add(wxMakeFormNewLine());
      }

      if (num_actions-(d+1)*ENTRIES_PER_DIALOG > 0)
	node_probs_dialog->Add(wxMakeFormMessage("Continued..."));
        
      node_probs_dialog->Go();

      if (node_probs_dialog->Completed() == wxOK) {
	gNumber dummy;
	for (i = 1; i <= actions_now; i++)
	  ef.SetChanceProb(n->GetInfoset(),
			   i + d * ENTRIES_PER_DIALOG,
			   FromText(prob_vector[i], dummy));
	outcomes_changed = TRUE;  // game changed -- delete solutions, etc
      }

      for (i = 1; i <= actions_now; i++) 
	delete [] prob_vector[i];
      delete [] prob_vector;
      delete node_probs_dialog;
    }
  }
  catch (gException &E) {
    if (node_probs_dialog)
      delete node_probs_dialog;
    if (prob_vector)
      delete [] prob_vector;
    
    guiExceptionDialog(E.Description(), pframe);
  }
}


//-----------------------------------------------------------------------
//                     TREE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                       TREE-LABEL MENU HANDLER
//***********************************************************************

Bool LongStringConstraint(int type, char *value, char *label,
			  char *msg_buffer)
{
  if (value && (strlen(value) >= 255) && (type == wxFORM_STRING)) {
    sprintf(msg_buffer, "Value for %s should be %d characters or less\n",
	    label, 255);
    return FALSE;
  }
  else
    return TRUE;
}


void TreeWindow::tree_label(void)
{
  char *label = 0;
  MyDialogBox *tree_label_dialog = 0;
  try {
    label = new char[256];
    strcpy(label, ef.GetTitle());

    tree_label_dialog = new MyDialogBox(pframe, "Label Tree",
                                                     EFG_TREE_HELP);
    wxFormItem *label_item = 
        wxMakeFormString("Label", &label, wxFORM_DEFAULT,
                         new wxList(wxMakeConstraintFunction(LongStringConstraint), 0), 
                         0, 0, 350);
    tree_label_dialog->Add(label_item);
    tree_label_dialog->AssociatePanel();
    ((wxText *)label_item->PanelItem)->SetFocus();
    tree_label_dialog->Go1();
    
    if (tree_label_dialog->Completed() == wxOK)
      ef.SetTitle(label);
    
    delete tree_label_dialog;
    delete [] label;
  }
  catch (gException &E) {
    if (label)
      delete [] label;
    if (tree_label_dialog)
      delete tree_label_dialog;
    
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                      TREE-DELETE MENU HANDLER
//***********************************************************************

void TreeWindow::tree_delete(void)
{
  MyMessageBox *tree_delete_dialog = 0;
    
  try {
    tree_delete_dialog = new MyMessageBox("Are you sure?",
					  "Delete Tree",
					  EFG_TREE_HELP, pframe);
        
    if (tree_delete_dialog->Completed() == wxOK) {
      nodes_changed = true;
      ef.DeleteTree(Cursor());
    }

    delete tree_delete_dialog;
  }
  catch (gException &E) {
    if (tree_delete_dialog)  
      delete tree_delete_dialog;

    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                       TREE-COPY MENU HANDLER
//***********************************************************************

void TreeWindow::tree_copy(void)
{
  nodes_changed = true; 
  try {
    ef.CopyTree(mark_node, Cursor());
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}

//***********************************************************************
//                       TREE-MOVE MENU HANDLER
//***********************************************************************

void TreeWindow::tree_move(void)
{
  nodes_changed = true;
  try {
    ef.MoveTree(mark_node, Cursor());
  }
  catch (gException &E) {
    guiExceptionDialog(E.Description(), pframe);
  }
}


//***********************************************************************
//                      TREE-PLAYERS MENU HANDLER
//***********************************************************************

#define PLAYERSD_INST // instantiate the players display dialog
#include "playersd.h"

void TreeWindow::tree_players(void)
{
  PlayerNamesDialog player_names(ef, pframe);
}

//***********************************************************************
//                      TREE-INFOSETS MENU HANDLER
//***********************************************************************

void TreeWindow::tree_infosets(void)
{
  InfosetDialog ID(ef, pframe);

  if (ID.GameChanged()) 
    infosets_changed = true;
}


//-----------------------------------------------------------------------
//                     SUBGAME MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

void LegalSubgameRoots(const Efg &efg, gList<Node *> &list);

void TreeWindow::subgame_solve(void)
{
  subgame_list.Flush();
  subgame_list.Append(SubgameEntry(ef.RootNode())); // Add the first subgame -- root subgame
  gList<Node *> subgame_roots;
  LegalSubgameRoots(ef, subgame_roots);
  ef.MarkSubgames(subgame_roots);

  for (int i = 1; i <= subgame_roots.Length(); i++) {
    if (subgame_roots[i] != ef.RootNode())
      subgame_list.Append(SubgameEntry(subgame_roots[i], true));
  }
  
  must_recalc = true;
}

void TreeWindow::subgame_set(void)
{
  if (Cursor()->GetSubgameRoot() == Cursor()) {
    // ignore silently
    return;
  }

  if (!ef.IsLegalSubgame(Cursor())) {
    wxMessageBox("This node is not a root of a valid subgame"); 
    return;
  }

  ef.DefineSubgame(Cursor());
  subgame_list.Append(SubgameEntry(Cursor(), true)); // collapse
  must_recalc = true;
}

void TreeWindow::subgame_clear_one(void)
{
  if (Cursor()->GetSubgameRoot() != Cursor()) {
    wxMessageBox("This node is not a subgame root");
    return;
  }
  
  if (Cursor()->GetSubgameRoot() == ef.RootNode()) {
    wxMessageBox("Root node is always a subgame root");
    return;
  }
    
  ef.RemoveSubgame(Cursor());

  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor())
      subgame_list.Remove(i);
  }

  must_recalc = true;
}

void TreeWindow::subgame_clear_all(void)
{
  ef.UnmarkSubgames(ef.RootNode());
  subgame_list.Flush();
  subgame_list.Append(SubgameEntry(ef.RootNode())); // Add the first subgame -- root subgame
  must_recalc = true;
}

void TreeWindow::subgame_collapse_one(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      subgame_list[i].expanded = false;
      must_recalc = true;
      return;
    }
  }

  wxMessageBox("This node is not a subgame root");
}

void TreeWindow::subgame_collapse_all(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++)
    subgame_list[i].expanded = false;

  must_recalc = true;
}

void TreeWindow::subgame_expand_one(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      subgame_list[i].expanded = true;
      must_recalc = true;
      return;
    }
  }

  wxMessageBox("This node is not a subgame root");
}

void TreeWindow::subgame_expand_branch(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      for (int j = 1; j <= subgame_list.Length(); j++) {
	if (ef.IsSuccessor(subgame_list[j].root, Cursor())) {
	  subgame_list[j].expanded = true;
	  must_recalc = true;
	}
      }

      return;
    }
  }
  
  wxMessageBox("This node is not a subgame root");
}


void TreeWindow::subgame_expand_all(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++)
    subgame_list[i].expanded = true;
  
  must_recalc = true;
}

void TreeWindow::subgame_toggle(void)
{
  for (int i = 1; i <= subgame_list.Length(); i++) {
    if (subgame_list[i].root == Cursor()) {
      subgame_list[i].expanded = !subgame_list[i].expanded; 
      must_recalc = true;
      return;
    }
  }

  wxMessageBox("This node is not a subgame root");
}



//***********************************************************************
//                      FILE-SAVE MENU HANDLER
//***********************************************************************
// see treewin.cc

//***********************************************************************
//                      FILE-OUTPUT MENU HANDLER
//***********************************************************************
void TreeWindow::output(void)
{
  wxOutputDialogBox od;

  if (od.Completed() == wxOK) {
    switch (od.GetMedia()) {
    case wxMEDIA_PRINTER: print(od.GetOption()); break;
    case wxMEDIA_PS:print_eps(od.GetOption()); break;
    case wxMEDIA_CLIPBOARD:print_mf(od.GetOption()); break;
    case wxMEDIA_METAFILE: print_mf(od.GetOption(), true); break;
    case wxMEDIA_PREVIEW: print(od.GetOption(), true); break;
    default:
      // We'll ignore this silently
      break;
    }
  }
}

#ifdef wx_msw
#include "wx_print.h"

class ExtensivePrintout: public wxPrintout {
private:
  TreeWindow *tree;
  wxOutputOption fit;
  int num_pages;
    
public:
  ExtensivePrintout(TreeWindow *s, wxOutputOption f,
		    const char *title = "ExtensivePrintout");
  Bool OnPrintPage(int page);
  Bool HasPage(int page);
  Bool OnBeginDocument(int startPage, int endPage);
  void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};


Bool ExtensivePrintout::OnPrintPage(int)
{
  // this is funky--I am playing around w/ the
  // different zoom settings.  So the zoom setting in draw_settings does not
  // equal to the zoom setting in the printer!
  wxDC *dc = GetDC();
  if (!dc) return FALSE;
    
  dc->SetBackgroundMode(wxTRANSPARENT);
  dc->Colour = FALSE;
  Bool color_outcomes;
  
  if (!dc->Colour) {
    color_outcomes = tree->draw_settings.ColorCodedOutcomes();
    tree->draw_settings.SetOutcomeColor(FALSE);
  }
    
  int win_w, win_h;
  tree->GetClientSize(&win_w, &win_h);    // that is the size of the window
  float old_zoom = tree->draw_settings.Zoom();
  // Now we have to check in case our real page size is reduced
  // (e.g. because we're drawing to a print preview memory DC)
  int pageWidth, pageHeight;
  float w, h;
  dc->GetSize(&w, &h);
  GetPageSizePixels(&pageWidth, &pageHeight);
  float pageScaleX = (float)w/pageWidth;
  float pageScaleY = (float)h/pageHeight;
  
  if (fit) { // fit to page
    int maxX = tree->draw_settings.MaxX(), maxY = tree->draw_settings.MaxY(); // size of tree
    // Figure out the 'fake' window zoom
    float zoom_x = (float)win_w/(float)maxX, zoom_y = (float)win_h/(float)maxY;
    float real_zoom = gmin(zoom_x, zoom_y);
    tree->draw_settings.SetZoom(real_zoom, true);
    // Figure out the 'real' printer zoom
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
    // Make the margins.  They are just 1" on all sides now.
    float marginX = 1*ppiPrinterX;
    float marginY = 1*ppiPrinterY;
        
    zoom_x = (float)((pageWidth-2*marginX)/(float)maxX)*pageScaleX;
    zoom_y = (float)((pageHeight-2*marginY)/(float)maxY)*pageScaleY;
    real_zoom = gmin(zoom_x, zoom_y);
        
    dc->SetUserScale(real_zoom, real_zoom);
    dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
  }
  else {  // WYSIWYG
    int ppiScreenX, ppiScreenY;
    GetPPIScreen(&ppiScreenX, &ppiScreenY);
    int ppiPrinterX, ppiPrinterY;
    GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
        
    // This scales the DC so that the printout roughly represents the
    // the screen scaling. The text point size _should_ be the right size
    // but in fact is too small for some reason. This is a detail that will
    // need to be addressed at some point but can be fudged for the
    // moment.
    float scaleX = (float)((float)ppiPrinterX/(float)ppiScreenX);
    float scaleY = (float)((float)ppiPrinterY/(float)ppiScreenY);
    
    // If printer pageWidth == current DC width, then this doesn't
    // change. But w might be the preview bitmap width, so scale down.
    float overallScaleX = scaleX * pageScaleX;
    float overallScaleY = scaleY * pageScaleY;
    dc->SetUserScale(overallScaleX, overallScaleY);
    
    // Make the margins.  They are just 1" on all sides now.
    float marginX = 1*ppiPrinterX, marginY = 1*ppiPrinterY;
    dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
    // Figure out the 'fake' window zoom
    float real_width = (pageWidth-2*marginX)/scaleX;
    float real_height = (pageHeight-2*marginY)/scaleY;
    float zoom_x = win_w/real_width, zoom_y = win_h/real_height;
    float real_zoom = gmax(zoom_x, zoom_y);
    tree->draw_settings.SetZoom(real_zoom, true);
  }
    
  tree->Render(*dc);
    
  tree->draw_settings.SetZoom(old_zoom);
  if (!dc->Colour)
    tree->draw_settings.SetOutcomeColor(color_outcomes);
    
  return TRUE;
}

Bool ExtensivePrintout::HasPage(int page)
{
  return (page <= 1);
}

ExtensivePrintout::ExtensivePrintout(TreeWindow *t, wxOutputOption f,
                                     const char *title)
  : tree(t), fit(f), wxPrintout((char *)title)
{ }

Bool ExtensivePrintout::OnBeginDocument(int startPage, int endPage)
{
  if (!wxPrintout::OnBeginDocument(startPage, endPage))
    return FALSE;
    
  return TRUE;
}

// Since we can not get at the actual device context in this function, we
// have no way to tell how many pages will be used in the wysiwyg mode. So,
// we have no choice but to disable the From:To page selection mechanism.
void ExtensivePrintout::GetPageInfo(int *minPage, int *maxPage,
                                    int *selPageFrom, int *selPageTo)
{
  num_pages = 1;
  *minPage = 0;
  *maxPage = num_pages;
  *selPageFrom = 0;
  *selPageTo = 0;
}
#endif

//***********************************************************************
//                      TREE-PRINT MENU HANDLER
//***********************************************************************
#ifdef wx_msw
void TreeWindow::print(wxOutputOption fit, bool preview)
{
  if (!preview) {
    wxPrinter printer;
    ExtensivePrintout printout(this, fit);
    printer.Print(pframe, &printout, TRUE);
  }
  else {
    wxPrintPreview *preview = new wxPrintPreview(new ExtensivePrintout(this, fit), new ExtensivePrintout(this, fit));
    wxPreviewFrame *ppframe = new wxPreviewFrame(preview, pframe, "Print Preview", 100, 100, 600, 650);
    ppframe->Centre(wxBOTH);
    ppframe->Initialize();
    ppframe->Show(TRUE);
  }
}
#else
void TreeWindow::print(wxOutputOption /*fit*/, bool preview)
{
  if (!preview)
    wxMessageBox("Printing not supported under X");
  else
    wxMessageBox("Print Preview is not supported under X");
}
#endif


//***********************************************************************
//                      TREE-PRINT EPS MENU HANDLER
//***********************************************************************

void TreeWindow::print_eps(wxOutputOption fit)
{
  wxPostScriptDC dc(NULL, TRUE);
  if (dc.Ok()) {
    float old_zoom = 1.0;

    if (fit == wxFITTOPAGE) {
      old_zoom = draw_settings.Zoom();
      int w, h;
      GetSize(&w, &h);
      draw_settings.SetZoom(gmin((float)w/(float)draw_settings.MaxX(),
				 (float)h/(float)draw_settings.MaxY()), true);
    }

    Bool color_outcomes = draw_settings.ColorCodedOutcomes();
    draw_settings.SetOutcomeColor(FALSE);
    dc.StartDoc("Gambit printout");
    dc.StartPage();
    Render(dc);
    dc.EndPage();
    dc.EndDoc();
    if (fit == wxFITTOPAGE)
      draw_settings.SetZoom(old_zoom);
    draw_settings.SetOutcomeColor(color_outcomes);
  }
}

//***********************************************************************
//                      TREE-PRINT MF MENU HANDLER
//***********************************************************************
#ifdef wx_msw
void TreeWindow::print_mf(wxOutputOption fit, bool save_mf)
{
  char *metafile_name = 0;

  if (save_mf)
    metafile_name = copystring(wxFileSelector("Save Metafile", 0, 0, ".wmf", "*.wmf"));

  wxMetaFileDC dc_mf(metafile_name);

  if (dc_mf.Ok()) {
    dc_mf.SetBackgroundMode(wxTRANSPARENT);
    float old_zoom;

    if (fit == wxFITTOPAGE) {
      old_zoom = draw_settings.Zoom();
      int w, h;
      GetSize(&w, &h);
      draw_settings.SetZoom(gmin((float)w/(float)draw_settings.MaxX(),
				 (float)h/(float)draw_settings.MaxY()), true);
    }

    if (!save_mf) 
      dc_mf.SetUserScale(2.0, 2.0);

    Render(dc_mf);

    if (fit == wxFITTOPAGE) 
      draw_settings.SetZoom(old_zoom);
        
    wxMetaFile *mf = dc_mf.Close();
    
    if (mf) {
      Bool success = mf->SetClipboard((int)(dc_mf.MaxX()+10), (int)(dc_mf.MaxY()+10));
      if (!success) 
	wxMessageBox("Copy Failed", "Error", wxOK | wxCENTRE, pframe);
      delete mf;
    }

    if (save_mf)
      wxMakeMetaFilePlaceable(
	  metafile_name, 0, 0, (int)(dc_mf.MaxX()+10), (int)(dc_mf.MaxY()+10));
  }
}
#else
void TreeWindow::print_mf(wxOutputOption /*fit*/, bool /*save_mf*/)
{
  wxMessageBox("Metafiles are not supported under X");
}
#endif

//-----------------------------------------------------------------------
//                     DISPLAY MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                      DISPLAY-ZOOM WINDOW MENU HANDLER
//***********************************************************************
void TreeWindow::display_zoom_win(void)
{
  if (!zoom_window) {
    zoom_window = new TreeZoomWindow(pframe, this, node_list, 
				     (const Infoset *&) hilight_infoset,
				     (const Infoset *&) hilight_infoset1, 
				     (const Node *&) mark_node, 
				     (const Node *&) subgame_node, 
				     draw_settings, GetNodeEntry(Cursor()));
  }
}

void TreeWindow::delete_zoom_win(void)
{
  // We don't delete this because this is called from zoom_window's
  // parent frame... would be bad :)
  // The virtual destructors should result in the window being deleted
  // by the system upon closing, so there ought to be no memory leakage
  zoom_window = 0;
}

//***********************************************************************
//                      DISPLAY-ZOOM MENU HANDLER
//***********************************************************************

void TreeWindow::display_set_zoom(float z)
{
    draw_settings.SetZoom(z);
    GetDC()->SetUserScale(draw_settings.Zoom(), draw_settings.Zoom());
    must_recalc = TRUE;
}

float TreeWindow::display_get_zoom(void)
{
    return draw_settings.Zoom();
}

#define MAX_WINDOW_WIDTH   750  // assuming an 800x600 display to be safe
#define MAX_WINDOW_HEIGHT  550
#define MIN_WINDOW_WIDTH   600  // at least 12 buttons
#define MIN_WINDOW_HEIGHT  300

void TreeWindow::display_zoom_fit(void)
{
    int width, height;
    Render(*GetDC());
    width = gmin(draw_settings.MaxX(), MAX_WINDOW_WIDTH);
    height = gmin(draw_settings.MaxY(), MAX_WINDOW_HEIGHT);
    
    double zoomx = (double)width/(double)draw_settings.MaxX();
    double zoomy = (double)height/(double)draw_settings.MaxY();
    
    zoomx = gmin(zoomx, 1.0); zoomy = gmin(zoomy, 1.0);  // never zoom in (only out)
    double zoom = gmin(zoomx, zoomy); // same zoom for vertical and horiz
    
    width = (int)(zoom*(double)draw_settings.MaxX());
    height = (int)(zoom*(double)draw_settings.MaxY());
    
    width = gmax(width, MIN_WINDOW_WIDTH);
    height = gmax(height, MIN_WINDOW_HEIGHT);
    
    draw_settings.SetZoom(zoom);
    GetDC()->SetUserScale(draw_settings.Zoom(), draw_settings.Zoom());
    pframe->SetClientSize(width, height+50); // +50 to account for the toolbar
    must_recalc = TRUE;
    ProcessCursor();
    OnPaint();
}


//***********************************************************************
//                      DISPLAY LEGENGS HANDLER
//***********************************************************************
// Controls what gets displayed above/below [node, branch]
void TreeWindow::display_legends(void)
{
    draw_settings.SetLegends();
}

//***********************************************************************
//                      DISPLAY OPTIONS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
#include "twflash.h"
void TreeWindow::display_options(void)
{
    draw_settings.SetOptions();
    // Must take care of flashing/nonflashing cursor here since draw_settings cannot
    if (draw_settings.FlashingCursor() == TRUE && flasher->Type() == myCursor)
    {
        delete (TreeNodeCursor *)flasher;
        flasher = new TreeNodeFlasher(GetDC());
    }
    
    if (draw_settings.FlashingCursor() == FALSE && flasher->Type() == myFlasher)
    {
        delete (TreeNodeFlasher *)flasher;
        flasher = new TreeNodeCursor(GetDC());
    }

    must_recalc = TRUE;
}

//***********************************************************************
//                      DISPLAY COLORS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
void TreeWindow::display_colors(void)
{
    gArray<gText> names(ef.NumPlayers());
    for (int i = 1; i <= names.Length(); i++)
        names[i] = (ef.Players()[i])->GetName();
    draw_settings.PlayerColorDialog(names);
    must_recalc = TRUE;
}

//***********************************************************************
//                      DISPLAY SAVE OPTIONS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
void TreeWindow::display_save_options(Bool def)
{
    if (def)
        draw_settings.SaveOptions();
    else
    {
        char *s = wxFileSelector("Save Configuration", NULL, NULL, NULL, "*.cfg");
        if (s)
        {
            draw_settings.SaveOptions(copystring(s)); 
            delete [] s;
        }
    }
}

//***********************************************************************
//                      DISPLAY LOAD OPTIONS HANDLER
//***********************************************************************

// Controls the size of the various tree parts
void TreeWindow::display_load_options(Bool def)
{
    if (def)
        draw_settings.LoadOptions(0);
    else
    {
        char *s = wxFileSelector("Load Configuration", NULL, NULL, NULL, "*.cfg");
        if (s)
        {
            draw_settings.LoadOptions(copystring(s));
            delete [] s;
        }
    }
    
    must_recalc = TRUE;
}

