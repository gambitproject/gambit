%module game
%include typemaps.i
%include exception.i

%exception ReadEfg {
  try {
    $action
  }
  catch (gbtFileNotOpen) {
    SWIG_exception(SWIG_IOError, "Unable to open file");
  }
  catch (gbtFileReadError) {
    SWIG_exception(SWIG_IOError, "Unable to read from file");
  }
  catch (gbtEfgParserException) {
    SWIG_exception(SWIG_IOError, "Not a valid extensive form file");
  }
}

%exception ReadNfg {
  try {
    $action
  }
  catch (gbtFileNotOpen) {
    SWIG_exception(SWIG_IOError, "Unable to open file");
  }
  catch (gbtFileReadError) {
    SWIG_exception(SWIG_IOError, "Unable to read from file");
  }
  catch (gbtNfgParserException) {
    SWIG_exception(SWIG_IOError, "Not a valid normal form file");
  }
}
  

%{
#include "../game/game.h"
#include <fstream>

// Exception thrown when file could not be opened
class gbtFileNotOpen {};

// Exception thrown when write error to file
class gbtFileWriteError {};

// Exception thrown when read error from file
class gbtFileReadError {};

gbtGame ReadEfg(char *filename)
{
  std::ifstream file(filename);
  if (file.is_open()) {
    gbtGame efg = ReadEfg(file);
    if (file.bad()) {
      throw gbtFileReadError();
    }
    else {
      return efg;
    }
  }
  else {
    throw gbtFileNotOpen();
  }
}

gbtGame ReadNfg(char *filename) 
{
  std::ifstream file(filename);
  if (file.is_open()) {
    gbtGame efg = ReadNfg(file);
    if (file.bad()) {
      throw gbtFileReadError();
    }
    else {
      return efg;
    }
  }
  else {
    throw gbtFileNotOpen();
  }
}

typedef gbtBehavProfile<double> gbtBehavProfileDouble;
typedef gbtBehavProfile<gbtRational> gbtBehavProfileRational;

typedef gbtMixedProfile<double> gbtMixedProfileDouble;
typedef gbtMixedProfile<gbtRational> gbtMixedProfileRational;

%}

typedef gbtBehavProfile<double> gbtBehavProfileDouble;
typedef gbtBehavProfile<gbtRational> gbtBehavProfileRational;

typedef gbtMixedProfile<double> gbtMixedProfileDouble;
typedef gbtMixedProfile<gbtRational> gbtMixedProfileRational;



%exception GetPlayer {
  try {
    $action
  }
  catch (gbtIndexException) {
    SWIG_exception(SWIG_IndexError, "Player index out of range");
  }
}

%exception GetOutcome {
  try {
    $action
  }
  catch (gbtIndexException) {
    SWIG_exception(SWIG_IndexError, "Outcome index out of range");
    return NULL;
  }
}


%nodefault;
class gbtGame {
public:
  ~gbtGame();
};

%exception {
  try {
    $action
  }
  catch (gbtGameUndefinedException) {
    SWIG_exception(SWIG_RuntimeError, "Operation not defined");
  }
  catch (gbtGameNullException) {
    SWIG_exception(SWIG_RuntimeError, "Attempting to operate on null object");
  }
  catch (gbtGameDeletedException) {
    SWIG_exception(SWIG_RuntimeError, "Attempting to operate on deleted object");
  }
  catch (gbtException &) {
    SWIG_exception(SWIG_RuntimeError, "An internal error occurred");
  } 	
}

%exception WriteEfg {
  try {
    $action
  }
  catch (gbtFileNotOpen) {
    SWIG_exception(SWIG_IOError, "Unable to open file");
  }
  catch (gbtFileWriteError) {
    SWIG_exception(SWIG_IOError, "Error in writing file");
  }
  catch (gbtGameUndefinedException) {
    SWIG_exception(SWIG_RuntimeError, "Operation not defined")
  }
}

%exception WriteNfg {
  try {
    $action
  }
  catch (gbtFileNotOpen) {
    SWIG_exception(SWIG_IOError, "Unable to open file");
  }
  catch (gbtFileWriteError) {
    SWIG_exception(SWIG_IOError, "Error in writing file");
  }
  catch (gbtGameUndefinedException) {
    SWIG_exception(SWIG_RuntimeError, "Operation not defined")
  }
}

%extend gbtGame {
  //--------------------------------------------------------------------
  //               Manipulation of titles and comments
  //--------------------------------------------------------------------
  void SetLabel(const std::string &s)      { (*self)->SetLabel(s); }
  std::string GetLabel(void) const         { return (*self)->GetLabel(); }
  void SetComment(const std::string &s)    { (*self)->SetComment(s); }
  std::string GetComment(void) const       { return (*self)->GetComment(); }
  
  //--------------------------------------------------------------------
  //               General information about the game
  //--------------------------------------------------------------------
  bool IsConstSum(void) const { return (*self)->IsConstSum(); }
  bool IsPerfectRecall(void) const { return (*self)->IsPerfectRecall(); }
  gbtRational GetMinPayoff(void) const { return (*self)->GetMinPayoff(); }
  gbtRational GetMaxPayoff(void) const { return (*self)->GetMaxPayoff(); }

  //--------------------------------------------------------------------
  //                Information about the game tree
  //--------------------------------------------------------------------
  bool HasTree(void) const { return (*self)->HasTree(); }
  gbtGameNode GetRoot(void) const { return (*self)->GetRoot(); }
  int NumNodes(void) const { return (*self)->NumNodes(); }

  //--------------------------------------------------------------------
  //                Information about the game table
  //--------------------------------------------------------------------
  gbtGameContingency NewContingency(void) const 
    { return (*self)->NewContingency(); }

  //--------------------------------------------------------------------
  //               Manipulation of players in the game
  //--------------------------------------------------------------------
  int NumPlayers(void) const { return (*self)->NumPlayers(); }
  gbtGamePlayer GetChance(void) const { return (*self)->GetChance(); }
  gbtGamePlayer NewPlayer(void) { return (*self)->NewPlayer(); }
  gbtGamePlayer GetPlayer(int i) const { return (*self)->GetPlayer(i); }

  //--------------------------------------------------------------------
  //               Manipulation of outcomes in the game
  //--------------------------------------------------------------------
  int NumOutcomes(void) const { return (*self)->NumOutcomes(); }
  gbtGameOutcome NewOutcome(void) { return (*self)->NewOutcome(); }
  gbtGameOutcome GetOutcome(int i) const { return (*self)->GetOutcome(i); }

  //--------------------------------------------------------------------
  //            Information about the dimensions of the game
  //--------------------------------------------------------------------
  int BehaviorProfileLength(void) const
    { return (*self)->BehaviorProfileLength(); }
  int StrategyProfileLength(void) const 
    { return (*self)->StrategyProfileLength(); }

  //--------------------------------------------------------------------
  //            Creating strategy profiles on the game
  //--------------------------------------------------------------------
  gbtMixedProfileDouble NewMixedProfileDouble(void) const
    { return (*self)->NewMixedProfile(0.0); }
  gbtMixedProfileRational NewMixedProfileRational(void) const
    { return (*self)->NewMixedProfile(gbtRational(0)); }

  gbtBehavProfileDouble NewBehavProfileDouble(void) const
    { return (*self)->NewBehavProfile(0.0); }
  gbtBehavProfileRational NewBehavProfileRational(void) const
    { return (*self)->NewBehavProfile(gbtRational(0)); }

  //--------------------------------------------------------------------
  //                     Writing data files
  //--------------------------------------------------------------------
  void WriteEfg(char *filename)  {
    std::ofstream file(filename);
    if (file.is_open()) {
      (*self)->WriteEfg(file);
      if (file.bad()) {
	throw gbtFileWriteError();
      }
    }
    else {
      throw gbtFileNotOpen();
    }
  }

  void WriteNfg(char *filename)  {
    std::ofstream file(filename);
    if (file.is_open()) {
      (*self)->WriteNfg(file);
      if (file.bad()) {
	throw gbtFileWriteError();
      }
    }
    else {
      throw gbtFileNotOpen();
    }
  }
};

gbtGame NewEfg(void);
gbtGame ReadEfg(char *);

gbtGame NewNfg(const gbtArray<int> &);
gbtGame ReadNfg(char *);
