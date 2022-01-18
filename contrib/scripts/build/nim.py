#
# Nim is a game in which players take turns removing pebbles from a pile
# On each turn, a player may remove up to K pebbles
# The player removing the last pebble is the winner
#

# Maximum number of pebbles that can be removed
K = 2

class NimHistory:
    # turn == player in [0,1] who has the turn; pile == number of stones left
    def __init__(self, turn, pile):
        self.turn = turn
        self.pile = pile

    def WithAction(self, action):
        return NimHistory((self.turn + 1) % 2, self.pile - int(action))

    def CurrentMove(self):   return self.turn

    def PebblesLeft(self):   return self.pile

class NimMove:
    def Apply(self, node, history):
        node.SetLabel("%d left" % history.PebblesLeft())
        moves = xrange(1, min(history.PebblesLeft(), K) + 1)

        player = node.GetGame().GetPlayer(history.CurrentMove() + 1)
        infoset = node.AppendMove(player, len(moves))
        for (i, move) in enumerate(moves):
            infoset.GetAction(i+1).SetLabel(str(move))

        return [ node.GetChild(i) for i in xrange(1, len(moves)+1) ]


class NimOutcome:
    def Apply(self, node, history):
        outcome = node.GetGame().NewOutcome()
        for pl in [ 0, 1 ]:
            outcome.SetPayoff(pl+1, str(int(history.CurrentMove() == pl)))

        node.SetOutcome(outcome)
        
        return [ ]
            

import torr

rules = [ { "condition": lambda history: history.PebblesLeft() <= K,
            "action":    NimOutcome() },
          { "condition": lambda history: True,
            "action":    NimMove() } ]

startPebbles = 5

tree = torr.BuildTree([ "One", "Two" ], rules, 
                      lambda: NimHistory(0, startPebbles))
tree.SetTitle("Nim with %d pebbles" % startPebbles)
print tree.AsEfgFile()
