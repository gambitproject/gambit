//
// FILE: dlefg.cc -- Extensive form-related dialog implementations
//
//
//

#include "wx.h"
#include "wxmisc.h"

#include "efgconst.h"

#include "efg.h"
#include "dlefgplayer.h"
#include "dlmoveadd.h"
#include "dlnodedelete.h"
#include "dlefgsave.h"


//=========================================================================
//                  dialogEfgSelectPlayer: Member functions
//=========================================================================

dialogEfgSelectPlayer::dialogEfgSelectPlayer(Efg &p_efg, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Select Player", TRUE), m_efg(p_efg)
{
  m_playerNameList = new wxListBox(this, 0, "Player");
  m_playerNameList->Append("Chance");

  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    const gText &name = m_efg.Players()[pl]->GetName();
    if (name != "")
      m_playerNameList->Append(name);
    else
      m_playerNameList->Append("Player" + ToText(pl));
  }

  // Force a selection -- some implementations (e.g. Motif) do not
  // automatically set any selection
  m_playerNameList->SetSelection(0); 

  NewLine();
  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);
  wxButton *helpButton = new wxButton(this, (wxFunction) CallbackHelp,
				      "Help");
  helpButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

dialogEfgSelectPlayer::~dialogEfgSelectPlayer()
{ }

void dialogEfgSelectPlayer::OnOK(void)
{
  m_playerSelected = m_playerNameList->GetSelection();
  m_completed = wxOK;
  Show(FALSE);
}

void dialogEfgSelectPlayer::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogEfgSelectPlayer::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

EFPlayer *dialogEfgSelectPlayer::GetPlayer(void)
{
  if (m_playerSelected == 0)
    return m_efg.GetChance();
  else
    return m_efg.Players()[m_playerSelected];
}

//=========================================================================
//                     dialogMoveAdd: Member functions
//=========================================================================

dialogMoveAdd::dialogMoveAdd(Efg &p_efg, EFPlayer *p_player,
			     Infoset *p_infoset, int p_branches,
			     wxFrame *p_frame)
  : wxDialogBox(p_frame, "Add Move", TRUE),
    m_efg(p_efg), m_branches(p_branches)
{
  m_playerItem = new wxListBox(this, (wxFunction) CallbackPlayer, "Player");
  m_playerItem->Append("Chance");
  m_playerItem->SetClientData(0, (char *) this);
  for (int pl = 1; pl <= m_efg.NumPlayers(); pl++) {
    m_playerItem->Append(ToText(pl) + ": " + m_efg.Players()[pl]->GetName());
  }
  m_playerItem->Append("New Player");
  if (p_player)
    m_playerItem->SetSelection(p_player->GetNumber());
  else
    m_playerItem->SetSelection(0);

  m_infosetItem = new wxListBox(this, (wxFunction) CallbackInfoset,
				"Infoset");
  m_infosetItem->Append("New");
  m_infosetItem->SetClientData(0, (char *) this);
  if (p_player) {
    for (int iset = 1; iset <= p_player->NumInfosets(); iset++) {
      m_infosetItem->Append(ToText(iset) + ": " +
			    p_player->Infosets()[iset]->GetName());
    }
  }

  if (p_infoset)
    m_infosetItem->SetSelection(p_infoset->GetNumber());
  else
    m_infosetItem->SetSelection(0);

  NewLine();
 
  m_actionItem = new wxText(this, 0, "Actions");
  m_actionItem->SetValue(ToText(p_branches));
  m_actionItem->Enable(m_infosetItem->GetSelection() == 0);

  wxButton *okButton = new wxButton(this, (wxFunction) CallbackOK, "Ok");
  okButton->SetClientData((char *) this);
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, (wxFunction) CallbackCancel,
					"Cancel");
  cancelButton->SetClientData((char *) this);

  Fit();
  Show(TRUE);
}

dialogMoveAdd::~dialogMoveAdd(void)
{ }

void dialogMoveAdd::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogMoveAdd::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

void dialogMoveAdd::OnPlayer(void)
{
  int playerNumber = m_playerItem->GetSelection();
  EFPlayer *player = 0;
  if (playerNumber == 0)
    player = m_efg.GetChance();
  else if (playerNumber <= m_efg.NumPlayers())
    player = m_efg.Players()[playerNumber];

  m_infosetItem->Clear();
  m_infosetItem->Append("New");
  m_infosetItem->SetClientData(0, (char *) this);
  if (player) {
    for (int iset = 1; iset <= player->NumInfosets(); iset++) {
      m_infosetItem->Append(ToText(iset) + ": " +
			    player->Infosets()[iset]->GetName());
    }
  }
  m_infosetItem->SetSelection(0);
  m_actionItem->SetValue(ToText(2));
  m_actionItem->Enable(TRUE);
}

void dialogMoveAdd::OnInfoset(void)
{
  int infosetNumber = m_infosetItem->GetSelection();
  if (infosetNumber > 0) {
    int playerNumber = m_playerItem->GetSelection();
    Infoset *infoset;
    if (playerNumber == 0)
      infoset = m_efg.GetChance()->Infosets()[infosetNumber];
    else
      infoset = m_efg.Players()[playerNumber]->Infosets()[infosetNumber];
    m_actionItem->Enable(FALSE);
    m_actionItem->SetValue(ToText(infoset->NumActions()));
  }
  else {
    m_actionItem->Enable(TRUE);
    m_actionItem->SetValue(ToText(2));
  }
}

Bool dialogMoveAdd::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

NodeAddMode dialogMoveAdd::GetAddMode(void) const
{
  return (m_infosetItem->GetSelection() == 0) ? NodeAddNew : NodeAddIset;
}

EFPlayer *dialogMoveAdd::GetPlayer(void) const
{
  int playerNumber = m_playerItem->GetSelection();

  if (playerNumber == 0)
    return m_efg.GetChance();
  else if (playerNumber <= m_efg.NumPlayers())
    return m_efg.Players()[playerNumber];
  else
    return 0;
}

Infoset *dialogMoveAdd::GetInfoset(void) const
{
  EFPlayer *player = GetPlayer();
  int infosetNumber = m_infosetItem->GetSelection();

  if (player && infosetNumber > 0)
    return player->Infosets()[infosetNumber];
  else
    return 0;
}

int dialogMoveAdd::GetActions(void) const
{
  Infoset *infoset = GetInfoset();
  if (infoset)
    return infoset->NumActions();
  else
    return (int) ToDouble(m_actionItem->GetValue());
}


//=========================================================================
//                   dialogNodeDelete: Member functions
//=========================================================================

dialogNodeDelete::dialogNodeDelete(Node *p_node, wxWindow *p_parent)
  : wxDialogBox(p_parent, "Delete Node", TRUE), m_node(p_node)
{
  SetLabelPosition(wxVERTICAL);
  m_branchList = new wxListBox(this, 0, "Keep subtree at branch");
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

void dialogNodeDelete::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogNodeDelete::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogNodeDelete::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

//=========================================================================
//                     dialogEfgSave: Member functions
//=========================================================================

dialogEfgSave::dialogEfgSave(const gText &p_name,
			     const gText &p_label, int p_decimals,
			     wxWindow *p_parent)
  : wxDialogBox(p_parent, "Save File", TRUE)
{
  m_fileName = new wxText(this, 0, "Path:");
  m_fileName->SetValue(p_name);

  wxButton *browseButton = new wxButton(this, (wxFunction) CallbackBrowse,
					"Browse...");
  browseButton->SetClientData((char *) this);
  NewLine();

  m_treeLabel = new wxText(this, 0, "Description:", p_label, -1, -1, 300);
  m_treeLabel->SetValue(p_label);
  NewLine();

  m_numDecimals = new wxSlider(this, 0, "Decimal places:",
			       p_decimals, 0, 25, 100);
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

void dialogEfgSave::OnOK(void)
{
  m_completed = wxOK;
  Show(FALSE);
}

void dialogEfgSave::OnCancel(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
}

Bool dialogEfgSave::OnClose(void)
{
  m_completed = wxCANCEL;
  Show(FALSE);
  return FALSE;
}

void dialogEfgSave::OnBrowse(void)
{
  char *file = wxFileSelector("Save data file", 
			      gPathOnly(m_fileName->GetValue()),
			      gFileNameFromPath(m_fileName->GetValue()),
			      ".efg", "*.efg");

  if (file) {
    m_fileName->SetValue(file);
  }
}


