%module behav
%include typemaps.i

%{
#include "../game/game.h"
%}

%nodefault;
class gbtBehavProfileDouble {
public:
  virtual ~gbtBehavProfileDouble();
};

%extend gbtBehavProfileDouble {
  //--------------------------------------------------------------------
  //             General information about the profile
  //--------------------------------------------------------------------
  gbtGame GetGame(void) const { return (*self)->GetGame(); }
  int BehaviorProfileLength(void) const 
    { return (*self)->BehaviorProfileLength(); }

  //--------------------------------------------------------------------
  //              Accessing the profile probabilities
  //--------------------------------------------------------------------
  double GetActionProb(const gbtGameAction &a) const
    { return (*self)->GetActionProb(a); }
  void SetActionProb(const gbtGameAction &a, const double &prob)
    { (*self)->SetActionProb(a, prob); }
  void SetCentroid(void) { (*self)->SetCentroid(); }

  //--------------------------------------------------------------------
  //                      Payoff information
  //--------------------------------------------------------------------
  double GetPayoff(const gbtGamePlayer &p) const 
    { return (*self)->GetPayoff(p); }
  double GetNodeValue(const gbtGameNode &n, const gbtGamePlayer &p) const
    { return (*self)->GetNodeValue(n, p); }
  double GetInfosetValue(const gbtGameInfoset &s) const
    { return (*self)->GetInfosetValue(s); }
  double GetActionValue(const gbtGameAction &a) const
    { return (*self)->GetActionValue(a); }

  double GetLiapValue(bool penalty) const
    { return (*self)->GetLiapValue(penalty); }

  //--------------------------------------------------------------------
  //                    Probability information  
  //--------------------------------------------------------------------

  double GetRealizProb(const gbtGameNode &n) const
    { return (*self)->GetRealizProb(n); }
  double GetBeliefProb(const gbtGameNode &n) const
    { return (*self)->GetBeliefProb(n); }
  double GetInfosetProb(const gbtGameInfoset &s) const
    { return (*self)->GetInfosetProb(s); }
};

%nodefault;
class gbtBehavProfileRational {
public:
  virtual ~gbtBehavProfileRational();
};

%extend gbtBehavProfileRational {
  //--------------------------------------------------------------------
  //             General information about the profile
  //--------------------------------------------------------------------
  gbtGame GetGame(void) const { return (*self)->GetGame(); }
  int BehaviorProfileLength(void) const 
    { return (*self)->BehaviorProfileLength(); }

  //--------------------------------------------------------------------
  //              Accessing the profile probabilities
  //--------------------------------------------------------------------
  gbtRational GetActionProb(const gbtGameAction &a) const
    { return (*self)->GetActionProb(a); }
  void SetActionProb(const gbtGameAction &a, const gbtRational &prob)
    { (*self)->SetActionProb(a, prob); }
  void SetCentroid(void) { (*self)->SetCentroid(); }

  //--------------------------------------------------------------------
  //                      Payoff information
  //--------------------------------------------------------------------
  gbtRational GetPayoff(const gbtGamePlayer &p) const 
    { return (*self)->GetPayoff(p); }
  gbtRational GetNodeValue(const gbtGameNode &n, const gbtGamePlayer &p) const
    { return (*self)->GetNodeValue(n, p); }
  gbtRational GetInfosetValue(const gbtGameInfoset &s) const
    { return (*self)->GetInfosetValue(s); }
  gbtRational GetActionValue(const gbtGameAction &a) const
    { return (*self)->GetActionValue(a); }

  gbtRational GetLiapValue(bool penalty) const
    { return (*self)->GetLiapValue(penalty); }

  //--------------------------------------------------------------------
  //                    Probability information  
  //--------------------------------------------------------------------

  gbtRational GetRealizProb(const gbtGameNode &n) const
    { return (*self)->GetRealizProb(n); }
  gbtRational GetBeliefProb(const gbtGameNode &n) const
    { return (*self)->GetBeliefProb(n); }
  gbtRational GetInfosetProb(const gbtGameInfoset &s) const
    { return (*self)->GetInfosetProb(s); }
};

