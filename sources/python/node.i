%module node
%include typemaps.i

%{
#include "../game/game.h"
%}

%exception GetChild {
  try {
    $action
  }
  catch (gbtIndexException) {
    SWIG_exception(SWIG_IndexError, "Child index out of range");
  }
}

%exception GetChildAction {
  try {
    $action
  }
  catch (gbtGameMismatchException) {
    SWIG_exception(SWIG_RuntimeError, "Parameters from different games");
  }
}

%exception SetOutcome {
  try {
    $action
  }
  catch (gbtGameMismatchException) {
    SWIG_exception(SWIG_RuntimeError, "Parameters from different games");
  }
}

%exception InsertMove {
  try {
    $action
  }
  catch (gbtGameNullException) {
    SWIG_exception(SWIG_RuntimeError, "Inserting into null information set");
  }
  catch (gbtGameMismatchException) {
    SWIG_exception(SWIG_RuntimeError, "Parameters from different games");
  }
}

%exception CopyTree {
  try {
    $action
  }
  catch (gbtGameNullException) {
    SWIG_exception(SWIG_RuntimeError, "Copying from null node");
  }
  catch (gbtGameMismatchException) {
    SWIG_exception(SWIG_RuntimeError, "Parameters from different games");
  }
}

%exception MoveTree {
  try {
    $action
  }
  catch (gbtGameNullException) {
    SWIG_exception(SWIG_RuntimeError, "Moving from null node");
  }
  catch (gbtGameMismatchException) {
    SWIG_exception(SWIG_RuntimeError, "Parameters from different games");
  }
}

%exception JoinInfoset {
  try {
    $action
  }
  catch (gbtGameNullException) {
    SWIG_exception(SWIG_RuntimeError, "Joining a null information set");
  }
  catch (gbtGameMismatchException) {
    SWIG_exception(SWIG_RuntimeError, "Parameters from different games");
  }
}

%nodefault;
class gbtGameNode {
  ~gbtGameNode();
};

%extend gbtGameNode {
  //--------------------------------------------------------------------
  //               General information about the node
  //--------------------------------------------------------------------
  bool IsNull(void) const { return self->IsNull(); }
  int GetId(void) const { return (*self)->GetId(); }
  void SetLabel(const std::string &s) { (*self)->SetLabel(s); }
  std::string GetLabel(void) const { return (*self)->GetLabel(); }
  bool IsDeleted(void) const { return (*self)->IsDeleted(); }

  //--------------------------------------------------------------------
  //                 Accessing relatives of the node
  //--------------------------------------------------------------------
  int NumChildren(void) const  { return (*self)->NumChildren(); }
  bool IsTerminal(void) const  { return (*self)->IsTerminal(); }
  bool IsNonterminal(void) const  { return (*self)->IsNonterminal(); }
  gbtGameNode GetChild(int i) const  { return (*self)->GetChild(i); }
  gbtGameNode GetChildAction(const gbtGameAction &a)
    { return (*self)->GetChild(a); }
  bool IsPredecessorOf(const gbtGameNode &node) const
    { return (*self)->IsPredecessorOf(node); }

  gbtGameNode GetParent(void) const   { return (*self)->GetParent(); }
  gbtGameAction GetPriorAction(void) const
    { return (*self)->GetPriorAction(); }
  gbtGameSequence GetSequence(const gbtGamePlayer &p) const
    { return (*self)->GetSequence(p); }

  gbtGameNode GetPriorSibling(void) const
    { return (*self)->GetPriorSibling(); }
  gbtGameNode GetNextSibling(void) const
    { return (*self)->GetNextSibling(); }

  //--------------------------------------------------------------------
  //            Accessing the information set of the node
  //--------------------------------------------------------------------

  gbtGameInfoset GetInfoset(void) const  { return (*self)->GetInfoset(); }
  int GetMemberId(void) const { return (*self)->GetMemberId(); }
  gbtGameNode GetPriorMember(void) const { return (*self)->GetPriorMember(); }
  gbtGameNode GetNextMember(void) const  { return (*self)->GetNextMember(); }
  gbtGamePlayer GetPlayer(void) const  { return (*self)->GetPlayer(); }

  //--------------------------------------------------------------------
  //                Accessing the outcome of the node
  //--------------------------------------------------------------------

  gbtGameOutcome GetOutcome(void) const  { return (*self)->GetOutcome(); }
  void SetOutcome(const gbtGameOutcome &c) { return (*self)->SetOutcome(c); }

  //--------------------------------------------------------------------
  //                     Editing the game tree
  //--------------------------------------------------------------------
  gbtGameNode InsertMove(gbtGameInfoset s) { return (*self)->InsertMove(s); }
  void DeleteMove(void) { (*self)->DeleteMove(); }
  void DeleteTree(void) { (*self)->DeleteTree(); }
  gbtGameNode CopyTree(gbtGameNode n) { return (*self)->CopyTree(n); }
  gbtGameNode MoveTree(gbtGameNode n) { return (*self)->MoveTree(n); }
  
  void JoinInfoset(const gbtGameInfoset &s) { (*self)->JoinInfoset(s); }
  gbtGameInfoset LeaveInfoset(void) { return (*self)->LeaveInfoset(); }

  //--------------------------------------------------------------------
  //                   Python-specific operators
  //--------------------------------------------------------------------
  bool __eq__(const gbtGameNode &n) { return *self == n; }
  bool __ne__(const gbtGameNode &n) { return *self != n; }
  int __hash__(void) { return (int) self->Get(); }
};

