// File: efgoutcd.cc -- code for the EFG outcome editing dialog
//  $Id$
#include "wx.h"
#include "wxmisc.h"
#include "spread.h"
#include "efg.h"
#include "treewin.h"
#include "efgoutcd.h"
#include "efgshow.h"

/****************************************************************************
                     EFG OUTCOME DIALOG
****************************************************************************/
class EfgOutcomeDialogC: public SpreadSheet3D
{
private:
	static void outcome_attach_func(wxButton &ob,wxEvent &);
	static void outcome_detach_func(wxButton &ob,wxEvent &);
	static void outcome_delete_func(wxButton &ob,wxEvent &);
	static void outcome_polyval_func(wxButton &ob,wxEvent &);
	static void settings_func(wxButton &ob,wxEvent &);
protected:
	EfgOutcomeDialog *parent;
	Efg	&ef;
	ParameterSetList &params;
	TreeWindow *tw;
	int prev_outc_num;
   bool &polyval;
	class OutcomeDragger;
	OutcomeDragger *outcome_drag;
   wxButton *polyval_but;
	void OnAttach(void);
	void OnDetach(void);
	virtual void OnDelete(void);
	void OnSettings(void);
   void OnPolyval(void);
	void CheckOutcome(int outc_num);
	virtual void PayoffPos(int outc_num,int player,int *row,int *col) = 0;
	virtual void NamePos(int outc_num,int *row,int *col) = 0;
	virtual Bool OnEventNew(wxMouseEvent &ev);
public:
	EfgOutcomeDialogC(int rows,int cols,Efg &ef,ParameterSetList &params,
                     TreeWindow *tw,EfgOutcomeDialog *parent);
	void SetCurOutcome(const gString &out_name);
	void OnHelp(int );
	// This implements the behavior that a new row is created automatically
	// below the greatest ENTERED row.  Also, if we move to a new row, the
	// previous row is automatically saved in the ef.
	virtual void OnSelectedMoved(int ,int ,SpreadMoveDir ) { };
	virtual void OnDoubleClick(int ,int ,int ,const gString &);
	virtual void UpdateValues(void);
	virtual void OnOk(void);
	virtual int  OutcomeNum(int row=0,int col=0) = 0;
   virtual int	 PlayerNum(int row=0, int col=0) = 0;
	virtual Bool OnClose(void);
};

wxCursor *outcome_cursor;
#define DRAG_NONE				0			// Current drag state
#define DRAG_START			1
#define DRAG_CONTINUE   2
#define DRAG_STOP       3
class EfgOutcomeDialogC::OutcomeDragger
{
private:
	EfgOutcomeDialogC *parent;
	TreeWindow *tw;
	int drag_now;
	int outcome;
	int x,y;
public:
	OutcomeDragger(EfgOutcomeDialogC *parent_,TreeWindow *tw_);
	int OnEvent(wxMouseEvent &ev);
};

EfgOutcomeDialogC::OutcomeDragger::OutcomeDragger(EfgOutcomeDialogC *parent_,
			TreeWindow *tw_):parent(parent_),tw(tw_),drag_now(0)
{
#ifdef wx_msw
 outcome_cursor=new wxCursor("OUTCOMECUR");
#else
 #include "bitmaps/outcome.xbm"
 outcome_cursor=new wxCursor(outcome_bits,outcome_width,outcome_height,-1,-1,outcome_bits);
#endif
}

int EfgOutcomeDialogC::OutcomeDragger::OnEvent(wxMouseEvent &ev)
{
int ret=(drag_now) ? DRAG_CONTINUE : DRAG_NONE;
if (ev.Dragging())
{
	if (!drag_now)
	{
		drag_now=1;
		parent->GetSheet()->SetCursor(outcome_cursor);
		parent->GetSheet()->CaptureMouse();
		outcome=parent->OutcomeNum();
		ret=DRAG_START;
	}
}
if (ev.LeftUp() && drag_now)
{
	drag_now=0;
	parent->GetSheet()->SetCursor(wxSTANDARD_CURSOR);
	parent->GetSheet()->ReleaseMouse();
	float x,y;ev.Position(&x,&y);
	if (x>2000) x-=65536.0; if (y>2000) y-=65536.0; // negative integer overflow
	int xi=(int)x,yi=(int)y;
	parent->GetSheet()->ClientToScreen(&xi,&yi);
	ret=DRAG_STOP;
	tw->node_outcome(outcome,xi,yi);
}
return ret;
}


// Constructor
EfgOutcomeDialogC::EfgOutcomeDialogC(int rows,int cols,Efg &ef_,
           ParameterSetList &params_, TreeWindow *tw_,EfgOutcomeDialog *parent_)
	  	   :SpreadSheet3D(rows,cols,1,"Outcomes [S]",(wxFrame *)tw_->GetParent(),ANY_BUTTON),
                    parent(parent_), ef(ef_), params(params_), tw(tw_),
		    polyval(params_.PolyVal())
{
MakeButtons(OK_BUTTON|PRINT_BUTTON|OPTIONS_BUTTON|HELP_BUTTON);
AddButton("Opt",(wxFunction)settings_func);
AddButtonNewLine();
AddButton("Attach",(wxFunction)outcome_attach_func);
AddButton("Detach",(wxFunction)outcome_detach_func);
AddButton("Delete",(wxFunction)outcome_delete_func);
polyval_but=AddButton("Poly",(wxFunction)outcome_polyval_func);
prev_outc_num=1;
outcome_drag=new OutcomeDragger(this,tw);
CanvasFocus();
}

// Handler functions -> stubs to actual functions
void EfgOutcomeDialogC::outcome_attach_func(wxButton &ob,wxEvent &)
{((EfgOutcomeDialogC *)ob.GetClientData())->OnAttach();}
void EfgOutcomeDialogC::outcome_detach_func(wxButton &ob,wxEvent &)
{((EfgOutcomeDialogC *)ob.GetClientData())->OnDetach();}
void EfgOutcomeDialogC::outcome_delete_func(wxButton &ob,wxEvent &)
{((EfgOutcomeDialogC *)ob.GetClientData())->OnDelete();}
void EfgOutcomeDialogC::outcome_polyval_func(wxButton &ob,wxEvent &)
{((EfgOutcomeDialogC *)ob.GetClientData())->OnPolyval();}

void EfgOutcomeDialogC::settings_func(wxButton &ob,wxEvent &)
{((EfgOutcomeDialogC *)ob.GetClientData())->OnSettings();}

// OnPolyval
void EfgOutcomeDialogC::OnPolyval(void)
{
	polyval=(polyval) ? false : true;
   polyval_but->SetLabel((polyval) ? "Eval" : "Poly");
   tw->OnPaint();
	UpdateValues(); Repaint(); CanvasFocus();
}

// OnAttach
void EfgOutcomeDialogC::OnAttach(void)
{tw->node_outcome(OutcomeNum());CanvasFocus();}
// OnDetach
void EfgOutcomeDialogC::OnDetach(void)
{tw->node_outcome(0);CanvasFocus();}
// OnDelete
void EfgOutcomeDialogC::OnDelete(void)
{
char tmp_str[256];
int outc_num=OutcomeNum();
gString outc_name=ef.Outcomes()[outc_num]->GetName();
sprintf(tmp_str,"Delete Outcome '%s'?",(const char *)outc_name);
if (wxMessageBox(tmp_str,"Confirm",wxOK|wxCANCEL)==wxOK)
{
	if (outc_num<=ef.NumOutcomes()) // not the last, blank row
	{
		EFOutcome *tmp=ef.Outcomes()[outc_num];;
		assert(tmp);
		ef.DeleteOutcome(tmp);
		tw->node_outcome(-1);
		// Derive to take care of resizing the new window
	}
}
CanvasFocus();
}
// OnSettings
void EfgOutcomeDialogC::OnSettings(void)
{
MyDialogBox *options_dialog=new MyDialogBox(this,"Outcome Settings",EFG_OUTCOME_HELP);
wxStringList *opt_list=new wxStringList("Compact Format","Long Entries",0);
char *opt_str=new char[25];
int dialog_type;
char *defaults_file="gambit.ini";
wxGetResource("Gambit","EFOutcome-Dialog-Type",&dialog_type,defaults_file);
strcpy(opt_str,(char *)opt_list->Nth(dialog_type)->Data());
options_dialog->Add(wxMakeFormString("Dialog Type",&opt_str,wxFORM_RADIOBOX,
			 new wxList(wxMakeConstraintStrings(opt_list), 0)));
options_dialog->Go();
if (options_dialog->Completed()==wxOK)
{
	int new_dialog_type=wxListFindString(opt_list,opt_str);
	if (new_dialog_type!=dialog_type)
	{
		wxMessageBox("New dialog type will be used\nnext time the outcome window is created","Outcome Display",wxOK|wxCENTRE,this);
		wxWriteResource("Gambit","EfgOutcome-Dialog-Type",new_dialog_type,defaults_file);
	}
}
delete options_dialog;
}

// OnHelp
void EfgOutcomeDialogC::OnHelp(int )
{wxHelpContents(EFG_OUTCOME_HELP);}

// SetCurOutcome
void EfgOutcomeDialogC::SetCurOutcome(const gString &out_name)
{
int out=0;
if (out_name!="")
	for (int i=1;i<=ef.NumOutcomes();i++)
		if ((ef.Outcomes()[i])->GetName()==out_name) out=i;
if (out)
{
	int row,col;
	NamePos(out,&row,&col);
	SetCurRow(row);SetCurCol(col);
	OnSelectedMoved(row,col,SpreadMoveJump);  // we could have been editing
}
}
// OnOk -- check if the current outcome has changed
void EfgOutcomeDialogC::OnOk(void)
{
CheckOutcome(OutcomeNum(CurRow(),CurCol()));
parent->OnOk();
}
// OnClose -- close the window, as if OK was pressed.
Bool EfgOutcomeDialogC::OnClose(void)
{OnOk();return FALSE;}

// OnEvent -- check if we are dragging an outcome
Bool EfgOutcomeDialogC::OnEventNew(wxMouseEvent &ev)
{
if (outcome_drag->OnEvent(ev)!=DRAG_NONE) return TRUE;
return FALSE;
}

void EfgOutcomeDialogC::UpdateValues(void)
{
int row,col;
EFOutcome *tmp;
gString payoff;
bool hilight;
for (int i=1;i<=ef.NumOutcomes();i++)
{
	tmp= ef.Outcomes()[i];
	for (int j=1;j<=ef.NumPlayers();j++)
	{
		PayoffPos(i,j,&row,&col);
      hilight=false;
      if (polyval==false)
      	payoff=ToString(ef.Payoff(tmp, j));
      else
      {
      	payoff=ToString(ef.Payoff(tmp, j).Evaluate(params.CurSet()));
         if (ef.Payoff(tmp, j).Degree()>0) hilight=true;
      }
		SetCell(row,col,payoff);
      HiLighted(row,col,0,hilight);
	}
	NamePos(i,&row,&col);
	SetCell(row,col,tmp->GetName());
}
}


void EfgOutcomeDialogC::CheckOutcome(int outc_num)
{
assert(outc_num>0 && outc_num<=ef.NumOutcomes()+1);
bool outcomes_changed=false;
EFOutcome *tmp;
// if a new outcome has created, append it to the list of outcomes
if (outc_num>ef.NumOutcomes())
{
	tmp=ef.NewOutcome();
	tmp->SetName("Outcome "+ToString(ef.NumOutcomes()));
}
else
	tmp=ef.Outcomes()[outc_num];
assert(tmp);
// check if the values have changed
int prow,pcol;
for (int j=1;j<=ef.NumPlayers();j++)
{
	PayoffPos(outc_num,j,&prow,&pcol);
	if (polyval==false)
   {
		gPoly<gNumber> payoff(ef.Parameters(),GetCell(prow,pcol),ef.ParamOrder());
		if (ef.Payoff(tmp, j)!=payoff)	{
			ef.SetPayoff(tmp, j, payoff);
			outcomes_changed=true;
		}
   }
   else
   {
		gNumber payoff;payoff=FromString(GetCell(prow,pcol),payoff);
      gNumber diff=abs(ef.Payoff(tmp, j).Evaluate(params.CurSet()) - payoff);
      gNumber eps=diff;gEpsilon(eps);
		if (diff>eps)	{
			ef.SetPayoff(tmp, j, gPoly<gNumber>(ef.Parameters(),payoff,ef.ParamOrder()));
			outcomes_changed=true;
		}
	}
}
// check if the name has changed
NamePos(outc_num,&prow,&pcol);
gString new_name=GetCell(prow,pcol);
if (new_name!=tmp->GetName())
	if (new_name!="")
	{
		tmp->SetName(new_name);
		tw->OnPaint();
	}
	else
	{
		SetCell(prow,pcol,tmp->GetName());
		OnPaint();
	}
if (outcomes_changed) tw->node_outcome(-1);
}


void EfgOutcomeDialogC::OnDoubleClick(int row,int col,int /*level*/,const gString &)
{
static bool busy=false;
if (busy) return;
int outc_num=OutcomeNum(row,col);
EFOutcome *tmp=ef.Outcomes()[outc_num];
int pl=PlayerNum(row,col);
busy=true;
if (pl==0) return; // double click only edits player payoffs.
gString s0=ToString(ef.Payoff(tmp, pl));
int x=GetSheet()->MaxX(col-1)+TEXT_OFF,y=GetSheet()->MaxY(row-1)+TEXT_OFF;
GetSheet()->ClientToScreen(&x,&y);
gString s1=gGetTextLine(s0,this,x,y);
if (s1!="" && s0!=s1)
{
	ef.SetPayoff(tmp, pl, gPoly<gNumber>(ef.Parameters(),s1,ef.ParamOrder()));
   UpdateValues();
   tw->node_outcome(-1);
	Repaint();
}
busy=false;
}

/****************************************************************************
												SHORT ENTRY OUTCOME DIALOG
****************************************************************************/


class EfgOutcomeDialogShort: public EfgOutcomeDialogC
{
protected:
	void OnDelete(void);
	int  OutcomeNum(int row=0,int col=0);
   int  PlayerNum(int row=0, int col=0);
	void PayoffPos(int outc_num,int player,int *row,int *col);
	void NamePos(int outc_num,int *row,int *col);
public:
	EfgOutcomeDialogShort(Efg &ef,ParameterSetList &params,TreeWindow *tw,EfgOutcomeDialog *parent);
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	virtual void OnOptionsChanged(unsigned int options=0);
};


EfgOutcomeDialogShort::EfgOutcomeDialogShort(Efg &ef_,ParameterSetList &params,
                              TreeWindow *tw_, EfgOutcomeDialog *parent_)
						: EfgOutcomeDialogC((ef_.NumOutcomes()) ? ef_.NumOutcomes() : 1,
														ef_.NumPlayers()+1,ef_,params,tw_,parent_)
{
DrawSettings()->SetLabels(S_LABEL_ROW|S_LABEL_COL);
DataSettings()->SetChange(S_CAN_GROW_ROW);
DataSettings()->SetAutoLabel(S_AUTO_LABEL_ROW);
DataSettings()->SetAutoLabelStr("Out:%d",S_AUTO_LABEL_ROW);
DrawSettings()->SetColWidth(9,GetCols()); // 'Outcome #'=9 chars
SetLabelCol(GetCols(),"Name");
int i,j;
for (j=1;j<=ef.NumPlayers();j++) DrawSettings()->SetColWidth(2+ToStringPrecision(),j);
// make all the cells string input
for (i=1;i<=GetRows();i++)
	for (j=1;j<=GetCols();j++)
		SetType(i,j,gSpreadStr);

for (i=1;i<=ef.NumPlayers();i++)
	SetLabelCol(i,(ef.Players()[i])->GetName());

UpdateValues();
Redraw();
}

// OnDelete

void EfgOutcomeDialogShort::OnDelete(void)
{
EfgOutcomeDialogC::OnDelete();
int outc_num=OutcomeNum();
DelRow(outc_num);
Redraw();
}

// This implements the behavior that a new row is created automatically
// below the greatest ENTERED row.  Also, if we move to a new row, the
// previous row is automatically saved in the ef.

void EfgOutcomeDialogShort::OnSelectedMoved(int row,int col,SpreadMoveDir )
{
if (OutcomeNum(row,col)!=prev_outc_num)
	{CheckOutcome(prev_outc_num);prev_outc_num=OutcomeNum(row,col);}
if (row==GetRows() && EnteredCell(row,1))
	{AddRow();Redraw();OnPaint();}
}

// Functions that determine the window layout

int EfgOutcomeDialogShort::OutcomeNum(int row,int )
{
if (row==0) row=CurRow();
return row;
}

int EfgOutcomeDialogShort::PlayerNum(int ,int col)
{
if (col==0) col=CurCol();
if (col==GetCols()) return 0;
return col;
}

void EfgOutcomeDialogShort::PayoffPos(int outc_num,int player,int *row,int *col)
{
*row=outc_num;*col=player;
}


void EfgOutcomeDialogShort::NamePos(int outc_num,int *row,int *col)
{
*row=outc_num;*col=ef.NumPlayers()+1;
}


void EfgOutcomeDialogShort::OnOptionsChanged(unsigned int options)
{
if (options&S_PREC_CHANGED)
{
	UpdateValues();
	for (int j=1;j<=ef.NumPlayers();j++)
	DrawSettings()->SetColWidth(2+ToStringPrecision(),j);
	Resize();Repaint();
}
}

/****************************************************************************
												LONG ENTRY OUTCOME DIALOG
****************************************************************************/


class EfgOutcomeDialogLong: public EfgOutcomeDialogC
{
protected:
	void OnDelete(void);
	int  OutcomeNum(int row=0,int col=0);
   int  PlayerNum(int row=0, int col=0);
	void PayoffPos(int outc_num,int player,int *row,int *col);
	void NamePos(int outc_num,int *row,int *col);
public:
	EfgOutcomeDialogLong(Efg &ef,ParameterSetList &params,TreeWindow *tw,EfgOutcomeDialog *parent);
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	virtual void OnOptionsChanged(unsigned int options=0);
};


EfgOutcomeDialogLong::EfgOutcomeDialogLong(Efg &ef_,ParameterSetList &params,
                                    TreeWindow *tw_,EfgOutcomeDialog *parent_)
				:EfgOutcomeDialogC((ef_.NumOutcomes() ? ef_.NumOutcomes() : 1)*ef_.NumPlayers(),
												3,ef_,params,tw_,parent_)
{
DrawSettings()->SetLabels(S_LABEL_ROW|S_LABEL_COL);
DataSettings()->SetChange(S_CAN_GROW_ROW);
DrawSettings()->SetColWidth(9,1);	// "Player #"=9 chars : player column
DrawSettings()->SetColWidth(12,2);		// values column
DrawSettings()->SetColWidth(9,3); // "Outcome #"=9 chars : outcome name column
SetLabelCol(1,"Player");
SetLabelCol(2,"Payoff");
SetLabelCol(3,"Name");
if (ef.NumOutcomes()==0) ef.NewOutcome();
int i,j;
for (i=1;i<=GetRows();i++)	SetType(i,2,gSpreadStr);
for (j=1;j<=ef.NumOutcomes();j++)				// set player and outcome names
{
	for (i=1;i<=ef.NumPlayers();i++)
	{
		SetCell((j-1)*ef.NumPlayers()+i,1,(ef.Players()[i])->GetName());
		Bold((j-1)*ef.NumPlayers()+i,1,0,TRUE);
	}
	SetCell((j-1)*ef.NumPlayers()+1,3,ef.Outcomes()[j]->GetName());
	SetLabelRow((j-1)*ef.NumPlayers()+1,"Out:"+ToString(j));
	SetType((j-1)*ef.NumPlayers()+1,3,gSpreadStr);
}
SetCurRow(1);SetCurCol(2);
UpdateValues();
Redraw();
}

// OnDelete

void EfgOutcomeDialogLong::OnDelete(void)
{
EfgOutcomeDialogC::OnDelete();
int outc_num=OutcomeNum();
int i;
for (i=1;i<=ef.NumPlayers();i++)
	DelRow(ef.NumPlayers()*outc_num-i+1);
for (i=outc_num+1;i<=ef.NumOutcomes();i++)
	SetLabelRow((i-1)*ef.NumPlayers()+1,"Out:"+ToString(i));

Redraw();
}

// This implements the behavior that a new row is created automatically
// below the greatest ENTERED row.  Also, if we move to a new row, the
// previous row is automatically saved in the ef. Also prevents the user
// from going to non-modifiable cells (player name column,some outcome name cells)

void EfgOutcomeDialogLong::OnSelectedMoved(int row,int col,SpreadMoveDir how)
{
if (col==1) {SetCurCol(2);return;}	// player name column is off limits
int outc_num=OutcomeNum(row,col);
if (col==3)                       	// outcome name column is special
{
	if (how==SpreadMoveRight || how==SpreadMoveJump)
		if (row!=ef.NumPlayers()*(outc_num-1)+1)
			{row=ef.NumPlayers()*(outc_num-1)+1;SetCurRow(row);}
	if (how==SpreadMoveUp && outc_num>1)	// jump to a previous outcome
	{
		int prow,pcol;
		int n_outc_num=OutcomeNum(row+1,col);
		NamePos(n_outc_num-1,&prow,&pcol);
		SetCurRow(prow);
	}
	if (how==SpreadMoveDown) // jump to next outcome
	{
		if (row>GetRows()-ef.NumPlayers())
			SetCurRow(GetRows()-ef.NumPlayers()+1);
		else
		{
			int prow,pcol;
			int n_outc_num=OutcomeNum(row-1,col);
			NamePos(n_outc_num+1,&prow,&pcol);
			SetCurRow(prow);
		}
	}
}
if (outc_num!=prev_outc_num)				// check/save outcome
	{CheckOutcome(prev_outc_num);prev_outc_num=outc_num;}
if (row==GetRows() && EnteredCell(row,2))	// add an outcome
{
	for (int i=1;i<=ef.NumPlayers();i++)
	{
		AddRow();
		SetCell(GetRows(),1,ef.Players()[i]->GetName());
		Bold(GetRows(),1,0,TRUE);
	}
	SetLabelRow(GetRows()-ef.NumPlayers()+1,"Out:"+ToString(ef.NumOutcomes()+1));
	Redraw();OnPaint();
}
}

// Functions that determine the window layout

int EfgOutcomeDialogLong::OutcomeNum(int row,int )
{
if (row==0) row=CurRow();
return (row-1)/ef.NumPlayers()+1;
}

int EfgOutcomeDialogLong::PlayerNum(int row,int col)
{
if (row==0) row=CurRow();
if (col==0) col=CurCol();
if (col!=2) return 0;

return (row-1)%ef.NumPlayers()+1;
}

void EfgOutcomeDialogLong::PayoffPos(int outc_num,int player,int *row,int *col)
{
*row=(outc_num-1)*ef.NumPlayers()+player;*col=2;
}


void EfgOutcomeDialogLong::NamePos(int outc_num,int *row,int *col)
{
*row=(outc_num-1)*ef.NumPlayers()+1;*col=3;
}


void EfgOutcomeDialogLong::OnOptionsChanged(unsigned int options)
{
if (options&S_PREC_CHANGED)
{
	UpdateValues();
	DrawSettings()->SetColWidth(2+ToStringPrecision(),2);
	Resize();Repaint();
}
}


/****************************************************************************
															OUTCOME DIALOG
****************************************************************************/
EfgOutcomeDialog::EfgOutcomeDialog(Efg &ef,EfgShow *es_):es(es_)
{
int dialog_type;
char *defaults_file="gambit.ini";
wxGetResource("Gambit","EfgOutcome-Dialog-Type",&dialog_type,defaults_file);
if (dialog_type==SHORT_ENTRY_OUTCOMES)
	d=new EfgOutcomeDialogShort(ef,es->Parameters(),es->tw,this);
else
	d=new EfgOutcomeDialogLong(ef,es->Parameters(),es->tw,this);
d->Show(TRUE);
}

EfgOutcomeDialog::~EfgOutcomeDialog()
{d->Show(FALSE);delete d;}

void EfgOutcomeDialog::SetOutcome(const gString &outc_name)
{d->SetCurOutcome(outc_name);d->SetFocus();}

void EfgOutcomeDialog::UpdateVals(void)
{d->UpdateValues();d->Repaint();}

void EfgOutcomeDialog::OnOk(void)
{es->ChangeOutcomes(DESTROY_DIALOG);}


