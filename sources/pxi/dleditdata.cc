//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing experimental data
//

#include "wx/wx.h"
#ifndef WX_PRECOMP
#include "wx/wxprec.h"
#endif  // WX_PRECOMP

#include "dleditdata.h"

//=========================================================================
//               class dialogEditData: Member functions
//=========================================================================

dialogEditData::dialogEditData(wxWindow *p_parent, const ExpData &p_data)
  : wxDialog(p_parent, -1, "Edit Experimental Data"),
    m_data(p_data)
{
  m_grid = new wxGrid(this, -1, wxDefaultPosition, wxSize(300, 300));
  m_grid->CreateGrid(p_data.NumPoints(), p_data.NumActions() + 1);
  m_grid->UpdateDimensions();

  m_grid->SetLabelValue(wxHORIZONTAL, "Lambda", 0);
  int col = 1;
  for (int iset = 1; iset <= m_data.NumInfosets(); iset++) {
    for (int act = 1; act <= m_data.NumActions(iset); act++) {
      m_grid->SetLabelValue(wxHORIZONTAL, 
			    wxString::Format("Infoset %d\nAction %d",
					     iset, act),
			    col++);
    }
  }

  wxButton *okButton = new wxButton(this, wxID_OK, "OK");
  okButton->SetDefault();
  wxButton *cancelButton = new wxButton(this, wxID_CANCEL, "Cancel");

  wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
  buttonSizer->Add(okButton, 0, wxALL, 5);
  buttonSizer->Add(cancelButton, 0, wxALL, 5);

  wxBoxSizer *topSizer = new wxBoxSizer(wxVERTICAL);
  topSizer->Add(m_grid, 0, wxALL | wxCENTER, 5);
  topSizer->Add(buttonSizer, 0, wxALL | wxCENTER, 5);

  SetAutoLayout(true);
  SetSizer(topSizer);
  topSizer->Fit(this);
  topSizer->SetSizeHints(this);
  Layout();

  UpdateValues();
}

void dialogEditData::UpdateValues(void)
{
  for (int i = 1; i <= m_data.NumPoints(); i++) {
    if (m_data.HaveMLEs()) {
      m_grid->SetCellValue(i-1, 0,
			   wxString::Format("%f", m_data.MLELambda(i)));
    }
    else {
      m_grid->SetCellValue(i-1, 0, "");
    }

    int col = 1;
    for (int iset = 1; iset <= m_data.NumInfosets(); iset++) {
      for (int act = 1; act <= m_data.NumActions(iset); act++) {
	m_grid->SetCellValue(i-1, col++, 
			     wxString::Format("%d",
					      m_data.GetDataPoint(i, iset, act)));
      }
    }
  }
}

ExpData dialogEditData::GetData(void) const
{
  ExpData data(m_data);

  while (data.NumPoints() > 0) {
    data.RemoveDataPoint(1);
  }

  for (int i = 0; i < m_grid->GetRows(); i++) {
    data.AddDataPoint();

    int col = 1;
    for (int iset = 1; iset <= data.NumInfosets(); iset++) {
      for (int act = 1; act <= data.NumActions(iset); act++) {
	data.SetDataPoint(i+1, iset, act, 
			  atoi(m_grid->GetCellValue(i, col++)));
      }
    }
  }

  return data;
}
