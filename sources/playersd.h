class PlayerNamesDialog: public MyDialogBox
{
private:
	BaseExtForm	&ef;
	// keep track of panel items
	wxChoice	*name_item;
	wxText		*new_name_item;
	char			*new_player_name;
	// player name
	gString		prev_name;
	// private functions
	static	 	void name_func(wxChoice &ob,wxCommandEvent &ev);
	void			UpdateName(void);
public:
	// Constructor
	PlayerNamesDialog(BaseExtForm &ef_,wxFrame *parent=0);
	// Override default OnOk behavior
	void OnOk(void);
};

//************************************** CONSTRUCTOR **************************
PlayerNamesDialog::PlayerNamesDialog(BaseExtForm &ef_,wxFrame *parent)
												:ef(ef_),MyDialogBox(parent,"Player Names")
{
wxStringList *player_list=new wxStringList;char *player_name=new char[20];
for (int i=1;i<=ef.NumPlayers();i++) player_list->Add((ef.PlayerList()[i])->GetName());
strcpy(player_name,ef.PlayerList()[1]->GetName());prev_name=player_name;
wxFormItem *name_fitem=Add(wxMakeFormString("Player", &player_name, wxFORM_CHOICE,
			new wxList(wxMakeConstraintStrings(player_list),0),NULL,wxVERTICAL));
new_player_name=new char[20];
strcpy(new_player_name,player_name);
wxFormItem *new_name_fitem=Add(wxMakeFormString("New name", &new_player_name,wxFORM_TEXT,
			NULL,NULL,wxVERTICAL,100));
AssociatePanel();

name_item=(wxChoice *)name_fitem->GetPanelItem();
new_name_item=(wxText *)new_name_fitem->GetPanelItem();

name_item->Callback((wxFunction)name_func);
name_item->SetClientData((char *)this);

// Init the first entry
wxCommandEvent ev(wxEVENT_TYPE_CHOICE_COMMAND);
ev.commandInt=0;
name_item->Command(ev);

Go1();
delete [] player_name;
}

void PlayerNamesDialog::UpdateName(void)
{
gString new_name=name_item->GetStringSelection();
Player *player=ef.GetPlayer(prev_name);
if (strcmp(player->GetName(),new_name_item->GetValue())!=0)	// name changed
{
	player->SetName(new_name_item->GetValue());	// might want to check if there already exists one
  if (new_name==prev_name) new_name=player->GetName();
	name_item->Clear();
	for (int i=1;i<=ef.NumPlayers();i++) name_item->Append((ef.PlayerList()[i])->GetName());
	name_item->SetStringSelection(new_name);
}
player=ef.GetPlayer(new_name);
new_name_item->SetValue(player->GetName());
prev_name=new_name;
}

void PlayerNamesDialog::OnOk(void)
{
Player *player=ef.GetPlayer(prev_name);
if (strcmp(player->GetName(),new_player_name)!=0)	// name changed
	player->SetName(new_player_name);	// might want to check if there already exists one

MyDialogBox::OnOk();
}


void PlayerNamesDialog::name_func(wxChoice &ob,wxCommandEvent &ev)
{
PlayerNamesDialog *parent=(PlayerNamesDialog *)ob.GetClientData();
parent->UpdateName();
}




