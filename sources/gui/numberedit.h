//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// wxWindows grid cell editor specialized to handle entry of 
// floating-point/rational numbers
//

#ifndef NUMBEREDIT_H
#define NUMBEREDIT_H

class NumberEditor : public wxGridCellTextEditor {
private:
  wxString m_valueOld;

protected:
  // string representation of m_valueOld
  wxString GetString() const;

public:
  NumberEditor(void);

  virtual void Create(wxWindow* parent, wxWindowID id,
		      wxEvtHandler* evtHandler);

  virtual bool IsAcceptedKey(wxKeyEvent& event);
  virtual void BeginEdit(int row, int col, wxGrid* grid);
  virtual bool EndEdit(int row, int col, wxGrid* grid);

  virtual void Reset();
  virtual void StartingKey(wxKeyEvent& event);

  virtual wxGridCellEditor *Clone() const
  { return new NumberEditor; }

  // parameters string format is "width,precision"
  virtual void SetParameters(const wxString& params);
};

#endif  // NUMBEREDIT_H

