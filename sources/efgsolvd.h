//
// FILE: efgsolvd.h -- the main dialog for running ExtensiveForm solution
//                     efg_algorithms. 
//
// $Id$
//

// You must add an entry here for each new efg_algorithm.
// Update: this dialog box now also includes the NormalForm solution
// efg_algorithms.  They are enabled by selecting the 'Use NF' box.  This is
// why "nfgsolvd.h" is included here

#define NFG_ALGORITHM_LIST	// we do not need all of nfgsolvd.h
#include "nfgsolvd.h"

#define STANDARD_NASH				0
#define	STANDARD_PERFECT		1
#define	STANDARD_SEQUENTIAL	2
#define	STANDARD_ONE				0
#define	STANDARD_TWO				1
#define	STANDARD_ALL				2

#define PARAMS_SECTION	"Algorithm Params"		// section in .ini file
bool IsPerfectRecall(const Efg &, Infoset *&, Infoset *&);
class EfgSolveSettings
{
protected:
	Bool use_nfg,normal,subgames,pick_solns,auto_inspect;
	int algorithm;
	int standard_type,standard_num;
	char *defaults_file;
	Bool	use_standard;
	int result;
	bool	solving;
protected:
	const Efg &ef;
	// call this to convert standard settings to actual solution parameters
	void StandardSettings(void)
	{
	int stopAfter,max_solns,dom_type;
	bool use_elimdom,all;
	// bool csum=ef.IsConstSum();
	Infoset *bad1,*bad2;
	bool perf=IsPerfectRecall(ef,bad1,bad2);

	// a separate case for each of the possible alg/num/game combinations
	// One Nash for 2 person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ONE && ef.NumPlayers()==2)
	{
	use_nfg=FALSE;
	if (perf)
		algorithm=(ef.IsConstSum()) ? EFG_CSUM_SOLUTION : EFG_LCP_SOLUTION;
	else
		algorithm=EFG_GOBIT_SOLUTION;
	stopAfter=1;max_solns=1;
	use_elimdom=true;all=true;dom_type=DOM_WEAK;
	subgames=TRUE;
	}
	// One Nash for n person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ONE && ef.NumPlayers()!=2)
	{
	use_nfg=TRUE;algorithm=NFG_SIMPDIV_SOLUTION;
	stopAfter=1;max_solns=1;
	use_elimdom=true;all=true;dom_type=DOM_WEAK;
	subgames=TRUE;
	}
	// Two Nash 2 person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_TWO && ef.NumPlayers()==2)
	{
	use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
	stopAfter=2;max_solns=2;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=FALSE;
	}
	// Two Nash n person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_TWO && ef.NumPlayers()!=2)
	{
	use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
	stopAfter=2;max_solns=2;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=FALSE;
	wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
	}
	// All Nash 2 person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ALL && ef.NumPlayers()==2)
	{
	use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
	stopAfter=0;max_solns=0;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=FALSE;
	}
	// ALL Nash n person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ALL && ef.NumPlayers()!=2)
	{
	use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
	stopAfter=0;max_solns=0;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=FALSE;
	Warn("Not guaranteed to find all solutions for 'All Nash n-person'\n");
	wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
	}
	// One Subgame Perfect (same as One Nash)
	// One Subgame Perfect for 2 person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ONE && ef.NumPlayers()==2)
	{
	use_nfg=FALSE;
	if (perf)
		algorithm=(ef.IsConstSum()) ? EFG_CSUM_SOLUTION : EFG_LCP_SOLUTION;
	else
		algorithm=EFG_GOBIT_SOLUTION;
	stopAfter=1;max_solns=1;
	use_elimdom=true;all=true;dom_type=DOM_WEAK;
	subgames=TRUE;
	}
	// One Subgame Pefect for n person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ONE && ef.NumPlayers()!=2)
	{
	use_nfg=TRUE;algorithm=NFG_SIMPDIV_SOLUTION;
	stopAfter=1;max_solns=1;
	use_elimdom=true;all=true;dom_type=DOM_WEAK;
	subgames=TRUE;
	}
	// Two Subgame Perfect 2 person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_TWO && ef.NumPlayers()==2)
	{
	use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
	stopAfter=2;max_solns=2;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=TRUE;
	Warn("Not guaranteed to find 2 solutions for 'Two Perfect'");
	}
	// Two Subgame Perfect n person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_TWO && ef.NumPlayers()!=2)
	{
	use_nfg=TRUE;algorithm=NFG_LIAP_SOLUTION;
	stopAfter=2;max_solns=2;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=TRUE;
	wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
	Warn("Not guaranteed to find 2 solutions for 'Two Perfect'");
	}
	// All Subgame Perfect 2 person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ALL && ef.NumPlayers()==2)
	{
	use_nfg=TRUE;algorithm=NFG_ENUMMIXED_SOLUTION;
	stopAfter=0;max_solns=0;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=TRUE;
	}
	// All Subgame Perfect n person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ALL && ef.NumPlayers()!=2)
	{
	use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
	stopAfter=0;max_solns=0;
	use_elimdom=true;all=true;dom_type=DOM_STRONG;
	subgames=TRUE;
	Warn("Not guaranteed to find all solutions for 'All Subgame Perfect n-person'\n");
	wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
	}
	// One Sequential
	if (standard_type==STANDARD_SEQUENTIAL && standard_num==STANDARD_ONE)
	{
	use_nfg=FALSE;algorithm=EFG_GOBIT_SOLUTION;
	stopAfter=1;max_solns=1;
	use_elimdom=false;all=true;dom_type=DOM_STRONG;
	subgames=FALSE;
	}
	// Two Sequential
	if (standard_type==STANDARD_SEQUENTIAL && standard_num==STANDARD_TWO)
	{
	use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
	stopAfter=2;max_solns=2;
	use_elimdom=false;all=true;dom_type=DOM_STRONG;
	subgames=FALSE;
	Warn("Not guaranteed to find all solutions for 'Two Sequential'\n");
	wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
	}
	// All Sequential
	if (standard_type==STANDARD_SEQUENTIAL && standard_num==STANDARD_ALL)
	{
	use_nfg=FALSE;algorithm=EFG_LIAP_SOLUTION;
	stopAfter=0;max_solns=0;
	use_elimdom=false;all=true;dom_type=DOM_STRONG;
	subgames=FALSE;
	Warn("Not guaranteed to find all solutions for 'All Sequential'\n");
	wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);
	}
	pick_solns=false; // pick solution subgames off for all standard solns
	// -------- now write the new settings to file
	wxWriteResource(SOLN_SECT,"Use-Nfg",use_nfg,defaults_file);
	char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
	wxWriteResource(SOLN_SECT,alg_sect,algorithm,defaults_file);
	wxWriteResource(PARAMS_SECTION,"Stop-After",stopAfter,defaults_file);
	wxWriteResource(PARAMS_SECTION,"Max-Solns",max_solns,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-All",(int)all,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Type",dom_type,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Use",(int)use_elimdom,defaults_file);
	wxWriteResource(SOLN_SECT,"Efg-Mark-Subgames",subgames,defaults_file);
	wxWriteResource(SOLN_SECT,"Efg-Interactive-Solns",pick_solns,defaults_file);
}
virtual void Warn(const char *warning) // only warn when solving
{if (solving) wxMessageBox((char *)warning,"Standard Solution");}
public:
	EfgSolveSettings(const Efg &ef_,bool solving_=true):solving(solving_),ef(ef_)
	{
	result=SD_SAVE;
	defaults_file="gambit.ini";
	wxGetResource(SOLN_SECT,"Use-Nfg",&use_nfg,defaults_file);
	char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
	wxGetResource(SOLN_SECT,alg_sect,&algorithm,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Nfg",&normal,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Mark-Subgames",&subgames,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Interactive-Solns",&pick_solns,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Auto-Inspect-Solns",&auto_inspect,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Use-Standard",&use_standard,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Standard-Type",&standard_type,defaults_file);
	wxGetResource(SOLN_SECT,"Efg-Standard-Num",&standard_num,defaults_file);
	if (use_standard) StandardSettings();
	}
	virtual ~EfgSolveSettings()
	{
	if (result!=SD_CANCEL)
	{
		wxWriteResource(SOLN_SECT,"Use-Nfg",use_nfg,defaults_file);
		char *alg_sect=(use_nfg) ? "Nfg-Algorithm" : "Efg-Algorithm";
		wxWriteResource(SOLN_SECT,alg_sect,algorithm,defaults_file);
		wxWriteResource(SOLN_SECT,"Efg-Nfg",normal,defaults_file);
		wxWriteResource(SOLN_SECT,"Efg-Interactive-Solns",pick_solns,defaults_file);
		wxWriteResource(SOLN_SECT,"Efg-Auto-Inspect-Solns",auto_inspect,defaults_file);
		wxWriteResource(SOLN_SECT,"Efg-Use-Standard",use_standard,defaults_file);
		wxWriteResource(SOLN_SECT,"Efg-Standard-Type",standard_type,defaults_file);
		wxWriteResource(SOLN_SECT,"Efg-Standard-Num",standard_num,defaults_file);
	}
	}
	bool UseNF(void) const	{	return use_nfg;	}
	EfgSolutionT GetEfgAlgorithm(void) const
	{assert(!UseNF() && "Wrong type: use Nfg");return (EfgSolutionT) algorithm;}
	NfgSolutionT GetNfgAlgorithm(void) const
	{assert(UseNF() && "Wrong type: use Efg");return (NfgSolutionT) algorithm;}
	bool MarkSubgames(void) const {return subgames;}
	bool AutoInspect(void) const {return auto_inspect;}
	bool UseStandard(void) const {return use_standard;}
	int  StandardType(void) const {return standard_type;}
	int  StandardNum(void) const {return standard_num;}
};

class EfgSolveParamsDialog: public NfgAlgorithmList, public EfgSolveSettings
{
private:
	wxDialogBox *d;
	wxButton *cancel_button,*solve_button,*inspect_button;
	wxRadioBox *efg_algorithm_box;
	wxRadioBox *nfg_algorithm_box;
	wxCheckBox *normal_box,*use_nfg_box,*pick_solns_box,*auto_inspect_box;
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
	static void algorithm_box_func(wxRadioBox &,wxEvent &)
	{/*((EfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_ALGORITHM);*/}
	static void help_button_func(wxButton &,wxEvent &)
	{wxHelpContents(EFG_CUSTOM_HELP);}
	// Event handlers: high level
	void OnEvent(int event)
	{
	algorithm=(use_nfg) ? nfg_algorithm_box->GetSelection() : efg_algorithm_box->GetSelection();
	result=event;
	normal=normal_box->GetValue();
	pick_solns=pick_solns_box->GetValue();
	auto_inspect=auto_inspect_box->GetValue();
	use_standard=FALSE;
	d->Show(FALSE);
	}
	void OnUseNF(Bool usenf)
	{
	nfg_algorithm_box->wxWindow::Enable(usenf);
	efg_algorithm_box->wxWindow::Enable(!usenf);
	use_nfg=usenf;
	}
public:
	EfgSolveParamsDialog(const Efg &ef,int have_nfg,wxWindow *parent=0)
									:EfgSolveSettings(ef,false)
	{
		d=new wxDialogBox(parent,"Solutions",TRUE);

		char *efg_algorithm_list[EFG_NUM_SOLUTIONS];
		efg_algorithm_list[EFG_GOBIT_SOLUTION]="Gobit";
		efg_algorithm_list[EFG_LIAP_SOLUTION]="Liap";
		efg_algorithm_list[EFG_LCP_SOLUTION]="LCP";
		efg_algorithm_list[EFG_PURENASH_SOLUTION]="PureNash";
		efg_algorithm_list[EFG_CSUM_SOLUTION]="LP";
		efg_algorithm_box=new	wxRadioBox(d,(wxFunction)algorithm_box_func,"Efg Algorithms",-1,-1,-1,-1,EFG_NUM_SOLUTIONS,efg_algorithm_list,2);
		efg_algorithm_box->SetClientData((char *)this);
		d->NewLine();

		nfg_algorithm_box=MakeNfgAlgorithmList(ef.NumPlayers(),ef.IsConstSum(),d,(wxFunction)algorithm_box_func);
		nfg_algorithm_box->SetClientData((char *)this);
		d->NewLine();

		normal_box=new wxCheckBox(d,0,"Normal Form");
		if (!have_nfg) normal_box->Enable(FALSE);
		use_nfg_box=new wxCheckBox(d,(wxFunction)use_nfg_func,"Use NF");
		use_nfg_box->SetClientData((char *)this);
		d->NewLine();
		pick_solns_box=new wxCheckBox(d,0,"Pick subgame solutions");
		d->NewLine();
		auto_inspect_box=new wxCheckBox(d,0,"Auto Inspect");
		d->NewLine();
		solve_button=new wxButton(d,(wxFunction)params_button_func,"Params");
		solve_button->SetClientData((char *)this);
		inspect_button=new wxButton(d,(wxFunction)save_button_func,"Save");
		inspect_button->SetClientData((char *)this);
		cancel_button=new wxButton(d,(wxFunction)cancel_button_func,"Cancel");
		cancel_button->SetClientData((char *)this);
		(void)new wxButton(d,(wxFunction)help_button_func,"?");
		// disable algorithms that can not work w/ this game
		if (ef.NumPlayers()!=2)
		{
			efg_algorithm_box->Enable(EFG_LCP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_LCP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_LP_SOLUTION,FALSE);
			nfg_algorithm_box->Enable(NFG_ENUMMIXED_SOLUTION,FALSE);
		}
		if (ef.NumPlayers()==2)
		{
			Infoset *bad1,*bad2;
			if (!IsPerfectRecall(ef,bad1,bad2))
			{
				efg_algorithm_box->Enable(EFG_LCP_SOLUTION,FALSE);
				efg_algorithm_box->Enable(EFG_CSUM_SOLUTION,FALSE);
			}
		}

		// set the defaults
		nfg_algorithm_box->wxWindow::Enable(use_nfg);
		efg_algorithm_box->wxWindow::Enable(!use_nfg);
		((use_nfg) ? nfg_algorithm_box : efg_algorithm_box)->SetSelection(algorithm);
		use_nfg_box->SetValue(use_nfg);
		pick_solns_box->SetValue(pick_solns);
		auto_inspect_box->SetValue(auto_inspect);

		d->Fit();
		d->Show(TRUE);
	}
	// Destructor
	virtual ~EfgSolveParamsDialog(void)	{delete d;}
	// Data access
	int GetResult(void) {return result;}
};

class EfgSolveStandardDialog:public EfgSolveSettings, public MyDialogBox
{
private:
	char *standard_type_str,*standard_num_str;
	wxStringList *standard_type_list,*standard_num_list;
public:
	EfgSolveStandardDialog(const Efg &ef,wxWindow *parent):
				EfgSolveSettings(ef,false),MyDialogBox(parent,"Standard Solution",EFG_STANDARD_HELP)
	{
	Bool expert=FALSE;char *defaults_file="gambit.ini";
	wxGetResource(SOLN_SECT,"Gambit-Expert",&expert,defaults_file);
	if (expert)
	standard_type_list=new wxStringList("Nash","Subgame Perfect","Sequential",0);
	else // No sequential
	standard_type_list=new wxStringList("Nash","Subgame Perfect",0);

	standard_num_list=new wxStringList("One","Two","All",0);
	standard_type_str=new char[20];standard_num_str=new char[20];
	strcpy(standard_type_str,(char *)standard_type_list->Nth(standard_type)->Data());
	strcpy(standard_num_str,(char *)standard_num_list->Nth(standard_num)->Data());
	Add(wxMakeFormString("Type",&standard_type_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(standard_type_list), 0)));
	Add(wxMakeFormString("Number",&standard_num_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(standard_num_list), 0)));
	Go();
	}
	virtual ~EfgSolveStandardDialog()
	{
	if (Completed()==wxOK)
	{
		standard_type=wxListFindString(standard_type_list,standard_type_str);
		standard_num=wxListFindString(standard_num_list,standard_num_str);
		use_standard=TRUE;
		result=SD_SAVE;
	}
	else
		result=SD_CANCEL;
	delete [] standard_type_str;
	delete [] standard_num_str;
	delete standard_type_list;
	delete standard_num_list;
	}
};

