%module player
%include typemaps.i

%{
#include "../game/game.h"
%}

%nodefault;
class gbtGameStrategy {
public:
  ~gbtGameStrategy();
};

%extend gbtGameStrategy {
  //--------------------------------------------------------------------
  //              General information about the strategy
  //--------------------------------------------------------------------
  bool IsNull(void) const { return self->IsNull(); }
  int GetId(void) const { return (*self)->GetId(); }
  void SetLabel(const std::string &s) { (*self)->SetLabel(s); }
  std::string GetLabel(void) const  { return (*self)->GetLabel(); }
  bool IsDeleted(void) const { return (*self)->IsDeleted(); }

  //--------------------------------------------------------------------
  //              Accessing information about the player
  //--------------------------------------------------------------------
  gbtGamePlayer GetPlayer(void) const { return (*self)->GetPlayer(); }
  gbtGameAction GetBehavior(const gbtGameInfoset &s) const
    { return (*self)->GetBehavior(s); }
};

%exception GetInfoset {
  try {
    $action
  }
  catch (gbtIndexException) {
    SWIG_exception(SWIG_IndexError, "Infoset index out of range");
  }
  catch (gbtGameUndefinedException) {
    SWIG_exception(SWIG_RuntimeError, "Operation not defined")
  }
}

%exception NewInfoset {
  try {
    $action
  }
  catch (gbtGameUndefinedException) {
    SWIG_exception(SWIG_RuntimeError, "Operation not defined")
  }
}

%exception GetStrategy {
  try {
    $action
  }
  catch (gbtIndexException) {
    SWIG_exception(SWIG_IndexError, "Strategy index out of range");
  }
}

%nodefault;
class gbtGamePlayer {
public:
  ~gbtGamePlayer();
};

%extend gbtGamePlayer {
  //--------------------------------------------------------------------
  //               General information about the player
  //--------------------------------------------------------------------
  bool IsNull(void) const { return self->IsNull(); }
  int GetId(void) const   { return (*self)->GetId(); }
  bool IsChance(void) const { return (*self)->IsChance(); }
  void SetLabel(const std::string &s) { (*self)->SetLabel(s); }
  std::string GetLabel(void) const   { return (*self)->GetLabel(); }
  bool IsDeleted(void) const { return (*self)->IsDeleted(); }

  //--------------------------------------------------------------------
  //             Accessing information sets of the player
  //--------------------------------------------------------------------
  int NumInfosets(void) const { return (*self)->NumInfosets(); }
  gbtGameInfoset GetInfoset(int i) const  { return (*self)->GetInfoset(i); }
  gbtGameInfoset NewInfoset(int i) { return (*self)->NewInfoset(i); }

  //--------------------------------------------------------------------
  //                Accessing sequences of the player
  //--------------------------------------------------------------------
  int NumSequences(void) const { return (*self)->NumSequences(); }

  //--------------------------------------------------------------------
  //                Accessing strategies of the player
  //--------------------------------------------------------------------
  int NumStrategies(void) const  { return (*self)->NumStrategies(); }
  gbtGameStrategy GetStrategy(int i) const { return (*self)->GetStrategy(i); }

  //--------------------------------------------------------------------
  //                   Python-specific operators
  //--------------------------------------------------------------------
  bool __eq__(const gbtGamePlayer &p) { return *self == p; }
  bool __ne__(const gbtGamePlayer &p) { return *self != p; }
  int __hash__(void) { return (int) self->Get(); }
};
