//
// FILE: Normgui.h	wrapper class form creating normal form GUI
//
// $Id$
//

#include "gmisc.h"
#include "gtext.h"
class wxFrame;
class Nfg;
class EfgNfgInterface;
template <class T> class gArray;
class NfgGUI
{
private:
	int GetNFParams(gArray<int> &dimensionality,gArray<gText> &names,wxFrame *parent);
public:
	NfgGUI(Nfg *nf=0, const gText infile_name=gText(),
   		EfgNfgInterface *inter=0, wxFrame *parent=0);
};

