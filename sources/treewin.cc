//*************************************************************************
//* Treewin.cc: This file contains the type-specific (templated) portion of
//* the extensive form rendering code.
// $Id$
//
#pragma hdrstop
#include "wx.h"
#include "wx_form.h"
#include "wxmisc.h"
#include "extform.h"
#include "infoset.h"
#include "node.h"
#include "gconvert.h"
#include "gmisc.h"
#include "treewin.h"
#include "extshow.h"


template <class T>
char *OutcomeToString(const gVector<T> &v,TreeDrawSettings &draw_settings,Bool color_coded=TRUE)
{
char tempstr[20];
static gString gvts;
gvts="";
for (int i=v.First();i<=v.Last();i++)
{
	gvts+=(i==1) ? '(' : ',';
	if (color_coded)
	{
		sprintf(tempstr,"\\C{%d}",draw_settings.GetPlayerColor(i));
		gvts+=tempstr;
	}
	sprintf(tempstr,"%2.2f",(double)v[i]);
	gvts+=tempstr;
}
gvts+=')';

return (char *)gvts;
}



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
TreeWindow<T>::TreeWindow(ExtForm<T> &ef_,ExtensiveShow<T> *frame_,
												int _subgame,int x,int y,int w,int h,int style)
												:	ef(ef_), frame(frame_), BaseTreeWindow(ef_,frame_,x,y,w,h,style)
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
double TreeWindow<T>::ProbAsDouble(const Node *n,int action)
{
return (double)frame->GetActionProb(n,action);
}

template <class T>
gString TreeWindow<T>::ProbAsString(const Node *n,int action)
{
return ToString(frame->GetActionProb(n,action));
}

template <class T>
gString TreeWindow<T>::OutcomeAsString(const Node *n)
{
if (n->GetOutcome())
  {
    Outcome *t=n->GetOutcome();
    OutcomeVector<T> *tv=(OutcomeVector<T> *)t;
    gVector<T> *ttv=(gVector<T> *)tv;
    return OutcomeToString(*ttv,draw_settings);
  }
return "";
}


//***********************************************************************
//                       NODE-OUTCOME MENU HANDLER
//***********************************************************************
template<class T> void TreeWindow<T>::node_outcome(const gString out_name)
{
Outcome *out;
if ((out=ef.GetOutcome(out_name))==0)
	{wxMessageBox("This outcome is not defined yet"); return;}
cursor->SetOutcome(out);
}
//***********************************************************************
//                      TREE-OUTCOME MENU HANDLER
//***********************************************************************
#include "outcomed.h"
template<class T> void TreeWindow<T>::tree_outcomes(const gString out_name)
{
int i,j,out=1;
static OutcomeDialog *outcome_dialog=0;
static gArray<gString> old_names;
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
		for (i=1;i<=num_players;i++) if ((ef.OutcomeList()[i])->GetName()==out_name) out=i;
	// create the dialog

	outcome_dialog=new OutcomeDialog(rows,cols,(BaseTreeWindow *)this,(wxFrame *)frame,out);
	for (i=1;i<=num_players;i++)
		outcome_dialog->SetLabelCol(i,(ef.PlayerList()[i])->GetName());
	outcome_dialog->SetType(1,cols+1,gSpreadStr);
	old_names=gArray<gString>(ef.NumOutcomes());
	for (i=1;i<=ef.NumOutcomes();i++)
	{
		OutcomeVector<T> *tmp=(OutcomeVector<T> *)(ef.OutcomeList()[i]);
		for (int j=1;j<=num_players;j++)
			outcome_dialog->SetCell(i,j,ToString((*tmp)[j]));
		outcome_dialog->SetType(i,cols+1,gSpreadStr);
		outcome_dialog->SetCell(i,cols+1,tmp->GetName());
		old_names[i]=tmp->GetName();
	}
	outcome_dialog->Redraw();
	outcome_dialog->Show(TRUE);
	return;
}
else	// either going go a new one by clicking on an outcome or closing it
{
	if (out_name!="")	// setting a new row
	{
		// figure out the # of this outcome
		for (i=1;i<=ef.NumOutcomes();i++) if ((ef.OutcomeList()[i])->GetName()==out_name) out=i;
		outcome_dialog->SetCurRow(out);return;
	}
	else	// this is an OK/Cancel action
	{
		if (outcome_dialog->Completed()==wxOK)
		{
			for (i=1;i<=outcome_dialog->GetRows()-1;i++)
			{
				Outcome *tmp;
				if (ef.NumOutcomes()<i)
				{
					tmp=ef.NewOutcome();
					tmp->SetName(ToString(i));
				}
				else
					tmp=ef.GetOutcome(old_names[i]);

				for (j=1;j<=num_players;j++)
					(*(OutcomeVector<T> *)tmp)[j]=gS2N(outcome_dialog->GetCell(i,j));
				if (outcome_dialog->EnteredCell(i,j) && outcome_dialog->GetCell(i,j)!="")
					tmp->SetName(outcome_dialog->GetCell(i,j));
			}
		}
		delete outcome_dialog;
		outcome_dialog=0;
	}
}
}

//***********************************************************************
//                      NODE-PROBS MENU HANDLER
//***********************************************************************
template<class T> void TreeWindow<T>::node_probs(void)
{
int 	i;

Node *n=cursor;
if (!n->GetPlayer()->IsChance())	// if this is not a chance player
{
	wxMessageBox("Probabilities only valid for CHANCE player","Error",wxOK | wxCENTRE,frame);
	return;
}
gArray<float> prob_vector(n->NumChildren());
MyDialogBox *node_probs_dialog=new MyDialogBox(frame,"Node Probabilities");
for (i=1;i<=n->NumChildren();i++)
{
	T temp_p=((ChanceInfoset<T> *)(n->GetInfoset()))->GetActionProb(i);
	prob_vector[i]=(float)temp_p;
	node_probs_dialog->Form()->Add(wxMakeFormFloat(NULL,&(prob_vector[i]),wxFORM_TEXT,NULL,NULL,wxVERTICAL,80));
}
node_probs_dialog->Go();
if (node_probs_dialog->Completed()==wxOK)
{
	for (i=1;i<=n->NumChildren();i++)
		((ChanceInfoset<T> *)n->GetInfoset())->SetActionProb(i,(T)prob_vector[i]);

}
delete node_probs_dialog;
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
TEMPLATE char *OutcomeToString(const gVector<double> &v,TreeDrawSettings &draw_settings,Bool color_coded=TRUE);

#ifdef GRATIONAL
	TEMPLATE class TreeWindow<gRational> ;
	TEMPLATE char *OutcomeToString(const gVector<gRational> &v,TreeDrawSettings &draw_settings,Bool color_coded=TRUE);
#endif

