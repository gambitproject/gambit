#
# This is an example of how to build poker-type games in Gambit
#


import gambit
import torr

#
# Functions to categorize public histories.
# These all use torr.GenericHistory, which is basically just a list
#
def AtRoot(history):  return len(history) == 0

def AliceRaise(history):
    return len(history) == 1 and history[0] in [ "Red", "Black" ]

def BobPlay(history):
    return (len(history) == 2 and
            history[0] in [ "Red", "Black" ] and
            history[1] in [ "Raise" ])

def AliceWins(history):
    return history[2] == "Pass"

def BobWins(history):
    return history[1] == "Fold"

def AliceWinsBig(history):
    return history[0] == "Red" and history[2] == "Meet"

def BobWinsBig(history):
    return history[0] == "Black" and history[2] == "Meet"


rules = [ { "condition": AtRoot,
            "action":    torr.ChanceMove([ "Red", "Black" ],
                                         [ gambit.Rational(1,2),
                                           gambit.Rational(1,2) ]) },
          { "condition": AliceRaise,
            "action":    torr.Move(1, [ "Raise", "Fold" ]) },
          { "condition": BobPlay,
            "action":    torr.Move(2, [ "Meet", "Pass" ]) },
          { "condition": BobWins,
            "action":    torr.TerminalPayoff([-1, 1]) },
          { "condition": AliceWins,
            "action":    torr.TerminalPayoff([1, -1]) },
          { "condition": BobWinsBig,
            "action":    torr.TerminalPayoff([-2, 2]) },
          { "condition": AliceWinsBig,
            "action":    torr.TerminalPayoff([2, -2]) } ]
          

tree = torr.BuildTree([ "Alice", "Bob" ], rules, torr.GenericHistory)

print tree.AsEfgFile()
