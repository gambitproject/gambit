//
// FILE: treewin1.cc: Companion file to treewin.cc.  This file includes all the
//                    functions for dealing with various menu choices in
//                    treewin.
//
// $Id$
//

#include "wx.h"
#include "wx_mf.h"
#include "wx_clipb.h"
#include "wxmisc.h"
#include "efg.h"
#include "treewin.h"
#include "nodeaddd.h"
#include "infosetd.h"

#define MAX_LABEL_LENGTH    25

// Validation function for string dialogs

Bool StringConstraint(int type, char *value, char *label, char *msg_buffer)
{
    if (value && (strlen(value) >= MAX_LABEL_LENGTH) && (type == wxFORM_STRING))
    {
        sprintf(msg_buffer, "Value for %s should be %d characters or less\n",
                label, MAX_LABEL_LENGTH-1);
        return FALSE;
    }
    else
        return TRUE;
}


// This function used to be in the ef and is used frequently
EFPlayer *EfgGetPlayer(const Efg &ef, const gText &name)
{
    for (int i = 1; i <= ef.NumPlayers(); i++)
    {
        if (ef.Players()[i]->GetName() == name) 
            return ef.Players()[i];
    }

    if (ef.GetChance()->GetName() == name) 
        return ef.GetChance();

    return 0;
}

//-----------------------------------------------------------------------
//                    EDIT MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

void TreeWindow::edit_copy_infoset(void)
{
    copied_infoset = cursor->GetInfoset();
    copied_outcome = 0;
    copied_subtree = 0;
}

void TreeWindow::edit_copy_outcome(void)
{
    copied_outcome = cursor->GetOutcome();
    copied_infoset = 0;
    copied_subtree = 0;
}

void TreeWindow::edit_copy_subtree(void)
{
    copied_subtree = cursor;
    copied_infoset = 0;
    copied_outcome = 0;
}

void TreeWindow::edit_paste(void)
{
    if (copied_infoset && cursor->NumChildren() == 0)
    {
        ef.AppendNode(cursor, copied_infoset);
        nodes_changed = TRUE;
    }
    else if (copied_subtree && cursor->NumChildren() == 0)
    {
        ef.CopyTree(copied_subtree, cursor);
        nodes_changed = TRUE;
    }
    else if (copied_outcome)
        cursor->SetOutcome(copied_outcome);
}

//-----------------------------------------------------------------------
//                    NODE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                        NODE-ADD MENU HANDLER
//***********************************************************************
void TreeWindow::node_add(void)
{
    static int branches = 2; // make this static so it remembers the last entry
    static EFPlayer *player = 0;
    static Infoset *infoset = 0;
    static Efg *last_ef = 0; // need this to make sure player,infoset are valid
    
    if (last_ef != &ef)
    {
        player = 0;
        infoset = 0;
        last_ef = &ef;
    }
    
    NodeAddDialog node_add_dialog(ef, player, infoset, branches, pframe);

    if (node_add_dialog.Completed() == wxOK)
    {
        nodes_changed = TRUE;
        NodeAddMode mode = node_add_dialog.GetAddMode();
        player = node_add_dialog.GetPlayer();
        infoset = node_add_dialog.GetInfoset();
        branches = node_add_dialog.GetBranches();
        Bool set_names = FALSE;

        if (cursor->NumChildren() == 0)
        {
#ifdef USE_EXCEPTIONS
            try
            {
#endif   // USE_EXCEPTIONS
                if (mode == NodeAddNew)
                {
                    ef.AppendNode(cursor, player, branches);
                    set_names = node_add_dialog.SetNames();
                }
                else
                    ef.AppendNode(cursor, infoset);
#ifdef USE_EXCEPTIONS
            }
            catch (Efg::Exception &e)
            {
                // internal error in Efg -- for now, ignore silently
            }
#endif   // USE_EXCEPTIONS
        }
        else
        {
#ifdef USE_EXCEPTIONS
            try
            {
#endif   // USE_EXCEPTIONS
                if (mode == NodeAddNew)
                {
                    ef.InsertNode(cursor, player, branches);
                    set_names = node_add_dialog.SetNames();
                    cursor = cursor->GetParent(); // old cursor is now the child
                }
                else
                    ef.InsertNode(cursor, infoset);
#ifdef USE_EXCEPTIONS
            }
            catch (Efg::Exception &e)
            {
                // internal error in Efg -- for now, ignore silently
            }
#endif   // USE_EXCEPTIONS
        }
        
        // take care of probs for chance nodes.
        if (set_names)
        { 
            node_label();
            infoset_label();
        }

        if (player == ef.GetChance())
            action_probs();
    }
}

//***********************************************************************
//                      NODE-DELETE MENU HANDLER
//***********************************************************************

void TreeWindow::node_delete(void)
{
    if (cursor == ef.RootNode() && cursor->NumChildren() == 0)
    {
        wxMessageBox("Cannot delete root node!", "Error", wxOK | wxCENTRE, pframe);
        return;
    }
    
    // Check for terminal nodes -- just take no action (why bother with an error?)
    if (cursor->NumChildren() == 0)  
        return;
    
    MyDialogBox *branch_num_dialog = 0;
    char *branch_name = 0;
    
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        branch_num_dialog = new MyDialogBox(this, "Keep Branch", EFG_NODE_HELP);
        wxStringList *branch_list = new wxStringList;
        branch_name = new char[MAX_LABEL_LENGTH];

        for (int i = 1; i <= cursor->NumChildren(); i++)
        {
            gText tmp = cursor->GetChild(i)->GetName();
            if (tmp == "") tmp = ToText(i);
            branch_list->Add(tmp);
        }
        
        branch_num_dialog->Form()->Add(
            wxMakeFormString("Branch", 
                             &branch_name, 
                             wxFORM_CHOICE,
                             new wxList(wxMakeConstraintStrings(branch_list), 0)));
        branch_num_dialog->Go();

        if (branch_num_dialog->Completed() == wxOK)
        {
            int keep_num = wxListFindString(branch_list, branch_name) + 1;
            Node *keep = cursor->GetChild(keep_num);
            nodes_changed = TRUE;
            cursor = ef.DeleteNode(cursor, keep);
        }
        
        delete [] branch_name;
        delete branch_num_dialog;
#ifdef USE_EXCEPTIONS
    }
    catch (...)
    {
        if (branch_name)   
            delete [] branch_name;

        if (branch_num_dialog)   
            delete branch_num_dialog;

        throw;
    }
#endif   // USE_EXCEPTIONS
}

//***********************************************************************
//                       NODE-LABEL MENU HANDLER
//***********************************************************************

void TreeWindow::node_label(void)
{
    char *label = 0;
    MyDialogBox *label_dialog = 0;
    
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        label = new char[MAX_LABEL_LENGTH];
        strcpy(label, cursor->GetName());
        label_dialog = new MyDialogBox(pframe, "Label Node", EFG_NODE_HELP);
        wxFormItem *label_item = 
            wxMakeFormString("Label", &label, wxFORM_DEFAULT,
                             new wxList(wxMakeConstraintFunction(StringConstraint), 0));
        label_dialog->Add(label_item);
        label_dialog->AssociatePanel();
        ((wxText *) label_item->GetPanelItem())->SetFocus();
        label_dialog->Go1();
        if (label_dialog->Completed() == wxOK)
            cursor->SetName(label);
        delete label_dialog;
        delete [] label;
#ifdef USE_EXCEPTIONS
    }
    catch (...)
    {
        if (label_dialog)   
            delete label_dialog;
        if (label)   
            delete [] label;
    }
#endif   // USE_EXCEPTIONS
}

//***********************************************************************
//                       NODE-OUTCOME MENU HANDLER
//***********************************************************************

#define DRAG_OUTCOME_END      7

void TreeWindow::node_outcome(int out, int x, int y)
{
    if (out > ef.NumOutcomes())
        MyMessageBox("This outcome is not defined yet", "Outcome",
                     EFG_OUTCOME_HELP, pframe);
    else
    {
        if (x != -1)  // dropped an outcome at the coordinates (x,y)
        {   
            ScreenToClient(&x, &y); // x,y are absolute screen coordinates
            float xf = x, yf = y;
            Node *tmp = GotObject(xf, yf, DRAG_OUTCOME_END);
            if (tmp)
                cursor = tmp;
            else
                return;
        }

        if (out > 0)
            cursor->SetOutcome(ef.Outcomes()[out]);

        if (out == 0)
            cursor->SetOutcome(0);

        // if (out == -1) just update all outcomes
        outcomes_changed = 1;
        OnPaint();
    }
}


//***********************************************************************
//                       NODE-SET-MARK MENU HANDLER
//***********************************************************************

void TreeWindow::node_set_mark(void)
{
    old_mark_node = mark_node;
    if (mark_node != cursor)
        mark_node = cursor;
    else
        mark_node = 0;                                   
}

//***********************************************************************
//                      NODE-GOTO-MARK MENU HANDLER
//***********************************************************************

void TreeWindow::node_goto_mark(void)
{
    if (mark_node)
    {
        cursor = mark_node;
        ProcessCursor();
    }
    else
        MyMessageBox("Mark not set!", "Error", EFG_NODE_HELP, pframe);
}

//-----------------------------------------------------------------------
//                   INFOSET MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                    INFOSET-MEMBER MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_merge(void)
{
    if (!mark_node)
    {
        MyMessageBox("The mark is not set", "Error", EFG_INFOSET_HELP, pframe);
        return;
    }
    
    if (!mark_node->GetInfoset())
    {
        wxMessageBox("Marked node belongs to no infosets");
        return;
    }
    
    if (!cursor->GetInfoset())
    {
        wxMessageBox("Cursor belongs to no infosets");
        return;
    }
    
    if (mark_node->GetSubgameRoot() != cursor->GetSubgameRoot())
    {
        MyMessageBox("Can not merge infosets across subgames", "Error",
                     EFG_INFOSET_HELP, pframe);
        return;
    }
    
    if (mark_node->GetPlayer() != cursor->GetPlayer())
    {
        MyMessageBox("Can not merge infosets with different players.\n"
                     "Change player and retry",
                     "Error", EFG_INFOSET_HELP, pframe); 
        return;
    }
    
    char *iset_name = wxGetTextFromUser("Merged infoset name");
    Infoset *new_iset = ef.MergeInfoset(cursor->GetInfoset(),
                                        mark_node->GetInfoset());
    
    if (iset_name)
        new_iset->SetName(iset_name);
    else 
        new_iset->SetName("Infoset"+ToText(new_iset->GetPlayer()->NumInfosets()));
    
    infosets_changed = TRUE;
}

//***********************************************************************
//                      INFOSET-BREAK MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_break(void)
{
    if (!cursor->GetInfoset())
    {
        wxMessageBox("Cursor belongs to no infosets");
        return;
    }
    
    char *iset_name = wxGetTextFromUser("New infoset name");
    Infoset *new_iset = ef.LeaveInfoset(cursor);
    if (iset_name)
        new_iset->SetName(iset_name);
    else 
        new_iset->SetName("Infoset"+ToText(new_iset->GetPlayer()->NumInfosets()));
    infosets_changed = TRUE;
}

//***********************************************************************
//                      INFOSET-SPLIT MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_split(void)
{
    if (!cursor->GetInfoset())
    {
        wxMessageBox("Cursor belongs to no infosets");
        return;
    }
    
    char *iset_name = wxGetTextFromUser("New infoset name");
    Infoset *new_iset = ef.SplitInfoset(cursor);
    if (iset_name)
        new_iset->SetName(iset_name);
    else
        new_iset->SetName("Infoset"+ToText(new_iset->GetPlayer()->NumInfosets()));
    infosets_changed = TRUE;
}

//***********************************************************************
//                       INFOSET-JOIN MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_join(void)
{
    if (!mark_node)
    {
        MyMessageBox("The mark is not set", "Error", EFG_INFOSET_HELP, pframe);
        return;
    }
    
    if (!mark_node->GetInfoset())
    {
        wxMessageBox("Marked node belongs to no infosets");
        return;
    }
    
    if (!cursor->GetInfoset())
    {
        wxMessageBox("Cursor belongs to no infosets");
        return;
    }
    
    if (mark_node->GetSubgameRoot() != cursor->GetSubgameRoot())
    {
        MyMessageBox("Can not join infosets across subgames", "Error",
                     EFG_INFOSET_HELP, pframe);
        return;
    }
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        ef.JoinInfoset(mark_node->GetInfoset(), cursor);
#ifdef USE_EXCEPTIONS
    }
    catch (Efg::Exception &e)
    {
        // error in Efg class -- for now, just silently ignore
    }
#endif   // USE_EXCEPTIONS
    infosets_changed = TRUE;
}

//***********************************************************************
//                       INFOSET-LABEL MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_label(void)
{
    if (!cursor->GetInfoset())
    {
        wxMessageBox("This node belongs to no infosets");
        return;
    }
    
    char *label = 0;
    MyDialogBox *label_dialog = 0;
    
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        label = new char[MAX_LABEL_LENGTH];
        Bool label_actions = TRUE;
        if (cursor->GetInfoset()->GetName() != "")
            strcpy(label, cursor->GetInfoset()->GetName());
        else
            strcpy(label, "Infoset"+ToText(cursor->GetPlayer()->NumInfosets()));
        
        label_dialog = new MyDialogBox(pframe, "Label Infoset", EFG_INFOSET_HELP);
        label_dialog->Add(wxMakeFormString(
            "Label", &label, wxFORM_DEFAULT,
            new wxList(wxMakeConstraintFunction(StringConstraint), 0)));
        label_dialog->Add(wxMakeFormNewLine());
        label_dialog->Add(wxMakeFormBool("Label Actions", &label_actions));
        label_dialog->Go();

        if (label_dialog->Completed() == wxOK)
        {
            cursor->GetInfoset()->SetName(label);
            if (label_actions) action_label();
        }

        delete label_dialog;
        delete [] label;
#ifdef USE_EXCEPTIONS
    }
    catch (...)
    {
        if (label_dialog)   
            delete label_dialog;

        if (label)    
            delete [] label;
    }
#endif   // USE_EXCEPTIONS
}


//***********************************************************************
//                       INFOSET-SWITCH-PLAYER MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_switch_player(void)
{
    if (!cursor->GetInfoset())
    {
        wxMessageBox("This node belongs to no infosets");
        return;
    }
    
    MyDialogBox *infoset_switch_dialog = 0;
    char *player_name = 0;
    
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        infoset_switch_dialog = new MyDialogBox(this, "Set New Player", EFG_INFOSET_HELP);
        wxStringList *player_list = new wxStringList;
        player_name = new char[20];
        
        if (ef.GetChance() != cursor->GetPlayer())
            player_list->Add(ef.GetChance()->GetName());

        for (int pl = 1; pl <= ef.NumPlayers(); pl++)
        {
            if (ef.Players()[pl] != cursor->GetPlayer())
                player_list->Add(ef.Players()[pl]->GetName());
        }

        infoset_switch_dialog->Add(
            wxMakeFormString("Player", &player_name,
                             wxFORM_CHOICE,
                             new wxList(wxMakeConstraintStrings(player_list), 0)));
        infoset_switch_dialog->Go();
        
        if (infoset_switch_dialog->Completed() == wxOK)
        {
            if (strcmp(player_name, ef.GetChance()->GetName()))
            {
                ef.SwitchPlayer(cursor->GetInfoset(), EfgGetPlayer(ef, player_name));
                infosets_changed = TRUE;
            }
            else
            {
                Infoset *s = ef.CreateInfoset(ef.GetChance(), cursor->NumChildren());
                ef.JoinInfoset(s, cursor);
                infosets_changed = TRUE;
            }
        }
        
        delete infoset_switch_dialog;
        delete [] player_name;
#ifdef USE_EXCEPTIONS
    }
    catch (...)
    {
        if (infoset_switch_dialog)  
            delete infoset_switch_dialog;

        if (player_name)   
            delete [] player_name;
    }
#endif    // USE_EXCEPTIONS
}

//***********************************************************************
//                       INFOSET-REVEAL MENU HANDLER
//***********************************************************************

void TreeWindow::infoset_reveal(void)
{
    if (!cursor->GetInfoset())
    {
        wxMessageBox("This node belongs to no infosets");
        return;
    }
    
    MyDialogBox *infoset_reveal_dialog = 0;
    char **player_names = 0;
    
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        infoset_reveal_dialog = new MyDialogBox(this, "Reveal to Players",
                                                EFG_INFOSET_HELP);
        player_names = new char *[ef.NumPlayers()];

        for (int i = 1; i <= ef.NumPlayers(); i++)
            player_names[i - 1] = 0;

        for (int i = 1; i <= ef.NumPlayers(); i++)
            player_names[i - 1] = copystring(ef.Players()[i]->GetName());
        
        wxListBox *player_item = new wxListBox(infoset_reveal_dialog, 0, "Players",
                                               wxMULTIPLE, -1, -1, -1, -1,
                                               ef.NumPlayers(), player_names);
        infoset_reveal_dialog->Go();

        if (infoset_reveal_dialog->Completed() == wxOK)
        {
            gBlock<EFPlayer *> players;

            for (int i = 1; i <= ef.NumPlayers(); i++)
            {
                if (player_item->Selected(i-1)) 
                    players.Append(ef.Players()[i]);
            }

            ef.Reveal(cursor->GetInfoset(), players);
            infosets_changed = TRUE;
        }
        
        delete infoset_reveal_dialog;

        for (int i = 1; i <= ef.NumPlayers(); i++)
            delete [] player_names[i - 1];

        delete [] player_names;
#ifdef USE_EXCEPTIONS
    }
    catch (...)
    {
        if (infoset_reveal_dialog)
            delete infoset_reveal_dialog;

        if (player_names)
        {
            for (int i = 1; i <= ef.NumPlayers(); i++)
            {
                if (player_names[i - 1])
                    delete [] player_names[i - 1];
            }

            if (player_names)   
                delete [] player_names;
        }
    }
#endif   // USE_EXCEPTIONS
}



//-----------------------------------------------------------------------
//                    ACTION MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                      ACTION-LABEL MENU HANDLER
//***********************************************************************

// The text input fields are stacked vertically up to ENTRIES_PER_DIALOG.
// If there are more than ENTRIES_PER_DIALOG actions for this infoset,
// consequtive dialogs will be created.

#define ENTRIES_PER_DIALOG 10

void TreeWindow::action_label(void)
{
    if (!cursor->GetInfoset())
    {
        wxMessageBox("This node belongs to no infosets");
        return;
    }
    
    if (cursor->GetInfoset()->NumActions() < 1)
    {
        wxMessageBox("No actions to label");
        return;
    }
    
    int num_actions = cursor->GetInfoset()->NumActions();
    int num_d = num_actions / ENTRIES_PER_DIALOG -
        ((num_actions % ENTRIES_PER_DIALOG) ? 0 : 1);

    for (int d = 0; d <= num_d; d++)
    {
        MyDialogBox *branch_label_dialog = new MyDialogBox(pframe, "Action Label",
                                                           EFG_ACTION_HELP);
        int actions_now = gmin(num_actions-d*ENTRIES_PER_DIALOG, ENTRIES_PER_DIALOG);
        char **action_names = new char *[actions_now];
        int i;

        for (i = 1; i <= actions_now; i++)
        {
            action_names[i-1] = new char[MAX_LABEL_LENGTH];
            strcpy(action_names[i-1], 
                   cursor->GetInfoset()->GetActionName(i+d*ENTRIES_PER_DIALOG));
            branch_label_dialog->Add(wxMakeFormString(
                "Action " + ToText(i + d * ENTRIES_PER_DIALOG), 
                &action_names[i-1], 
                wxFORM_DEFAULT,
                new wxList(wxMakeConstraintFunction(StringConstraint), 0), 0, 0, 220));
            branch_label_dialog->Add(wxMakeFormNewLine());
        }

        if (num_actions-(d+1)*ENTRIES_PER_DIALOG > 0)
            branch_label_dialog->Add(wxMakeFormMessage("Continued..."));

        branch_label_dialog->Go();

        if (branch_label_dialog->Completed() == wxOK)
        {
            for (i = 1; i <= actions_now; i++)
                cursor->GetInfoset()->SetActionName(i+d*ENTRIES_PER_DIALOG, 
                                                    action_names[i-1]);
        }

        delete branch_label_dialog;

        for (i = 1; i <= actions_now; i++)
            delete [] action_names[i-1];

        delete [] action_names;
    }
}

//***********************************************************************
//                     ACTION-INSERT MENU HANDLER
//***********************************************************************

void TreeWindow::action_insert(void)
{
    int num_children = cursor->NumChildren();
    
    if (num_children == 0)
    {
        MyMessageBox("Terminal node: cannot insert branch", "Error",
                     EFG_ACTION_HELP, pframe);
        return;
    }
    
    MyDialogBox *branch_insert_dialog = new MyDialogBox(pframe, "Insert Branch",
                                                        EFG_ACTION_HELP);
    Infoset *iset = cursor->GetInfoset();
    wxStringList *action_list = new wxStringList;
    char *action_str = new char[30];

    for (int i = 1; i <= iset->NumActions(); i++) 
        action_list->Add(iset->GetActionName(i));

    branch_insert_dialog->Add(wxMakeFormString(
        "Where", &action_str,
        wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(action_list), 0)));
    branch_insert_dialog->Go();

    if (branch_insert_dialog->Completed() == wxOK)
    {
        nodes_changed = TRUE;
        
        for (int act = 1; act <= iset->NumActions(); act++)
        {
            if (iset->Actions()[act]->GetName() == action_str)
                ef.InsertAction(iset, iset->Actions()[act]);
        }
    }

    delete branch_insert_dialog;
    delete [] action_str;
}

//***********************************************************************
//                       ACTION-DELETE MENU HANDLER
//***********************************************************************

void TreeWindow::action_delete(void)
{
    int num_children = cursor->NumChildren();
    
    if (num_children == 0)
    {
        MyMessageBox("Terminal node: cannot delete branch", "Error",
                     EFG_ACTION_HELP, pframe);
        return;
    }
    
    MyDialogBox *branch_delete_dialog = 
        new MyDialogBox(pframe, "Delete Branch", EFG_ACTION_HELP);
    Infoset *iset = cursor->GetInfoset();
    wxStringList *action_list = new wxStringList;
    char *action_str = new char[30];

    for (int i = 1; i <= iset->NumActions(); i++) 
        action_list->Add(iset->GetActionName(i));

    branch_delete_dialog->Add(wxMakeFormString(
        "Which", &action_str,
        wxFORM_CHOICE,
        new wxList(wxMakeConstraintStrings(action_list), 0)));
    branch_delete_dialog->Go();

    if (branch_delete_dialog->Completed() == wxOK)
    {
        nodes_changed = TRUE;
        for (int act = 1; act <= iset->NumActions(); act++)
            if (iset->Actions()[act]->GetName() == action_str)
                ef.DeleteAction(iset, iset->Actions()[act]);
    }

    delete branch_delete_dialog;
    delete [] action_str;
}

//***********************************************************************
//                      NODE-PROBS MENU HANDLER
//***********************************************************************

// The text input fields are stacked vertically up to ENTRIES_PER_DIALOG.
// If there are more than ENTRIES_PER_DIALOG actions for this infoset,
// consecutive dialogs will be created.

#define ENTRIES_PER_DIALOG  10

void TreeWindow::action_probs(void)
{
    Node *n = cursor;
    int i;
    
    if (!n->GetInfoset())
    {
        wxMessageBox("Probs can only be set for a CHANCE player", "Error",
                     wxOK | wxCENTRE, pframe);
        return;
    }
    
    if (!n->GetPlayer()->IsChance())  // if this is not a chance player
    {   
        wxMessageBox("Probs can only be set for a CHANCE player", "Error",
                     wxOK | wxCENTRE, pframe);
        return;
    }
    
    int num_actions = cursor->NumChildren();
    int num_d = num_actions/ENTRIES_PER_DIALOG-((num_actions%ENTRIES_PER_DIALOG) ? 0 : 1);
    
    for (int d = 0; d <= num_d; d++)
    {
        MyDialogBox *node_probs_dialog = new MyDialogBox(pframe, "Node Probabilities");
        int actions_now = gmin(num_actions-d*ENTRIES_PER_DIALOG, ENTRIES_PER_DIALOG);
        char **prob_vector = new char *[actions_now+1];

        for (i = 1; i <= actions_now; i++)
        {
            gNumber temp_p = ef.GetChanceProb(n->GetInfoset(),
                                              i + d * ENTRIES_PER_DIALOG);
            gText temp_s = ToText(temp_p);
            prob_vector[i] = new char[temp_s.Length()+1];
            strcpy(prob_vector[i], temp_s);
            node_probs_dialog->Add(wxMakeFormString(
                "", &(prob_vector[i]), wxFORM_TEXT, NULL, NULL, wxVERTICAL, 80));
            node_probs_dialog->Add(wxMakeFormNewLine());
        }

        if (num_actions-(d+1)*ENTRIES_PER_DIALOG > 0)
            node_probs_dialog->Add(wxMakeFormMessage("Continued..."));
        
        node_probs_dialog->Go();

        if (node_probs_dialog->Completed() == wxOK)
        {
            gNumber dummy;
            for (i = 1; i <= actions_now; i++)
                ef.SetChanceProb(n->GetInfoset(),
                                 i + d * ENTRIES_PER_DIALOG,
                                 FromText(prob_vector[i], dummy));
            outcomes_changed = TRUE;  // game changed -- delete solutions, etc
        }

        for (i = 1; i <= actions_now; i++) 
            delete [] prob_vector[i];
        delete [] prob_vector;
        delete node_probs_dialog;
    }
}


//-----------------------------------------------------------------------
//                     TREE MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                       TREE-LABEL MENU HANDLER
//***********************************************************************

static Bool LongStringConstraint(int type, char *value, char *label, char *msg_buffer)
{
    if (value && (strlen(value) >= 255) && (type == wxFORM_STRING))
    {
        sprintf(msg_buffer, "Value for %s should be %d characters or less\n",
                label, 255);
        return FALSE;
    }
    else
        return TRUE;
}


void TreeWindow::tree_label(void)
{
    char *label = new char[256];
    
    strcpy(label, ef.GetTitle());
    MyDialogBox *tree_label_dialog = new MyDialogBox(pframe, "Label Tree",
                                                     EFG_TREE_HELP);
    wxFormItem *label_item = 
        wxMakeFormString("Label", &label, wxFORM_DEFAULT,
                         new wxList(wxMakeConstraintFunction(LongStringConstraint), 0), 
                         0, 0, 350);
    tree_label_dialog->Add(label_item);
    tree_label_dialog->AssociatePanel();
    ((wxText *)label_item->PanelItem)->SetFocus();
    tree_label_dialog->Go1();
    
    if (tree_label_dialog->Completed() == wxOK)
        ef.SetTitle(label);
    
    delete tree_label_dialog;
    delete [] label;
}

//***********************************************************************
//                      TREE-DELETE MENU HANDLER
//***********************************************************************

void TreeWindow::tree_delete(void)
{
    if (cursor->NumChildren() == 0)
    {
        //  Ignore this error silently
        //    wxMessageBox("Terminal node: cannot delete tree", "Error",
        //                   wxOK | wxCENTRE, pframe);
        return;
    }
    
    
    MyMessageBox *tree_delete_dialog = 0;
    
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        tree_delete_dialog = new MyMessageBox("Are you sure?",
                                              "Delete Tree",
                                              EFG_TREE_HELP, pframe);
        
        if (tree_delete_dialog->Completed() == wxOK)
        {
            nodes_changed = true;
#ifdef USE_EXCEPTIONS
            try
            {
#endif   // USE_EXCEPTIONS
                ef.DeleteTree(cursor);
#ifdef USE_EXCEPTIONS
            }
            catch (Efg::Exception &e)
            {
                // internal error in Efg class.. ignore silently for now
            }
#endif   // USE_EXCEPTIONS
        }
        
        delete tree_delete_dialog;
#ifdef USE_EXCEPTIONS
    }
    catch (...)
    {
        if (tree_delete_dialog)  
            delete tree_delete_dialog;
    }
#endif   // USE_EXCEPTIONS
}

//***********************************************************************
//                       TREE-COPY MENU HANDLER
//***********************************************************************

void TreeWindow::tree_copy(void)
{
    if (!mark_node)
    {
        MyMessageBox("The mark is not set", "Error",
                     EFG_TREE_HELP, pframe);
        return;
    }

    if (cursor->GetSubgameRoot() != mark_node->GetSubgameRoot())
    {
        MyMessageBox("Can not copy across subgames", "Error", EFG_TREE_HELP, pframe);
        return;
    }

    nodes_changed = true;
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        ef.CopyTree(mark_node, cursor);
#ifdef USE_EXCEPTIONS
    }
    catch (Efg::Exception &e)
    {
        // internal error in Efg class..  ignore silently for now
    }
#endif  // USE_EXCEPTIONS
}

//***********************************************************************
//                       TREE-MOVE MENU HANDLER
//***********************************************************************

void TreeWindow::tree_move(void)
{
    if (!mark_node)
    {
        MyMessageBox("The mark is not set", "Error", EFG_TREE_HELP, pframe);
        return;
    }

    if (cursor->GetSubgameRoot() != mark_node->GetSubgameRoot())
    {
        MyMessageBox("Can not copy across subgames", "Error", EFG_TREE_HELP, pframe);
        return;
    }

    nodes_changed = true;
#ifdef USE_EXCEPTIONS
    try
    {
#endif   // USE_EXCEPTIONS
        ef.MoveTree(mark_node, cursor);
#ifdef USE_EXCEPTIONS
    }
    catch (Efg::Exception &e)
    {
        // internal error in Efg class.. ignore silently for now
    }
#endif   // USE_EXCEPTIONS
}


//***********************************************************************
//                      TREE-PLAYERS MENU HANDLER
//***********************************************************************

#define PLAYERSD_INST // instantiate the players display dialog
#include "playersd.h"

void TreeWindow::tree_players(void)
{
    PlayerNamesDialog player_names(ef, pframe);
}

//***********************************************************************
//                      TREE-INFOSETS MENU HANDLER
//***********************************************************************

void TreeWindow::tree_infosets(void)
{
    InfosetDialog ID(ef, pframe);

    if (ID.GameChanged()) 
        infosets_changed = true;
}


//-----------------------------------------------------------------------
//                     SUBGAME MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

void LegalSubgameRoots(const Efg &efg, gList<Node *> &list);

void TreeWindow::subgame_solve(void)
{
    subgame_list.Flush();
    subgame_list.Append(SubgameEntry(ef.RootNode())); // Add the first subgmame -- root subgame
    gList<Node *> subgame_roots;
    LegalSubgameRoots(ef, subgame_roots);
    ef.MarkSubgames(subgame_roots);

    for (int i = 1; i <= subgame_roots.Length(); i++)
    {
        if (subgame_roots[i] != ef.RootNode())
            subgame_list.Append(SubgameEntry(subgame_roots[i], true));
    }

    must_recalc = true;
}

void TreeWindow::subgame_set(void)
{
    if (cursor->GetSubgameRoot() == cursor)
    {
        wxMessageBox("This node is already a subgame root");
        return;
    }

    if (!ef.IsLegalSubgame(cursor))
    {
        wxMessageBox("This node is not a root of a valid subgame"); 
        return;
    }

    ef.DefineSubgame(cursor);
    subgame_list.Append(SubgameEntry(cursor, true)); // collapse
    must_recalc = true;
}

void TreeWindow::subgame_clear_one(void)
{
    if (cursor->GetSubgameRoot() != cursor)
    {
        wxMessageBox("This node is not a subgame root");
        return;
    }

    if (cursor->GetSubgameRoot() == ef.RootNode())
    {
        wxMessageBox("Root node is always a subgame root");
        return;
    }
    
    ef.RemoveSubgame(cursor);

    for (int i = 1; i <= subgame_list.Length(); i++)
    {
        if (subgame_list[i].root == cursor)
            subgame_list.Remove(i);
    }

    must_recalc = true;
}

void TreeWindow::subgame_clear_all(void)
{
    ef.UnmarkSubgames(ef.RootNode());
    subgame_list.Flush();
    subgame_list.Append(SubgameEntry(ef.RootNode())); // Add the first subgmame -- root subgame
    must_recalc = true;
}

void TreeWindow::subgame_collapse_one(void)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
    {
        if (subgame_list[i].root == cursor)
        {
            subgame_list[i].expanded = false;
            must_recalc = true;
            return;
        }
    }

    wxMessageBox("This node is not a subgame root");
}

void TreeWindow::subgame_collapse_all(void)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
        subgame_list[i].expanded = false;

    must_recalc = true;
}

void TreeWindow::subgame_expand_one(void)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
    {
        if (subgame_list[i].root == cursor)
        {
            subgame_list[i].expanded = true;
            must_recalc = true;
            return;
        }
    }

    wxMessageBox("This node is not a subgame root");
}

void TreeWindow::subgame_expand_branch(void)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
    {
        if (subgame_list[i].root == cursor)
        {
            for (int j = 1; j <= subgame_list.Length(); j++)
            {
                if (ef.IsSuccessor(subgame_list[j].root, cursor))
                {
                    subgame_list[j].expanded = true;
                    must_recalc = true;
                }
            }

            return;
        }
    }

    wxMessageBox("This node is not a subgame root");
}


void TreeWindow::subgame_expand_all(void)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
        subgame_list[i].expanded = true;

    must_recalc = true;
}

void TreeWindow::subgame_toggle(void)
{
    for (int i = 1; i <= subgame_list.Length(); i++)
    {
        if (subgame_list[i].root == cursor)
        {
            subgame_list[i].expanded = !subgame_list[i].expanded; 
            must_recalc = true;
            return;
        }
    }

    wxMessageBox("This node is not a subgame root");
}



//***********************************************************************
//                      FILE-SAVE MENU HANDLER
//***********************************************************************
// see treewin.cc

//***********************************************************************
//                      FILE-OUTPUT MENU HANDLER
//***********************************************************************
void TreeWindow::output(void)
{
    wxOutputDialogBox od;

    if (od.Completed() == wxOK)
    {
        switch (od.GetMedia())
        {
        case wxMEDIA_PRINTER: print(od.GetOption()); break;
        case wxMEDIA_PS:print_eps(od.GetOption()); break;
        case wxMEDIA_CLIPBOARD:print_mf(od.GetOption()); break;
        case wxMEDIA_METAFILE: print_mf(od.GetOption(), true); break;
        case wxMEDIA_PREVIEW: print(od.GetOption(), true); break;
        default:
            wxMessageBox("Contact the author\ngambit@cco.caltech.edu",
                         "Internal Error");
            break;
        }
    }
}

#ifdef wx_msw
#include "wx_print.h"

class ExtensivePrintout: public wxPrintout
{
private:
    TreeWindow *tree;
    wxOutputOption fit;
    int num_pages;
    
public:
    ExtensivePrintout(TreeWindow *s, wxOutputOption f,
                      const char *title = "ExtensivePrintout");
    Bool OnPrintPage(int page);
    Bool HasPage(int page);
    Bool OnBeginDocument(int startPage, int endPage);
    void GetPageInfo(int *minPage, int *maxPage, int *selPageFrom, int *selPageTo);
};


Bool ExtensivePrintout::OnPrintPage(int)
{
    // this is funky--I am playing around w/ the
    // different zoom settings.  So the zoom setting in draw_settings does not
    // equal to the zoom setting in the printer!
    wxDC *dc = GetDC();
    if (!dc) return FALSE;
    
    dc->SetBackgroundMode(wxTRANSPARENT);
    dc->Colour = FALSE;
    Bool color_outcomes;

    if (!dc->Colour)
    {
        color_outcomes = tree->draw_settings.ColorCodedOutcomes();
        tree->draw_settings.SetOutcomeColor(FALSE);
    }
    
    int win_w, win_h;
    tree->GetClientSize(&win_w, &win_h);    // that is the size of the window
    float old_zoom = tree->draw_settings.Zoom();
    // Now we have to check in case our real page size is reduced
    // (e.g. because we're drawing to a print preview memory DC)
    int pageWidth, pageHeight;
    float w, h;
    dc->GetSize(&w, &h);
    GetPageSizePixels(&pageWidth, &pageHeight);
    float pageScaleX = (float)w/pageWidth;
    float pageScaleY = (float)h/pageHeight;
    
    if (fit) // fit to page
    {  
        int maxX = tree->draw_settings.MaxX(), maxY = tree->draw_settings.MaxY(); // size of tree
        // Figure out the 'fake' window zoom
        float zoom_x = (float)win_w/(float)maxX, zoom_y = (float)win_h/(float)maxY;
        float real_zoom = gmin(zoom_x, zoom_y);
        tree->draw_settings.SetZoom(real_zoom, true);
        // Figure out the 'real' printer zoom
        int ppiPrinterX, ppiPrinterY;
        GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
        // Make the margins.  They are just 1" on all sides now.
        float marginX = 1*ppiPrinterX;
        float marginY = 1*ppiPrinterY;
        
        zoom_x = (float)((pageWidth-2*marginX)/(float)maxX)*pageScaleX;
        zoom_y = (float)((pageHeight-2*marginY)/(float)maxY)*pageScaleY;
        real_zoom = gmin(zoom_x, zoom_y);
        
        dc->SetUserScale(real_zoom, real_zoom);
        dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
    }
    else  // WYSIWYG
    {       
        int ppiScreenX, ppiScreenY;
        GetPPIScreen(&ppiScreenX, &ppiScreenY);
        int ppiPrinterX, ppiPrinterY;
        GetPPIPrinter(&ppiPrinterX, &ppiPrinterY);
        
        // This scales the DC so that the printout roughly represents the
        // the screen scaling. The text point size _should_ be the right size
        // but in fact is too small for some reason. This is a detail that will
        // need to be addressed at some point but can be fudged for the
        // moment.
        float scaleX = (float)((float)ppiPrinterX/(float)ppiScreenX);
        float scaleY = (float)((float)ppiPrinterY/(float)ppiScreenY);
        
        // If printer pageWidth == current DC width, then this doesn't
        // change. But w might be the preview bitmap width, so scale down.
        float overallScaleX = scaleX * pageScaleX;
        float overallScaleY = scaleY * pageScaleY;
        dc->SetUserScale(overallScaleX, overallScaleY);
        
        // Make the margins.  They are just 1" on all sides now.
        float marginX = 1*ppiPrinterX, marginY = 1*ppiPrinterY;
        dc->SetDeviceOrigin(marginX*pageScaleX, marginY*pageScaleY);
        // Figure out the 'fake' window zoom
        float real_width = (pageWidth-2*marginX)/scaleX;
        float real_height = (pageHeight-2*marginY)/scaleY;
        float zoom_x = win_w/real_width, zoom_y = win_h/real_height;
        float real_zoom = gmax(zoom_x, zoom_y);
        tree->draw_settings.SetZoom(real_zoom, true);
    }
    
    tree->Render(*dc);
    
    tree->draw_settings.SetZoom(old_zoom);
    if (!dc->Colour)
        tree->draw_settings.SetOutcomeColor(color_outcomes);
    
    return TRUE;
}

Bool ExtensivePrintout::HasPage(int page)
{
    return (page <= 1);
}

ExtensivePrintout::ExtensivePrintout(TreeWindow *t, wxOutputOption f,
                                     const char *title)
    :tree(t), fit(f), wxPrintout((char *)title)
{ }

Bool ExtensivePrintout::OnBeginDocument(int startPage, int endPage)
{
    if (!wxPrintout::OnBeginDocument(startPage, endPage))
        return FALSE;
    
    return TRUE;
}

// Since we can not get at the actual device context in this function, we
// have no way to tell how many pages will be used in the wysiwyg mode. So,
// we have no choice but to disable the From:To page selection mechanism.
void ExtensivePrintout::GetPageInfo(int *minPage, int *maxPage,
                                    int *selPageFrom, int *selPageTo)
{
    num_pages = 1;
    *minPage = 0;
    *maxPage = num_pages;
    *selPageFrom = 0;
    *selPageTo = 0;
}
#endif

//***********************************************************************
//                      TREE-PRINT MENU HANDLER
//***********************************************************************
#ifdef wx_msw
void TreeWindow::print(wxOutputOption fit, bool preview)
{
    if (!preview)
    {
        wxPrinter printer;
        ExtensivePrintout printout(this, fit);
        printer.Print(pframe, &printout, TRUE);
    }
    else
    {
        wxPrintPreview *preview = new wxPrintPreview(new ExtensivePrintout(this, fit), new ExtensivePrintout(this, fit));
        wxPreviewFrame *ppframe = new wxPreviewFrame(preview, pframe, "Print Preview", 100, 100, 600, 650);
        ppframe->Centre(wxBOTH);
        ppframe->Initialize();
        ppframe->Show(TRUE);
    }
}
#else
void TreeWindow::print(wxOutputOption /*fit*/, bool preview)
{
    if (!preview)
        wxMessageBox("Printing not supported under X");
    else
        wxMessageBox("Print Preview is not supported under X");
}
#endif


//***********************************************************************
//                      TREE-PRINT EPS MENU HANDLER
//***********************************************************************

void TreeWindow::print_eps(wxOutputOption fit)
{
    wxPostScriptDC dc(NULL, TRUE);
    if (dc.Ok())
    {
        float old_zoom;

        if (fit == wxFITTOPAGE)
        {
            old_zoom = draw_settings.Zoom();
            int w, h;
            GetSize(&w, &h);
            draw_settings.SetZoom(gmin((float)w/(float)draw_settings.MaxX(),
                                       (float)h/(float)draw_settings.MaxY()), true);
        }

        Bool color_outcomes = draw_settings.ColorCodedOutcomes();
        draw_settings.SetOutcomeColor(FALSE);
        dc.StartDoc("Gambit printout");
        dc.StartPage();
        Render(dc);
        dc.EndPage();
        dc.EndDoc();
        if (fit == wxFITTOPAGE)
            draw_settings.SetZoom(old_zoom);
        draw_settings.SetOutcomeColor(color_outcomes);
    }
}

//***********************************************************************
//                      TREE-PRINT MF MENU HANDLER
//***********************************************************************
#ifdef wx_msw
void TreeWindow::print_mf(wxOutputOption fit, bool save_mf)
{
    char *metafile_name = 0;

    if (save_mf)
        metafile_name = copystring(wxFileSelector("Save Metafile", 0, 0, ".wmf", "*.wmf"));

    wxMetaFileDC dc_mf(metafile_name);

    if (dc_mf.Ok())
    {
        dc_mf.SetBackgroundMode(wxTRANSPARENT);
        float old_zoom;

        if (fit == wxFITTOPAGE)
        {
            old_zoom = draw_settings.Zoom();
            int w, h;
            GetSize(&w, &h);
            draw_settings.SetZoom(gmin((float)w/(float)draw_settings.MaxX(),
                                       (float)h/(float)draw_settings.MaxY()), true);
        }

        if (!save_mf) 
            dc_mf.SetUserScale(2.0, 2.0);

        Render(dc_mf);

        if (fit == wxFITTOPAGE) 
            draw_settings.SetZoom(old_zoom);
        
        wxMetaFile *mf = dc_mf.Close();

        if (mf)
        {
            Bool success = mf->SetClipboard((int)(dc_mf.MaxX()+10), (int)(dc_mf.MaxY()+10));
            if (!success) 
                wxMessageBox("Copy Failed", "Error", wxOK | wxCENTRE, pframe);
            delete mf;
        }

        if (save_mf)
            wxMakeMetaFilePlaceable(
                metafile_name, 0, 0, (int)(dc_mf.MaxX()+10), (int)(dc_mf.MaxY()+10));
    }
}
#else
void TreeWindow::print_mf(wxOutputOption /*fit*/, bool /*save_mf*/)
{
    wxMessageBox("Metafiles are not supported under X");
}
#endif

//***********************************************************************
//                      FILE LOGGING  HANDLER
//***********************************************************************

Bool TreeWindow::logging(void)
{
    wxMessageBox("Not implemented yet"); return FALSE;
#ifdef UNUSED
    if (!log)
    {
        char *s = copystring(wxFileSelector("Start/Resume Logging", NULL, NULL, ".log", "*.log"));
        if (strcmp(s, "") != 0)
        {
            if (FileExists(s))
            {
                MyDialogBox *d = new MyDialogBox(pframe, "File Exists");
                char *choices[2] = {"Append", "Overwrite"};
                wxRadioBox *r = new wxRadioBox(d, 0, "", -1, -1, -1, -1, 2, choices);
                d->Go();
                if (d->Completed() == wxOK)
                {
                    if (r->GetSelection() == 0)
                    {
                        FILE *fp = fopen(s, "a");
                        log = new gFileOutput(fp);
                    }
                    else
                        log = new gFileOutput(s);
                }
                delete d;
            }
            else
                log = new gFileOutput(s);
        }
    }
    else
    {
        delete log;
        log = 0;
    }
    return ((log) ? TRUE : FALSE);
#endif   // UNUSED
}

void TreeWindow::Log(const gText &s)
{
    if (!log) return;
    (*log) << s << '\n';
}

//-----------------------------------------------------------------------
//                     DISPLAY MENU HANDLER FUNCTIONS
//-----------------------------------------------------------------------

//***********************************************************************
//                      DISPLAY-ZOOM WINDOW MENU HANDLER
//***********************************************************************
Bool TreeWindow::display_zoom_win(void)
{
    if (zoom_window)   // already exists, delete it
    { 
        // note that zoom_window itself is just the canvas, must delete the frame!
        delete zoom_window->GetParent();
        zoom_window = 0;
    }
    else   
    {
        zoom_window = 
            new TreeZoomWindow(pframe, this, node_list, 
                               (const Infoset *&)hilight_infoset,
                               (const Infoset *&)hilight_infoset1, 
                               (const Node *&)mark_node, 
                               (const Node *&)cursor,
                               (const Node *&)subgame_node, 
                               draw_settings, GetNodeEntry(cursor));
    }
    return (zoom_window) ? TRUE : FALSE;
}

//***********************************************************************
//                      DISPLAY-ZOOM MENU HANDLER
//***********************************************************************

void TreeWindow::display_set_zoom(float z)
{
    draw_settings.SetZoom(z);
    GetDC()->SetUserScale(draw_settings.Zoom(), draw_settings.Zoom());
    must_recalc = TRUE;
}

float TreeWindow::display_get_zoom(void)
{
    return draw_settings.Zoom();
}

#define MAX_WINDOW_WIDTH   750  // assuming an 800x600 display to be safe
#define MAX_WINDOW_HEIGHT  550
#define MIN_WINDOW_WIDTH   600  // at least 12 buttons
#define MIN_WINDOW_HEIGHT  300

void TreeWindow::display_zoom_fit(void)
{
    int width, height;
    Render(*GetDC());
    width = gmin(draw_settings.MaxX(), MAX_WINDOW_WIDTH);
    height = gmin(draw_settings.MaxY(), MAX_WINDOW_HEIGHT);
    
    double zoomx = (double)width/(double)draw_settings.MaxX();
    double zoomy = (double)height/(double)draw_settings.MaxY();
    
    zoomx = gmin(zoomx, 1.0); zoomy = gmin(zoomy, 1.0);  // never zoom in (only out)
    double zoom = gmin(zoomx, zoomy); // same zoom for vertical and horiz
    
    width = (int)(zoom*(double)draw_settings.MaxX());
    height = (int)(zoom*(double)draw_settings.MaxY());
    
    width = gmax(width, MIN_WINDOW_WIDTH);
    height = gmax(height, MIN_WINDOW_HEIGHT);
    
    draw_settings.SetZoom(zoom);
    GetDC()->SetUserScale(draw_settings.Zoom(), draw_settings.Zoom());
    pframe->SetClientSize(width, height+50); // +50 to account for the toolbar
    must_recalc = TRUE;
    ProcessCursor();
    OnPaint();
}


//***********************************************************************
//                      DISPLAY LEGENGS HANDLER
//***********************************************************************
// Controls what gets displayed above/below [node, branch]
void TreeWindow::display_legends(void)
{
    draw_settings.SetLegends();
}

//***********************************************************************
//                      DISPLAY OPTIONS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
#include "twflash.h"
void TreeWindow::display_options(void)
{
    draw_settings.SetOptions();
    // Must take care of flashing/nonflashing cursor here since draw_settings cannot
    if (draw_settings.FlashingCursor() == TRUE && flasher->Type() == myCursor)
    {
        delete (TreeNodeCursor *)flasher;
        flasher = new TreeNodeFlasher(GetDC());
    }
    
    if (draw_settings.FlashingCursor() == FALSE && flasher->Type() == myFlasher)
    {
        delete (TreeNodeFlasher *)flasher;
        flasher = new TreeNodeCursor(GetDC());
    }

    must_recalc = TRUE;
}

//***********************************************************************
//                      DISPLAY COLORS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
void TreeWindow::display_colors(void)
{
    gArray<gText> names(ef.NumPlayers());
    for (int i = 1; i <= names.Length(); i++)
        names[i] = (ef.Players()[i])->GetName();
    draw_settings.PlayerColorDialog(names);
    must_recalc = TRUE;
}

//***********************************************************************
//                      DISPLAY SAVE OPTIONS HANDLER
//***********************************************************************
// Controls the size of the various tree parts
void TreeWindow::display_save_options(Bool def)
{
    if (def)
        draw_settings.SaveOptions();
    else
    {
        char *s = wxFileSelector("Save Configuration", NULL, NULL, NULL, "*.cfg");
        if (s)
        {
            draw_settings.SaveOptions(copystring(s)); 
            delete [] s;
        }
    }
}

//***********************************************************************
//                      DISPLAY LOAD OPTIONS HANDLER
//***********************************************************************

// Controls the size of the various tree parts
void TreeWindow::display_load_options(Bool def)
{
    if (def)
        draw_settings.LoadOptions(0);
    else
    {
        char *s = wxFileSelector("Load Configuration", NULL, NULL, NULL, "*.cfg");
        if (s)
        {
            draw_settings.LoadOptions(copystring(s));
            delete [] s;
        }
    }
    
    must_recalc = TRUE;
}

