// Extgui.h  the wrapper class for creating the extensive form GUI
// $Id$
#include "gmisc.h"
#include "gstring.h"
class wxFrame;
class Efg;
class EfgNfgInterface;
class EfgGUI
{
private:
	int GetEFParams(void);
public:
	EfgGUI(Efg *ef=0, const gString infile_name=gString(),
			 EfgNfgInterface *inter=0, wxFrame *parent=0);
};

