//
// FILE: playersd.h -- Defines a dialog to inspect/create EFPlayers for the EF
//
// $Id$
//

#ifndef PLAYERSD_H
#define PLAYERSD_H

class PlayerNamesDialog: public MyDialogBox
{
private:
	BaseEfg	&ef;
	// keep track of panel items
	wxChoice	*name_item;
	wxText		*new_name_item;
	char			*new_player_name;
	// player name
	gString		prev_name;
	// private functions
	static	 	void name_func(wxChoice &ob,wxCommandEvent &ev);
	static		void new_func(wxButton &ob,wxCommandEvent &ev);
	void			NewPlayer(void);
	void			UpdateName(void);
public:
	// Constructor
	PlayerNamesDialog(BaseEfg &ef_,wxWindow *parent=0);
	// Override default OnOk behavior
	void OnOk(void);
	// Returns the last selected player
	EFPlayer	*GetPlayer(void);
};

#ifdef PLAYERSD_INST // instantiate only once
EFPlayer *EfgGetPlayer(const BaseEfg &ef,const gString &name);

//************************************** CONSTRUCTOR **************************
PlayerNamesDialog::PlayerNamesDialog(BaseEfg &ef_,wxWindow *parent)
												:ef(ef_),MyDialogBox(parent,"Player Names",EFG_TREE_HELP)
{
wxStringList *player_list=new wxStringList;char *player_name=new char[20];
player_list->Add(ef.GetChance()->GetName());  // CHANCE is always first
if (ef.NumPlayers()!=0)
{
	for (int i=1;i<=ef.NumPlayers();i++) player_list->Add((ef.Players()[i])->GetName());
	strcpy(player_name,ef.Players()[1]->GetName());
}
else
	strcpy(player_name,ef.GetChance()->GetName());
prev_name=player_name;
wxFormItem *name_fitem=Add(wxMakeFormString("Player", &player_name, wxFORM_CHOICE,
			new wxList(wxMakeConstraintStrings(player_list),0),NULL,wxVERTICAL));
new_player_name=new char[20];
strcpy(new_player_name,player_name);
wxFormItem *new_name_fitem=Add(wxMakeFormString("New name", &new_player_name,wxFORM_TEXT,
			NULL,NULL,wxVERTICAL,100));
Add(wxMakeFormNewLine());
wxFormItem *new_button=Add(wxMakeFormButton("New Player",(wxFunction)new_func));
AssociatePanel();

name_item=(wxChoice *)name_fitem->GetPanelItem();
new_name_item=(wxText *)new_name_fitem->GetPanelItem();

name_item->Callback((wxFunction)name_func);
name_item->SetClientData((char *)this);

((wxButton *)new_button->GetPanelItem())->SetClientData((char *)this);
// Init the first entry
wxCommandEvent ev(wxEVENT_TYPE_CHOICE_COMMAND);
ev.commandInt=0;
name_item->Command(ev);

Go1();
delete [] player_name;
}

void PlayerNamesDialog::UpdateName(void)
{
EFPlayer *player=EfgGetPlayer(ef,prev_name);assert(player);

gString new_name=name_item->GetStringSelection();
if (strcmp(player->GetName(),new_name_item->GetValue())!=0)	// name changed
{
	player->SetName(new_name_item->GetValue());	// might want to check if there already exists one
	if (new_name==prev_name) new_name=player->GetName();
	name_item->Clear();
	for (int i=1;i<=ef.NumPlayers();i++) name_item->Append((ef.Players()[i])->GetName());
	name_item->SetStringSelection(new_name);
}
player=EfgGetPlayer(ef,new_name);
new_name_item->SetValue(player->GetName());
prev_name=new_name;
}

void PlayerNamesDialog::NewPlayer(void)
{
char def_name[20];sprintf(def_name,"Player %d",ef.NumPlayers()+1);
char *new_name=wxGetTextFromUser("New Player's name","Enter Name",def_name,this);
if (new_name)
{
	new_name=copystring(new_name);
	EFPlayer *new_player=ef.NewPlayer();
	new_player->SetName(new_name);
	name_item->Append(new_name);
	prev_name=new_name;
	new_name_item->SetValue(new_name);
	name_item->SetStringSelection(new_name);
	UpdateName();
}
}

EFPlayer *PlayerNamesDialog::GetPlayer(void)
{
if (Completed()!=wxOK) return 0;
return EfgGetPlayer(ef,new_player_name);
}

void PlayerNamesDialog::OnOk(void)
{
EFPlayer *player=EfgGetPlayer(ef,prev_name);
if (player)
	if (strcmp(player->GetName(),new_player_name)!=0)	// name changed
		player->SetName(new_player_name);	// might want to check if there already exists one

MyDialogBox::OnOk();
}


void PlayerNamesDialog::name_func(wxChoice &ob,wxCommandEvent &)
{((PlayerNamesDialog *)ob.GetClientData())->UpdateName();}

void PlayerNamesDialog::new_func(wxButton &ob,wxCommandEvent &)
{((PlayerNamesDialog *)ob.GetClientData())->NewPlayer();}


#endif // instantiate only once
#endif
