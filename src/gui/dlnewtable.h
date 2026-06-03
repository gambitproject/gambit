#ifndef GAMBIT_GUI_DLNEWTABLE_H
#define GAMBIT_GUI_DLNEWTABLE_H

#include <vector>

#include <wx/dialog.h>

class wxChoice;
class wxSpinCtrl;
class wxStaticText;
class wxBoxSizer;

namespace Gambit::GUI {

class NewTableDialog : public wxDialog {
public:
  explicit NewTableDialog(wxWindow *parent);

  std::vector<int> GetDimensions() const;

private:
  void CreateControls();
  void LayoutControls();

  void RebuildStrategyControls();

  void OnPlayerCountChanged(wxSpinEvent &p_event);

  wxSpinCtrl *m_playerCountCtrl{nullptr};

  wxBoxSizer *m_topSizer{nullptr};
  wxBoxSizer *m_strategyRowsSizer{nullptr};

  std::vector<wxStaticText *> m_strategyLabels;
  std::vector<wxSpinCtrl *> m_strategyCtrls;
};

} // namespace Gambit::GUI

#endif // GAMBIT_GUI_DLNEWTABLE_H
