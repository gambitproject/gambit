//
// FILE: Normgui.h	wrapper class form creating normal form GUI
//
// $Id$
//

#include "gmisc.h"
#include "gstring.h"
class wxFrame;
class Nfg;
class EfgNfgInterface;
template <class T> class gArray;
class NfgGUI
{
private:
	int GetNFParams(gArray<int> &dimensionality);
public:
	NfgGUI(Nfg *nf=0, const gString infile_name=gString(),
   		EfgNfgInterface *inter=0, wxFrame *parent=0);
};

