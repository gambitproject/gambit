//*************************************************************************
//* Treewin.cc: This file contains the type-specific (templated) portion of
//* the extensive form rendering code.
// @(#)treewin.cc	1.18 7/4/95
//
#pragma hdrstop
#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "efg.h"
#include "infoset.h"
#include "node.h"
#include "gmisc.h"
#include "treewin.h"
#include "extshow.h"


//=====================================================================
//                      TREEWINDOW MEMBER FUNCTIONS
//=====================================================================

//---------------------------------------------------------------------
//                TREEWINDOW: CONSTRUCTOR AND DESTRUCTOR
//---------------------------------------------------------------------
//************************* private constructor & assignment *******
template <class T>
TreeWindow<T>::TreeWindow(const TreeWindow<T> &t):BaseTreeWindow(t),ef(t.ef)
{assert(0);}
template <class T>
void TreeWindow<T>::operator=(const TreeWindow<T> &t)
{assert(0);}


//************************* normal constructor ********************
template <class T>
TreeWindow<T>::TreeWindow(Efg<T> &ef_,ExtensiveShow<T> *frame_)
												:	ef(ef_), frame(frame_), BaseTreeWindow(ef_,frame_)
{ }

template <class T>
TreeWindow<T>::~TreeWindow()
{
#ifdef SUBGAMES
if (subgame!=1) ;		// if I am a subgame, notify parent of my death
//	 ((ExtensiveShow<T> *)frame->parent)->tw->CloseSubgame(subgame);
else
#endif
delete &ef;
}

template <class T>
double TreeWindow<T>::ProbAsDouble(const Node *n,int action) const
{
return (double)frame->BranchProb(n,action);
}

template <class T>
gString TreeWindow<T>::OutcomeAsString(const Node *n) const
{
if (n->GetOutcome())
{
	OutcomeVector<T> *tv=(OutcomeVector<T> *)n->GetOutcome();;
	gVector<T> &v=*tv;
	ToStringPrecision(2);
	gString tmp="(";
	for (int i=v.First();i<=v.Last();i++)
	{
		if (i!=1) tmp+=",";
		if (draw_settings.ColorCodedOutcomes())
			tmp+=("\\C{"+ToString(draw_settings.GetPlayerColor(i))+"}");
		tmp+=ToString(v[i]);
	}
	if (draw_settings.ColorCodedOutcomes()) tmp+=("\\C{"+ToString(WX_COLOR_LIST_LENGTH-1)+"}");
	tmp+=")";

	return tmp;
}
else
	return "";
}

//***********************************************************************
//                      TREE-OUTCOME MENU HANDLER
//***********************************************************************
// If the dialog does not exist, create it.  If it exists and save_num==0,
// delete it.  If out_name!="", find the outcome # and set the current row.
// If out_name=="" and save_num>0, update that outcome, if save_num<0, delete
// that outcome.
#include "outcomed.h"
Outcome *EfgGetOutcome(const BaseEfg &ef,const gString &n)
{
for (int i=1;i<=ef.NumOutcomes();i++)
	if (ef.OutcomeList()[i]->GetName()==n) return ef.OutcomeList()[i];
return 0;
}

template<class T> void TreeWindow<T>::tree_outcomes(const gString out_name,int save_num)
{
int i,j,out=1;
static OutcomeDialog *outcome_dialog=0;
static gBlock<gString> old_names;
int num_players=ef.NumPlayers();
if (!outcome_dialog)	// creating a new one
{
	// Check if there are any players to build outcomes for
	if (num_players<1)
		{wxMessageBox("No players exist, can not build an outcome","Error",wxOK | wxCENTRE,frame);return;}
	int rows=(ef.NumOutcomes()) ? ef.NumOutcomes() : 1;
	int cols=num_players;
	// figure out the # of this outcome
	if (out_name!=gString())
		for (i=1;i<=ef.NumOutcomes();i++) if ((ef.OutcomeList()[i])->GetName()==out_name) out=i;
	// create the dialog
	outcome_dialog=new OutcomeDialog(rows,cols,(BaseTreeWindow *)this,(wxFrame *)frame,out);
	if (out>0) outcome_dialog->SetCurRow(out);
	for (i=1;i<=num_players;i++)
		outcome_dialog->SetLabelCol(i,(ef.PlayerList()[i])->GetName());
	old_names=gBlock<gString>(ef.NumOutcomes());
	for (i=1;i<=ef.NumOutcomes();i++)
	{
		OutcomeVector<T> *tmp=(OutcomeVector<T> *)(ef.OutcomeList()[i]);
		for (int j=1;j<=num_players;j++)
			outcome_dialog->SetCell(i,j,ToString((*tmp)[j]));
		outcome_dialog->SetCell(i,cols+1,tmp->GetName());
		old_names[i]=tmp->GetName();
	}
	outcome_dialog->Redraw();
	outcome_dialog->Show(TRUE);
	return;
}
else	// either going to a new one by clicking on an outcome, closing, or saving
{
	if (out_name!="")	// setting a new row
	{
		// figure out the # of this outcome
		for (i=1;i<=ef.NumOutcomes();i++) if ((ef.OutcomeList()[i])->GetName()==out_name) out=i;
		outcome_dialog->SetCurRow(out);return;
	}
	else	// this is an OK, save, or delete action
	{
		if (save_num>0)	// save action
		{
			T payoff;
			OutcomeVector<T> *tmp;
			// if a new row was created, append an entry to old_names.
			if (save_num>old_names.Length())
			{
				gString new_name="Outcome "+ToString(ef.NumOutcomes()+1);
				old_names+=new_name;
				tmp=ef.NewOutcome();
				tmp->SetName(new_name);
			}
			else
				tmp=(OutcomeVector<T> *)EfgGetOutcome(ef,old_names[save_num]);
			assert(tmp);
			// check if the values have changed
			for (j=1;j<=num_players;j++)
			{
				FromString(outcome_dialog->GetCell(save_num,j),payoff);
				if ((*tmp)[j]!=payoff)
					{(*tmp)[j]=payoff;outcomes_changed=TRUE;}
			}
			// check if the name has changed
			if (outcome_dialog->GetCell(save_num,j)!=old_names[save_num])
				if (outcome_dialog->GetCell(save_num,j)!="")
					tmp->SetName(outcome_dialog->GetCell(save_num,j));
				else
				{
					outcome_dialog->SetCell(save_num,num_players+1,old_names[save_num]);
					outcome_dialog->OnPaint();
				}
			if (outcomes_changed) OnPaint();
		}
		if (save_num==0)	// OK action
		{
			delete outcome_dialog;
			outcome_dialog=0;
		}
		if (save_num<0)	// delete action
		{
			int del_num=-save_num;
			if (del_num<=old_names.Length()) // not the last, blank row
			{
				Outcome *tmp=EfgGetOutcome(ef,old_names[del_num]);
				assert(tmp);
				ef.DeleteOutcome(tmp);
				old_names.Remove(del_num);
				outcomes_changed=TRUE;
				outcome_dialog->DelRow(del_num);
				outcome_dialog->Resize();
				OnPaint();
			}
		}
	}
}
}

//***********************************************************************
//                      NODE-PROBS MENU HANDLER
//***********************************************************************
// The text input fields are stacked vertically up to ENTRIES_PER_DIALOG.
// If there are more than ENTRIES_PER_DIALOG actions for this infoset,
// consequtive dialogs will be created.
#define ENTRIES_PER_DIALOG	10

template<class T> void TreeWindow<T>::action_probs(void)
{
Node *n=cursor;
int 	i;

if (!n->GetPlayer()->IsChance())	// if this is not a chance player
{
	wxMessageBox("Probabilities only valid for CHANCE player","Error",wxOK | wxCENTRE,frame);
	return;
}
ToStringPrecision(4);

int num_actions=cursor->NumChildren();
int num_d=num_actions/ENTRIES_PER_DIALOG-((num_actions%ENTRIES_PER_DIALOG) ? 0 : 1);

for (int d=0;d<=num_d;d++)
{
	MyDialogBox *node_probs_dialog=new MyDialogBox(frame,"Node Probabilities");
	int actions_now=gmin(num_actions-d*ENTRIES_PER_DIALOG,ENTRIES_PER_DIALOG);
	char **prob_vector=new char *[actions_now+1];
	for (i=1;i<=actions_now;i++)
	{
		T temp_p=((ChanceInfoset<T> *)(n->GetInfoset()))->GetActionProb(i+d*ENTRIES_PER_DIALOG);
		prob_vector[i]=new char[20];
		strcpy(prob_vector[i],ToString(temp_p));
		node_probs_dialog->Add(wxMakeFormString("",&(prob_vector[i]),wxFORM_TEXT,NULL,NULL,wxVERTICAL,80));
		node_probs_dialog->Add(wxMakeFormNewLine());
	}
	if (num_actions-(d+1)*ENTRIES_PER_DIALOG>0)
		node_probs_dialog->Add(wxMakeFormMessage("Continued..."));

	node_probs_dialog->Go();
	if (node_probs_dialog->Completed()==wxOK)
	{
		T dummy;
		for (i=1;i<=actions_now;i++)
			((ChanceInfoset<T> *)n->GetInfoset())->SetActionProb(i+d*ENTRIES_PER_DIALOG,FromString(prob_vector[i],dummy));
	}
	for (i=1;i<=actions_now;i++) delete [] prob_vector[i];delete [] prob_vector;
	delete node_probs_dialog;
}
}

//***********************************************************************
//                      FILE-SAVE MENU HANDLER
//***********************************************************************
template <class T> void TreeWindow<T>::file_save(void)
{
char *s=wxFileSelector("Save data file", 0, 0, 0, "*.efg");
if (s)
{
	gFileOutput out((const char *)s);
	ef.WriteEfgFile(out);
}
}



#ifdef __GNUG__
	#define TEMPLATE template
#elif defined __BORLANDC__
	#pragma option -Jgd
	#define TEMPLATE
#endif   // __GNUG__, __BORLANDC__
TEMPLATE class TreeWindow<double> ;

#ifdef GRATIONAL
	TEMPLATE class TreeWindow<gRational> ;
#endif

