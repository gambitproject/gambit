//
// FILE: dialogsupportselect.cc -- Support selection dialog
//
// $Id$
//

#include "wx/wxprec.h"

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#ifndef WX_PRECOMP
#include "wx/wx.h"
#include "wx/mdi.h"
#endif

#include "gstream.h"
#include "glist.h"
#include "gtext.h"
#include "gnumber.h"

#include "nfg.h"
#include "nfstrat.h"

#include "dialogauto.h"
#include "dialogsupportselect.h"

//=========================================================================
//                class dialogSupportSelect: Member functions
//=========================================================================

dialogSupportSelect::dialogSupportSelect(wxWindow *p_parent,
					 const gList<NFSupport *> &p_supports,
					 int p_current, const gText &p_caption)
  : guiAutoDialog(p_parent, p_caption)
{
  m_supportList = new wxListBox(this, -1);
  for (int s = 1; s <= p_supports.Length(); s++) {
    NFSupport *support = p_supports[s];
    gText item = ToText(s) + ": " + support->GetName();

    item += (" (" + ToText(support->NumStrats(1)) + ", " +
	     ToText(support->NumStrats(2)));
    if (support->Game().NumPlayers() > 2) {
      item += ", " + ToText(support->NumStrats(3));
      if (support->Game().NumPlayers() > 3) 
	item += ",...)";
      else
	item += ")";
    }
    else
      item += ")";

    m_supportList->Append((char *) item);
    if (s == p_current) {
      m_supportList->SetSelection(s - 1);
    }
  }

  m_okButton->GetConstraints()->top.SameAs(m_supportList, wxBottom, 10);
  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  m_supportList->SetConstraints(new wxLayoutConstraints);
  m_supportList->GetConstraints()->left.SameAs(m_okButton, wxLeft);
  m_supportList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_supportList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_supportList->GetConstraints()->height.AsIs();
  
  AutoSize();
}

#ifdef UNUSED
dialogSupportSelect::dialogSupportSelect(const gList<EFSupport *> &p_supports,
					 EFSupport *p_current,
					 const gText &p_caption,
					 wxWindow *p_parent /*=0*/)
  : guiAutoDialog(p_parent, p_caption)
{
  SetLabelPosition(wxVERTICAL);
  m_supportList = new wxListBox(this, 0, "Support", wxSINGLE, 1, 1);
  for (int s = 1; s <= p_supports.Length(); s++) {
    EFSupport *support = p_supports[s];
    gText item = ToText(s) + ": " + support->GetName();
    m_supportList->Append(item);
    if (p_supports[s] == p_current) {
      m_supportList->SetSelection(s - 1);
    }
  }

  m_okButton->GetConstraints()->top.SameAs(m_supportList, wxBottom, 10);
  m_okButton->GetConstraints()->left.SameAs(this, wxLeft, 10);
  m_okButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_okButton->GetConstraints()->height.AsIs();

  m_cancelButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_cancelButton->GetConstraints()->left.SameAs(m_okButton, wxRight, 10);
  m_cancelButton->GetConstraints()->width.AsIs();
  m_cancelButton->GetConstraints()->height.AsIs();

  m_helpButton->GetConstraints()->centreY.SameAs(m_okButton, wxCentreY);
  m_helpButton->GetConstraints()->left.SameAs(m_cancelButton, wxRight, 10);
  m_helpButton->GetConstraints()->width.SameAs(m_cancelButton, wxWidth);
  m_helpButton->GetConstraints()->height.AsIs();

  m_supportList->SetConstraints(new wxLayoutConstraints);
  m_supportList->GetConstraints()->left.SameAs(m_okButton, wxLeft);
  m_supportList->GetConstraints()->top.SameAs(this, wxTop, 10);
  m_supportList->GetConstraints()->right.SameAs(m_helpButton, wxRight);
  m_supportList->GetConstraints()->height.AsIs();
  
  Go();
}
#endif  // UNUSED
