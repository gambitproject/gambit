%module infoset
%include typemaps.i

%{
#include "../game/game.h"
%}

%nodefault;
class gbtGameAction {
};

%extend gbtGameAction {
  //--------------------------------------------------------------------
  //               General information about the action
  //--------------------------------------------------------------------
  bool IsNull(void) const { return self->IsNull(); }
  int GetId(void) const { return (*self)->GetId(); }
  void SetLabel(const std::string &s) { (*self)->SetLabel(s); }
  std::string GetLabel(void) const  { return (*self)->GetLabel(); }
  
  //--------------------------------------------------------------------
  //                  Information about the game tree
  //--------------------------------------------------------------------
  gbtGameInfoset GetInfoset(void) const  { return (*self)->GetInfoset(); }
  bool Precedes(const gbtGameNode &n) const { return (*self)->Precedes(n); }

  //--------------------------------------------------------------------
  //                    Manipulating chance actions
  //--------------------------------------------------------------------
  void SetChanceProb(const gbtRational &p) { (*self)->SetChanceProb(p); }
  gbtRational GetChanceProb(void) const { return (*self)->GetChanceProb(); }

  //--------------------------------------------------------------------
  //                      Editing the game tree
  //--------------------------------------------------------------------
  void DeleteAction(void) { (*self)->DeleteAction(); }

  //--------------------------------------------------------------------
  //                    Python-specific operators
  //--------------------------------------------------------------------
  bool __eq__(const gbtGameAction &a) { return *self == a; }
  bool __ne__(const gbtGameAction &a) { return *self != a; }
  int __hash__(void) { return (int) self->Get(); }
};

%exception GetMember {
  try {
    $action
  }
  catch (gbtIndexException) {
    PyErr_SetString(PyExc_IndexError, "Member index out of range");
    return NULL;
  }
}

%exception GetAction {
  try {
    $action
  }
  catch (gbtIndexException) {
    PyErr_SetString(PyExc_IndexError, "Action index out of range");
    return NULL;
  }
}

%nodefault;
class gbtGameInfoset {
};

%extend gbtGameInfoset {
  //--------------------------------------------------------------------
  //           General information about the information set
  //--------------------------------------------------------------------
  bool IsNull(void) const { return self->IsNull(); }
  int GetId(void) const { return (*self)->GetId(); }
  void SetLabel(const std::string &s) { (*self)->SetLabel(s); }
  std::string GetLabel(void) const  { return (*self)->GetLabel(); }

  //--------------------------------------------------------------------
  //             Accessing information about the player
  //--------------------------------------------------------------------
  bool IsChanceInfoset(void) const { return (*self)->IsChanceInfoset(); }
  gbtGamePlayer GetPlayer(void) const  { return (*self)->GetPlayer(); }
  void SetPlayer(const gbtGamePlayer &p) { (*self)->SetPlayer(p); }

  //--------------------------------------------------------------------
  //             Accessing information about the actions
  //--------------------------------------------------------------------
  int NumActions(void) const   { return (*self)->NumActions(); }
  gbtGameAction GetAction(int i) const   { return (*self)->GetAction(i); }

  //--------------------------------------------------------------------
  //             Accessing information about the members
  //--------------------------------------------------------------------
  int NumMembers(void) const   { return (*self)->NumMembers(); }
  gbtGameNode GetMember(int i) const     { return (*self)->GetMember(i); }

  //--------------------------------------------------------------------
  //                     Editing the game tree
  //--------------------------------------------------------------------
  bool Precedes(const gbtGameNode &n) const  { return (*self)->Precedes(n); }
  void MergeInfoset(const gbtGameInfoset &s) { (*self)->MergeInfoset(s); }
  gbtGameAction InsertAction(int i) { return (*self)->InsertAction(i); }
  void Reveal(const gbtGamePlayer &p) { (*self)->Reveal(p); }

  //--------------------------------------------------------------------
  //                   Python-specific operators
  //--------------------------------------------------------------------
  bool __eq__(const gbtGameInfoset &s) { return *self == s; }
  bool __ne__(const gbtGameInfoset &s) { return *self != s; }
  int __hash__(void) { return (int) self->Get(); }
};
