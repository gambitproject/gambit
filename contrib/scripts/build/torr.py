#
# TORR is Tree Output by Recursive Rules
#

import gambit

class ChanceMove:
    def __init__(self, actions, probs):
        self.actions = actions
        self.probs = probs

    def Apply(self, node, history):
        if hasattr(self, "infoset"):
            node.AppendMove(self.infoset)
        else:
            game = node.GetGame()
            self.infoset = node.AppendMove(game.GetChance(),
                                           len(self.actions))
            for (i, (name, prob)) in enumerate(zip(self.actions, self.probs)):
                self.infoset.GetAction(i+1).SetLabel(name)
                self.infoset.SetActionProb(i+1, str(prob))
        return [ node.GetChild(i) for i in xrange(1, len(self.actions)+1) ]

class Move:
    def __init__(self, player, actions):
        self.player = player
        self.actions = actions

    def Apply(self, node, history):
        if hasattr(self, "infoset"):
            node.AppendMove(self.infoset)
        else:
            game = node.GetGame()
            self.infoset = node.AppendMove(game.GetPlayer(self.player),
                                           len(self.actions))
            for (i, name) in enumerate(self.actions):
                self.infoset.GetAction(i+1).SetLabel(name)
        return [ node.GetChild(i) for i in xrange(1, len(self.actions)+1) ]
            
class TerminalPayoff:
    def __init__(self, payoffs):
        self.payoffs = payoffs

    def Apply(self, node, history):
        if not hasattr(self, "outcome"):
            self.outcome = node.GetGame().NewOutcome()
            for (i, payoff) in enumerate(self.payoffs):
                self.outcome.SetPayoff(i+1, str(payoff))
                
        node.SetOutcome(self.outcome)
        return [ ]
        

class GenericHistory:
    def __init__(self, actions = [ ]):
        self.actions = actions[:]

    def WithAction(self, action):
        return GenericHistory(self.actions + [action])

    def __getitem__(self, i):  return self.actions[i]

    def __len__(self): return len(self.actions)
    


def BuildSubtree(rules, node, history):
    for rule in rules:
        if rule["condition"](history):
            children = rule["action"].Apply(node, history)
            for child in children:
                BuildSubtree(rules, child,
                             history.WithAction(child.GetPriorAction().GetLabel()))
            return

def BuildTree(players, rules, starthistory):
    tree = gambit.NewTree()
    for player in players:
        tree.NewPlayer().SetLabel(player)

    BuildSubtree(rules, tree.GetRoot(), starthistory())
    
    return tree
    
