//
// FILE: efggui.h -- the wrapper class for creating the extensive form GUI
//
// $Id$
//

#include "gmisc.h"
#include "gtext.h"

class wxFrame;
class Efg;
class EfgNfgInterface;

class EfgGUI {
private:
  int GetParams(Efg &, wxFrame *parent);

public:
  EfgGUI(Efg *p_efg, const gText &p_filename,
	 EfgNfgInterface *p_interface = 0, wxFrame *p_parent = 0);
};

