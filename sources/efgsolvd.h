// File: efgsolvd.h -- the main dialog for running ExtensiveForm solution
// efg_algorithms.  You must add an entry here for each new efg_algorithm.
// Update: this dialog box now also includes the NormalForm solution
// efg_algorithms.  They are enabled by selecting the 'Use NF' box.  This is
// why "nfgsolvd.h" is included here

#define SD_CANCEL			-1
#define SD_SOLVE			1
#define	SD_ALGORITHM 	2
#define SD_INSPECT 		3

#include "nfgsolvd.h"

class EfgSolveParamsDialog: public NfgAlgorithmList
{
private:
	wxDialogBox *d;
	wxButton *cancel_button,*solve_button,*inspect_button;
	wxRadioBox *efg_algorithm_box;
	wxRadioBox *nfg_algorithm_box;
	wxCheckBox *normal_box,*use_nfg_box;
	int result,algorithm,normal;
	gBlock<int> solns;
	enum {useEFG,useNFG} use;
	// Event Handlers: low level
	static void use_nfg_func(wxCheckBox &ob,wxEvent &ev)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnUseNF(ob.GetValue());}
	static void solve_button_func(wxButton &ob,wxEvent &ev)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_SOLVE);}
	static void inspect_button_func(wxButton &ob,wxEvent &ev)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_INSPECT);}
	static void cancel_button_func(wxButton &ob,wxEvent &ev)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_CANCEL);}
	static void algorithm_box_func(wxRadioBox &ob,wxEvent &ev)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_ALGORITHM);}
	// Event handlers: high level
	void OnEvent(int event)
	{
	algorithm=(use==useEFG) ? efg_algorithm_box->GetSelection() : nfg_algorithm_box->GetSelection();
	algorithm+=(use==useEFG) ? 0 : EFG_NUM_SOLUTIONS;
	if (event!=SD_ALGORITHM)	// one of the buttons
	{
		result=event;
		normal=normal_box->GetValue();
		d->Show(FALSE);
	}
	else	// new efg_algorithm selected
		inspect_button->Enable(solns.Contains(algorithm));
	}
	void OnUseNF(Bool u)
	{
	if (u)
	{
		nfg_algorithm_box->wxWindow::Enable(TRUE);
		efg_algorithm_box->wxWindow::Enable(FALSE);
		use=useNFG;
	}
	else
	{
		nfg_algorithm_box->wxWindow::Enable(FALSE);
		efg_algorithm_box->wxWindow::Enable(TRUE);
		use=useEFG;
	}
	}

public:
	EfgSolveParamsDialog(const gBlock<int> &got_solns,int have_nfg,int num_players,wxWindow *parent=0):
				solns(got_solns)
	{
		use=useEFG;
		d=new wxDialogBox(parent,"Solutions",TRUE);

		char *efg_algorithm_list[EFG_NUM_SOLUTIONS];
		efg_algorithm_list[EFG_EGOBIT_SOLUTION]="eGobit";
		efg_algorithm_list[EFG_ELIAP_SOLUTION]="eLiap";
		efg_algorithm_box=new	wxRadioBox(d,(wxFunction)algorithm_box_func,"Efg Algorithms",-1,-1,-1,-1,EFG_NUM_SOLUTIONS,efg_algorithm_list,2);
		efg_algorithm_box->SetClientData((char *)this);
		d->NewLine();

		nfg_algorithm_box=MakeNfgAlgorithmList(num_players,d,(wxFunction)algorithm_box_func);
		nfg_algorithm_box->SetClientData((char *)this);
		nfg_algorithm_box->wxWindow::Enable(FALSE);
		d->NewLine();

		normal_box=new wxCheckBox(d,0,"Normal Form");
		if (!have_nfg) normal_box->Enable(FALSE);
		use_nfg_box=new wxCheckBox(d,(wxFunction)use_nfg_func,"Use NF");
		use_nfg_box->SetClientData((char *)this);
		d->NewLine();
		solve_button=new wxButton(d,(wxFunction)solve_button_func,"Solve");
		solve_button->SetClientData((char *)this);
		inspect_button=new wxButton(d,(wxFunction)inspect_button_func,"Look");
		inspect_button->SetClientData((char *)this);
		cancel_button=new wxButton(d,(wxFunction)cancel_button_func,"Cancel");
		cancel_button->SetClientData((char *)this);
		OnEvent(SD_ALGORITHM);
		d->Fit();
		d->Show(TRUE);
	}
	~EfgSolveParamsDialog(void)
	{delete d;}
	// If we use the EF, it just returns the alg #, otherwise, it returns the alg#
	// +EFG_NUM_SOLUTIONS
	bool UseNF(void) {return use==useNFG;}
	EfgSolutionT GetEfgAlgorithm(void) {assert(!UseNF() && "Wrong type: use Nfg");return algorithm;}
	NfgSolutionT GetNfgAlgorithm(void) {assert(UseNF() && "Wrong type: use Efg");return algorithm-EFG_NUM_SOLUTIONS;}
	int GetResult(void) {return result;}
};

