%module outcome
%include typemaps.i

%{
#include "../game/game.h"
%}

typedef gbtHandle<gbtGameOutcomeRep> gbtGameOutcome;

%nodefault;
class gbtGameOutcome {
};

%extend gbtGameOutcome {
  //--------------------------------------------------------------------
  //               General information about the outcome
  //--------------------------------------------------------------------
  bool IsNull(void) const  { return self->IsNull(); }
  int GetId(void) const { return (*self)->GetId(); }
  void SetLabel(const std::string &label)  { (*self)->SetLabel(label); }
  std::string GetLabel(void) const { return (*self)->GetLabel(); }

  //--------------------------------------------------------------------
  //           Accessing payoff information about the outcome
  //--------------------------------------------------------------------
  gbtRational GetPayoff(const gbtGamePlayer &player) const
    { return (*self)->GetPayoff(player); }
  void SetPayoff(const gbtGamePlayer &player, const gbtRational &value)
    { (*self)->SetPayoff(player, value); }

  //--------------------------------------------------------------------
  //                 Editing the outcomes in a game
  //--------------------------------------------------------------------
  void DeleteOutcome(void)   { (*self)->DeleteOutcome(); }

  //--------------------------------------------------------------------
  //                   Python-specific operators
  //--------------------------------------------------------------------
  bool __eq__(const gbtGameOutcome &c) { return *self == c; }
  bool __ne__(const gbtGameOutcome &c) { return *self != c; }
  int __hash__(void) { return (int) self->Get(); }
};

