//
// FILE: normgui.h -- wrapper class for creating normal form window
//
// $Id$
//

#include "gmisc.h"
#include "gtext.h"

class wxFrame;
class Nfg;
class EfgNfgInterface;
template <class T> class gArray;

class NfgGUI {
private:
  int GetPlayers(wxFrame *);
  int GetStrategies(gArray<int> &, wxFrame *);

public:
  NfgGUI(Nfg * = 0, const gText infile_name = "",
	 EfgNfgInterface *inter=0, wxFrame *parent=0);
};

