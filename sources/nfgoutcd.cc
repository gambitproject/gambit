// File: outcomed.cc -- code for the NFG outcome editing dialog
// $Id$
#include "wx.h"
#include "wxmisc.h"
#include "spread.h"
#include "nfg.h"
#include "normshow.h"
#include "nfgoutcd.h"
#include "nfplayer.h"

/****************************************************************************
												BASE OUTCOME DIALOG
****************************************************************************/
#define NFG_OUTCOME_HELP	"Outcomes GUI"

class BaseNFOutcomeDialogC: public SpreadSheet3D
{
private:
	BaseNFOutcomeDialog *parent;
	NFGameForm &nf;
protected:
	BaseNormShow *bns;
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
	BaseNFOutcomeDialogC(int rows,int cols,NFGameForm &nf,BaseNormShow *bns,BaseNFOutcomeDialog *parent);
	void SetCurOutcome(const gString &out_name);
	void OnHelp(int );
	// This implements the behavior that a new row is created automatically
	// below the greatest ENTERED row.  Also, if we move to a new row, the
	// previous row is automatically saved in the nf.
	virtual void OnSelectedMoved(int ,int ,SpreadMoveDir ) { };
	virtual void OnOk(void);
	virtual int  OutcomeNum(int row=0,int col=0) = 0;
	virtual Bool OnClose(void);
};

extern wxCursor *outcome_cursor; // defined in efgoutcd.cc
#define DRAG_NONE				0			// Current drag state
#define DRAG_START			1
#define DRAG_CONTINUE   2
#define DRAG_STOP       3
class BaseNFOutcomeDialogC::OutcomeDragger
{
private:
	BaseNFOutcomeDialogC *parent;
	BaseNormShow *bns;
	int drag_now;
	int outcome;
	int x,y;
public:
	OutcomeDragger(BaseNFOutcomeDialogC *parent,BaseNormShow *bns);
	int OnEvent(wxMouseEvent &ev);
};

BaseNFOutcomeDialogC::OutcomeDragger::OutcomeDragger(BaseNFOutcomeDialogC *parent_,
			BaseNormShow *bns_):parent(parent_),bns(bns_),drag_now(0)
{
if (!outcome_cursor)
{
#ifdef wx_msw
 outcome_cursor=new wxCursor("OUTCOMECUR");
#else
 #include "bitmaps/outcome.xbm"
 outcome_cursor=new wxCursor(outcome_bits,outcome_width,outcome_height,-1,-1,outcome_bits);
#endif
}
}

int BaseNFOutcomeDialogC::OutcomeDragger::OnEvent(wxMouseEvent &ev)
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
	bns->SetOutcome(outcome,xi,yi);
}
return ret;
}


// Constructor
BaseNFOutcomeDialogC::BaseNFOutcomeDialogC(int rows,int cols,NFGameForm &ef_,
																BaseNormShow *bns_,BaseNFOutcomeDialog *parent_)
						:SpreadSheet3D(rows,cols,1,"Outcomes [S]",0/*(wxFrame *)bns_->GetParent()*/,ANY_BUTTON),
						 parent(parent_),nf(ef_),bns(bns_)
{
MakeButtons(OK_BUTTON|PRINT_BUTTON|OPTIONS_BUTTON|HELP_BUTTON);
AddButton("Opt",(wxFunction)settings_func);
AddButtonNewLine();
AddButton("Attach",(wxFunction)outcome_attach_func);
AddButton("Detach",(wxFunction)outcome_detach_func);
AddButton("Delete",(wxFunction)outcome_delete_func);
prev_outc_num=1;
outcome_drag=new OutcomeDragger(this,bns);
}

// Handler functions -> stubs to actual functions
void BaseNFOutcomeDialogC::outcome_attach_func(wxButton &ob,wxEvent &)
{((BaseNFOutcomeDialogC *)ob.GetClientData())->OnAttach();}
void BaseNFOutcomeDialogC::outcome_detach_func(wxButton &ob,wxEvent &)
{((BaseNFOutcomeDialogC *)ob.GetClientData())->OnDetach();}
void BaseNFOutcomeDialogC::outcome_delete_func(wxButton &ob,wxEvent &)
{((BaseNFOutcomeDialogC *)ob.GetClientData())->OnDelete();}
void BaseNFOutcomeDialogC::settings_func(wxButton &ob,wxEvent &)
{((BaseNFOutcomeDialogC *)ob.GetClientData())->OnSettings();}
// OnAttach
void BaseNFOutcomeDialogC::OnAttach(void)
{bns->SetOutcome(OutcomeNum());CanvasFocus();}
// OnDetach
void BaseNFOutcomeDialogC::OnDetach(void)
{bns->SetOutcome(0);CanvasFocus();}
// OnDelete
void BaseNFOutcomeDialogC::OnDelete(void)
{
char tmp_str[256];
int outc_num=OutcomeNum();
gString outc_name=nf.Outcomes()[outc_num]->GetName();
sprintf(tmp_str,"Delete Outcome '%s'?",(const char *)outc_name);
if (wxMessageBox(tmp_str,"Confirm",wxOK|wxCANCEL)==wxOK)
{
	if (outc_num<=nf.NumOutcomes()) // not the last, blank row
	{
		NFOutcome *tmp=nf.Outcomes()[outc_num];;
		assert(tmp);
		nf.DeleteOutcome(tmp);
		bns->SetOutcome(-1);
	}
}
CanvasFocus();
}
// OnSettings
void BaseNFOutcomeDialogC::OnSettings(void)
{
MyDialogBox *options_dialog=new MyDialogBox(this,"Outcome Settings",NFG_OUTCOME_HELP);
wxStringList *opt_list=new wxStringList("Compact Format","Long Entries",0);
char *opt_str=new char[25];
int dialog_type;
char *defaults_file="gambit.ini";
wxGetResource("Gambit","NFOutcome-Dialog-Type",&dialog_type,defaults_file);
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
		wxWriteResource("Gambit","NFOutcome-Dialog-Type",new_dialog_type,defaults_file);
	}
}
delete options_dialog;
}

// OnHelp
void BaseNFOutcomeDialogC::OnHelp(int )
{wxHelpContents(NFG_OUTCOME_HELP);}


// SetCurOutcome
void BaseNFOutcomeDialogC::SetCurOutcome(const gString &out_name)
{
int out=0;
if (out_name!="")
	for (int i=1;i<=nf.NumOutcomes();i++)
		if ((nf.Outcomes()[i])->GetName()==out_name) out=i;
if (out)
{
	int row,col;
	NamePos(out,&row,&col);
	SetCurRow(row);SetCurCol(col);
	OnSelectedMoved(row,col,SpreadMoveJump);  // we could have been editing
}
}
// OnOk -- check if the current outcome has changed
void BaseNFOutcomeDialogC::OnOk(void)
{parent->OnOk();}
// OnClose -- close the window, as if OK was pressed.
Bool BaseNFOutcomeDialogC::OnClose(void)
{OnOk();return FALSE;}

// OnEvent -- check if we are dragging an outcome
Bool BaseNFOutcomeDialogC::OnEventNew(wxMouseEvent &ev)
{
if (outcome_drag->OnEvent(ev)!=DRAG_NONE) return TRUE;
return FALSE;
}



/****************************************************************************
											 COMMON OUTCOME DIALOG
****************************************************************************/

template <class T>
class NFOutcomeDialogC : public BaseNFOutcomeDialogC
{
protected:
	Nfg<T> &nf;
	virtual void UpdateValues(void);
	void CheckOutcome(int outc_num);
	NFOutcomeDialogC(int rows,int cols,Nfg<T> &nf,BaseNormShow *bns,BaseNFOutcomeDialog *parent);
public:
	virtual void OnOk(void);
};

template <class T>
NFOutcomeDialogC<T>::NFOutcomeDialogC(int rows,int cols, Nfg<T> &nf_,BaseNormShow *bns_,
																		BaseNFOutcomeDialog *parent_)
							: BaseNFOutcomeDialogC(rows,cols,nf_.GameForm(),bns_,parent_),nf(nf_)
{}


template <class T>
void NFOutcomeDialogC<T>::UpdateValues(void)
{
int row,col;
NFOutcome *tmp;
for (int i=1;i<=nf.NumOutcomes();i++)
{
	tmp= nf.Outcomes()[i];
	for (int j=1;j<=nf.NumPlayers();j++)
	{
		PayoffPos(i,j,&row,&col);
		SetCell(row,col,ToString(nf.Payoff(tmp, j)));
	}
	NamePos(i,&row,&col);
	SetCell(row,col,tmp->GetName());
}
}

template <class T>
void NFOutcomeDialogC<T>::CheckOutcome(int outc_num)
{
assert(outc_num>0 && outc_num<=nf.NumOutcomes()+1);
bool outcomes_changed=false;
T payoff;
NFOutcome *tmp;
// if a new outcome has created, append it to the list of outcomes
if (outc_num>nf.NumOutcomes())
{
	tmp=nf.GameForm().NewOutcome();
	tmp->SetName("Outcome "+ToString(nf.NumOutcomes()));
}
else
	tmp=nf.Outcomes()[outc_num];
assert(tmp);
// check if the values have changed
int prow,pcol;
for (int j=1;j<=nf.NumPlayers();j++)
{
	PayoffPos(outc_num,j,&prow,&pcol);
	FromString(GetCell(prow,pcol),payoff);
	if (nf.Payoff(tmp, j)!=payoff)	{
		nf.SetPayoff(tmp, j, payoff);
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
		outcomes_changed=true;
	}
	else
	{
		SetCell(prow,pcol,tmp->GetName());
		OnPaint();
	}
if (outcomes_changed) bns->SetOutcome(-1);
}

template <class T>
void NFOutcomeDialogC<T>::OnOk(void)
{
CheckOutcome(OutcomeNum(CurRow(),CurCol()));
BaseNFOutcomeDialogC::OnOk();
}

/****************************************************************************
												SHORT ENTRY OUTCOME DIALOG
****************************************************************************/

template <class T>
class NFOutcomeDialogShort: public NFOutcomeDialogC<T>
{
protected:
	void OnDelete(void);
	int  OutcomeNum(int row=0,int col=0);
	void PayoffPos(int outc_num,int player,int *row,int *col);
	void NamePos(int outc_num,int *row,int *col);
public:
	NFOutcomeDialogShort(Nfg<T> &nf,BaseNormShow *bns,BaseNFOutcomeDialog *parent);
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	virtual void OnOptionsChanged(unsigned int options=0);
};

template <class T>
NFOutcomeDialogShort<T>::NFOutcomeDialogShort(Nfg<T> &nf_,BaseNormShow *bns_,
																				BaseNFOutcomeDialog *parent_)
						: NFOutcomeDialogC<T>((nf_.NumOutcomes()) ? nf_.NumOutcomes() : 1,
														nf_.NumPlayers()+1,nf_,bns_,parent_)
{
DrawSettings()->SetLabels(S_LABEL_ROW|S_LABEL_COL);
DataSettings()->SetChange(S_CAN_GROW_ROW);
DataSettings()->SetAutoLabel(S_AUTO_LABEL_ROW);
DataSettings()->SetAutoLabelStr("Out:%d",S_AUTO_LABEL_ROW);
DrawSettings()->SetColWidth(9,GetCols()); // 'Outcome #'=9 chars
SetLabelCol(GetCols(),"Name");
int i,j;
for (j=1;j<=nf.NumPlayers();j++) DrawSettings()->SetColWidth(2+ToStringPrecision(),j);
// make all the cells string input
for (i=1;i<=GetRows();i++)
	for (j=1;j<=GetCols();j++)
		SetType(i,j,gSpreadStr);

for (i=1;i<=nf.NumPlayers();i++)
	SetLabelCol(i,(nf.Players()[i])->GetName());

UpdateValues();
Redraw();
}

// OnDelete
template <class T>
void NFOutcomeDialogShort<T>::OnDelete(void)
{
BaseNFOutcomeDialogC::OnDelete();
int outc_num=OutcomeNum();
DelRow(outc_num);
Redraw();
}

// This implements the behavior that a new row is created automatically
// below the greatest ENTERED row.  Also, if we move to a new row, the
// previous row is automatically saved in the nf.
template <class T>
void NFOutcomeDialogShort<T>::OnSelectedMoved(int row,int col,SpreadMoveDir )
{
if (OutcomeNum(row,col)!=prev_outc_num)
	{CheckOutcome(prev_outc_num);prev_outc_num=OutcomeNum(row,col);}
if (row==GetRows() && EnteredCell(row,1))
	{AddRow();Redraw();OnPaint();}
}

// Functions that determine the window layout
template <class T>
int NFOutcomeDialogShort<T>::OutcomeNum(int row,int )
{
if (row==0) row=CurRow();
return row;
}

template <class T>
void NFOutcomeDialogShort<T>::PayoffPos(int outc_num,int player,int *row,int *col)
{
*row=outc_num;*col=player;
}

template <class T>
void NFOutcomeDialogShort<T>::NamePos(int outc_num,int *row,int *col)
{
*row=outc_num;*col=nf.NumPlayers()+1;
}

template <class T>
void NFOutcomeDialogShort<T>::OnOptionsChanged(unsigned int options)
{
if (options&S_PREC_CHANGED)
{
	UpdateValues();
	for (int j=1;j<=nf.NumPlayers();j++)
	DrawSettings()->SetColWidth(2+ToStringPrecision(),j);
	Resize();Repaint();
}
}

/****************************************************************************
												LONG ENTRY OUTCOME DIALOG
****************************************************************************/

template <class T>
class NFOutcomeDialogLong: public NFOutcomeDialogC<T>
{
protected:
	void OnDelete(void);
	int  OutcomeNum(int row=0,int col=0);
	void PayoffPos(int outc_num,int player,int *row,int *col);
	void NamePos(int outc_num,int *row,int *col);
public:
	NFOutcomeDialogLong(Nfg<T> &nf,BaseNormShow *bns,BaseNFOutcomeDialog *parent);
	void OnSelectedMoved(int row,int col,SpreadMoveDir how);
	virtual void OnOptionsChanged(unsigned int options=0);
};

template <class T>
NFOutcomeDialogLong<T>::NFOutcomeDialogLong(Nfg<T> &ef_,BaseNormShow *bns_,
																				BaseNFOutcomeDialog *parent_)
				:NFOutcomeDialogC<T>((ef_.NumOutcomes() ? ef_.NumOutcomes() : 1)*ef_.NumPlayers(),
												3,ef_,bns_,parent_)
{
DrawSettings()->SetLabels(S_LABEL_ROW|S_LABEL_COL);
DataSettings()->SetChange(S_CAN_GROW_ROW);
DrawSettings()->SetColWidth(9,1);	// "Player #"=9 chars : player column
DrawSettings()->SetColWidth(ToStringPrecision()+2,2);		// values column
DrawSettings()->SetColWidth(9,3); // "Outcome #"=9 chars : outcome name column
SetLabelCol(1,"Player");
SetLabelCol(2,"Payoff");
SetLabelCol(3,"Name");
if (nf.NumOutcomes()==0) nf.GameForm().NewOutcome();
int i,j;
for (i=1;i<=GetRows();i++)	SetType(i,2,gSpreadStr);
for (j=1;j<=nf.NumOutcomes();j++)				// set player and outcome names
{
	for (i=1;i<=nf.NumPlayers();i++)
	{
		SetCell((j-1)*nf.NumPlayers()+i,1,(nf.Players()[i])->GetName());
		Bold((j-1)*nf.NumPlayers()+i,1,0,TRUE);
	}
	SetCell((j-1)*nf.NumPlayers()+1,3,nf.Outcomes()[j]->GetName());
	SetLabelRow((j-1)*nf.NumPlayers()+1,"Out:"+ToString(j));
	SetType((j-1)*nf.NumPlayers()+1,3,gSpreadStr);
}
SetCurRow(1);SetCurCol(2);
UpdateValues();
Redraw();
}

// OnDelete
template <class T>
void NFOutcomeDialogLong<T>::OnDelete(void)
{
BaseNFOutcomeDialogC::OnDelete();
int outc_num=OutcomeNum();
int i;
for (i=1;i<=nf.NumPlayers();i++)
	DelRow(nf.NumPlayers()*outc_num-i+1);
for (i=outc_num+1;i<=nf.NumOutcomes();i++)
	SetLabelRow((i-1)*nf.NumPlayers()+1,"Out:"+ToString(i));

Redraw();
}

// This implements the behavior that a new row is created automatically
// below the greatest ENTERED row.  Also, if we move to a new row, the
// previous row is automatically saved in the nf. Also prevents the user
// from going to non-modifiable cells (player name column,some outcome name cells)
template <class T>
void NFOutcomeDialogLong<T>::OnSelectedMoved(int row,int col,SpreadMoveDir how)
{
if (col==1) {SetCurCol(2);return;}	// player name column is off limits
int outc_num=OutcomeNum(row,col);
if (col==3)                       	// outcome name column is special
{
	if (how==SpreadMoveRight || how==SpreadMoveJump)
		if (row!=nf.NumPlayers()*(outc_num-1)+1)
			{row=nf.NumPlayers()*(outc_num-1)+1;SetCurRow(row);}
	if (how==SpreadMoveUp && outc_num>1)	// jump to a previous outcome
	{
		int prow,pcol;
		int n_outc_num=OutcomeNum(row+1,col);
		NamePos(n_outc_num-1,&prow,&pcol);
		SetCurRow(prow);
	}
	if (how==SpreadMoveDown) // jump to next outcome
	{
		if (row>GetRows()-nf.NumPlayers())
			SetCurRow(GetRows()-nf.NumPlayers()+1);
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
	for (int i=1;i<=nf.NumPlayers();i++)
	{
		AddRow();
		SetCell(GetRows(),1,nf.Players()[i]->GetName());
		Bold(GetRows(),1,0,TRUE);
	}
	SetLabelRow(GetRows()-nf.NumPlayers()+1,"Out:"+ToString(nf.NumOutcomes()+1));
	Redraw();OnPaint();
}
}

// Functions that determine the window layout
template <class T>
int NFOutcomeDialogLong<T>::OutcomeNum(int row,int )
{
if (row==0) row=CurRow();
return (row-1)/nf.NumPlayers()+1;
}

template <class T>
void NFOutcomeDialogLong<T>::PayoffPos(int outc_num,int player,int *row,int *col)
{
*row=(outc_num-1)*nf.NumPlayers()+player;*col=2;
}

template <class T>
void NFOutcomeDialogLong<T>::NamePos(int outc_num,int *row,int *col)
{
*row=(outc_num-1)*nf.NumPlayers()+1;*col=3;
}

template <class T>
void NFOutcomeDialogLong<T>::OnOptionsChanged(unsigned int options)
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
BaseNFOutcomeDialog::BaseNFOutcomeDialog(BaseNormShow *bns_):bns(bns_)
{ }
BaseNFOutcomeDialog::~BaseNFOutcomeDialog()
{d->Show(FALSE);delete d;}

void BaseNFOutcomeDialog::SetOutcome(const gString &outc_name)
{d->SetCurOutcome(outc_name);d->SetFocus();}

void BaseNFOutcomeDialog::OnOk(void)
{bns->OutcomeDialogDied();}

template <class T>
NFOutcomeDialog<T>::NFOutcomeDialog(Nfg<T> &nf,BaseNormShow *bns):BaseNFOutcomeDialog(bns)
{
int dialog_type;
char *defaults_file="gambit.ini";
wxGetResource("Gambit","NFOutcome-Dialog-Type",&dialog_type,defaults_file);
if (dialog_type==SHORT_ENTRY_OUTCOMES)
	d=new NFOutcomeDialogShort<T>(nf,bns,this);
else
	d=new NFOutcomeDialogLong<T>(nf,bns,this);
d->Show(TRUE);
}


template class NFOutcomeDialogC<double>;
template class NFOutcomeDialogShort<double>;
template class NFOutcomeDialogLong<double>;
template class NFOutcomeDialog<double>;
#include "rational.h"
template class NFOutcomeDialogC<gRational>;
template class NFOutcomeDialogShort<gRational>;
template class NFOutcomeDialogLong<gRational>;
template class NFOutcomeDialog<gRational>;
