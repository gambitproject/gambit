// File: outcomed.cc -- code for the EFG outcome editing dialog
// $Id$
#include "wx.h"
#include "wxmisc.h"
#include "spread.h"
#include "efg.h"
#include "treewin.h"
#include "efgoutcd.h"

/****************************************************************************
												BASE OUTCOME DIALOG
****************************************************************************/
class BaseOutcomeDialogC: public SpreadSheet3D
{
private:
	BaseOutcomeDialog *parent;
	BaseEfg	&ef;
protected:
	BaseTreeWindow *tw;
	int prev_outc_num;
	class OutcomeDragger;
	OutcomeDragger *outcome_drag;
	static void outcome_attach_func(wxButton &ob,wxEvent &);
	static void outcome_detach_func(wxButton &ob,wxEvent &);
	static void outcome_delete_func(wxButton &ob,wxEvent &);
	static void settings_func(wxButton &ob,wxEvent &);
	void OnAttach(void);
	void OnDetach(void);
	virtual void OnDelete(void);
	void OnSettings(void);
	virtual void UpdateValues(void) = 0;
	virtual void PayoffPos(int outc_num,int player,int *row,int *col) = 0;
	virtual void NamePos(int outc_num,int *row,int *col) = 0;
	virtual Bool OnEventNew(wxMouseEvent &ev);
public:
	BaseOutcomeDialogC(int rows,int cols,BaseEfg &ef,BaseTreeWindow *tw,BaseOutcomeDialog *parent);
	void SetCurOutcome(const gString &out_name);
	void OnHelp(int );
	// This implements the behavior that a new row is created automatically
	// below the greatest ENTERED row.  Also, if we move to a new row, the
	// previous row is automatically saved in the ef.
	virtual void OnSelectedMoved(int ,int ,SpreadMoveDir ) { };
	virtual void OnOk(void);
	virtual int  OutcomeNum(int row=0,int col=0) = 0;
	virtual Bool OnClose(void);
};

wxCursor *outcome_cursor;
#define DRAG_NONE				0			// Current drag state
#define DRAG_START			1
#define DRAG_CONTINUE   2
#define DRAG_STOP       3
class BaseOutcomeDialogC::OutcomeDragger
{
private:
	BaseOutcomeDialogC *parent;
	BaseTreeWindow *tw;
	int drag_now;
	int outcome;
	int x,y;
public:
	OutcomeDragger(BaseOutcomeDialogC *parent_,BaseTreeWindow *tw_);
	int OnEvent(wxMouseEvent &ev);
};

BaseOutcomeDialogC::OutcomeDragger::OutcomeDragger(BaseOutcomeDialogC *parent_,
			BaseTreeWindow *tw_):parent(parent_),tw(tw_),drag_now(0)
{
#ifdef wx_msw
 outcome_cursor=new wxCursor("OUTCOMECUR");
#else
 #include "bitmaps/outcome.xbm"
 outcome_cursor=new wxCursor(outcome_bits,outcome_width,outcome_height,-1,-1,outcome_bits);
#endif
}

int BaseOutcomeDialogC::OutcomeDragger::OnEvent(wxMouseEvent &ev)
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
BaseOutcomeDialogC::BaseOutcomeDialogC(int rows,int cols,BaseEfg &ef_,
																BaseTreeWindow *tw_,BaseOutcomeDialog *parent_)
						:SpreadSheet3D(rows,cols,1,"Outcomes [S]",(wxFrame *)tw_->GetParent(),ANY_BUTTON),parent(parent_),ef(ef_),
						 tw(tw_)
{
MakeButtons(OK_BUTTON|PRINT_BUTTON|OPTIONS_BUTTON|HELP_BUTTON);
AddButton("Opt",(wxFunction)settings_func);
AddButtonNewLine();
AddButton("Attach",(wxFunction)outcome_attach_func);
AddButton("Detach",(wxFunction)outcome_detach_func);
AddButton("Delete",(wxFunction)outcome_delete_func);
prev_outc_num=1;
outcome_drag=new OutcomeDragger(this,tw);
}

// Handler functions -> stubs to actual functions
void BaseOutcomeDialogC::outcome_attach_func(wxButton &ob,wxEvent &)
{((BaseOutcomeDialogC *)ob.GetClientData())->OnAttach();}
void BaseOutcomeDialogC::outcome_detach_func(wxButton &ob,wxEvent &)
{((BaseOutcomeDialogC *)ob.GetClientData())->OnDetach();}
void BaseOutcomeDialogC::outcome_delete_func(wxButton &ob,wxEvent &)
{((BaseOutcomeDialogC *)ob.GetClientData())->OnDelete();}
void BaseOutcomeDialogC::settings_func(wxButton &ob,wxEvent &)
{((BaseOutcomeDialogC *)ob.GetClientData())->OnSettings();}
// OnAttach
void BaseOutcomeDialogC::OnAttach(void)
{tw->node_outcome(OutcomeNum());CanvasFocus();}
// OnDetach
void BaseOutcomeDialogC::OnDetach(void)
{tw->node_outcome(0);CanvasFocus();}
// OnDelete
void BaseOutcomeDialogC::OnDelete(void)
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
void BaseOutcomeDialogC::OnSettings(void)
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
		wxWriteResource("Gambit","EFOutcome-Dialog-Type",new_dialog_type,defaults_file);
	}
}
delete options_dialog;
}

// OnHelp
void BaseOutcomeDialogC::OnHelp(int )
{wxHelpContents(EFG_OUTCOME_HELP);}

// SetCurOutcome
void BaseOutcomeDialogC::SetCurOutcome(const gString &out_name)
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
void BaseOutcomeDialogC::OnOk(void)
{parent->OnOk();}
// OnClose -- close the window, as if OK was pressed.
Bool BaseOutcomeDialogC::OnClose(void)
{OnOk();return FALSE;}

// OnEvent -- check if we are dragging an outcome
Bool BaseOutcomeDialogC::OnEventNew(wxMouseEvent &ev)
{
if (outcome_drag->OnEvent(ev)!=DRAG_NONE) return TRUE;
return FALSE;
}


/****************************************************************************
											 COMMON OUTCOME DIALOG
****************************************************************************/

template <class T>
class OutcomeDialogC : public BaseOutcomeDialogC
{
protected:
	Efg<T> &ef;
	virtual void UpdateValues(void);
	void CheckOutcome(int outc_num);
	OutcomeDialogC(int rows,int cols,Efg<T> &ef,BaseTreeWindow *tw,BaseOutcomeDialog *parent);
public:
	virtual void OnOk(void);
};

template <class T>
OutcomeDialogC<T>::OutcomeDialogC(int rows,int cols, Efg<T> &ef_,BaseTreeWindow *tw_,
																		BaseOutcomeDialog *parent_)
							: BaseOutcomeDialogC(rows,cols,ef_,tw_,parent_),ef(ef_)
{}


template <class T>
void OutcomeDialogC<T>::UpdateValues(void)
{
int row,col;
EFOutcome *tmp;
for (int i=1;i<=ef.NumOutcomes();i++)
{
	tmp= ef.Outcomes()[i];
	for (int j=1;j<=ef.NumPlayers();j++)
	{
		PayoffPos(i,j,&row,&col);
		SetCell(row,col,ToString(ef.Payoff(tmp, j)));
	}
	NamePos(i,&row,&col);
	SetCell(row,col,tmp->GetName());
}
}

template <class T>
void OutcomeDialogC<T>::CheckOutcome(int outc_num)
{
assert(outc_num>0 && outc_num<=ef.NumOutcomes()+1);
bool outcomes_changed=false;
T payoff;
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
	FromString(GetCell(prow,pcol),payoff);
	if (ef.Payoff(tmp, j)!=payoff)	{
		ef.SetPayoff(tmp, j, payoff);
		outcomes_changed=true;
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

template <class T>
void OutcomeDialogC<T>::OnOk(void)
{
CheckOutcome(OutcomeNum(CurRow(),CurCol()));
BaseOutcomeDialogC::OnOk();
}

/****************************************************************************
												SHORT ENTRY OUTCOME DIALOG
****************************************************************************/

template <class T>
class OutcomeDialogShort: public OutcomeDialogC<T>
{
protected:
	void OnDelete(void);
	int  OutcomeNum(int row=0,int col=0);
	void PayoffPos(int outc_num,int player,int *row,int *col);
	void NamePos(int outc_num,int *row,int *col);
public:
	OutcomeDialogShort(Efg<T> &ef,BaseTreeWindow *tw,BaseOutcomeDialog *parent);
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	virtual void OnOptionsChanged(unsigned int options=0);
};

template <class T>
OutcomeDialogShort<T>::OutcomeDialogShort(Efg<T> &ef_,BaseTreeWindow *tw_,
																				BaseOutcomeDialog *parent_)
						: OutcomeDialogC<T>((ef_.NumOutcomes()) ? ef_.NumOutcomes() : 1,
														ef_.NumPlayers()+1,ef_,tw_,parent_)
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
template <class T>
void OutcomeDialogShort<T>::OnDelete(void)
{
BaseOutcomeDialogC::OnDelete();
int outc_num=OutcomeNum();
DelRow(outc_num);
Redraw();
}

// This implements the behavior that a new row is created automatically
// below the greatest ENTERED row.  Also, if we move to a new row, the
// previous row is automatically saved in the ef.
template <class T>
void OutcomeDialogShort<T>::OnSelectedMoved(int row,int col,SpreadMoveDir )
{
if (OutcomeNum(row,col)!=prev_outc_num)
	{CheckOutcome(prev_outc_num);prev_outc_num=OutcomeNum(row,col);}
if (row==GetRows() && EnteredCell(row,1))
	{AddRow();Redraw();OnPaint();}
}

// Functions that determine the window layout
template <class T>
int OutcomeDialogShort<T>::OutcomeNum(int row,int )
{
if (row==0) row=CurRow();
return row;
}

template <class T>
void OutcomeDialogShort<T>::PayoffPos(int outc_num,int player,int *row,int *col)
{
*row=outc_num;*col=player;
}

template <class T>
void OutcomeDialogShort<T>::NamePos(int outc_num,int *row,int *col)
{
*row=outc_num;*col=ef.NumPlayers()+1;
}

template <class T>
void OutcomeDialogShort<T>::OnOptionsChanged(unsigned int options)
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

template <class T>
class OutcomeDialogLong: public OutcomeDialogC<T>
{
protected:
	void OnDelete(void);
	int  OutcomeNum(int row=0,int col=0);
	void PayoffPos(int outc_num,int player,int *row,int *col);
	void NamePos(int outc_num,int *row,int *col);
public:
	OutcomeDialogLong(Efg<T> &ef,BaseTreeWindow *tw,BaseOutcomeDialog *parent);
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	virtual void OnOptionsChanged(unsigned int options=0);
};

template <class T>
OutcomeDialogLong<T>::OutcomeDialogLong(Efg<T> &ef_,BaseTreeWindow *tw_,
																				BaseOutcomeDialog *parent_)
				:OutcomeDialogC<T>((ef_.NumOutcomes() ? ef_.NumOutcomes() : 1)*ef_.NumPlayers(),
												3,ef_,tw_,parent_)
{
DrawSettings()->SetLabels(S_LABEL_ROW|S_LABEL_COL);
DataSettings()->SetChange(S_CAN_GROW_ROW);
DrawSettings()->SetColWidth(9,1);	// "Player #"=9 chars : player column
DrawSettings()->SetColWidth(ToStringPrecision()+2,2);		// values column
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
template <class T>
void OutcomeDialogLong<T>::OnDelete(void)
{
BaseOutcomeDialogC::OnDelete();
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
template <class T>
void OutcomeDialogLong<T>::OnSelectedMoved(int row,int col,SpreadMoveDir how)
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
template <class T>
int OutcomeDialogLong<T>::OutcomeNum(int row,int )
{
if (row==0) row=CurRow();
return (row-1)/ef.NumPlayers()+1;
}

template <class T>
void OutcomeDialogLong<T>::PayoffPos(int outc_num,int player,int *row,int *col)
{
*row=(outc_num-1)*ef.NumPlayers()+player;*col=2;
}

template <class T>
void OutcomeDialogLong<T>::NamePos(int outc_num,int *row,int *col)
{
*row=(outc_num-1)*ef.NumPlayers()+1;*col=3;
}

template <class T>
void OutcomeDialogLong<T>::OnOptionsChanged(unsigned int options)
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
BaseOutcomeDialog::BaseOutcomeDialog(BaseTreeWindow *tw_):tw(tw_)
{ }
BaseOutcomeDialog::~BaseOutcomeDialog()
{d->Show(FALSE);delete d;}

void BaseOutcomeDialog::SetOutcome(const gString &outc_name)
{d->SetCurOutcome(outc_name);d->SetFocus();}

void BaseOutcomeDialog::OnOk(void)
{tw->OutcomeDialogDied();}

template <class T>
OutcomeDialog<T>::OutcomeDialog(Efg<T> &ef,BaseTreeWindow *tw):BaseOutcomeDialog(tw)
{
int dialog_type;
char *defaults_file="gambit.ini";
wxGetResource("Gambit","Outcome-Dialog-Type",&dialog_type,defaults_file);
if (dialog_type==SHORT_ENTRY_OUTCOMES)
	d=new OutcomeDialogShort<T>(ef,tw,this);
else
	d=new OutcomeDialogLong<T>(ef,tw,this);
d->Show(TRUE);
}


template class OutcomeDialogC<double>;
template class OutcomeDialogShort<double>;
template class OutcomeDialogLong<double>;
template class OutcomeDialog<double>;
#include "rational.h"
template class OutcomeDialogC<gRational>;
template class OutcomeDialogShort<gRational>;
template class OutcomeDialogLong<gRational>;
template class OutcomeDialog<gRational>;

