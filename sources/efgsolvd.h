#define SD_CANCEL			-1
#define SD_SOLVE			1
#define	SD_ALGORITHM 	2
#define SD_INSPECT 		3

class EfgSolveParamsDialog
{
private:
	wxDialogBox *d;
	wxButton *cancel_button,*solve_button,*inspect_button;
	wxRadioBox *algorithm_box;
	wxCheckBox *normal_box;
	int result,algorithm,normal;
	gBlock<int> solns;
public:
	EfgSolveParamsDialog(const gBlock<int> &got_solns,int have_nfg,wxWindow *parent=0):
				solns(got_solns)
	{
		d=new wxDialogBox(parent,"Solutions",TRUE);
		char *algorithm_list[4];
		algorithm_list[EFG_NORMAL_SOLUTION]="Create NF";
		algorithm_list[EFG_EGOBIT_SOLUTION]="eGobit";
		algorithm_list[EFG_ELIAP_SOLUTION]="eLiap";
		algorithm_list[EFG_ALL_SOLUTION]="All";
		algorithm_box=new	wxRadioBox(d,(wxFunction)algorithm_box_func,"Algorithm",-1,-1,-1,-1,4,algorithm_list,2);
		algorithm_box->SetClientData((char *)this);
		d->NewLine();
		normal_box=new wxCheckBox(d,0,"Normal Form");
		if (!have_nfg) normal_box->Enable(FALSE);
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
	void OnEvent(int event)
	{
	if (event!=SD_ALGORITHM)	// one of the buttons
	{
		result=event;algorithm=algorithm_box->GetSelection();
		normal=normal_box->GetValue();
		d->Show(FALSE);
	}
	else	// new algorithm selected
	{
		inspect_button->Enable(solns.Contains(algorithm_box->GetSelection()));
		solve_button->Enable(algorithm_box->GetSelection()!=EFG_ALL_SOLUTION);
	}
	}
	int GetAlgorithm(void) {return algorithm;}
	int GetResult(void) {return result;}

	static void solve_button_func(wxButton &ob,wxEvent &ev)
	{
		EfgSolveParamsDialog *d=(EfgSolveParamsDialog *)ob.GetClientData();
		d->OnEvent(SD_SOLVE);
	}
	static void inspect_button_func(wxButton &ob,wxEvent &ev)
	{
		EfgSolveParamsDialog *d=(EfgSolveParamsDialog *)ob.GetClientData();
		d->OnEvent(SD_INSPECT);
	}
	static void cancel_button_func(wxButton &ob,wxEvent &ev)
	{
		EfgSolveParamsDialog *d=(EfgSolveParamsDialog *)ob.GetClientData();
		d->OnEvent(SD_CANCEL);
	}
	static void algorithm_box_func(wxRadioBox &ob,wxEvent &ev)
	{
		EfgSolveParamsDialog *d=(EfgSolveParamsDialog *)ob.GetClientData();
		d->OnEvent(SD_ALGORITHM);
	}
};

