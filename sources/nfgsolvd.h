#define SD_CANCEL			-1
#define SD_SOLVE			1
#define	SD_ALGORITHM 	2
#define SD_INSPECT 		3

class NfgSolveParamsDialog
{
private:
	wxDialogBox *d;
	wxButton *cancel_button,*solve_button,*inspect_button;
	wxRadioBox *algorithm_box;
	int result,algorithm;
	gBlock<int> solns;
public:
	NfgSolveParamsDialog(const gBlock<int> &got_solns,wxWindow *parent=0):solns(got_solns)
	{
		d=new wxDialogBox(parent,"Solutions",TRUE);
		char *algorithm_list[5];
		algorithm_list[NFG_PURENASH_SOLUTION]="Pure Nash";
		algorithm_list[NFG_LEMKE_SOLUTION]="Lemke";
		algorithm_list[NFG_LYAPUNOV_SOLUTION]="Lyapunov";
		algorithm_list[NFG_ELIMDOM_SOLUTION]="ElimDom";
		algorithm_list[NFG_ALL_SOLUTION]="All";
		algorithm_box=new	wxRadioBox(d,(wxFunction)algorithm_box_func,"Algorithm",-1,-1,-1,-1,5,algorithm_list,2);
		algorithm_box->SetClientData((char *)this);
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
		solve_button->Enable(algorithm_box->GetSelection()!=NFG_ALL_SOLUTION);
	}
	}
	int GetAlgorithm(void) {return algorithm;}
	int GetResult(void) {return result;}

	static void solve_button_func(wxButton &ob,wxEvent &ev)
	{
		NfgSolveParamsDialog *parent=(NfgSolveParamsDialog *)ob.GetClientData();
		parent->OnEvent(SD_SOLVE);
	}
	static void inspect_button_func(wxButton &ob,wxEvent &ev)
	{
		NfgSolveParamsDialog *d=(NfgSolveParamsDialog *)ob.GetClientData();
		d->OnEvent(SD_INSPECT);
	}
	static void cancel_button_func(wxButton &ob,wxEvent &ev)
	{
		NfgSolveParamsDialog *d=(NfgSolveParamsDialog *)ob.GetClientData();
		d->OnEvent(SD_CANCEL);
	}
	static void algorithm_box_func(wxRadioBox &ob,wxEvent &ev)
	{
		NfgSolveParamsDialog *d=(NfgSolveParamsDialog *)ob.GetClientData();
		d->OnEvent(SD_ALGORITHM);
	}
};

