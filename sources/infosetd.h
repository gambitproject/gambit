// File: infosetd.h -- Declaration of a class to inspect infosets
// @(#)infosetd.h	1.1 11/20/96
#ifndef INFOSETD_H
#define INFOSETD_H

class InfosetDialog: public MyDialogBox
{
private:
	BaseEfg	&ef;
	// keep track of panel items
	wxListBox	*player_item,*iset_item;
	wxText		*iset_name_item;
	wxText		*branches_item,*nodes_item;
	// previous infoset
	Infoset		*prev_iset;
	// keep track of changes
	bool			game_changed;
	// private functions
	static	 	void player_func(wxListBox &ob,wxCommandEvent &ev);
	static		void iset_func(wxListBox &ob,wxCommandEvent &ev);
	static		void new_func(wxButton &ob,wxCommandEvent &ev);
	static		void remove_func(wxButton &ob,wxCommandEvent &ev);
	static		void ok_func(wxButton &ob,wxCommandEvent &ev);
	static		void cancel_func(wxButton &ob,wxCommandEvent &ev);
	static		void help_func(wxButton &ob,wxCommandEvent &ev);

	void			OnOk(void);
	void			OnCancel(void);
	void			OnHelp(void);

	void			OnPlayer(int );
  void			OnIset(int );

	void			NewInfoset(void);
	void			RemoveInfoset(void);
public:
	// Constructor
	InfosetDialog(BaseEfg &ef_,wxFrame *parent=0);
	bool GameChanged(void);
};

//************************************** CONSTRUCTOR **************************
InfosetDialog::InfosetDialog(BaseEfg &ef_,wxFrame *parent)
												:ef(ef_),MyDialogBox(parent,"Infosets Info",EFG_TREE_HELP)
{
SetLabelPosition(wxVERTICAL);
player_item = new wxListBox(this, (wxFunction)player_func, "Player", wxSINGLE, 11, 3, 104, 125, 0, NULL, 0, "player_item");
player_item->wxEvtHandler::SetClientData((char *)this);
iset_item = new wxListBox(this, (wxFunction)iset_func, "Infoset", wxSINGLE, 130, 4, 100, 125, 0, NULL, 0, "iset_item");
iset_item->wxEvtHandler::SetClientData((char *)this);
iset_name_item = new wxText(this, 0, "Iset Name", "", 251, 12, 174, 58, 0, "iset_name_item");
branches_item = new wxText(this, 0, "Branches", "", 330, 76, 80, 54,wxTE_READONLY, "branches_item");
nodes_item = new wxText(this, 0, "Nodes", "", 253, 77, 51, 53, wxTE_READONLY, "nodes_item");

wxButton *ok_button = new wxButton(this, (wxFunction)ok_func, "Ok", 13, 162, -1, -1, 0, "ok_button");
ok_button->SetClientData((char *)this);
wxButton *cancel_button = new wxButton(this, (wxFunction)cancel_func, "Cancel", 71, 163, -1, -1, 0, "cancel_button");
cancel_button->SetClientData((char *)this);
wxButton *new_button = new wxButton(this, (wxFunction)new_func, "New", 162, 164, -1, -1, 0, "new_button");
new_button->SetClientData((char *)this);
wxButton *remove_button = new wxButton(this, (wxFunction)remove_func, "Remove", 234, 165, -1, -1, 0, "remove_button");
remove_button->SetClientData((char *)this);
wxButton *help_button = new wxButton(this, (wxFunction)help_func, "Help", 342, 164, -1, -1, 0, "help_button");
help_button->SetClientData((char *)this);

// Fill in the listboxes
int i;char tmp_str[50];
for (i=1;i<=ef.NumPlayers();i++)
{
	if (ef.PlayerList()[i]->GetName()!="")
		player_item->Append(ef.PlayerList()[i]->GetName());
	else
		{sprintf(tmp_str,"Player %d",i);player_item->Append(tmp_str);}
}
Fit();
prev_iset=0;
game_changed=false;
// Init the first entry
OnPlayer(0);
Show(TRUE);
}

void InfosetDialog::OnPlayer(int num)
{
if (ef.NumPlayers()==0)
{
	if (player_item->Number()==0) player_item->Append("None");return;
}
player_item->SetSelection(num);
EFPlayer *player=ef.PlayerList()[num+1];
iset_item->Clear();
for (int i=1;i<=player->NumInfosets();i++) iset_item->Append(ToString(i));
OnIset(0);
}

void InfosetDialog::OnIset(int num)
{
if (prev_iset)
	if (strcmp(prev_iset->GetName(),iset_name_item->GetValue())!=0)
		prev_iset->SetName(iset_name_item->GetValue());

EFPlayer *player=ef.PlayerList()[player_item->GetSelection()+1];
if (player->NumInfosets()==0)  // can have players w/ no isets
{
	if (iset_item->Number()==0) iset_item->Append("None");return;
}
iset_item->SetSelection(num);
Infoset *iset=player->InfosetList()[num+1];
iset_name_item->SetValue(iset->GetName());
branches_item->SetValue(ToString(iset->NumActions()));
nodes_item->SetValue(ToString(iset->NumMembers()));
prev_iset=iset;
}

void InfosetDialog::NewInfoset(void)
{
MyDialogBox *new_iset_dialog=new MyDialogBox(this,"New Infoset");
wxStringList *player_list=new wxStringList;char *player_name=new char[20];
char tmp_str[50];
for (int i=1;i<=ef.NumPlayers();i++)
{
	if (ef.PlayerList()[i]->GetName()!="")
		player_list->Add(ef.PlayerList()[i]->GetName());
	else
		{sprintf(tmp_str,"Player %d",i);player_list->Add(tmp_str);}
}
sprintf(player_name,(char *)player_list->Nth(player_item->GetSelection())->Data());
new_iset_dialog->Add(wxMakeFormString("Player",&player_name, wxFORM_CHOICE,
			new wxList(wxMakeConstraintStrings(player_list),0),NULL,wxVERTICAL));
char *iset_name=new char[40];sprintf(iset_name,"infoset");
new_iset_dialog->Add(wxMakeFormString("Iset Name",&iset_name,wxFORM_TEXT,
			NULL,NULL,wxVERTICAL,100));
static int num_branches=2; // remember last value used
new_iset_dialog->Add(wxMakeFormShort("Branches",&num_branches,wxFORM_TEXT,
			new wxList(wxMakeConstraintRange(1,1000),0),0,wxVERTICAL,40));
new_iset_dialog->Go();
if (new_iset_dialog->Completed()==wxOK)
{
	int pl=wxListFindString(player_list,player_name);
	EFPlayer *player=ef.PlayerList()[pl+1];
	Infoset *iset=ef.CreateInfoset(player,num_branches);
	iset->SetName(iset_name);
	game_changed=true;
	if (pl==player_item->GetSelection()) iset_item->Append(ToString(player->NumInfosets()));
}
delete new_iset_dialog;
delete [] iset_name;delete [] player_name;
}

void InfosetDialog::RemoveInfoset(void)
{
EFPlayer *player=ef.PlayerList()[player_item->GetSelection()+1];
Infoset *iset=player->InfosetList()[iset_item->GetSelection()+1];
if (iset->NumMembers()!=0)
	{wxMessageBox("This infoset is not empty.\nOnly empty infosets can be delted");return;}
ef.DeleteEmptyInfoset(iset);
game_changed=true;
}

void InfosetDialog::OnOk(void)
{
EFPlayer *player=ef.PlayerList()[player_item->GetSelection()+1];
Infoset *iset=player->InfosetList()[iset_item->GetSelection()+1];
if (strcmp(iset->GetName(),iset_name_item->GetValue())!=0)	// name changed
	iset->SetName(iset_name_item->GetValue());

Show(FALSE);
}

bool InfosetDialog::GameChanged(void)
{return game_changed;}

void InfosetDialog::OnCancel(void)
{Show(FALSE);}

void InfosetDialog::OnHelp(void)
{wxHelpContents("");}



void InfosetDialog::player_func(wxListBox &ob,wxCommandEvent &ev)
{((InfosetDialog *)ob.wxEvtHandler::GetClientData())->OnPlayer(ev.commandInt);}

void InfosetDialog::iset_func(wxListBox &ob,wxCommandEvent &ev)
{((InfosetDialog *)ob.wxEvtHandler::GetClientData())->OnIset(ev.commandInt);}

void InfosetDialog::new_func(wxButton &ob,wxCommandEvent &)
{((InfosetDialog *)ob.GetClientData())->NewInfoset();}

void InfosetDialog::remove_func(wxButton &ob,wxCommandEvent &)
{((InfosetDialog *)ob.GetClientData())->RemoveInfoset();}

void InfosetDialog::ok_func(wxButton &ob,wxCommandEvent &)
{((InfosetDialog *)ob.GetClientData())->OnOk();}

void InfosetDialog::cancel_func(wxButton &ob,wxCommandEvent &)
{((InfosetDialog *)ob.GetClientData())->OnCancel();}

void InfosetDialog::help_func(wxButton &ob,wxCommandEvent &)
{((InfosetDialog *)ob.GetClientData())->OnHelp();}

#endif
