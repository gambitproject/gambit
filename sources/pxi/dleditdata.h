//
// $Source$
// $Date$
// $Revision$
//
// DESCRIPTION:
// Dialog for viewing and editing experimental data
//

#ifndef DLEDITDATA_H
#define DLEDITDATA_H

#include "wx/grid.h"
#include "expdata.h"

class dialogEditData : public wxDialog {
private:
  wxGrid *m_grid;
  ExpData m_data;

  void UpdateValues(void);

public:
  dialogEditData(wxWindow *p_parent, const ExpData &p_data);

  ExpData GetData(void) const;
};

#endif   // DLEDITDATA_H

