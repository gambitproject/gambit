// File: nfgsolvd.h -- the main dialog for running NormalForm solution
// algorithms.  You must add an entry here for each new algorithm.
// Update: this now takes the number of players so that it can turn
// of the algorithms that will not work with this number of players.
// Please list these explicitly
// Only work for 2 players: Lemke, Enum
// $Id$

#define SD_CANCEL			-1
#define SD_PARAMS			1
#define	SD_SAVE			 	2

#define		SOLN_SECT			"Soln-Defaults"

//#ifndef NFG_SOLVE_HELP	// if this is included in efgsolvd.h, we do not need it
//#define NFG_SOLVE_HELP	""
//#endif
// solution module constants.  Do not change the order.  Add new ones
// just before NFG_NUM_SOLUTIONS.
typedef enum {NFG_NO_SOLUTION=-1,NFG_ENUMPURE_SOLUTION,NFG_ENUMMIXED_SOLUTION,
							NFG_LP_SOLUTION,NFG_LCP_SOLUTION,
							NFG_GOBIT_SOLUTION,NFG_GOBITALL_SOLUTION,NFG_LIAP_SOLUTION,
							NFG_SIMPDIV_SOLUTION,NFG_NUM_SOLUTIONS} NfgSolutionT;

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
		nfg_algorithm_list[NFG_ENUMPURE_SOLUTION]="EnumPure";
		nfg_algorithm_list[NFG_ENUMMIXED_SOLUTION]="EnumMixed";
		nfg_algorithm_list[NFG_LP_SOLUTION]="LP";
		nfg_algorithm_list[NFG_LCP_SOLUTION]="LCP";
		nfg_algorithm_list[NFG_GOBIT_SOLUTION]="Gobit";
		nfg_algorithm_list[NFG_GOBITALL_SOLUTION]="GobitAll";
		nfg_algorithm_list[NFG_LIAP_SOLUTION]="Liap";
		nfg_algorithm_list[NFG_SIMPDIV_SOLUTION]="SimpDiv";
		wxRadioBox *nfg_algorithm_box=new	wxRadioBox(parent,func,"Nfg Algorithms",-1,-1,-1,-1,NFG_NUM_SOLUTIONS,nfg_algorithm_list,2);
		if (num_players!=2)
		{
			nfg_algorithm_box->Enable(NFG_LCP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_LP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_ENUMMIXED_SOLUTION,FALSE);
		}
		return nfg_algorithm_box;
	}
};

class NfgSolveSettings
{
protected:
	int algorithm;
	int result;
	Bool extensive;
public:
	NfgSolveSettings(void)
	{
	result=SD_SAVE;
	char *defaults_file="gambit.ini";
	wxGetResource(SOLN_SECT,"Nfg-Algorithm",&algorithm,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-Efg",&extensive,defaults_file);
	}
	~NfgSolveSettings()
	{
	if (result!=SD_CANCEL)
	{
		char *defaults_file="gambit.ini";
		wxWriteResource(SOLN_SECT,"Nfg-Algorithm",algorithm,defaults_file);
		wxWriteResource(SOLN_SECT,"Nfg-Efg",extensive,defaults_file);
	}
	}
	NfgSolutionT GetAlgorithm(void) {return algorithm;}
	Bool GetExtensive(void) {return extensive;}
};

class NfgSolveParamsDialog: public NfgAlgorithmList, public NfgSolveSettings
{
private:
	wxDialogBox *d;
	wxRadioBox *nfg_algorithm_box;
	wxCheckBox *extensive_box;
	wxButton *inspect_button;
	gList<int> solns;
// Static event handlers
	static void params_button_func(wxButton &ob,wxEvent &)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_PARAMS);}
	static void save_button_func(wxButton &ob,wxEvent &)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_SAVE);}
	static void cancel_button_func(wxButton &ob,wxEvent &)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_CANCEL);}
	static void help_button_func(wxButton &,wxEvent &)
	{wxHelpContents(NFG_SOLVE_HELP);}
	static void nfg_algorithm_box_func(wxRadioBox &ob,wxEvent &)
	{/*((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_ALGORITHM);*/}
	// Event handlers: high level
	void OnEvent(int event)
	{
	result=event;
	algorithm=nfg_algorithm_box->GetSelection();
	extensive=extensive_box->GetValue();
	d->Show(FALSE);
	}

public:
// Constructor
	NfgSolveParamsDialog(const gList<int> &got_solns,int have_efg,int num_players,wxWindow *parent=0):solns(got_solns)
	{
		d=new wxDialogBox(parent,"Solutions",TRUE);
		nfg_algorithm_box=MakeNfgAlgorithmList(num_players,d,(wxFunction)nfg_algorithm_box_func);
		nfg_algorithm_box->SetClientData((char *)this);
		nfg_algorithm_box->SetSelection(algorithm);
		d->NewLine();
		extensive_box=new wxCheckBox(d,0,"Extensive Form");
		extensive_box->Enable(have_efg);
		extensive_box->SetValue(extensive && have_efg);
		d->NewLine();
		wxButton *solve_button=new wxButton(d,(wxFunction)params_button_func,"Params");
		solve_button->SetClientData((char *)this);
		inspect_button=new wxButton(d,(wxFunction)save_button_func,"Save");
		inspect_button->SetClientData((char *)this);
		wxButton *cancel_button=new wxButton(d,(wxFunction)cancel_button_func,"Cancel");
		cancel_button->SetClientData((char *)this);
		new wxButton(d,(wxFunction)help_button_func,"?");
		d->Fit();
		d->Show(TRUE);
	}
	~NfgSolveParamsDialog(void) {delete d;}
	// Data access
	int GetResult(void) {return result;}
};

