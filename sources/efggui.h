//
// FILE: efggui.h -- the wrapper class for creating the extensive form GUI
//
// $Id$
//

#include "gmisc.h"
#include "gtext.h"

class wxFrame;
class FullEfg;
class EfgNfgInterface;

class EfgGUI {
private:
  int GetParams(FullEfg &, wxFrame *parent);

public:
  EfgGUI(FullEfg *p_efg, const gText &p_filename,
	 EfgNfgInterface *p_interface = 0, wxFrame *p_parent = 0);
};

