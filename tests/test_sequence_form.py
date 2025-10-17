import pygambit as gbt
import numpy as np

def test_sequence_form(game, correspondence, payoffs):
    test_object = TestSequenceForm(game, correspondence, payoffs)
    test_object.check_subtensor(0)

class TestSequenceForm:

    def __init__(self, game, correspondence, payoffs):
        self.correspondence = correspondence
        self.payoffs = payoffs
        self.sequence_form = gbt.GameSequenceForm(game)
        self.players = game.players
        self.seq_dict = {}
        self.seq_idx_list = [0]*len(self.players)

    def test(self):
        self.check_subtensor(0)

    def get_seq_idx(self, player_idx, action):
        for i in range(len(self.correspondence[player_idx])):
            if action.label == self.correspondence[player_idx][i]:
                return i + 1
        return None

    def check_component(self):
        for player_idx in range(len(self.players)):
            player = self.players[player_idx]
            payoff_tensor = self.payoffs[player_idx]
            assert self.sequence_form.get_payoff(self.seq_dict, player) == payoff_tensor[tuple(self.seq_idx_list)]

    def check_subtensor(self, player_idx):
        if player_idx == len(self.players):
            self.check_component()
        else:
            self.check_empty_sequence_subtensor(player_idx)
            self.check_non_empty_sequence_subtensors(player_idx)

    def check_empty_sequence_subtensor(self, player_idx):
        player = self.players[player_idx]
        self.seq_dict[player] = None
        self.seq_idx_list[player_idx] = 0
        self.check_subtensor(player_idx + 1)

    def check_non_empty_sequence_subtensors(self,player_idx):
        player = self.players[player_idx]
        for action in player.actions:
            self.seq_dict[player] = action
            seq_idx = self.get_seq_idx(player_idx, action)
            self.seq_idx_list[player_idx] = seq_idx
            self.check_subtensor(player_idx + 1)



g = gbt.Game.new_tree(
    players=["Buyer", "Seller"],
    title="One-shot trust game, after Kreps (1990)"
)

g.append_move(
    g.root,  # This is the node to append the move to
    player="Buyer",
    actions=["A", "B"]
)

g.set_outcome(
    g.root.children[0],
    outcome=g.add_outcome(
        payoffs=[4, 1],
        label="10"
    )
)

g.set_outcome(
    g.root.children[1],
    outcome=g.add_outcome(
        payoffs=[4, 1],
        label="1"
    )
)

cor = [["A", "B"], []]

buyer_payoffs = np.array([[0], [4], [4]])
seller_payoffs = np.array([[0],[1],[1]])
poffs = [buyer_payoffs, seller_payoffs]

test_sequence_form(g,cor,poffs)
