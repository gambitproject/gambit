//
// FILE: nfgtable.h -- Declaration of normal form table class
//
// $Id$
//

#ifndef NFGTABLE_H
#define NFGTABLE_H

#include "wx/grid.h"

class NfgTable : public wxPanel {
private:
  Nfg &m_nfg;
  wxWindow *m_parent;
  wxGrid *m_grid;
  bool m_editable, m_cursorMoving;
  int m_rowPlayer, m_colPlayer;
  NFSupport m_support;
  MixedSolution *m_solution;

  int m_showProb, m_showDom, m_showValue;

  class Settings {
  private:
    int m_decimals, m_outcomeValues;

  public:
    Settings(void);
    virtual ~Settings();

    void SetDecimals(int p_decimals) { m_decimals = p_decimals; }
    int GetDecimals(void) const { return m_decimals; }
    void SetOutcomeValues(bool p_outcomeValues) 
      { m_outcomeValues = p_outcomeValues; }
    bool OutcomeValues(void) const { return m_outcomeValues; }

    void LoadSettings(void);
    void SaveSettings(void) const;
  };

  Settings m_settings;


  // Event handlers
  void OnCellSelect(wxGridEvent &);
  void OnLeftDoubleClick(wxGridEvent &);
  void OnLabelLeftClick(wxGridEvent &);

public:
  NfgTable(Nfg &, wxWindow *);
  virtual ~NfgTable() { }

  void SetProfile(const gArray<int> &profile);
  gArray<int> GetProfile(void) const;

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

  bool OutcomeValues(void) const { return m_settings.OutcomeValues(); }
  void SetOutcomeValues(bool p_outcomeValues) 
    { m_settings.SetOutcomeValues(p_outcomeValues); }
  int GetDecimals(void) const { return m_settings.GetDecimals(); }
  void SetDecimals(int p_decimals) { m_settings.SetDecimals(p_decimals); }
  void SaveSettings(void) const { m_settings.SaveSettings(); }
  void LoadSettings(void) { m_settings.LoadSettings(); }

  void SetCellFont(const wxFont &p_font) 
    { m_grid->SetDefaultCellFont(p_font); }
  void SetLabelFont(const wxFont &p_font) { m_grid->SetLabelFont(p_font); }

  void SetSupport(const NFSupport &);
  const NFSupport &GetSupport(void) const { return m_support; }

  bool IsEditable(void) const { return m_editable; }
  void SetEditable(bool p_editable) { m_editable = p_editable; }

  int GetRowPlayer(void) const { return m_rowPlayer; }
  int GetColPlayer(void) const { return m_colPlayer; }

  void SetSolution(const MixedSolution &);
  const MixedSolution &GetSolution(void) const { return *m_solution; }
  void ClearSolution(void);

  DECLARE_EVENT_TABLE()
};

#endif  // NFGTABLE_H






