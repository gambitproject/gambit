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
	int result,algorithm;
	gBlock<int> solns;
public:
	EfgSolveParamsDialog(const gBlock<int> &got_solns,wxWindow *parent=NULL)
	{
		solns=got_solns;
		d=new wxDialogBox(parent,"Solutions",TRUE);
		wxPanel *p=new wxPanel(d);
		char *algorithm_list[3];
		algorithm_list[EFG_NORMAL_SOLUTION]="Create NF";
		algorithm_list[EFG_EGAMBIT_SOLUTION]="eGambit";
		algorithm_list[EFG_ALL_SOLUTION]="All";
		algorithm_box=new	wxRadioBox(p,(wxFunction)algorithm_box_func,"Algorithm",-1,-1,-1,-1,3,algorithm_list,2);
		algorithm_box->SetClientData((char *)this);
		p->NewLine();
		solve_button=new wxButton(p,(wxFunction)solve_button_func,"Solve");
		solve_button->SetClientData((char *)this);
		inspect_button=new wxButton(p,(wxFunction)inspect_button_func,"Look");
		inspect_button->SetClientData((char *)this);
		cancel_button=new wxButton(p,(wxFunction)cancel_button_func,"Cancel");
		cancel_button->SetClientData((char *)this);
		OnEvent(SD_ALGORITHM);
		p->Fit();
		d->Fit();
		d->Show(TRUE);
	}
	void OnEvent(int event)
	{
	if (event!=SD_ALGORITHM)	// one of the buttons
	{
		result=event;algorithm=algorithm_box->GetSelection();
		d->Show(FALSE); delete d;
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

