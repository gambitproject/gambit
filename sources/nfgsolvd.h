// File: nfgsolvd.h -- the main dialog for running NormalForm solution
// algorithms.  You must add an entry here for each new algorithm.
// Update: this now takes the number of players so that it can turn
// of the algorithms that will not work with this number of players.
// Please list these explicitly
// Only work for 2 players: Lemke, Enum
#define SD_CANCEL			-1
#define SD_SOLVE			1
#define	SD_ALGORITHM 	2
#define SD_INSPECT 		3

#ifndef NFG_SOLVE_HELP	// if this is included in efgsolvd.h, we do not need it
#define NFG_SOLVE_HELP	""
#endif
// solution module constants.  Do not change the order.  Add new ones
// just before NFG_NUM_SOLUTIONS.
typedef enum {NFG_NO_SOLUTION=-1,NFG_PURENASH_SOLUTION,NFG_LEMKE_SOLUTION,
							NFG_NLIAP_SOLUTION,NFG_ELIMDOM_SOLUTION,NFG_GRID_SOLUTION,
							NFG_NGOBIT_SOLUTION,NFG_SIMPDIV_SOLUTION,NFG_ENUM_SOLUTION,
							NFG_NUM_SOLUTIONS} NfgSolutionT;

// This mini-class is necessary to allow the extensive form solution dialog
// box to display the same normal form algorithms as the normal form.  Since
// we do not want to maintain the same code in two places, EfgSolveParamsDialog
// is derived from NfgAlgorithmList class.
class NfgAlgorithmList
{
public:
	wxRadioBox *MakeNfgAlgorithmList(int num_players,wxPanel *parent,wxFunction func=0)
	{
		char *nfg_algorithm_list[NFG_NUM_SOLUTIONS];
		nfg_algorithm_list[NFG_PURENASH_SOLUTION]="Pure Nash";
		nfg_algorithm_list[NFG_LEMKE_SOLUTION]="Lemke";
		nfg_algorithm_list[NFG_NLIAP_SOLUTION]="nLiap";
		nfg_algorithm_list[NFG_ELIMDOM_SOLUTION]="ElimDom";
		nfg_algorithm_list[NFG_GRID_SOLUTION]="GridSolve";
		nfg_algorithm_list[NFG_NGOBIT_SOLUTION]="nGobit";
		nfg_algorithm_list[NFG_SIMPDIV_SOLUTION]="Simpdiv";
		nfg_algorithm_list[NFG_ENUM_SOLUTION]="Enum";
		wxRadioBox *nfg_algorithm_box=new	wxRadioBox(parent,func,"Nfg Algorithms",-1,-1,-1,-1,NFG_NUM_SOLUTIONS,nfg_algorithm_list,2);
		if (num_players!=2)
		{
			nfg_algorithm_box->Enable(NFG_LEMKE_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_ENUM_SOLUTION,FALSE);
		}
		return nfg_algorithm_box;
	}
};

class NfgSolveParamsDialog: public NfgAlgorithmList
{
private:
	wxDialogBox *d;
	wxRadioBox *nfg_algorithm_box;
	wxCheckBox *extensive_box;
	wxButton *inspect_button;
	NfgSolutionT algorithm;
	int result,extensive;
	gList<int> solns;
// Static event handlers
	static void solve_button_func(wxButton &ob,wxEvent &ev)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_SOLVE);}
	static void inspect_button_func(wxButton &ob,wxEvent &ev)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_INSPECT);}
	static void cancel_button_func(wxButton &ob,wxEvent &ev)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_CANCEL);}
	static void help_button_func(wxButton &ob,wxEvent &ev)
	{wxHelpContents(NFG_SOLVE_HELP);}
	static void nfg_algorithm_box_func(wxRadioBox &ob,wxEvent &ev)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_ALGORITHM);}

public:
// Constructor
	NfgSolveParamsDialog(const gList<int> &got_solns,int have_efg,int num_players,wxWindow *parent=0):solns(got_solns)
	{
		d=new wxDialogBox(parent,"Solutions",TRUE);
		nfg_algorithm_box=MakeNfgAlgorithmList(num_players,d,(wxFunction)nfg_algorithm_box_func);
		nfg_algorithm_box->SetClientData((char *)this);
		d->NewLine();
		extensive_box=new wxCheckBox(d,0,"Extensive Form");
		extensive_box->Enable(have_efg);
		d->NewLine();
		wxButton *solve_button=new wxButton(d,(wxFunction)solve_button_func,"Solve");
		solve_button->SetClientData((char *)this);
		inspect_button=new wxButton(d,(wxFunction)inspect_button_func,"Look");
		inspect_button->SetClientData((char *)this);
		wxButton *cancel_button=new wxButton(d,(wxFunction)cancel_button_func,"Cancel");
		cancel_button->SetClientData((char *)this);
		new wxButton(d,(wxFunction)help_button_func,"?");
		OnEvent(SD_ALGORITHM);
		d->Fit();
		d->Show(TRUE);
	}
	~NfgSolveParamsDialog(void) {delete d;}
// Main event handler
	void OnEvent(int event)
	{
	if (event!=SD_ALGORITHM)	// one of the buttons
	{
		result=event;algorithm=nfg_algorithm_box->GetSelection();
		extensive=extensive_box->GetValue();
		d->Show(FALSE);
	}
	else	// new algorithm selected
	{
		inspect_button->Enable(solns.Contains(nfg_algorithm_box->GetSelection()));
	}
	}
	// Data access
	NfgSolutionT GetAlgorithm(void) {return algorithm;}
	int GetResult(void) {return result;}
	int GetExtensive(void) {return extensive;}
};

