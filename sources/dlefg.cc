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


