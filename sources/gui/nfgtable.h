//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Panel to display normal form games in tabular format
//

#ifndef NFGTABLE_H
#define NFGTABLE_H

#include "wx/grid.h"

class NfgTableSettings {
private:
  bool m_outcomeValues;
  long m_decimals;
  wxFont m_dataFont, m_labelFont;

  static void LoadFont(const wxString &, const wxConfig &, wxFont &);
  static void SaveFont(const wxString &, wxConfig &, const wxFont &);

public:
  NfgTableSettings(void);

  void SetDecimals(int p_decimals) { m_decimals = p_decimals; }
  long GetDecimals(void) const { return m_decimals; }

  void SetOutcomeValues(bool p_outcomeValues) 
    { m_outcomeValues = p_outcomeValues; }
  bool OutcomeValues(void) const { return m_outcomeValues; }

  void SetDataFont(const wxFont &p_font) { m_dataFont = p_font; }
  const wxFont &GetDataFont(void) const { return m_dataFont; }

  void SetLabelFont(const wxFont &p_font) { m_labelFont = p_font; }
  const wxFont &GetLabelFont(void) const { return m_labelFont; }

  void LoadSettings(void);
  void SaveSettings(void) const;
};


class NfgTable : public wxPanel {
private:
  Nfg &m_nfg;
  wxWindow *m_parent;
  wxGrid *m_grid;
  bool m_editable, m_cursorMoving;
  int m_rowPlayer, m_colPlayer;
  NFSupport m_support;
  MixedSolution *m_profile;

  int m_showProb, m_showDom, m_showValue;

  NfgTableSettings m_settings;

  // Event handlers
  void OnCellSelect(wxGridEvent &);
  void OnLeftDoubleClick(wxGridEvent &);
  void OnLabelLeftClick(wxGridEvent &);

public:
  NfgTable(Nfg &, wxWindow *);
  virtual ~NfgTable() { }

  void SetContingency(const gArray<int> &profile);
  gArray<int> GetContingency(void) const;

  void SetStrategy(int p_player, int p_strategy);
  void SetPlayers(int p_rowPlayer, int p_colPlayer);

  int GetRowStrategy(void) const { return m_grid->GetCursorRow() + 1; }
  int GetColStrategy(void) const { return m_grid->GetCursorColumn() + 1; }

  void ToggleProbs(void);
  int ShowProbs(void) const { return m_showProb; }
  void ToggleDominance(void);
  int ShowDominance(void) const { return m_showDom; }
  void ToggleValues(void);
  int ShowValues(void) const { return m_showValue; }

  const NfgTableSettings &GetSettings(void) const { return m_settings; }
  NfgTableSettings &GetSettings(void) { return m_settings; }

  void SetDataFont(const wxFont &p_font) 
    { m_settings.SetDataFont(p_font); m_grid->SetDefaultCellFont(p_font); }
  void SetLabelFont(const wxFont &p_font) 
    { m_settings.SetLabelFont(p_font); m_grid->SetLabelFont(p_font); }

  void SetSupport(const NFSupport &);
  const NFSupport &GetSupport(void) const { return m_support; }

  bool IsEditable(void) const { return m_editable; }
  void SetEditable(bool p_editable) { m_editable = p_editable; }

  int GetRowPlayer(void) const { return m_rowPlayer; }
  int GetColPlayer(void) const { return m_colPlayer; }

  void SetProfile(const MixedSolution &);
  const MixedSolution &GetProfile(void) const { return *m_profile; }
  void ClearProfile(void);
  
  void RefreshTable(void);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGTABLE_H






