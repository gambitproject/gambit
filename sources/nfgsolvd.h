// File: nfgsolvd.h -- the main dialog for running NormalForm solution
// algorithms.  You must add an entry here for each new algorithm.
// Update: this now takes the number of players so that it can turn
// of the algorithms that will not work with this number of players.
// $Id$
#ifndef NFGSOLVD_H
#define NFGSOLVD_H

#define SD_CANCEL			-1
#define SD_PARAMS			1
#define	SD_SAVE			 	2

#define		SOLN_SECT				"Soln-Defaults"
#define 	PARAMS_SECTION	"Algorithm Params"		// section in .ini file

#define STANDARD_NASH				0
#define	STANDARD_PERFECT		1
#define	STANDARD_SEQUENTIAL	2
#define	STANDARD_ONE				0
#define	STANDARD_TWO				1
#define	STANDARD_ALL				2

#define DOM_WEAK				0
#define	DOM_STRONG			1


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
	wxRadioBox *MakeNfgAlgorithmList(int num_players,bool const_sum,wxPanel *parent,wxFunction func=0)
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
		if (!const_sum) nfg_algorithm_box->Enable(NFG_LP_SOLUTION,FALSE);

		return nfg_algorithm_box;
	}
};

#ifndef NFG_ALGORITHM_LIST
class NfgSolveSettings
{
protected:
	int algorithm;
	int result;
	Bool extensive,auto_inspect;
	int standard_type,standard_num;
	Bool	use_standard;
	bool	solving;
protected:
	char *defaults_file;
	const BaseNfg &nf;
		// call this to convert standard settings to actual solution parameters
	void StandardSettings(void)
	{
	int stopAfter,dom_type;
	bool use_elimdom,all;
	// a separate case for each of the possible alg/num/game combinations
	// One Nash 2 person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ONE && nf.NumPlayers()==2)
	{algorithm=NFG_LCP_SOLUTION;stopAfter=1;dom_type=DOM_WEAK;all=TRUE;use_elimdom=TRUE;}
	// One Nash n person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ONE && nf.NumPlayers()!=2)
	{algorithm=NFG_SIMPDIV_SOLUTION;stopAfter=1;dom_type=DOM_WEAK;all=TRUE;use_elimdom=TRUE;}
	// Two Nash 2 person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_TWO && nf.NumPlayers()==2)
	{algorithm=NFG_ENUMMIXED_SOLUTION;stopAfter=2;dom_type=DOM_STRONG;all=TRUE;use_elimdom=TRUE;}
	// Two Nash n person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_TWO && nf.NumPlayers()!=2)
	{algorithm=NFG_LIAP_SOLUTION;stopAfter=2;dom_type=DOM_STRONG;all=TRUE;use_elimdom=TRUE;
	 wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);}
	// All Nash 2 person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ALL && nf.NumPlayers()==2)
	{algorithm=NFG_ENUMMIXED_SOLUTION;stopAfter=0;dom_type=DOM_STRONG;all=TRUE;use_elimdom=TRUE;}
	// All Nash n person
	if (standard_type==STANDARD_NASH && standard_num==STANDARD_ALL && nf.NumPlayers()!=2)
	{algorithm=NFG_LIAP_SOLUTION;stopAfter=0;dom_type=DOM_STRONG;all=TRUE;use_elimdom=TRUE;
	 Warn("Not guaranteed to find all solutions for 'All Nash' n-person games\n");
	 wxWriteResource(PARAMS_SECTION,"Liap-Ntries",2*stopAfter,defaults_file);}
	// One Perfect 2 person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ONE && nf.NumPlayers()==2)
	{algorithm=NFG_LCP_SOLUTION;stopAfter=1;dom_type=DOM_WEAK;all=TRUE;use_elimdom=TRUE;}
	// One Perfect n person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ONE && nf.NumPlayers()!=2)
	{Warn("One Perfect not implemented for n person games\nUsing current settings\n");}
	// Two Perfect 2 person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_TWO && nf.NumPlayers()==2)
	{algorithm=NFG_LCP_SOLUTION;stopAfter=2;dom_type=DOM_WEAK;all=TRUE;use_elimdom=TRUE;}
	// Two Perfect n person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_TWO && nf.NumPlayers()!=2)
	{Warn("Two Perfect not implemented for n person games\nUsing current settings\n");}
	// All Perfect 2 person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ALL && nf.NumPlayers()==2)
	{algorithm=NFG_LCP_SOLUTION;stopAfter=0;dom_type=DOM_WEAK;all=TRUE;use_elimdom=TRUE;
	 Warn("Not guaranteed to find all solutions for 'All Perfect' 2-person games\n");}

	// All Perfect n person
	if (standard_type==STANDARD_PERFECT && standard_num==STANDARD_ALL && nf.NumPlayers()!=2)
	{Warn("All Perfect not implemented for n person games\nUsing current settings\n");}

	// -------- now write the new settings to file
	wxWriteResource(SOLN_SECT,"Nfg-Algorithm",algorithm,defaults_file);
	wxWriteResource(PARAMS_SECTION,"Stop-After",stopAfter,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-All",all,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Type",dom_type,defaults_file);
	wxWriteResource(SOLN_SECT,"Nfg-ElimDom-Use",use_elimdom,defaults_file);
}
virtual void Warn(const char *warning) // only warn when solving
{if (solving) wxMessageBox((char *)warning,"Standard Solution");}
public:
	NfgSolveSettings(const BaseNfg &nf_,bool solving_=true):solving(solving_),nf(nf_)
	{
	result=SD_SAVE;
	defaults_file="gambit.ini";
	wxGetResource(SOLN_SECT,"Nfg-Algorithm",&algorithm,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-Efg",&extensive,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-Use-Standard",&use_standard,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-Standard-Type",&standard_type,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-Standard-Num",&standard_num,defaults_file);
	wxGetResource(SOLN_SECT,"Nfg-Auto-Inspect-Solns",&auto_inspect,defaults_file);
	if (use_standard) StandardSettings();
	}
	virtual ~NfgSolveSettings()
	{
	if (result!=SD_CANCEL)
	{
		char *defaults_file="gambit.ini";
		wxWriteResource(SOLN_SECT,"Nfg-Algorithm",algorithm,defaults_file);
		wxWriteResource(SOLN_SECT,"Nfg-Efg",extensive,defaults_file);
		wxWriteResource(SOLN_SECT,"Nfg-Use-Standard",use_standard,defaults_file);
		wxWriteResource(SOLN_SECT,"Nfg-Standard-Type",standard_type,defaults_file);
		wxWriteResource(SOLN_SECT,"Nfg-Standard-Num",standard_num,defaults_file);
		wxWriteResource(SOLN_SECT,"Nfg-Auto-Inspect-Solns",auto_inspect,defaults_file);
	}
	}
	NfgSolutionT GetAlgorithm(void) {return algorithm;}
	Bool GetExtensive(void) {return extensive;}
	bool AutoInspect(void) const {return auto_inspect;}
	bool UseStandard(void) const {return use_standard;}
	int  StandardType(void) const {return standard_type;}
	int  StandardNum(void) const {return standard_num;}
};
#define NFG_SOLVE_HELP						"Normal Form Solutions"
#define NFG_STANDARD_HELP					"NFG Standard Solutions"

class NfgSolveParamsDialog: public NfgAlgorithmList, public NfgSolveSettings
{
private:
	wxDialogBox *d;
	wxRadioBox *nfg_algorithm_box;
	wxCheckBox *extensive_box,*auto_inspect_box;
	wxButton *inspect_button;
//	gList<int> solns;
// Static event handlers
	static void params_button_func(wxButton &ob,wxEvent &)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_PARAMS);}
	static void save_button_func(wxButton &ob,wxEvent &)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_SAVE);}
	static void cancel_button_func(wxButton &ob,wxEvent &)
	{((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_CANCEL);}
	static void help_button_func(wxButton &,wxEvent &)
	{wxHelpContents(NFG_SOLVE_HELP);}
	static void nfg_algorithm_box_func(wxRadioBox &,wxEvent &)
	{/*((NfgSolveParamsDialog *)ob.GetClientData())->OnEvent(SD_ALGORITHM);*/}
	// Event handlers: high level
	void OnEvent(int event)
	{
	result=event;
	algorithm=nfg_algorithm_box->GetSelection();
	extensive=extensive_box->GetValue();
	auto_inspect=auto_inspect_box->GetValue();
  use_standard=FALSE;
	d->Show(FALSE);
	}

public:
// Constructor
	NfgSolveParamsDialog(const BaseNfg &nf,int have_efg,wxWindow *parent=0)
						:NfgSolveSettings(nf,false)
	{
		d=new wxDialogBox(parent,"Solutions",TRUE);
		nfg_algorithm_box=MakeNfgAlgorithmList(nf.NumPlayers(),nf.IsConstSum(),d,(wxFunction)nfg_algorithm_box_func);
		nfg_algorithm_box->SetClientData((char *)this);
		nfg_algorithm_box->SetSelection(algorithm);
		d->NewLine();
		extensive_box=new wxCheckBox(d,0,"Extensive Form");
		extensive_box->Enable(have_efg);
		extensive_box->SetValue(extensive && have_efg);
		d->NewLine();
		auto_inspect_box=new wxCheckBox(d,0,"Auto Inspect");
		auto_inspect_box->SetValue(auto_inspect);
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

class NfgSolveStandardDialog:public NfgSolveSettings, public MyDialogBox
{
private:
	char *standard_type_str,*standard_num_str;
	wxStringList *standard_type_list,*standard_num_list;
public:
	NfgSolveStandardDialog(const BaseNfg &nf,wxWindow *parent):
				NfgSolveSettings(nf,false),MyDialogBox(parent,"Standard Solution",NFG_STANDARD_HELP)
	{
	standard_type_list=new wxStringList("Nash","Perfect",0);
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
	~NfgSolveStandardDialog()
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

#endif

#endif
