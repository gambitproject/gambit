// File: paramsd.cc -- Implements the parameter modification class for normal
// and extensive forms
// $Id$

#include "wx.h"
#include "wxmisc.h"
#include "spread.h"
#include "prepoly.h"
#include "gnumber.h"
#include "paramsd.h"
class ParameterDialogC: public SpreadSheet3D
{
private:
	gSpace *space;
   ParameterSetList &params;
   ParametrizedGame *game;
   int &cur_set;
   wxChoice *set_set_item;
   static void add_var_func(wxButton &ob,wxEvent &ev);
   static void add_set_func(wxButton &ob,wxEvent &ev);
   static void set_set_func(wxChoice &ob,wxEvent &ev);
   void UpdateVals(void);
	void CheckVals(void);
   void OnAddSet(void);
   void OnAddVar(void);
   void OnSetSet(void);
public:
	ParameterDialogC(gSpace *space,ParameterSetList &params,wxFrame *parent,
                    ParametrizedGame *game);
   void OnSelectedMoved(int row,int col,SpreadMoveDir how=SpreadMoveJump);
   void OnOk(void);
   Bool OnClose(void);
};

ParameterDialogC::ParameterDialogC(gSpace *space_,ParameterSetList &params_,
				   wxFrame *parent,ParametrizedGame *game_):
		SpreadSheet3D(space_->Dmnsn(),2,1,params_.Name(),parent,ANY_BUTTON),
                space(space_), params(params_), game(game_),
                cur_set(params_.CurSetNum())
{
AddButton("Create Set",(wxFunction)add_set_func);
char **set_names=new char *[params.Length()];
for (int i=0;i<params.Length();i++) set_names[i]=copystring(params[i+1].Name());
set_set_item=new wxChoice(Panel(),(wxFunction)set_set_func,"",
									-1,-1,-1,-1,params.Length(),set_names);
set_set_item->SetClientData((char *)this);
AddButtonNewLine();
AddButton("Add Param",(wxFunction)add_var_func);
AddButtonNewLine();
MakeButtons(OK_BUTTON|PRINT_BUTTON|OPTIONS_BUTTON|HELP_BUTTON);
SetLabelCol(1,"Name");SetLabelCol(2,"Value");
DrawSettings()->SetLabels(S_LABEL_COL);
DrawSettings()->SetColWidth(10,1);
DrawSettings()->SetColWidth(8,2);
set_set_item->SetSelection(cur_set-1);

for (int i=1;i<=space->Dmnsn();i++)
	{SetCell(i,1,space->GetVariableName(i));Bold(i,1,1,TRUE);}
UpdateVals();
SetCurCol(2);
Redraw();
Show(TRUE);
CanvasFocus();
}

void ParameterDialogC::UpdateVals(void)
{
for (int i=1;i<=space->Dmnsn();i++) SetCell(i,2,ToString(params[cur_set][i]));
Repaint();
}
#define UPDATE1_DIALOG	4
#define PARAMS_ADD_VAR	5
void ParameterDialogC::CheckVals(void)
{
gNumber tmp;
bool changed=false;
for (int i=1;i<=space->Dmnsn();i++)
{
	tmp=FromString(GetCell(i,2),tmp);
   if (params[cur_set][i]!=tmp) {params[cur_set][i]=tmp;changed=true;}
}
if (cur_set!=set_set_item->GetSelection()+1)
	{cur_set=set_set_item->GetSelection()+1;changed=true;}
if (changed) game->ChangeParameters(UPDATE1_DIALOG);
}

void ParameterDialogC::OnSetSet(void)
{
CheckVals();
UpdateVals();
CanvasFocus();
}

void ParameterDialogC::OnAddSet(void)
{
gString new_name="Set "+ToString(params.Length()+1);
char *s=wxGetTextFromUser("New Parameter Set Name","Parameter Set",new_name,this);
if (s)
{
	int i=params.Append(ParameterSet(params[cur_set].Length(),s));
   set_set_item->Append(s);
   set_set_item->SetSize(-1,-1,-1,-1);
   set_set_item->SetSelection(i-1);
   OnSetSet();
}
CanvasFocus();
}

void ParameterDialogC::OnAddVar(void)
{
gString new_name=(char)('a'+('x'-'a'+space->Dmnsn())%('z'-'a'+1));
char *s=wxGetTextFromUser("New Variable Name","Variable",new_name,this);
if (s)
{
   game->ChangeParameters(PARAMS_ADD_VAR);
   space->SetVariableName(space->Dmnsn(),s);
   AddRow();
	SetCell(space->Dmnsn(),1,s);Bold(space->Dmnsn(),1,1,TRUE);
   Redraw();
}
CanvasFocus();
}

void ParameterDialogC::OnSelectedMoved(int /*row*/,int col,SpreadMoveDir /*how*/)
{
if (col==1) SetCurCol(2);
CheckVals();
}

void ParameterDialogC::OnOk(void)
{
CheckVals();
SpreadSheet3D::OnOk();
game->ChangeParameters(DESTROY_DIALOG);
}

Bool ParameterDialogC::OnClose(void)
{
OnOk();
return FALSE;
}

void ParameterDialogC::add_var_func(wxButton &ob,wxEvent &)
{((ParameterDialogC *)ob.GetClientData())->OnAddVar();}
void ParameterDialogC::add_set_func(wxButton &ob,wxEvent &)
{((ParameterDialogC *)ob.GetClientData())->OnAddSet();}
void ParameterDialogC::set_set_func(wxChoice &ob,wxEvent &)
{((ParameterDialogC *)ob.GetClientData())->OnSetSet();}

ParameterDialog::ParameterDialog(gSpace *space,ParametrizedGame *game,wxFrame *parent)
{
d=0;
if (space->Dmnsn()==0)
{
	wxMessageBox("The parameter space is empty.\nNo parameters have been defined.\nPlease create the first parameter now.\n");
	gString new_name='x';
	char *s=wxGetTextFromUser("New Variable Name","Variable",new_name,parent);
	if (s)
	{
      game->ChangeParameters(PARAMS_ADD_VAR);
	   space->SetVariableName(space->Dmnsn(),s);
   }
}
if (space->Dmnsn()>0)
	d=new ParameterDialogC(space,game->Parameters(),parent,game);
}

ParameterDialog::~ParameterDialog()
{d->Show(FALSE); delete d;}


#include "glist.imp"
gOutput &operator <<(gOutput &o,const ParameterSet &) {return o;}
// @@ For gcc's benefit only.  Avoids ambiguity in == operator 
int gList<ParameterSet>::Find(const ParameterSet &) const {return 0;} 
template class gList<ParameterSet>;
template class gNode<ParameterSet>;
