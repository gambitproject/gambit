//
// FILE: gambdraw.h -- Generic display parameters
//
// $Id$
//

#ifndef GAMBDRAW_H
#define GAMBDRAW_H

#include "gblock.h"
#include "gtext.h"

class GambitDrawSettings {
protected:
  mutable gBlock<wxColour> m_colors;

public:
  GambitDrawSettings(void);
  virtual ~GambitDrawSettings() { }

  void SetPlayerColor(int pl, const wxColour &);
  const wxColour &GetPlayerColor(int pl) const;

  void LoadOptions(void);
  void SaveOptions(void) const;
};

#endif  // GAMBDRAW_H

