class ElimDomParamsDialog
{
private:
	wxDialogBox *d;
	wxButton 		*cancel_button,*ok_button;
	wxListBox 	*player_box;
	wxCheckBox	*all_box,*compress_box;
	wxRadioBox	*dom_type_box;
	Bool 				all,compress;
	int					completed,num_players;
	int					dom_type;
	gBlock<int>	players;
	// now come the private functions
	void 				EnablePlayerBox(Bool e)
	{((wxWindow *)player_box)->Enable(e);}
	void OnEvent(int result)
	{
	completed=result;
	if (completed==wxOK)
	{
		all=all_box->GetValue();compress=compress_box->GetValue();
		dom_type=dom_type_box->GetSelection();
		if (all)
		{
			int num_selections,*selections=new int[num_players];
			num_selections=player_box->GetSelections(&selections);
			players=gBlock<int>(num_selections);
			for (int i=1;i<=num_selections;i++) players[i]=selections[i]+1;
		}
	}
	d->Show(FALSE);delete d;
	}
	static void ok_button_func(wxButton &ob,wxEvent &ev)
	{
	ElimDomParamsDialog *p=(ElimDomParamsDialog *)ob.GetClientData();
	p->OnEvent(wxOK);
	}
	static void cancel_button_func(wxButton &ob,wxEvent &ev)
	{
	ElimDomParamsDialog *p=(ElimDomParamsDialog *)ob.GetClientData();
	p->OnEvent(wxCANCEL);
	}
	static void all_box_func(wxCheckBox &ob,wxEvent &ev)
	{
	ElimDomParamsDialog *p=(ElimDomParamsDialog *)ob.GetClientData();
	p->EnablePlayerBox(ob.GetValue());
	}

public:
	ElimDomParamsDialog(int numplayers,wxWindow *parent=NULL)
	{
	num_players=numplayers;
	all=FALSE,compress=FALSE;
	d=new wxDialogBox(parent,"ElimDom Parameters",TRUE);
	all_box=new wxCheckBox(d,(wxFunction)all_box_func,"Find All");
	all_box->SetClientData((char *)this);
	compress_box=new wxCheckBox(d,NULL,"Compress");
	d->NewLine();
	char *dom_type_list[2]={"Weak","Strong"};
	dom_type_box=new wxRadioBox(d,NULL,"Dom type",-1,-1,-1,-1,2,dom_type_list,2);
	wxStringList *player_list=wxStringListInts(numplayers);
	player_box=new wxListBox(d,NULL,"Players",wxMULTIPLE,-1,-1,-1,-1,num_players,
		player_list->ListToArray());
	d->NewLine();

	wxButton *ok_button=new wxButton(d,(wxFunction)ok_button_func,"Ok");
	ok_button->SetClientData((char *)this);
	wxButton *cancel_button=new wxButton(d,(wxFunction)cancel_button_func,"Cancel");
	cancel_button->SetClientData((char *)this);
	EnablePlayerBox(FALSE);
	d->Fit();
	d->Show(TRUE);
	}
	// Data access functions
	gBlock<int> Players(void) {return players;}
	Bool FindAll(void) {return all;}
	Bool Compress(void) {return compress;}
	int	 DomType(void) {return dom_type;}
	int  Completed(void) {return completed;}

};

class ElimDomInspectDialog:public MyDialogBox
{
private:
	const BaseNormalForm &nf;
	wxText *msg_item;
	char *dom_strat_str;
	wxStringList *dom_strat_list;
	Bool compress;

	static void sset_func(wxChoice &ob,wxEvent &ev)
	{((ElimDomInspectDialog *)ob.GetClientData())->OnSset(ob.GetSelection()+1);}
	void OnSset(int cur_sset)
	{msg_item->SetValue(array_to_string(nf.Dimensionality(cur_sset)));}
	static gString array_to_string(const gArray<int> &a)
	{
	gString tmp='(';
	for (int i=1;i<=a.Length();i++) tmp+=ToString(a[i])+((i==a.Length()) ? ")" : ",");
	return tmp;
	}
public:
	ElimDomInspectDialog(const BaseNormalForm &nf_,int cur_sset,wxWindow *parent=0)
		: MyDialogBox(parent,"ElimDomInspect Options"),nf(nf_)
	{
	msg_item=new wxText(this,0,"Dim:",array_to_string(nf.Dimensionality(cur_sset)),-1,-1,150,-1,wxREADONLY);
	dom_strat_list=wxStringListInts(nf.NumStratSets());
	dom_strat_str=new char[10];strcpy(dom_strat_str,ToString(cur_sset));
	wxFormItem *sset_fitem=Add(wxMakeFormString("Strategy Set",&dom_strat_str,wxFORM_CHOICE,
					new wxList(wxMakeConstraintStrings(dom_strat_list),0)));
	compress=FALSE;
	Add(wxMakeFormNewLine());
	Add(wxMakeFormBool("Compress",&compress));
	AssociatePanel();
	wxChoice *sset_item=(wxChoice *)sset_fitem->GetPanelItem();
	sset_item->SetClientData((char *)this);
	sset_item->Callback((wxFunction)sset_func);
	Go1();
	}
	// Data Access members
	int 	Sset(void) {return (wxListFindString(dom_strat_list,dom_strat_str)+1);}
	Bool 	Compress(void) {return compress;}
};
