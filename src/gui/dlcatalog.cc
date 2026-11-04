//
// This file is part of Gambit
// Copyright (c) 1994-2026, The Gambit Project (https://www.gambit-project.org)
//
// FILE: src/gui/dlcatalog.cc
// Dialog for browsing and opening a game from Gambit's games catalog
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#include <algorithm>

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // WX_PRECOMP
#include <wx/dataview.h>
#include <wx/srchctrl.h>

#include "dlcatalog.h"

namespace Gambit::GUI {

namespace {

wxString FormatLabel(const CatalogEntry &p_entry)
{
  return p_entry.isTree ? wxString(_("Extensive")) : wxString(_("Strategic"));
}

} // namespace

CatalogBrowserDialog::CatalogBrowserDialog(wxWindow *p_parent)
  : wxDialog(p_parent, wxID_ANY, _("Browse Games Catalog"), wxDefaultPosition,
             wxSize(FromDIP(780), FromDIP(520)), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
  m_catalogRoot = FindCatalogRoot();
  wxString error;
  if (!m_catalogRoot.IsEmpty()) {
    m_allEntries = LoadCatalogManifest(m_catalogRoot, &error);
  }
  else {
    error = _("Gambit could not locate its games catalog on this installation.");
  }

  CreateControls();

  if (m_allEntries.empty()) {
    m_searchCtrl->Disable();
    m_formatFilter->Disable();
    m_list->Disable();
    m_detailDescription->SetValue(!error.IsEmpty() ? error
                                                   : wxString(_("The games catalog is empty.")));
  }
  else {
    ApplyFilter();
  }
}

void CatalogBrowserDialog::CreateControls()
{
  const int S = FromDIP(5);
  auto *topSizer = new wxBoxSizer(wxVERTICAL);

  auto *filterSizer = new wxBoxSizer(wxHORIZONTAL);
  m_searchCtrl = new wxSearchCtrl(this, wxID_ANY);
  m_searchCtrl->ShowSearchButton(true);
  m_searchCtrl->ShowCancelButton(true);
  m_searchCtrl->SetDescriptiveText(_("Search catalog"));
  filterSizer->Add(m_searchCtrl, 1, wxALL | wxALIGN_CENTER_VERTICAL, S);

  wxString formatChoices[] = {_("All formats"), _("Extensive form"), _("Strategic form")};
  m_formatFilter =
      new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 3, formatChoices);
  m_formatFilter->SetSelection(0);
  filterSizer->Add(m_formatFilter, 0, wxALL | wxALIGN_CENTER_VERTICAL, S);

  topSizer->Add(filterSizer, 0, wxEXPAND);

  auto *contentSizer = new wxBoxSizer(wxHORIZONTAL);

  m_list = new wxDataViewListCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(FromDIP(420), -1));
  m_list->AppendTextColumn(_("Title"), wxDATAVIEW_CELL_INERT, FromDIP(190));
  m_list->AppendTextColumn(_("Source"), wxDATAVIEW_CELL_INERT, FromDIP(150));
  m_list->AppendTextColumn(_("Players"), wxDATAVIEW_CELL_INERT, FromDIP(55));
  m_list->AppendTextColumn(_("Format"), wxDATAVIEW_CELL_INERT, FromDIP(75));
  contentSizer->Add(m_list, 1, wxALL | wxEXPAND, S);

  auto *detailSizer = new wxBoxSizer(wxVERTICAL);
  m_detailTitle = new wxStaticText(this, wxID_ANY, wxEmptyString);
  wxFont titleFont = m_detailTitle->GetFont();
  titleFont.SetWeight(wxFONTWEIGHT_BOLD);
  titleFont.SetPointSize(titleFont.GetPointSize() + 2);
  m_detailTitle->SetFont(titleFont);
  detailSizer->Add(m_detailTitle, 0, wxALL | wxEXPAND, S);

  m_detailThumbnail = new wxStaticBitmap(this, wxID_ANY, wxNullBitmap, wxDefaultPosition,
                                         wxSize(FromDIP(280), FromDIP(190)));
  detailSizer->Add(m_detailThumbnail, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, S);

  m_detailDescription =
      new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize,
                     wxTE_MULTILINE | wxTE_READONLY | wxBORDER_NONE);
  detailSizer->Add(m_detailDescription, 1, wxALL | wxEXPAND, S);

  contentSizer->Add(detailSizer, 1, wxEXPAND);
  topSizer->Add(contentSizer, 1, wxEXPAND);

  if (auto *buttonSizer = CreateStdDialogButtonSizer(wxOK | wxCANCEL)) {
    topSizer->Add(buttonSizer, 0, wxALL | wxEXPAND, S);
  }
  m_openButton = static_cast<wxButton *>(FindWindow(wxID_OK));
  if (m_openButton) {
    m_openButton->SetLabel(_("Open"));
    m_openButton->Disable();
  }

  SetSizer(topSizer);
  topSizer->SetSizeHints(this);
  CenterOnParent();

  m_searchCtrl->Bind(wxEVT_TEXT, &CatalogBrowserDialog::OnSearchText, this);
  m_formatFilter->Bind(wxEVT_CHOICE, &CatalogBrowserDialog::OnFormatFilter, this);
  m_list->Bind(wxEVT_DATAVIEW_SELECTION_CHANGED, &CatalogBrowserDialog::OnSelectionChanged, this);
  m_list->Bind(wxEVT_DATAVIEW_ITEM_ACTIVATED, &CatalogBrowserDialog::OnItemActivated, this);
  Bind(wxEVT_BUTTON, &CatalogBrowserDialog::OnOpen, this, wxID_OK);
}

void CatalogBrowserDialog::ApplyFilter()
{
  wxString needle = m_searchCtrl->GetValue().Lower();
  int formatSelection = m_formatFilter->GetSelection(); // 0 = all, 1 = efg, 2 = nfg

  m_visibleIndices.clear();
  m_list->DeleteAllItems();

  for (size_t i = 0; i < m_allEntries.size(); i++) {
    const CatalogEntry &entry = m_allEntries[i];
    if (formatSelection == 1 && entry.format != "efg") {
      continue;
    }
    if (formatSelection == 2 && entry.format != "nfg") {
      continue;
    }
    if (!needle.IsEmpty()) {
      wxString haystack =
          (entry.title + " " + entry.description + " " + entry.group + " " + entry.category)
              .Lower();
      if (haystack.Find(needle) == wxNOT_FOUND) {
        continue;
      }
    }

    m_visibleIndices.push_back(i);
    wxVector<wxVariant> row;
    row.push_back(wxVariant(entry.title));
    row.push_back(wxVariant(entry.group));
    row.push_back(wxVariant(wxString::Format("%d", entry.numPlayers)));
    row.push_back(wxVariant(FormatLabel(entry)));
    m_list->AppendItem(row);
  }

  UpdateDetailsPane();
}

int CatalogBrowserDialog::GetSelectedEntryIndex() const
{
  int row = m_list->GetSelectedRow();
  if (row == wxNOT_FOUND || static_cast<size_t>(row) >= m_visibleIndices.size()) {
    return -1;
  }
  return static_cast<int>(m_visibleIndices[row]);
}

void CatalogBrowserDialog::UpdateDetailsPane()
{
  int index = GetSelectedEntryIndex();
  if (index < 0) {
    m_detailTitle->SetLabel(wxEmptyString);
    m_detailDescription->SetValue(wxEmptyString);
    m_detailThumbnail->SetBitmap(wxNullBitmap);
    m_openButton->Disable();
    return;
  }

  const CatalogEntry &entry = m_allEntries[static_cast<size_t>(index)];
  m_detailTitle->SetLabel(entry.title);
  m_detailDescription->SetValue(entry.description);

  wxString thumbnailPath = m_catalogRoot + "/" + entry.thumbnail;
  wxImage image;
  if (!entry.thumbnail.IsEmpty() && wxFileExists(thumbnailPath) &&
      image.LoadFile(thumbnailPath, wxBITMAP_TYPE_PNG)) {
    wxSize target = m_detailThumbnail->GetSize();
    double scale = std::min(static_cast<double>(target.GetWidth()) / image.GetWidth(),
                            static_cast<double>(target.GetHeight()) / image.GetHeight());
    if (scale < 1.0 && scale > 0.0) {
      image.Rescale(static_cast<int>(image.GetWidth() * scale),
                    static_cast<int>(image.GetHeight() * scale), wxIMAGE_QUALITY_HIGH);
    }
    m_detailThumbnail->SetBitmap(wxBitmap(image));
  }
  else {
    m_detailThumbnail->SetBitmap(wxNullBitmap);
  }

  m_openButton->Enable();
  Layout();
}

wxString CatalogBrowserDialog::GetSelectedFilePath() const { return m_selectedFilePath; }

void CatalogBrowserDialog::OnSearchText(wxCommandEvent &) { ApplyFilter(); }

void CatalogBrowserDialog::OnFormatFilter(wxCommandEvent &) { ApplyFilter(); }

void CatalogBrowserDialog::OnSelectionChanged(wxDataViewEvent &) { UpdateDetailsPane(); }

void CatalogBrowserDialog::OnItemActivated(wxDataViewEvent &)
{
  wxCommandEvent dummy;
  OnOpen(dummy);
}

void CatalogBrowserDialog::OnOpen(wxCommandEvent &)
{
  int index = GetSelectedEntryIndex();
  if (index < 0) {
    return;
  }
  m_selectedFilePath =
      CatalogGameFilePath(m_catalogRoot, m_allEntries[static_cast<size_t>(index)]);
  EndModal(wxID_OK);
}

} // namespace Gambit::GUI
