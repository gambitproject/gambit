// File: efgsolvd.h -- the main dialog for running ExtensiveForm solution
// efg_algorithms.  You must add an entry here for each new efg_algorithm.
// Update: this dialog box now also includes the NormalForm solution
// efg_algorithms.  They are enabled by selecting the 'Use NF' box.  This is
// why "nfgsolvd.h" is included here
// $Id$
#include "nfgsolvd.h"

class EfgSolveSettings
{
protected:
	Bool use_nfg,normal,subgames;
	int algorithm;
	char *defaults_file;
	int result;
public:
	EfgSolveSettings(void)
	{
	result=SD_SAVE;
	defaults_file="gambit.ini";
	wxGetResource(SOLN_SECT,"Use-Nfg",&use_nfg,defaults_file);
	char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
	wxGetResource(SOLN_SECT,alg_sect,&algorithm,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Nfg",&normal,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Mark-Subgames",&subgames,defaults_file);
	}
	~EfgSolveSettings()
	{
	if (result!=SD_CANCEL)
	{
		wxWriteResource(SOLN_SECT,"Use-Nfg",use_nfg,defaults_file);
		char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
		wxWriteResource(SOLN_SECT,alg_sect,algorithm,defaults_file);
		wxWriteResource(SOLN_SECT,"Efg-Nfg",normal,defaults_file);
	}
	}
	bool UseNF(void)
	{	return use_nfg;	}
	EfgSolutionT GetEfgAlgorithm(void)
	{assert(!UseNF() && "Wrong type: use Nfg");return algorithm;}
	NfgSolutionT GetNfgAlgorithm(void)
	{assert(UseNF() && "Wrong type: use Efg");return algorithm;}
  bool MarkSubgames(void) {return subgames;}
};

class EfgSolveParamsDialog: public NfgAlgorithmList, public EfgSolveSettings
{
private:
	wxDialogBox *d;
	wxButton *cancel_button,*solve_button,*inspect_button;
	wxRadioBox *efg_algorithm_box;
	wxRadioBox *nfg_algorithm_box;
	wxCheckBox *normal_box,*use_nfg_box;
	gBlock<int> solns;
	// Event Handlers: low level
	static void use_nfg_func(wxCheckBox &ob,wxEvent &)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnUseNF(ob.GetValue());}
	static void params_button_func(wxButton &ob,wxEvent &)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_PARAMS);}
	static void save_button_func(wxButton &ob,wxEvent &)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_SAVE);}
	static void cancel_button_func(wxButton &ob,wxEvent &)
	{((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_CANCEL);}
	static void algorithm_box_func(wxRadioBox &ob,wxEvent &)
	{/*((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_ALGORITHM);*/}
	static void help_button_func(wxButton &,wxEvent &)
	{wxHelpContents(EFG_SOLVE_HELP);}
	// Event handlers: high level
	void OnEvent(int event)
	{
	algorithm=(use_nfg) ? nfg_algorithm_box->GetSelection() : efg_algorithm_box->GetSelection();
	result=event;
	normal=normal_box->GetValue();
	d->Show(FALSE);
	}
	void OnUseNF(Bool usenf)
	{
	nfg_algorithm_box->wxWindow::Enable(usenf);
	efg_algorithm_box->wxWindow::Enable(!usenf);
	use_nfg=usenf;
	}

public:
	EfgSolveParamsDialog(const gBlock<int> &got_solns,int have_nfg,int num_players,wxWindow *parent=0):
				solns(got_solns)
	{
		d=new wxDialogBox(parent,"Solutions",TRUE);

		char *efg_algorithm_list[EFG_NUM_SOLUTIONS];
		efg_algorithm_list[EFG_GOBIT_SOLUTION]="Gobit";
		efg_algorithm_list[EFG_LIAP_SOLUTION]="Liap";
		efg_algorithm_list[EFG_LCP_SOLUTION]="LCP";
		efg_algorithm_box=new	wxRadioBox(d,(wxFunction)algorithm_box_func,"Efg Algorithms",-1,-1,-1,-1,EFG_NUM_SOLUTIONS,efg_algorithm_list,2);
		efg_algorithm_box->SetClientData((char *)this);
		d->NewLine();

		nfg_algorithm_box=MakeNfgAlgorithmList(num_players,d,(wxFunction)algorithm_box_func);
		nfg_algorithm_box->SetClientData((char *)this);
		d->NewLine();

		normal_box=new wxCheckBox(d,0,"Normal Form");
		if (!have_nfg) normal_box->Enable(FALSE);
		use_nfg_box=new wxCheckBox(d,(wxFunction)use_nfg_func,"Use NF");
		use_nfg_box->SetClientData((char *)this);
		d->NewLine();
		solve_button=new wxButton(d,(wxFunction)params_button_func,"Params");
		solve_button->SetClientData((char *)this);
		inspect_button=new wxButton(d,(wxFunction)save_button_func,"Save");
		inspect_button->SetClientData((char *)this);
		cancel_button=new wxButton(d,(wxFunction)cancel_button_func,"Cancel");
		cancel_button->SetClientData((char *)this);
		(void)new wxButton(d,(wxFunction)help_button_func,"?");
		if (num_players!=2) // disable algorithms that can not work w/ this game
		{
			efg_algorithm_box->Enable(EFG_LCP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_LCP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_LP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_ENUMMIXED_SOLUTION,FALSE);
		}
		// set the defaults
		nfg_algorithm_box->wxWindow::Enable(use_nfg);
		efg_algorithm_box->wxWindow::Enable(!use_nfg);
		((use_nfg) ? nfg_algorithm_box : efg_algorithm_box)->SetSelection(algorithm);
		use_nfg_box->SetValue(use_nfg);

		//OnEvent(SD_ALGORITHM);
		d->Fit();
		d->Show(TRUE);
	}
	// Destructor
	~EfgSolveParamsDialog(void)	{delete d;}
	// Data access
	int GetResult(void) {return result;}
};


