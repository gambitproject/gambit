// Extgui.h  the wrapper class for creating the extensive form GUI
// $Id$
#include "gmisc.h"
#include "gtext.h"
class wxFrame;
class Efg;
class EfgNfgInterface;
class EfgGUI
{
private:
	int GetEFParams(gArray<gText> &names,wxFrame *parent);
public:
	EfgGUI(Efg *ef=0, const gText infile_name=gText(),
			 EfgNfgInterface *inter=0, wxFrame *parent=0);
};

