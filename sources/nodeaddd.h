//
// FILE: nodeaddd.h -- Node Add Dialog.  Handles creation of new nodes for the
//                      extensive form display.
//
// $Id$
//

typedef enum {NodeAddNew,NodeAddIset} NodeAddMode;

class NodeAddDialog : public MyDialogBox  {
private:
  BaseEfg &ef;
  int branches;
  char *player_name;
  char *iset_name;
  Bool set_names;

  // keep track of the panel items
  wxChoice *player_item;
  wxChoice *iset_item;
  wxText *branch_item;

  // event handlers-low level
  static void player_func(wxChoice &ob,wxCommandEvent &ev);
  static void iset_func(wxChoice &ob,wxCommandEvent &ev);
  static void branch_func(wxText &ob,wxCommandEvent &ev);

  // event handlers-high level
  void OnPlayer(const char *name);
  void OnIset(const char *name);
  void OnBranch(void);

public:
  // Constructor and Destructor
  NodeAddDialog(BaseEfg &ef_, EFPlayer *player=0, Infoset *infoset=0,
		int branches=2,wxFrame *frame=0);
  ~NodeAddDialog(void);

  // DataAccess
  NodeAddMode GetAddMode(void) const;
  EFPlayer *GetPlayer(void);
  Infoset *GetInfoset(EFPlayer *);
  int GetBranches(void);
  Bool SetNames(void) const;
};



NodeAddDialog::NodeAddDialog(BaseEfg &ef_, EFPlayer *player,
			     Infoset *infoset, int branches_, wxFrame *frame)
  : MyDialogBox(frame, "Add Node", EFG_NODE_HELP), ef(ef_),
    branches(branches_), player_name(new char[20]), iset_name(new char[20]), 
    set_names(TRUE)
{
  wxStringList *player_list = new wxStringList;
  player_list->Add("Chance");
  for (int pl = 1; pl <= ef.NumPlayers(); pl++)
    player_list->Add(ToString(pl) + ": " + ef.Players()[pl]->GetName());
  player_list->Add("New Player");

  if (player)
    strcpy(player_name, player->GetName());
  else
    strcpy(player_name, "New Player");

  wxFormItem *player_fitem = 
    Add(wxMakeFormString("Player", &player_name, wxFORM_CHOICE,
			 new wxList(wxMakeConstraintStrings(player_list),0)));
  Add(wxMakeFormNewLine());

  wxFormItem *branch_fitem = Add(wxMakeFormShort("Branches", &branches,
						 wxFORM_TEXT));

  wxStringList *infoset_list = new wxStringList;
  infoset_list->Add("New Infoset");
  if (infoset) 
    strcpy(iset_name, infoset->GetName());
  else
    strcpy(iset_name, "New");

  wxFormItem *iset_fitem =
    Add(wxMakeFormString("Iset", &iset_name, wxFORM_CHOICE,
			 new wxList(wxMakeConstraintStrings(infoset_list),0)));

  Add(wxMakeFormNewLine());

  Add(wxMakeFormBool("Set names", &set_names));
  AssociatePanel();

  player_item= (wxChoice *) player_fitem->GetPanelItem();
  player_item->Callback((wxFunction)player_func);
  player_item->SetClientData((char *)this);

  iset_item=(wxChoice *)iset_fitem->GetPanelItem();
  iset_item->Callback((wxFunction)iset_func);
  iset_item->SetClientData((char *)this);
  iset_item->SetStringSelection("New");

  branch_item=(wxText *)branch_fitem->GetPanelItem();
  branch_item->Callback((wxFunction)branch_func);
  branch_item->SetClientData((char *)this);

  // Fake a player selection
  wxCommandEvent ev(wxEVENT_TYPE_CHOICE_COMMAND);
  ev.commandInt=(player) ? player_item->FindString(player->GetName()) : 0;
  player_item->Command(ev);

  // Fake an infoset selection
  if (infoset)  {
    wxCommandEvent ev(wxEVENT_TYPE_CHOICE_COMMAND);
    ev.commandInt=iset_item->FindString(infoset->GetName());
    iset_item->Command(ev);
  }

// Start the dialog
  Go1();
}

NodeAddDialog::~NodeAddDialog(void)
{
  delete [] player_name;
  delete [] iset_name;
}

//-------------------------------------------------------------------------
//                         Data access members
//-------------------------------------------------------------------------


NodeAddMode NodeAddDialog::GetAddMode(void) const
{
  return (iset_item->GetSelection()==0) ? NodeAddNew : NodeAddIset;
}

EFPlayer *NodeAddDialog::GetPlayer(void)
{
  EFPlayer *player;
  if (strcmp(player_name,"New Player") == 0)  {
    player = ef.NewPlayer();
    player->SetName(ToString(ef.NumPlayers()));
  }
  else if (strcmp(player_name, "Chance") == 0)
    player = ef.GetChance();
  else  {
    char tmp[5];
    strncpy(tmp, player_name, (int) (strchr(player_name, ':') - player_name) - 1);
    int number;
    sscanf(tmp, "%d", &number);
    assert(number > 0);
    player = ef.Players()[number];
  }

  return player;
}

Infoset *NodeAddDialog::GetInfoset(EFPlayer *player)
{
  if (GetAddMode() == NodeAddIset)  {
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
    if (player->Infosets()[iset]->GetName() == iset_name)
      return player->Infosets()[iset];
  }
  return 0;
}

int NodeAddDialog::GetBranches(void)
{ return branches; }

Bool NodeAddDialog::SetNames(void) const
{ return set_names; }

//----------------------------------------------------------------------------
//                      Event handlers -- High level
//----------------------------------------------------------------------------

void NodeAddDialog::OnPlayer(const char *name)
{
  iset_item->Clear();
  iset_item->Append("New");
  EFPlayer *player = 0;
  if (strcmp(name, "Chance") == 0)
    player = ef.GetChance();
  else  if (strcmp(name, "New Player") != 0)  {
    char tmp[5];
    strncpy(tmp, name, (int) (strchr(name, ':') - name) - 1);
    int number;
    sscanf(tmp, "%d", &number);
    assert(number > 0);
    player = ef.Players()[number];
  }

  if (player)
    for (int iset = 1; iset <= player->NumInfosets(); iset++)
      iset_item->Append(player->Infosets()[iset]->GetName());
  iset_item->SetSelection(iset_item->FindString("New"));
  iset_item->SetSize(-1,-1,-1,-1); // force it to resize
}

void NodeAddDialog::OnIset(const char *name)
{
  EFPlayer *player = 0;
  if (strcmp(player_item->GetStringSelection(), "Chance") == 0)
    player = ef.GetChance();
  else  if (strcmp(player_item->GetStringSelection(), "New Player") != 0)  {
    char tmp[5];
    strncpy(tmp, player_item->GetStringSelection(),
	    (int) (strchr(player_item->GetStringSelection(), ':') -
		   player_item->GetStringSelection()) - 1);
    int number;
    sscanf(tmp, "%d", &number);
    assert(number > 0);
    player = ef.Players()[number];
  }

  if (!player) return;

  Infoset *infoset = 0;
  for (int iset = 1; iset <= player->NumInfosets(); iset++)
    if (player->Infosets()[iset]->GetName() == name)  {
      infoset = player->Infosets()[iset];
      break;
    }

  if (!infoset) return;
  branch_item->SetValue(ToString(infoset->NumActions()));
}

void NodeAddDialog::OnBranch(void)
{
if (iset_item->GetSelection()!=0) iset_item->SetSelection(0);
}

//-------------------------------------------------------------------------
//                      Event handlers -- Low level
//-------------------------------------------------------------------------

void NodeAddDialog::player_func(wxChoice &ob, wxCommandEvent &)
{
  NodeAddDialog *parent = (NodeAddDialog *)ob.GetClientData();
  parent->OnPlayer(ob.GetStringSelection());
}

void NodeAddDialog::iset_func(wxChoice &ob,wxCommandEvent &)
{
  NodeAddDialog *parent=(NodeAddDialog *)ob.GetClientData();
  parent->OnIset(ob.GetStringSelection());
}

void NodeAddDialog::branch_func(wxText &ob,wxCommandEvent &)
{
  NodeAddDialog *parent=(NodeAddDialog *)ob.GetClientData();
  parent->OnBranch();
}




