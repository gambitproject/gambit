//
// FILE: player.h -- Declaration of Player data type
//
// $Id$
//

#ifndef PLAYER_H
#define PLAYER_H

#include "gstring.h"
#include "gset.h"
#include "gmapset.h"

#include "infoset.h"
#include "node.h"

class Player   {
  private:
    gString name;
    gMapSet<Infoset *> infosets;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// initialize a player to defaults
    Player(void) : name("UNNAMED")   { }
	// initialize a player, with a specific name
    Player(const gString &s) : name(s)   { }
	// copy constructor
    Player(const Player &);
	// clean up after a player
    ~Player();

	// OPERATOR OVERLOADING
	// assignment operator
    Player &operator=(const Player &);

	// GAME-RELATED OPERATIONS
	// add a group of information sets for a subgame
    int AppendGame(int game)
      { return infosets.CreatePartition(); }
	// add a group of information sets for a subgame game
    int CreateGame(int game)
      { return infosets.CreatePartition(game); }
	// remove a group of information sets for a subgame
    void RemoveGame(int game);
	// returns the number of games
    int NumGames(void) const
      { return infosets.NumPartitions(); }
    int GetNthGameNumber(int n) const
      { return infosets.GetNthPartition(n); }

	// OPERATIONS ON INFOSETS
	// create a new infoset at which the player has the decision
	//  returns the number of the newly-created infoset
    int CreateInfoset(int game, int branches)
      { return infosets.AddElement(new Infoset(branches), game); }

	// remove an infoset
    void RemoveInfoset(int game, int iset)
      { delete infosets.RemoveElement(game, iset); }

	// returns the name of an infoset
    gString GetInfosetName(int game, int iset) const
      { return infosets(game, iset)->GetInfosetName(); }

	// set the name of an infoset
    void SetInfosetName(int game, int iset, const gString &name)
      { infosets(game, iset)->SetInfosetName(name); }

	// return the total number of infosets at which player has the decision
    int NumInfosets(void) const
      { return infosets.NumElements(); }

	// return the number of infosets at which player has the decision
	// in a game
    int NumInfosets(int game) const
      { return infosets.NumElements(game); }

	// OPERATIONS ON BRANCHES
	// returns the number of branches in an infoset
    int NumBranches(int game, int iset) const
      { return infosets(game, iset)->NumActions(); }

	// set the name of a branch in an infoset
    void SetBranchName(int game, int iset, int br, const gString &name)
      { infosets(game, iset)->SetActionName(br, name); }

	// returns the name of a branch in an infoset
    gString GetBranchName(int game, int iset, int br) const
      { return infosets(game, iset)->GetActionName(br); }

	// set the probabilities of branches in an infoset
    void SetBranchProbs(int game, int iset, const gVector<double> &probs)
      { infosets(game, iset)->SetActionProbs(probs); }

	// get the probabilities of branches in an infoset
    gVector<double> GetBranchProbs(int game, int iset) const
      { return infosets(game, iset)->GetActionProbs(); }

	// get the probability associated with one branch of an infoset
    double GetBranchProb(int game, int iset, int br) const
      { return infosets(game, iset)->GetActionProb(br); }

	// remove a branch from an infoset
    void RemoveBranch(int game, int iset, int br)
      { infosets(game, iset)->RemoveAction(br); }

	// insert a branch in an infoset
    void InsertBranch(int game, int iset, int br)
      { infosets(game, iset)->InsertAction(br); }

	// append a branch in an infoset
    void AppendBranch(int game, int iset, const gString &name = "")
      { infosets(game, iset)->AppendAction();
        infosets(game,iset)->SetActionName(infosets(game,iset)->NumActions(),
					   name);
      }

	// NAMING OPERATIONS
	// set the player's name
    void SetPlayerName(const gString &s)   { name = s; }

	// returns the player's name
    gString GetPlayerName(void) const    { return name; }
};


// The PlayerSet is a gSet with the special feature that there are
// by default two players defined:  -1 (dummy) and 0 (chance)
// Players are thus (for external purposes) sequentially numbered from
// -1 through the highest "real" player number.

class PlayerSet   {
  private:
    gSet<Player *> players;

  public:
	// CONSTRUCTORS AND DESTRUCTOR
	// initialize the default PlayerSet
    PlayerSet(int from_file = 0);
	// copy constructor
    PlayerSet(const PlayerSet &);
	// clean up after a PlayerSet
    ~PlayerSet();

	// OPERATOR OVERLOADING
	// assignment operator
    PlayerSet &operator=(const PlayerSet &);

    int yyparse(int game);

	// OPERATIONS ON PLAYERS
	// add player number p to the set
    void AddPlayer(int p);

	// append a player to the player set, with name name
    void AddPlayer(const gString &name); 

	// returns the number of players in the set (not including
	//   dummy or chance)
    int NumPlayers(void) const
      { return (players.Length() - 2); }

	// set the name of a player
    void SetPlayerName(int p, const gString &name)
      { players[p + 2]->SetPlayerName(name); }

	// returns the name of a player
    gString GetPlayerName(int p) const
      { return players[p + 2]->GetPlayerName(); }

	// OPERATIONS ON GAMES
	// create game number game in all players in the PlayerSet
    void CreateGame(int game, int trivial = 1);

	// remove game number game from all players in the PlayerSet
    void RemoveGame(int game);

    	// OPERATIONS ON INFOSETS
	// create a new infoset for player p in game game
	//  returns the number of the new infoset
    int CreateInfoset(int p, int game, int branches) 
      { return players[p + 2]->CreateInfoset(game, branches); }

	// remove an infoset for player p in game game
    void RemoveInfoset(int p, int game, int iset)
      { players[p + 2]->RemoveInfoset(game, iset); }

	// returns the name of an infoset
    gString GetInfosetName(int p, int game, int iset) const
      { return players[p + 2]->GetInfosetName(game, iset); }

	// set the name of an infoset
    void SetInfosetName(int p, int game, int iset, const gString &s)
      { players[p + 2]->SetInfosetName(game, iset, s); }

	// returns the total number of infosets
    int NumInfosets(void) const;

	// returns the total number of infosets in a game
    int NumInfosetsInGame(int game) const;

	// returns the number of infosets for a player
    int NumInfosets(int p) const
      { return players[p + 2]->NumInfosets(); }

	// returns the number of infosets for a player in a game
    int NumInfosets(int p, int game) const
      { return players[p + 2]->NumInfosets(game); }

	// OPERATIONS ON BRANCHES
	// returns the number of branches in an infoset
    int NumBranches(int p, int game, int iset) const
      { return players[p + 2]->NumBranches(game, iset); }

	// set the name of a branch
    void SetBranchName(int p, int game, int iset, int br, const gString &name)
      { players[p + 2]->SetBranchName(game, iset, br, name); }

	// returns the name of a branch
    gString GetBranchName(int p, int game, int iset, int br) const
      { return players[p + 2]->GetBranchName(game, iset, br); }

	// set the probabilities at an infoset
	//  Note: this is only available for the chance player
    void SetBranchProbs(int game, int iset, const gVector<double> &probs)
      { players[2]->SetBranchProbs(game, iset, probs); }

	// get the probabilities at an infoset
	//  Note: this is only available for the chance player
    gVector<double> GetBranchProbs(int game, int iset) const
      { return players[2]->GetBranchProbs(game, iset); }

	// get the probability of a branch in an infoset
	//  Note: this is only available for the chance player
    double GetBranchProb(int game, int iset, int br) const
      { return players[2]->GetBranchProb(game, iset, br); }

	// remove a branch from an infoset
    void RemoveBranch(int p, int game, int iset, int br)
      { players[p + 2]->RemoveBranch(game, iset, br); }

	// insert a branch in an infoset
    void InsertBranch(int p, int game, int iset, int br)
      { players[p + 2]->InsertBranch(game, iset, br); }

	// append a branch in an infoset
    void AppendBranch(int p, int game, int iset, const gString &name = "")
      { players[p + 2]->AppendBranch(game, iset, name); }

};
#endif    // PLAYER_H


