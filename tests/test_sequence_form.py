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
            if float(self.sequence_form.get_payoff(self.seq_dict, player)) != payoff_tensor[tuple(self.seq_idx_list)]:
                print(player_idx)
                print(self.seq_idx_list)
                print(self.sequence_form.get_payoff(self.seq_dict, player))
                print(payoff_tensor[tuple(self.seq_idx_list)])
            assert float(self.sequence_form.get_payoff(self.seq_dict, player)) == payoff_tensor[tuple(self.seq_idx_list)]

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

def test_constraints(game, infosets, actions, constraints):
    test_object = TestConstraints(game, infosets, actions, constraints)
    test_object.test()


class TestConstraints:

    def __init__(self, game, infosets, actions, constraints):
        self.sequence_form = gbt.GameSequenceForm(game)
        self.game = game
        self.infosets = infosets
        self.actions = actions
        self.constraints = constraints

    def get_seq_idx(self, action):
        for i in range(len(self.actions)):
            if action.label == self.actions[i]:
                return i
        return None

    def get_infoset_idx(self, infoset):
        for i in range(len(self.infosets)):
            if infoset.label == self.infosets[i]:
                return i
        return None

    def test(self):
        for infoset_player in self.game.players:
            for infoset in infoset_player.infosets:
                infoset_idx = self.get_infoset_idx(infoset)
                for player in self.game.players:
                    for action in player.actions:
                        action_idx = self.get_seq_idx(action)
                        actual = self.sequence_form.get_constraint_entry(infoset, action)
                        expected = self.constraints[infoset_idx][action_idx]
                        if actual != expected:
                            print(infoset_idx, action_idx)
                            print(actual, expected)
                        assert actual == expected


# Simple constraint test

tree = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Test tree"
)

tree.append_move(
    tree.root,
    player="player1",
    actions=["A", "B"]
)

tree.append_move(
    tree.root.children[0],
    player="player2",
    actions=["a", "b"]
)

tree.append_move(
    tree.root.children[0].children[0],
    player="player1",
    actions=["C", "D"]
)

tree.root.infoset.label = "root"
tree.root.children[0].infoset.label = "A"
tree.root.children[0].children[0].infoset.label = "a"


infosets = ["root", "A", "a"]
actions = ["A", "B", "C", "D", "a", "b"]

constraints = [[-1, -1, 0, 0, 0, 0], [0, 0, 0, 0, -1, -1], [1, 0, -1, -1, 0, 0]]

test_constraints(tree, infosets, actions, constraints)



# Test tree

tree = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Test tree"
)

tree.append_move(
    tree.root,
    player="player1",
    actions=["A", "B", "C", "D"]
)

tree.append_move(
    tree.root.children[0],
    player="player2",
    actions=["a", "b"]
)

tree.append_move(
    tree.root.children[1],
    player="player2",
    actions=["c", "d"]
)

tree.append_move(
    tree.root.children[2],
    player="player2",
    actions=["e", "f"]
)

tree.append_move(
    tree.root.children[0].children[1],
    player="player2",
    actions=["g", "h"]
)

tree.append_move(
    tree.root.children[2].children[0],
    player="player1",
    actions=["E", "F"]
)

tree.append_infoset(tree.root.children[2].children[1],
                    tree.root.children[2].children[0].infoset)

tree.set_outcome(
    tree.root.children[0].children[0],
    outcome=tree.add_outcome(
        payoffs=[2, 8],
        label="Aa"
    )
)

tree.set_outcome(
    tree.root.children[0].children[1].children[0],
    outcome=tree.add_outcome(
        payoffs=[0, 1],
        label="Abg"
    )
)

tree.set_outcome(
    tree.root.children[0].children[1].children[1],
    outcome=tree.add_outcome(
        payoffs=[5, 2],
        label="Abh"
    )
)

tree.set_outcome(
    tree.root.children[1].children[0],
    outcome=tree.add_outcome(
        payoffs=[7, 6],
        label="Bc"
    )
)

tree.set_outcome(
    tree.root.children[1].children[1],
    outcome=tree.add_outcome(
        payoffs=[4, 2],
        label="Bd"
    )
)

tree.set_outcome(
    tree.root.children[2].children[0].children[0],
    outcome=tree.add_outcome(
        payoffs=[3, 7],
        label="CeE"
    )
)

tree.set_outcome(
    tree.root.children[2].children[0].children[1],
    outcome=tree.add_outcome(
        payoffs=[8, 3],
        label="CeF"
    )
)

tree.set_outcome(
    tree.root.children[2].children[1].children[0],
    outcome=tree.add_outcome(
        payoffs=[7, 8],
        label="CfE"
    )
)

tree.set_outcome(
    tree.root.children[2].children[1].children[1],
    outcome=tree.add_outcome(
        payoffs=[2, 2],
        label="CfF"
    )
)

tree.set_outcome(
    tree.root.children[3],
    outcome=tree.add_outcome(
        payoffs=[6, 4],
        label="D"
    )
)

player1_correspondence = ["A", "B", "C", "D", "E", "F"]
player2_correspondence = ["a", "b", "c", "d", "e", "f", "g", "h"]
correspondence = [player1_correspondence, player2_correspondence]

n1 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
n2 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
A1 = [0, 2, 0, 0, 0, 0, 0, 0, 5]
A2 = [0, 8, 0, 0, 0, 0, 0, 1, 2]
B1 = [0, 0, 0, 7, 4, 0, 0, 0, 0]
B2 = [0, 0, 0, 6, 2, 0, 0, 0, 0]
C1 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
C2 = [0, 0, 0, 0, 0, 0, 0, 0, 0]
D1 = [6, 0, 0, 0, 0, 0, 0, 0, 0]
D2 = [4, 0, 0, 0, 0, 0, 0, 0, 0]
E1 = [0, 0, 0, 0, 0, 3, 7, 0, 0]
E2 = [0, 0, 0, 0, 0, 7, 8, 0, 0]
F1 = [0, 0, 0, 0, 0, 8, 2, 0, 0]
F2 = [0, 0, 0, 0, 0, 3, 2, 0, 0]

player1_payoffs = [n1, A1, B1, C1, D1, E1, F1]
player2_payoffs = [n2, A2, B2, C2, D2, E2, F2]
payoffs = np.array([player1_payoffs, player2_payoffs])

test_sequence_form(tree, correspondence, payoffs)


# Simple Three player game

three_player = gbt.Game.new_tree(
    players=["player1", "player2", "player3"],
    title="Simple three player game"
)

three_player.append_move(
    three_player.root,
    player="player1",
    actions=["A", "B"]
)

three_player.append_move(
    three_player.root.children[0],
    player="player2",
    actions=["a", "b"]
)

three_player.append_move(
    three_player.root.children[1],
    player="player3",
    actions=["alpha", "beta"]
)

three_player.append_move(
    three_player.root.children[0].children[0],
    player="player3",
    actions=["gamma", "delta"]
)

three_player.append_infoset(three_player.root.children[0].children[1],
                    three_player.root.children[0].children[0].infoset)

three_player.set_outcome(
    three_player.root.children[0].children[0],
    outcome=three_player.add_outcome(
        payoffs=[1, 2, 3],
        label="Aa"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[1],
    outcome=three_player.add_outcome(
        payoffs=[4, 5, 6],
        label="Ab"
    )
)

three_player.set_outcome(
    three_player.root.children[1].children[0],
    outcome=three_player.add_outcome(
        payoffs=[7, 8, 9],
        label="Balpha"
    )
)

three_player.set_outcome(
    three_player.root.children[1].children[1],
    outcome=three_player.add_outcome(
        payoffs=[10, 11, 12],
        label="Bbeta"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[0].children[0],
    outcome=three_player.add_outcome(
        payoffs=[13, 14, 15],
        label="Aagamma"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[0].children[1],
    outcome=three_player.add_outcome(
        payoffs=[16, 17, 18],
        label="Aadelta"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[1].children[0],
    outcome=three_player.add_outcome(
        payoffs=[19, 20, 21],
        label="Abgamma"
    )
)

three_player.set_outcome(
    three_player.root.children[0].children[1].children[1],
    outcome=three_player.add_outcome(
        payoffs=[22, 23, 24],
        label="Abdelta"
    )
)

correspondence = [["A", "B"], ["a", "b"], ["alpha", "beta", "gamma", "delta"]]

An1 = [0, 0, 0, 0, 0]
An2 = [0, 0, 0, 0, 0]
An3 = [0, 0, 0, 0, 0]
Aa1 = [1, 0, 0, 13, 16]
Aa2 = [2, 0, 0, 14, 17]
Aa3 = [3, 0, 0, 15, 18]
Ab1 = [4, 0, 0, 19, 22]
Ab2 = [5, 0, 0, 20, 23]
Ab3 = [6, 0, 0, 21, 24]
Bn1 = [0, 7, 10, 0, 0]
Bn2 = [0, 8, 11, 0, 0]
Bn3 = [0, 9, 12, 0, 0]
Ba1 = [0, 0, 0, 0, 0]
Ba2 = [0, 0, 0, 0, 0]
Ba3 = [0, 0, 0, 0, 0]
Bb1 = [0, 0, 0, 0, 0]
Bb2 = [0, 0, 0, 0, 0]
Bb3 = [0, 0, 0, 0, 0]

null = [[0, 0, 0, 0, 0], [0, 0, 0, 0, 0], [0, 0, 0, 0, 0]]
A1 = [An1, Aa1, Ab1]
A2 = [An2, Aa2, Ab2]
A3 = [An3, Aa3, Ab3]
B1 = [Bn1, Ba1, Bb1]
B2 = [Bn2, Ba2, Bb2]
B3 = [Bn3, Ba3, Bb3]

player1_payoffs = [null, A1, B1]
player2_payoffs = [null, A2, B2]
player3_payoffs = [null, A3, B3]

payoffs = np.array([player1_payoffs, player2_payoffs, player3_payoffs])

test_sequence_form(three_player, correspondence, payoffs)


# Five player no action game

just_root = gbt.Game.new_tree(
    players=["player1", "player2", "player3", "player4", "player5"],
    title="Just root"
)

just_root.set_outcome(
    just_root.root,
    outcome=just_root.add_outcome(
        payoffs=[1, 2, 3, 4, 5],
        label="root"
    )
)

correspondence = [[],[],[],[],[]]
payoffs1 = [[[[[1]]]]]
payoffs2 = [[[[[2]]]]]
payoffs3 = [[[[[3]]]]]
payoffs4 = [[[[[4]]]]]
payoffs5 = [[[[[5]]]]]
payoffs = np.array([payoffs1, payoffs2, payoffs3, payoffs4, payoffs5])
test_sequence_form(just_root, correspondence, payoffs)


# Player 2 at root

p2_at_root = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Just root"
)

p2_at_root.append_move(
    p2_at_root.root,
    player="player2",
    actions=["a", "b"]
)

p2_at_root.append_move(
    p2_at_root.root.children[0],
    player="player1",
    actions=["A", "B"]
)

p2_at_root.set_outcome(
    p2_at_root.root.children[0].children[0],
    outcome=p2_at_root.add_outcome(
        payoffs=[1, 2],
        label="aA"
    )
)

p2_at_root.set_outcome(
    p2_at_root.root.children[0].children[1],
    outcome=p2_at_root.add_outcome(
        payoffs=[3, 4],
        label="aB"
    )
)

p2_at_root.set_outcome(
    p2_at_root.root.children[1],
    outcome=p2_at_root.add_outcome(
        payoffs=[5, 6],
        label="b"
    )
)

correspondence = [["A","B"],["a","b"]]

n1 = [0, 0, 5]
n2 = [0, 0, 6]
A1 = [0, 1, 0]
A2 = [0, 2, 0]
B1 = [0, 3, 0]
B2 = [0, 4, 0]

payoffs = np.array([[n1, A1, B1], [n2, A2, B2]])
test_sequence_form(p2_at_root, correspondence, payoffs)

# Chance at root

chance_at_root = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Chance at root"
)

chance_at_root.append_move(
    chance_at_root.root,
    player=chance_at_root.players.chance,
    actions=["l", "r"]
)

chance_at_root.set_chance_probs(chance_at_root.root.infoset, [0.2,0.8])

chance_at_root.append_move(
    chance_at_root.root.children[0],
    player="player1",
    actions=["A", "B"]
)

chance_at_root.append_infoset(chance_at_root.root.children[1], chance_at_root.root.children[0].infoset)

chance_at_root.append_move(
    chance_at_root.root.children[0].children[0],
    player="player2",
    actions=["a", "b"]
)

chance_at_root.set_outcome(
    chance_at_root.root.children[0].children[0].children[0],
    outcome=chance_at_root.add_outcome(
        payoffs=[1, 1],
        label="lAa"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[0].children[0].children[1],
    outcome=chance_at_root.add_outcome(
        payoffs=[0, 1],
        label="lAb"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[0].children[1],
    outcome=chance_at_root.add_outcome(
        payoffs=[1, 1],
        label="lB"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[1].children[0],
    outcome=chance_at_root.add_outcome(
        payoffs=[1, 0],
        label="rA"
    )
)

chance_at_root.set_outcome(
    chance_at_root.root.children[1].children[1],
    outcome=chance_at_root.add_outcome(
        payoffs=[0, 1],
        label="rB"
    )
)

correspondence = [["A", "B"], ["a", "b"]]

n1 = [0, 0, 0]
n2 = [0, 0, 0]
A1 = [0.8, 0.2, 0]
A2 = [0, 0.2, 0.2]
B1 = [0.2, 0, 0]
B2 = [1, 0, 0]

payoffs = np.array([[n1, A1, B1], [n2, A2, B2]])
test_sequence_form(chance_at_root, correspondence, payoffs)


# Chance in middle

chance_in_middle = gbt.Game.new_tree(
    players=["player1", "player2"],
    title="Chance in middle"
)

chance_in_middle.append_move(
    chance_in_middle.root,
    player="player1",
    actions=["A", "B"]
)

chance_in_middle.append_move(
    chance_in_middle.root.children[0],
    player=chance_in_middle.players.chance,
    actions=["l", "r"]
)

chance_in_middle.append_move(
    chance_in_middle.root.children[0].children[0],
    player="player2",
    actions=["a", "b"]
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[0].children[0].children[0],
    outcome=chance_in_middle.add_outcome(
        payoffs=[1, 2],
        label="Ala"
    )
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[0].children[0].children[1],
    outcome=chance_in_middle.add_outcome(
        payoffs=[3, 4],
        label="Alb"
    )
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[0].children[1],
    outcome=chance_in_middle.add_outcome(
        payoffs=[5, 6],
        label="Ar"
    )
)

chance_in_middle.set_outcome(
    chance_in_middle.root.children[1],
    outcome=chance_in_middle.add_outcome(
        payoffs=[7, 8],
        label="B"
    )
)

correspondence = [["A", "B"], ["a", "b"]]

n1 = [0, 0, 0]
n2 = [0, 0, 0]
A1 = [2.5, 0.5, 1.5]
A2 = [3, 1, 2]
B1 = [7, 0, 0]
B2 = [8, 0, 0]

payoffs = np.array([[n1, A1, B1], [n2, A2, B2]])

test_sequence_form(chance_in_middle, correspondence, payoffs)

seq_form = gbt.GameSequenceForm(chance_in_middle)
print(seq_form.get_constraint_entry(chance_in_middle.root.infoset, chance_in_middle.players[1].actions[0]))




















